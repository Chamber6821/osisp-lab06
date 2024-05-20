#pragma once

#include <stdint.h>
#include <time.h>

/**
 * Convert Unix Timestamp to Modified Julian Date by
 * reversed formula on https://en.wikipedia.org/wiki/Julian_day#Variants
 */
static inline double toMJ(time_t timestamp) {
  return timestamp / 86400.0 + 40587;
}

/**
 * Convert Modified Julian Date to Unix Timestamp by
 * formula on https://en.wikipedia.org/wiki/Julian_day#Variants
 */
static inline time_t fromMJ(double jd) { return (jd - 40587) * 86400.0; }

static inline time_t
timestamp(int year, int month, int day, int hour, int minute, int second) {
  struct tm tm = {
      .tm_year = year - 1900,
      .tm_mon = month - 1,
      .tm_mday = day,
      .tm_hour = hour,
      .tm_min = minute,
      .tm_sec = second
  };
  return timegm(&tm);
}
