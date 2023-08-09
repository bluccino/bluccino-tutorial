//==============================================================================
// bl_vl6180x.h
// driver for (STM) VL6180X Time-of-Flight sensor
//
// Created by Hugo Pristauz on 2022-Sep-29
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================
// note: this driver needs the following configuration lines in prj.conf:
//       CONFIG_I2C_NRFX=y
//       CONFIG_I2C_0=y
//       CONFIG_I2C=y
//==============================================================================

#ifndef __BL_VL6180X_H__
#define __BL_VL6180X_H__

	#include <zephyr/drivers/i2c.h>

//==============================================================================
// VL6180X Logging
//==============================================================================

  #ifndef CFG_LOG_VL6180X
    #define CFG_LOG_VL6180X    1           // VL6180X logging is by default on
  #endif

  #if (CFG_LOG_VL6180X)
    #define LOG_VL6180X(l,f,...)    BL_LOG(CFG_LOG_VL6180X-1+l,f,##__VA_ARGS__)
    #define LOGO_VL6180X(l,f,o,v)   bl_logo(CFG_LOG_VL6180X-1+l,f,o,v)
  #else
    #define LOG_VL6180X(l,f,...)    {}     // empty
    #define LOGO_VL6180X(l,f,o,v)   {}     // empty
  #endif

//==============================================================================
// PMI
//==============================================================================

  int bl_vl6180x(BL_ob *o, int val);

#endif // __BL_VL6180X_H__
