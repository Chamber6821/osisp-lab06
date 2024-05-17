#pragma once

#include <stdint.h>

struct row {
  double date;
  uint64_t no;
};

struct index {
  uint64_t size;
  struct row rows[];
};
