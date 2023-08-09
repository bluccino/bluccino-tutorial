//==============================================================================
// bl_timer.c
// Bluccino (swiss knife) timer support
//
// Created by Hugo Pristauz on 2022-Aug-27
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================
// - 'swiss knife' timer functionality for majority usage
// - posting a Bluccino message to a module on periodic or single post basis
//   to (an OVAL interface based) receiver module
// - auto initializing Zephyr's strukt k_timer structure
// - initializing macro BL_TIMER() to setup receiver module of timed message
// - one single function to control all timer activities (single start, repeat
//   start, stop with and without stop message emission)
//
//==============================================================================

  #include "bluccino.h"

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO                     "bl_timer:"

  #define LOG                     LOG_TIMER
  #define LOGO(lvl,col,o,val)     LOGO_TIMER(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_TIMER(lvl,col,o,val)

//==============================================================================
// callback: handle zephyr timer firing callback
//==============================================================================

  static void fire(struct k_timer *t)
  {
    BL_timer *p = (BL_timer*)k_timer_user_data_get(t);

    if (!p)
    {
      bl_err(-1,"timer fires, but empty user data");
      return;
    }

    if ( !bl_is(&p->oo,_VOID,VOID_) )
    {
      LOGO(5,"timer firing:",&p->oo,p->val);
      p->oval(&p->oo,p->val);          // post message to receiver module
    }
    else
      LOGO(5,"ignore timer firing:",&p->oo,p->val);
  }

//==============================================================================
// callback: handle zephyr timer stop callback
//==============================================================================

  static void stop(struct k_timer *t)
  {
    BL_timer *p = (BL_timer*)k_timer_user_data_get(t);

    if (!p)
    {
      bl_err(-1,"stop timer: empty user data");
      return;
    }

    LOGO(5,"stop timer:",&p->oo,p->val);

    if (p && !bl_is(&p->oo,_VOID,VOID_))
      p->oval(&p->oo,p->val);          // run oval callback
  }

//==============================================================================
// timer start/stop function calls
// - usage: static BL_timer timer = BL_timer(module);
//          bl_timer(&timer,o,val,50); // start repeat timer @ 50 ms period
//          bl_timer(&timer,o,val,-8); // start single shot timer @ 8 ms later
//          bl_timer(&timer,o,val,0);  // stop timer and post stop event message
//          bl_timer(&timer,NULL,0,0); // stop timer without stop event message
//==============================================================================

  int bl_timer(BL_timer *p, BL_ob *o, int val, int ms)
  {
    struct k_timer *t = &p->timer;

    if (bl_is(&p->oo,_VOID,VOID_))     // is p->work not yet initialized?
    {
      LOG(5,"init timer ...");
      k_timer_init(t, fire, stop);
      k_timer_user_data_set(t,p);
    }

      // a potential [VOID:VOID] value in p->oo will now be overwritten

    if (ms != 0 && o == NULL)          // stop timer without stop event message
    {
      BL_ob oo = {_VOID,VOID_,0,NULL};
      p->oo = oo;  p->val = val;
    }
    else
    {
      p->oo = *o;  p->val = val;       // copy message ID and message args
    }

      // start/stop timer

    if (ms > 0)                        // repeat timer start
    {
      LOG(5,"start repeat timer @ %d ms @ [%s|%s @%d,%d]",
          ms, BL_IDTXT(bl_id(o)),bl_ix(o),val);
      k_timer_start(t, K_MSEC(ms), K_MSEC(ms));
    }
    else if (ms < 0)                   // single timer start
    {
      LOG(5,"start single timer @ %d ms @ [%s|%s @%d,%d]",
          -ms, BL_IDTXT(bl_id(o)),bl_ix(o),val);
      k_timer_start(t, K_MSEC(-ms), K_NO_WAIT);
    }
    else
    {
      LOG(5,"stop timer @ [%s|%s @%d,%d]", BL_IDTXT(bl_id(o)),bl_ix(o),val);
      k_timer_stop(t);
    }

    return 0;
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
