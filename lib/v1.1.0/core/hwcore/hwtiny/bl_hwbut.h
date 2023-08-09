//==============================================================================
// bl_hwbut.h
// Bluccino button driver supporting button basic functions
//
// Created by Hugo Pristauz on 2022-Feb-18
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================
// BUTTON interface
// - button presses notify with [BUTTON:PRESS @ix 1] with @ix = 1..4
// - button releases notify with [BUTTON:RELEASE @ix 0] with @ix = 1..4
//
// SWITCH interface
// - each button (1..4) is assigned with a logical switch which is toggled
//   on [BUTTON:PRESS @ix,sts] events
// - each change of the logical switch state is notified by a
//   [SWITCH:SET @ix,onoff] event message
//==============================================================================

#ifndef __BL_HWBUT_H__
#define __BL_HWBUT_H__

//==============================================================================
// BUTTON Logging
//==============================================================================

#ifndef CFG_LOG_BUTTON
    #define CFG_LOG_BUTTON    1           // BUTTON logging is by default on
#endif

#if (CFG_LOG_BUTTON)
    #define LOG_BUTTON(l,f,...)    BL_LOG(CFG_LOG_BUTTON-1+l,f,##__VA_ARGS__)
    #define LOGO_BUTTON(l,f,o,v)   bl_logo(CFG_LOG_BUTTON-1+l,f,o,v)
#else
    #define LOG_BUTTON(l,f,...)    {}     // empty
    #define LOGO_BUTTON(l,f,o,v)   {}     // empty
#endif

//==============================================================================
// config defaults
//==============================================================================

#ifndef CFG_COOLDOWN_MS
  #define CFG_DEBOUNCE_TIME    50      // 50 ms debounce time
#endif

//==============================================================================
// public module interface
//==============================================================================

  int bl_hwbut(BL_ob *o, int val);    // button module interface

#endif // __BL_HWBUT_H__
