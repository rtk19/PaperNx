#pragma once
#include <stdint.h>
#include <time.h>

/* Newlib used by devkitA64 has no timegm. Compute UTC without changing TZ,
 * which would race the document and thumbnail contexts. */
static inline time_t papernx_timegm(struct tm *value) {
    int64_t year = (int64_t)value->tm_year + 1900;
    int month = value->tm_mon + 1;
    while (month <= 0) { month += 12; --year; }
    while (month > 12) { month -= 12; ++year; }
    int64_t days = 0;
    if (year >= 1970) {
        for (int64_t y = 1970; y < year; ++y)
            days += 365 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
    } else {
        for (int64_t y = year; y < 1970; ++y)
            days -= 365 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
    }
    static const int lengths[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    for (int m = 1; m < month; ++m) {
        days += lengths[m - 1];
        if (m == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ++days;
    }
    int64_t seconds = ((days + value->tm_mday - 1) * 24 + value->tm_hour) * 3600
        + value->tm_min * 60 + value->tm_sec;
    time_t result = (time_t)seconds;
    return (int64_t)result == seconds ? result : (time_t)-1;
}
