#include "julian-time.h"
#include <stdio.h>

#include <time.h>

int main() {
  time_t now = time(NULL);
  // now = timestamp(1900, 01, 01, 12, 00, 00);
  printf("Hum:  %s", asctime(gmtime(&now)));
  printf("Unix: %ld\n", now);
  printf("MJ:   %lf\n", toMJ(now));
  printf("Conv: %ld\n", fromMJ(toMJ(now)));
}
