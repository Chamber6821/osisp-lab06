#include "structs.h"
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <memory.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

struct range {
  int accepted;
  struct row *begin;
  struct row *end;
};

struct {
  struct range *begin;
  struct range *end;
} ranges;

pthread_mutex_t mutex;
pthread_barrier_t barrier;

int compareRows(const struct row *a, const struct row *b) {
  return a->date - b->date;
}

int rowsComparator(const void *a, const void *b) { return compareRows(a, b); }

void sorting() {
  printf("[%7d]: Sorting\n", gettid());
  struct range *target = ranges.begin;
  while (1) {
    pthread_mutex_lock(&mutex);
    for (; target != ranges.end; ++target)
      if (!target->accepted) {
        target->accepted = 1;
        break;
      }
    pthread_mutex_unlock(&mutex);
    if (target == ranges.end) break;
    qsort(
        target->begin,
        (target->end - target->begin),
        sizeof(*target->begin),
        rowsComparator
    );
    printf("[%7d]: Sorted %p\n", gettid(), (void *)target);
  }
}

struct row *insert(struct row *begin, struct row *end, struct row *what) {
  struct row *it = begin;
  for (; it != end && compareRows(it, what) < 0; ++it)
    ;
  // printf("BEFORE MOVE\n");
  // for (int i = 0; i < (end - begin); i++) {
  //   printf("#%2d %s\n", i, row2str(&begin[i]));
  // }
  // printf("IT:   %s\n", row2str(it));
  // printf("WHAT: %s\n", row2str(what));
  memmove(it + 1, it, (end - it) * sizeof(*it));
  // printf("AFTER MOVE\n");
  // for (int i = 0; i < (end - begin) + 1; i++) {
  //   printf("#%2d %s\n", i, row2str(&begin[i]));
  // }
  *it = *what;
  return it;
}

void mergeRanges(struct range *to, struct range *from) {
  // printf("TO\n");
  // for (int i = 0; i < (to->end - to->begin); i++) {
  //   printf("#%2d %s\n", i, row2str(&to->begin[i]));
  // }
  // printf("FROM\n");
  // for (int i = 0; i < (from->end - from->begin); i++) {
  //   printf("#%2d %s\n", i, row2str(&from->begin[i]));
  // }
  struct row *nextPlace = to->begin;
  for (; from->begin != from->end; ++to->end, ++from->begin) {
    struct row copy = *from->begin;
    nextPlace = insert(nextPlace, to->end, &copy);
    // printf("AFTER INSERT\n");
    // for (int i = 0; i < (to->end - to->begin) + 1; i++) {
    //   printf("#%2d %s\n", i, row2str(&to->begin[i]));
    // }
  }
}

void merging() {
  printf("[%7d]: Merging\n", gettid());
  struct range *target = ranges.begin;
  while (1) {
    pthread_mutex_lock(&mutex);
    for (; target + 1 < ranges.end; target += 2)
      if (!target[0].accepted && !target[1].accepted) {
        target[0].accepted = 1;
        target[1].accepted = 1;
        break;
      }
    pthread_mutex_unlock(&mutex);
    if (target + 1 >= ranges.end) break;
    mergeRanges(&target[0], &target[1]);
    printf(
        "[%7d]: Merged %p and %p\n",
        gettid(),
        (void *)&target[0],
        (void *)&target[1]
    );
  }
}

void *worker(void *arg) {
  (void)arg;
  sorting();
  printf("[%7d]: All sorted\n", gettid());
  pthread_barrier_wait(&barrier);
  pthread_barrier_wait(&barrier);
  while (ranges.begin != ranges.end) {
    merging();
    pthread_barrier_wait(&barrier);
    pthread_barrier_wait(&barrier);
  }
  printf("[%7d]: All merged\n", gettid());
  return NULL;
}

int main(int argc, char **argv) {
  if (argc < 4)
    perror("Expected 3 arguments: <filename> <block count> <thread count>\n");

  const char *filename = argv[1];
  int blockCount = 0;
  int threadCount = 0;
  if (sscanf(argv[2], "%d", &blockCount) != 1)
    perror("Could not parse block count (2nd arg)\n");
  if (sscanf(argv[3], "%d", &threadCount) != 1)
    perror("Could not parse thread count (3nd arg)\n");

  int fd = open(filename, O_RDWR, S_IRUSR | S_IWUSR);
  struct stat stat;
  if (fstat(fd, &stat) == -1) perror("Could not get stat of file\n");
  struct index *index =
      mmap(NULL, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  blockCount =
      (uint64_t)blockCount > index->size ? (int)index->size : blockCount;
  int blockSize = index->size / blockCount;
  struct range rangesBuffer[blockCount];
  for (int i = 0; i < blockCount; i++) {
    rangesBuffer[i] = (struct range
    ){.accepted = 0,
      .begin = &index->rows[i * blockSize],
      .end = &index->rows[(i + 1) * blockSize]};
  }
  rangesBuffer[blockCount - 1].end = &index->rows[index->size];

  ranges.begin = rangesBuffer;
  ranges.end = rangesBuffer + blockCount;

  pthread_mutex_init(&mutex, NULL);
  pthread_barrier_init(&barrier, NULL, threadCount + 1);
  pthread_t threads[threadCount];
  for (int i = 0; i < threadCount; i++) {
    pthread_create(&threads[i], NULL, worker, NULL);
  }

  pthread_barrier_wait(&barrier);
  for (struct range *it = ranges.begin; it != ranges.end; ++it)
    it->accepted = 0;
  printf("[%7d]: Ranges updated\n", gettid());
  pthread_barrier_wait(&barrier);

  while (1) {
    pthread_barrier_wait(&barrier);
    if (ranges.end - ranges.begin <= 1) {
      ranges.begin = ranges.end;
      printf("[%7d]: Ranges updated\n", gettid());
      pthread_barrier_wait(&barrier);
      break;
    }
    for (struct range *to = ranges.begin, *from = ranges.begin;
         from < ranges.end;
         to += 1, from += 2)
      *to = *from;
    ranges.end -= (ranges.end - ranges.begin) / 2;
    for (struct range *it = ranges.begin; it != ranges.end; ++it)
      it->accepted = 0;
    printf("[%7d]: Ranges updated\n", gettid());
    pthread_barrier_wait(&barrier);
  }

  for (int i = 0; i < threadCount; i++)
    pthread_join(threads[i], NULL);
  pthread_barrier_destroy(&barrier);
  pthread_mutex_destroy(&mutex);

  munmap(index, stat.st_size);
  close(fd);
}
