#include "julian-time.h"
#include "structs.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    time_t date = fromMJ(rows[i].date);
    char buffer[] = "1900-01-01 12:00:00";
    strftime(buffer, sizeof(buffer), "%F %T", gmtime(&date));
    printf("%*lu: %s #%lu\n", rowNumberColumns, i, buffer, rows[i].no);
  }
  free(rows);
}
