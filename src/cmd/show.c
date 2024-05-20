#include "structs.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Expected single argument: <filename>\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  struct index index = {0};
  fread(&index, sizeof(index), 1, file);
  struct row *rows = calloc(index.size, sizeof(*rows));
  fread(rows, sizeof(*rows), index.size, file);
  fclose(file);
  int rowNumberColumns = (int)ceil(log10(index.size));
  for (uint64_t i = 0; i < index.size; i++) {
    printf("%*lu: %s\n", rowNumberColumns, i, row2str(&rows[i]));
  }
  free(rows);
}
