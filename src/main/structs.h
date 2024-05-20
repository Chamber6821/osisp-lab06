#pragma once

#include "julian-time.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

struct row {
  double date;
  uint64_t no;
};

struct index {
  uint64_t size;
  struct row rows[];
};

static const char *row2str(struct row *row) {
  static char buffer[] = "1900-01-01 12:00:00 #18446744073709551615";
  time_t time = fromMJ(row->date);
  char date[] = "1900-01-01 12:00:00";
  strftime(date, sizeof(date), "%F %T", gmtime(&time));
  sprintf(buffer, "%s #%lu", date, row->no);
  return buffer;
}
