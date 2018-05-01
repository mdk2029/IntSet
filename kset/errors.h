#pragma once

#include <stdexcept>

#ifndef NDEBUG

#define ASSERT_IMPLIES(X,Y) \
    if(!(X) || (Y)) {       \
    }  else {               \
      throw std::logic_error("assert_implies failed :[ " #X " ] [ " #Y " ]"); \
    }

#define ASSERT(X) \
    if(!(X)) { \
        throw std::logic_error("assert failed: [" #X "]"); \
    }

#else

#define ASSERT_IMPLIES(X,Y)
#define ASSERT(X)

#endif
