//==============================================================================
// bl_work.c
// Bluccino (swiss knife) work package submission
//
// Created by Hugo Pristauz on 2022-Aug-27
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================
// Bluccino work package submission
// - target: simple method to submit work to a Bluccino OVAL-interface based
//   worker function which offloads execution and datato a seperate thread
// - Zephyr work package data structures are auto initialized during the first
//   work package submission, an explicite call to an initializing function on
//   Bluccino level is not required
// - two data structures are used:
//   1) static BL_work structure which comprises a (Zephyr) strukt k_work
//   2) fuction pointer of a worker module which processes the work package
//      in a separate thread
//   3) BL_ob and int data elements to hold copy values of an event message
//      which triggers the worker module to start processing the work package
// - an initializer macro BL_WORK(worker) is provided to
//   1) init the worker module function reference
//   2) and to init BL_ob data with the (invalid) message ID [VOID:VOID]
// - a work package submission function bl_submit(&work,o,val) is provided to
//   submit a work package which
//   1) auto-initializes struct k_work if BL_ob data has an in initial
//      [VOID:VOID] value
//   2) stores the message data given by o and val into the BL_work structure
//      (overwriting the [VOID:VOID] initial value)
//   3) submits the Zephyr work package to the Zephyr work queue
// - there is an internal worker (executed in a separate task) which processes
//   the submitted work and posts the Bluccino message to the worker module
//==============================================================================

  #include "bluccino.h"                // access to Bluccino stuff
  #include "bl_work.h"

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO                     "bl_work:"

  #define LOG                     LOG_WORK
  #define LOGO(lvl,col,o,val)     LOGO_WORK(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_WORK(lvl,col,o,val)

//==============================================================================
// helper: internal worker
//==============================================================================

  static void worker (struct k_work *w)
  {
    BL_work *p = CONTAINER_OF(w,BL_work,work);  // points to BL_work structure

    LOGO(5,"post work package to worker module",&p->oo,p->val);
    p->oval(&p->oo, p->val);
  }

//==============================================================================
// Bluccino work data structure and work processing in a thread
// -usage: static BL_work work = BL_WORK(module);
//         bl_submit(&work,o,val);     // submit work to execute immediately
//==============================================================================

  int bl_submit(BL_work *p, BL_ob *o, int val)
  {
    if (bl_is(&p->oo,_VOID,VOID_))     // is p->work not yet initialized?
    {
      LOG(5,"init work ...");
      k_work_init(&p->work, worker);   // submit work to internal worker
    }

      //a potential [VOID:VOID] value in p->oo will now be overwritten

    p->oo = *o;  p->val = val;    // copy message ID and message args

    LOG(5,"bl_submit: [%s|%s @%d,%d] work package submission",
          BL_IDTXT(bl_id(o)),bl_ix(o),val);

    k_work_submit(&p->work);
    return 0;
  }

//==============================================================================
// Bluccino work data structure and work processing in a thread
// -usage: static BL_work work = BL_WORK(worker);
//         bl_sched(&work,o,val,time); // submit work to execute @ time
//==============================================================================
/*
  int bl_sched(BL_work *p, BL_ob *o, int val, BL_ms time)
  {
    if (bl_is(&p->oo,_VOID,VOID_))     // is p->work not yet initialized?
    {
      LOG(4,"init work");
      k_work_init(&p->work, worker);   // sibmit work to internal worker
    }

      //a potential [VOID:VOID] value in p->oo will now be overwritten

    p->oo = *o;  p->val = val;    // copy message ID and message args

    LOG(4,"bl_sched: [%s|%s @%d,%d] work package scheduling @ +%d ms",
          (int)(time-bl_ms()), BL_IDTXT(bl_id(o)),bl_ix(o),val);

    k_timeout_t delay = (k_timeout_t)(time - bl_ms());
    k_work_schedule(&p->work, delay >= 0 ? delay : 0);
    //k_work_submit(&p->work);
    return 0;
  }
*/
//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
