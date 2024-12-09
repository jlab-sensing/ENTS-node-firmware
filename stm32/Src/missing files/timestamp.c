#include "timestamp.h"

time_t TimestampNow(void) { 
  // temporary static timestamp
  struct tm time_info = {
      .tm_year = 124,  // 2021 (since 1900)
      .tm_mon = 1,    // January (0-based)
      .tm_mday = 22,
      .tm_hour = 0,
      .tm_min = 0,
      .tm_sec = 0,
  };

  // format into unix epochs
  time_t unix_epoch = mktime(&time_info);

  return unix_epoch;
}

time_t TimestampUpdate(void) {
  return 0;
}