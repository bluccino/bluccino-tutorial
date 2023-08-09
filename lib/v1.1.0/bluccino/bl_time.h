//==============================================================================
// bl_time.h
// Time related Bluccino stuff
//
// Created by Hugo Pristauz on 2022-Apr-03
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

#ifndef __BL_TIME_H__
#define __BL_TIME_H__

#include "bl_type.h"
#include "bl_log.h"

//==============================================================================
// us/ms clock
//==============================================================================

  BL_us bl_zero(void);                 // reset clock
  BL_us bl_us(void);                   // get current clock time in us
  BL_ms bl_ms(void);                   // get current clock time in ms

//==============================================================================
// periode detection
// - usage: ok = bl_period(o,ms)        // is tick/tock time meeting a period?
// - note: works only if ms is a multiple of tick/tock period
//==============================================================================

  static inline bool bl_period(BL_ob *o, BL_ms ms)
  {
    BL_pace *p = (BL_pace*)bl_data(o);
    return p ? ((p->time != 0) && ((p->time % ms) == 0)) : 0;
  }

//==============================================================================
// duty edge detection
// - usage: ok = bl_duty(o,duty,period)  // is tick/tock time meeting duty edge?
// - note: works only if duty and period are a multiple of tick/tock period
//==============================================================================

  static inline bool bl_duty(BL_ob *o, int duty, BL_ms period)
  {
    BL_pace *p = (BL_pace*)bl_data(o);
    return p ? (((p->time - duty)% period) == 0) : 0;
  }

//==============================================================================
// after system start (one time TRUE condition)
// - usage: ok = bl_after(o,ms)        // tick/tock time after system startd?
// - note: works only if ms is a multiple of tick/tock period
//==============================================================================

  static inline bool bl_after(BL_ob *o, BL_ms ms)
  {
    BL_pace *p = (BL_pace*)bl_data(o);
    return p ? (p->time == ms) : 0;
  }

//==============================================================================
// timing & sleep, system halt
//==============================================================================

  bool bl_due(BL_ms *pdue, BL_ms ms);  // check if time if due & update
  void bl_sleep(BL_ms ms);             // deep sleep for given milliseconds
  void bl_halt(BL_txt msg, BL_ms ms);  // halt system

//==============================================================================
// tic/toc functions (elapsed time benchmarking)
// - usage: bl_tic(o,10000);           // run 10000 loops
// -        for (int i=0; i < bl_ix(o); i++)
// -          { ... }  // do some work (e.g. call function with OVAL interface)
// -        bl_toc(o,"OVAL call");
//==============================================================================

  static inline int bl_tic(BL_ob *o, int n)
  {
    BL_us now = bl_us();
//bl_prt("now: %d us\n",(int)now);
    o->data = (const void*)(int)now;
    return (o->ix = n);                // save number of loops in object's @ix
  }

  static inline void bl_toc(BL_ob *o, BL_txt msg)
  {
    int now = (int)bl_us();
    int begin = (int)o->data;
    int elapsed = (100*(now - begin)) / bl_ix(o);    // devide by number of runs
//bl_prt("begin: %d us, now: %d us\n",begin,now);
    if (bl_now(1))
      bl_prt("%s: %d.%02d us\n", msg, elapsed/100, elapsed%100);
  }

//==============================================================================
// elapsed time measurement
// - usage: BL_us tic = bl_us();
//          do_something();
//          BL_us toc = bl_us();
//          bl_log(1,"elapsed time: %d us", bl_elapse(tic,toc))
//==============================================================================

  static int inline bl_elapse(BL_us tic, BL_us toc)
  {
    return (int)(toc - tic);
  }

//==============================================================================
// run an idle loop
// - usage: bl_idle(10);     // loop forever with sleeping 10ms
//==============================================================================

  static inline void bl_idle(BL_ms period)
  {
    for (;;) bl_sleep(period);
  }

#endif // __BL_TIME_H__
