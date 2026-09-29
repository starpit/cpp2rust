// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <sys/time.h>
#include <time.h>

typedef struct tm t1;
typedef struct timeval t2;
typedef struct timespec t3;

time_t f1(time_t *t) { return time(t); }

int f2(clockid_t clk_id, struct timespec *tp) {
  return clock_gettime(clk_id, tp);
}

struct tm *f4(const time_t *timer, struct tm *result) {
  return gmtime_r(timer, result);
}

struct tm *f5(const time_t *timer, struct tm *result) {
  return localtime_r(timer, result);
}

size_t f6(char *s, size_t maxsize, const char *format, const struct tm *tp) {
  return strftime(s, maxsize, format, tp);
}

int f7(const char *file, const struct timeval tvp[2]) {
  return utimes(file, tvp);
}

#if defined(__linux__)
int f8(struct timeval *tv, struct timezone *tz) {
  return gettimeofday(tv, tz);
}
#elif defined(__APPLE__)
int f8(struct timeval *tv, void *tz) {
  return gettimeofday(tv, tz);
}
#else
#error "Unsupported platform for gettimeofday"
#endif

clockid_t f9() {
  return CLOCK_REALTIME;
}

clockid_t f10() { return CLOCK_MONOTONIC; }

#ifdef __linux__
clockid_t f11() { return CLOCK_MONOTONIC_RAW; }
#endif

// f12 -- `difftime(t1, t0)`.  PURE ARITHMETIC: the standard defines it as the
// difference t1 - t0 expressed in seconds as a double, and on glibc `time_t` is a
// signed integer count of seconds, so the subtraction IS the answer -- no model,
// no approximation, nothing to fabricate.  1 of dxp_standalone.cpp's placeholders
// (dxp_standalone.cpp:174).  Both models get the same body: the parameters are
// scalars, so the refcount lowering is identical.
double f12(time_t a0, time_t a1) { return difftime(a0, a1); }
