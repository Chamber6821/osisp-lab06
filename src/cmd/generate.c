#include "julian-time.h"
#include "structs.h"
#include "tools.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Expected single argument: <filename>\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "w");
  struct index index = {.size = 5};
  fwrite(&index, sizeof(index), 1, file);
  double minDate = toMJ(timestamp(1900, 01, 01, 12, 00, 00));
  double maxDate = toMJ(time(NULL)) - 1;
  for (uint64_t i = 0; i < index.size; i++) {
    struct row row = {
        .date = (maxDate - minDate) * randomProportion() + minDate,
        .no = rand()
    };
    fwrite(&row, sizeof(row), 1, file);
  }
  fclose(file);
}
