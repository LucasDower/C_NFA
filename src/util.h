#ifndef UTIL_H
#define UTIL_H

#ifdef C_NFA_ASSERT_ENABLED
#define C_NFA_ASSERT(condition) assert(condition)
#else
#define C_NFA_ASSERT(condition)
#endif

#define C_NFA_MAX(a, b) ((a) > (b) ? (a) : (b))

#endif