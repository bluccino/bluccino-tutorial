//==============================================================================
// bl_timer.h
// Bluccino timer support
//
// Created by Hugo Pristauz on 2022-Aug-27
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

#ifndef __BL_TIMER_H__
#define __BL_TIMER_H__

//==============================================================================
// BL_timer data structure and related BL_TIMER() initializer
// - usage: static BL_timer timer = BL_TIMER(cb); // static storage class !!!
//==============================================================================

  typedef struct BL_timer
          {
            struct k_timer timer;      // Zephyr timer structure
            BL_oval oval;              // OVAL timer callback
            BL_ob oo;                  // Bluccino message object
            int val;                   // transmitted value to OVAL callback
          } BL_timer;

    // macro to define init agregate for BL_timer (marks timer as uninitialized)

  #define BL_TIMER(cb) {oval:cb, oo:{_VOID,VOID_,0,NULL}, val:0}

//==============================================================================
// timer start/stop function calls
// - usage: static BL_timer timer = BL_timer(module);
//          bl_timer(&timer,o,val,50); // start repeat timer @ 50 ms period
//          bl_timer(&timer,o,val,-8); // start single shot timer @ 8 ms later
//          bl_timer(&timer,o,val,0);  // stop timer and post stop event message
//          bl_timer(&timer,NULL,0,0); // stop timer without stop event message
//==============================================================================

  int bl_timer(BL_timer *p, BL_ob *o, int val, int ms);

#endif // __BL_TIMER_H__
