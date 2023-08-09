//==============================================================================
//  bl_util.h
//  utilities
//
//  Created by Hugo Pristauz on 2022-06-18
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_UTIL_H__
#define __BL_UTIL_H__

  #include <zephyr/random/rand32.h>           // random generator

//==============================================================================
// get a random int value (passes general randomness tests)
// - usage: val = bl_rand(modulus) // get random number (not cryptocally secure)
//          val = bl_rand(0)       // get rand integer number (+/-)
//==============================================================================

  static inline int bl_rand(int modulus)
  {
    int n = sys_rand32_get();
    return modulus ? (int)((uint32_t)n % (uint32_t)BL_ABS(modulus)) : n;
  }

#endif // __BL_UTIL_H__
