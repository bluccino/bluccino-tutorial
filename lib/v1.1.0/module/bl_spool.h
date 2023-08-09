//==============================================================================
// bl_spool.h
// mesh publisher (repeated sending of scheduled mesh messages)
//
// Created by Hugo Pristauz on 2022-JUN-11
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

#ifndef __BL_SPOOL_H__
#define __BL_SPOOL_H__

//==============================================================================
// MPUB Logging
//==============================================================================

  #ifndef CFG_LOG_MPUB
    #define CFG_LOG_MPUB    1           // MPUB logging is by default on
  #endif

  #if (CFG_LOG_MPUB)
    #define LOG_MPUB(l,f,...)    BL_LOG(CFG_LOG_MPUB-1+l,f,##__VA_ARGS__)
    #define LOGO_MPUB(l,f,o,v)   bl_logo(CFG_LOG_MPUB-1+l,f,o,v)
  #else
    #define LOG_MPUB(l,f,...)    {}     // empty
    #define LOGO_MPUB(l,f,o,v)   {}     // empty
  #endif

//==============================================================================
// - [SET:REPEAT cnt] set number of message repeats
// - [SET:INTERVAL ms] set repeat interval (ms) for message repeats
//==============================================================================

  #define SET_REPEAT_0_0_cnt      BL_ID(_SET,REPEAT_)
  #define SET_INTERVAL_0_0_ms     BL_ID(_SET,INTERVAL_)

    // augmented messages

  #define _SET_REPEAT_0_0_cnt     _BL_ID(_SET,REPEAT_)
  #define _SET_INTERVAL_0_0_ms    _BL_ID(_SET,INTERVAL_)

//==============================================================================
// public module interface
//==============================================================================

  int bl_spool(BL_ob *o, int val);

#endif // __BL_SPOOL_H__
