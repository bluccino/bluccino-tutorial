//==============================================================================
// bl_time.c
// Time related Bluccino stuff
//
// Created by Hugo Pristauz on 2022-Apr-03
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

  #include "bluccino.h"

//==============================================================================
// TIME level logging shorthands
//==============================================================================

  #define LOG                     LOG_TIME
  #define LOGO(lvl,col,o,val)     LOGO_TIME(lvl,col"api:",o,val)
  #define LOG0(lvl,col,o,val)     LOGO_TIME(lvl,col,o,val)

//==============================================================================
// us/ms clock
//==============================================================================

  static BL_us offset = 0;             // offset for us clock

  static BL_us now_us()                // system clock in us
  {
    #ifdef __ZEPHYR__
      uint64_t cyc = k_uptime_ticks();
      uint64_t f = sys_clock_hw_cycles_per_sec();
      return (BL_us)((1000000*cyc)/f);
    #else
      return (BL_us)timer_now();
    #endif
  }

//==============================================================================
// reset us clock
//==============================================================================

  BL_us bl_zero(void)                  // reset us clock
  {
    return offset = now_us();
  }

//==============================================================================
// get us clock time
//==============================================================================

  BL_us bl_us(void)                    // get current clock time in us
  {
    BL_us us = now_us();

    if (offset == 0)                   // initially always: offset == 0
      us = bl_zero();                  // in this case reset us clock

    return us  - offset;               // return us clock time since last reset
  }

//==============================================================================
// get ms clock time
//==============================================================================

  BL_ms bl_ms(void)                    // get current clock time in ms
  {
    BL_us us = bl_us();
    return us/1000;
  }

//==============================================================================
// timing & sleep
//==============================================================================

  bool bl_due(BL_ms *pdue, BL_ms ms)   // check if time if due & update
  {
    BL_ms now = bl_ms();

    if (now < *pdue)                   // time for tick action is due?
      return false;                    // no, not yet due!

    *pdue = now + ms;                  // catch-up due time
    return true;                       // yes, tick time due!
  }

//void bl_sleep(int ms)                // deep sleep for given milliseconds
//{
//  nrf_delay_ms(ms);
//}

  void bl_sleep(BL_ms ms)              // deep sleep for given milliseconds
  {
    if (ms > 0)
      BL_SLEEP((int)ms);
  }

  void bl_halt(BL_txt msg, BL_ms ms)   // halt system
  {
    LOG(0,BL_R"%s: system halted", msg);
    for (;;)
      bl_sleep(ms);
  }
