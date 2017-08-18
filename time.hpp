#ifndef _TIME_HPP_
#define _TIME_HPP_

#ifdef __LINUX__
#include <time.h>
#endif

namespace Time {
  /** Returns current nanosecond timestamp */
  uint64_t get_curr_nanosec() {
    #ifdef __LINUX__
      struct timespec time_spec;
      clock_gettime(CLOCK_MONOTONIC, &time_spec);
      return time_spec.tv_nsec;
    #elif
      #error Windows and other platforms are unsupported
    #endif
  }
}

#endif // _TIME_HPP_
