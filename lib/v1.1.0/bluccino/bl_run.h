//==============================================================================
//  bl_run.h
//  Bluccino init/tic/toc engine and integration of test modules
//
//  Created by Hugo Pristauz on 2022-04-03
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================
//
// Example 1: use bl_run() for separate app() module and when() callback
//
//   #include "bluccino.h"
//
//   int when(BL_ob *o, int val);      // separate callback to handle events
//   int app(BL_ob *o, int val);       // public APP module interface
//
//   void main(void)                   // typical main() function
//   {
//     bl_hello(4,"Bluccino engine for app/when case");
//     bl_run(app,10,1000,when);       // run app with 10/1000 ms tick/tock
//   }
//
//==============================================================================
//
// Example 2: use bl_run() for separate app() module and when() callback
// with additional integration of a main test module test()
//
//   #include "bluccino.h"
//
//   int test(BL_ob *o, int val);      // main test module
//   int when(BL_ob *o, int val);      // separate callback to handle events
//   int app(BL_ob *o, int val);       // public APP module interface
//
//   void main(void)                   // typical main() function
//   {
//     bl_hello(4,"Bluccino engine for app/when case");
//     bl_test(test);                  // config test module inclusion
//     bl_run(app,10,1000,when);       // run app with 10/1000 ms tick/tock
//   }
//
//==============================================================================
//
// Example 3: (standard scenario) use bl_run() if app() handles also callbacks
// with additional integration of a main test module test()
//
//   #include "bluccino.h"
//
//   #if CFG_SUPPORT_TEST
//     #define TEST  ft_test           // test module: ft_test()
//   #else
//     #define TEST  NULL              // test module not supported
//   #endif
//
//   int test(BL_ob *o, int val);      // main test module
//   int app(BL_ob *o, int val);       // public APP module interface
//
//     // note: the call bl_engine(app,ms_tick,ms_tock) is nothing but syntactic
//     // sugar for the call bl_run(app,ms_tick,ms_tock,app)
//
//   void main(void)                   // typical main() function
//   {
//     bl_hello(4,"Bluccino engine for app/when case");
//     bl_test(TEST);                  // config test module inclusion
//     bl_engine(app,10,1000);         // run app with 10/1000 ms tick/tock
//   }
//
//==============================================================================
//
// Typical Hierarchy and output message flow with test module integration,
// demonstrating that
//
// +- MAIN => bl_run(app,tick_ms,tock_ms,when)
//     |
//     +- BLUCCINO =======================>|    (Bluccino module)
//     |   |                               |
//     |   +- BL_TOP =====================>|    (top gear)
//     |   |   (^)<====================|   |
//     |   |                           |   |
//     |   +- BL_UP ==================>|   |    (up gear)
//     |   |   (^)<================|   |   |
//     |   |                       |   |   |
//     |   +- BL_DOWN ========>|   |   |   |    (down gear)
//     |       |               |   |   |   |
//     |       |   (v)<========|   |   |   |
//     |       +- BL_CORE ========>|   |   |    (core system)
//     |                               |   |
//     +- APP ============================>|    (application)
//     |  (^)<=============================|
//     |                               |   |
//     |   (v)<========================|   |
//     +- TEST ===========================>|    (main test module)
//
// Message Flow Chart: Initializing using bl_engine()
// Note that test() may occasionally reconfigure message flow!
//
//  MAIN  BLUCCINO     BL_DOWN BL_CORE  BL_UP  BL_TOP       APP     TEST
//   |       |           |       |       |       |           |       |
//   |[SYS:INIT <app>]   |       |       |       |           |       |
//   o------>#==============================================>|       |
//   |       |           |       |       |       |           |       |
//   |       |       [SYS:INIT <app>]    |       |           |       |
//   |       o---------------------------------->#==========>|       |
//   |       |           |       |       |       |           |       |
//   |       |       [SYS:INIT <bl_top>] |       |           |       |
//   |       o-------------------------->#======>|           |       |
//   |       |           |       |       |       |           |       |
//   |       |[SYS:INIT <bl_up>] |       |       |           |       |
//   |       o---------->|       |       |       |           |       |
//   |       |           |       |       |       |           |       |
//   |       |           |[SYS:INIT <*>] |       |           |       |
//   |       |           o------>#======>|       |           |       |
//   |       |           |       |       |       |           |       |
//   |       | [SYS:INIT <app>]  |       |       |           |       |
//   o------------------------------------------------------>|       |
//   |       |           |       |       |       |       +===#       |
//   |       |           |       |       |       |       |   |       |
//   |       |           |       |       |       |       +==>|       |
//   |       | [SYS:INIT <app>]  |       |       |           |       |
//   o-------------------------------------------------------------->|
//   |       |           |       |       |       |           |<======#
//   |       |           |       |       |       |           |       |
//
//==============================================================================

