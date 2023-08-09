//==============================================================================
// bl_hwled.h
// basic LED functions
//
// Created by Hugo Pristauz on 2022-Feb-18
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================
// LED interface:
// - LED messages [LED:SET @ix onoff] control the onoff state of one of the four
// - LEDs @1..@4. LED @0 is the status LED which will be remapped to LED @1
//==============================================================================

#ifndef __BL_HWLED_H__
#define __BL_HWLED_H__

//==============================================================================
// public module interface
//==============================================================================
//
// (!) := (<parent>);
//                  +--------------------+
//                  |        LED         |
//                  +--------------------+
//                  |        SYS:        | SYS interface
// (!)->     INIT ->|       <out>        | init module, ignore <out> callback
//                  +--------------------+
//                  |        LED:        | LED interface
// (!)->      SET ->|      @ix,onoff     | set LED's onoff state
// (!)->   TOGGLE ->|        @ix         | toggle LED's onoff state
//                  +--------------------+
//
//==============================================================================

  int bl_hwled(BL_ob *o, int val);       // HW core module interface

//==============================================================================
// syntactic sugar: LED moduler init
// - usage: bl_hwled_init(cb)
//==============================================================================

  static inline int bl_hwled_init(BL_oval cb)
  {
    return bl_init(bl_hwled,cb);
  }

#endif // __BL_HWLED_H__
