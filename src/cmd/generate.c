#include "structs.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Expected single argument: <filename>\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "w");
  uint64_t size = 256;
  fwrite(&size, sizeof(size), 1, file);
  while (size--) {
    struct row row = {.date = rand(), .no = rand()};
    fwrite(&row, sizeof(row), 1, file);
  }
  fclose(file);
}