#ifndef __BL_RUN_H__
#define __BL_RUN_H__

//==============================================================================
// monitoring structure for bl_run performance
//==============================================================================

  typedef struct BL_run           // run monitoring record
          {
            BL_ms due;            // when is assessment period due (ms)?
            BL_us start;          // start clock us-time of assessment segment
            BL_us tic;            // tic (begin) time of assessment segment
            BL_us duty;           // cumulated duty us-time of period
            BL_us total;          // total cumulated time of assessment
            BL_ms period;         // run logging period
            int late;             // count late (sleep negative time)
            int tick;             // tick period in ms
            int tock;             // tock period in ms
          } BL_run;

//==============================================================================
// message definitions
//==============================================================================
//
//                  +--------------------+
//                  |        SYS:        | SYS interface
//            RUN --|  <BL_run>,permill  | notify run monitoring record
//                  +--------------------+
//
//==============================================================================

  #define SYS_RUN_0_BL_run_permill   BL_ID(_SYS,RUN_)

    // augmented messages

  #define _SYS_RUN_0_BL_run_permill  _BL_ID(_SYS,RUN_)

//==============================================================================
// syntactig sugar: init a module
// usage:  bl_init(module,cb)       // init module, <out> goes to callback
//==============================================================================

  static inline int bl_init(BL_oval module,BL_oval cb)
  {
    return bl_msg(module,_SYS,INIT_, 0,(void*)cb,0);  // init module
  }

//==============================================================================
// enable/disable interrupts
// - usage: bl_irq(0)   // disable interrupts
// -        bl_irq(1)   // enable interrupts
//==============================================================================

  void bl_irq(bool enable);

//==============================================================================
// syntactic sugar: tick a module
// - usage: bl_tick(module,count)   // (MODULE)<-[SYS:TICK @ix,count]
//==============================================================================

  static inline int bl_tick(BL_oval module, int id, int cnt)
  {
    return bl_msg(module,_SYS,TICK_, id,NULL,cnt);
  }

//==============================================================================
// syntactic sugar: tock a module
// - usage: bl_tock((module),count)    // (MODULE)<-[SYS:TOCK @ix,count]
//==============================================================================

  static inline int bl_tock(BL_oval module, int id, int cnt)
  {
    return bl_msg(module,_SYS,TOCK_, id,NULL,cnt);
  }

//==============================================================================
// set callback value (warn if new callback deviates from provided default)
// - usage: out = bl_cb(o,(out));
//==============================================================================

  BL_oval bl_cb(BL_ob *o, BL_oval def, BL_txt msg);

//==============================================================================
// setup a initializing, ticking and tocking for a test module
// - usage: bl_test((module))          // controlled by bl_run()
//==============================================================================

  int bl_test(BL_oval module);

//==============================================================================
// pace maker: emit tick/tock events when time has come, return ms to sleep
// - usage: ms = bl_pace(app,&tick,&tock)    // send tick/tock events to app
//          bl_pace(NULL,&tick,&tock);       // reset pace maker
// - example:
//    BL_pace tick = BL_PACE(10,0);  BL_pace tock = BL_PACE(1000,0);
//    for(;;) bl_sleep(bl_pace(app,&tick,&tock)); // send tick/tock msg's to app
//==============================================================================

  int bl_pace(BL_oval to, BL_pace *tick, BL_pace *tock);

//==============================================================================
// run app with given tick/tock periods and provided when-callback
// - usage: bl_run((app),10,100,(when)) // run app with 10/1000 tick/tock periods
//==============================================================================

  void bl_run(BL_oval app, int tick_ms, int tock_ms, BL_oval when);

//==============================================================================
// syntactic sugar: run app where app()and when() function are the same
// - usage: bl_engine((app),10,100)    // run app with 10/1000 tick/tock periods
//==============================================================================

  static inline void bl_engine(BL_oval app, int tick_ms, int tock_ms)
  {
    bl_run(app, tick_ms,tock_ms, app); // callbacks are the same
  }

//==============================================================================
// syntactic sugar: work bench setup
// - usage: bl_bench(4,"init workbench",NULL);  // no event subscription
//          bl_bench(4,"init workbench",cb);    // events go to callback (cb)
//==============================================================================

  static inline int bl_bench(int verbose, BL_txt txt, BL_oval cb)
  {
    bl_hello(verbose,txt);
    bl_init(bl_gear,cb);
    return bl_init(cb,NULL);
  }

#endif // __BL_RUN_H__
