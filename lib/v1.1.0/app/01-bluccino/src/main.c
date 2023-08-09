//==============================================================================
// main.c
// main program for 01-bluccino
//
// Created by Hugo Pristauz on 2022-Jan-04
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================
//
// Event Flow in unprovisioned state ("device" state)
// - any button press toggles (local) LED @2 (red LED on 52840 dongle)
//
//     +-----------+
//     | BUTTON @1 |---+
//     +-----------+   |
//                     |
//     +-----------+   |
//     | BUTTON @2 |---+
//     +-----------+   |                                        +--------+
//                     +--------------------------------------->| LED @2 |
//     +-----------+   |                                        +--------+
//     | BUTTON @3 |---+
//     +-----------+   |
//                     |
//     +-----------+   |
//     | BUTTON @4 |---+
//     +-----------+
//
// Event Flow in provisioned state ("node" state)
// - any button switch state is posted via GOOCLI to mesh network
//
//     +-----------+
//     | BUTTON @1 |---+
//     +-----------+   |
//                     |
//     +-----------+   |
//     | BUTTON @2 |---+
//     +-----------+   |    +-----------+       +-----------+   +--------+
//                     +--->| GOOCLI @1 | ))))) | GOOSRV @1 |-->| LED @k |
//     +-----------+   |    +-----------+       +-----------|   +--------+
//     | BUTTON @3 |---+                                         k=2,3,4
//     +-----------+   |
//                     |
//     +-----------+   |
//     | BUTTON @4 |---+
//     +-----------+
//
// APP has to implement the following event message flow:
// - [SWITCH:STS @ix,val] events are forwarded to [GOOCLI:SET @1,val] posts
// - mesh network has to be configured as follows:
//     -- GOOCLI @1 has to post to group G1
//     -- GOOSRV @1 has to subscribe group G1
// - [GOOSRV:STS @1,val] events are forwarded to [LED:SET @2,val] calls
// - bl_led(ix,val) is a helper routine for posting [LED:SET @ix,val] events
//
//==============================================================================

  #include "bluccino.h"
  #include "bl_node.h"
  #include "bl_spool.h"
  #include "bl_core.h"
  #include "bl_hw.h"
  #include "bl_gonoff.h"

//==============================================================================
// PMI definition and logging shorthands
//==============================================================================

  int app(BL_ob *o, int val);          // forward declaration

  #define WHO                     "main:"

  #define LOG                     LOG_MAIN
  #define LOGO(lvl,col,o,val)     LOGO_MAIN(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_MAIN(lvl,col,o,val)

  #define PMI  app                     // public module interface

//==============================================================================
// defines & locals
//==============================================================================

  #define T_BLINK   1000                    // 1000 ms RGB blink period

  static volatile int ix = 0;               // THE LED index
  static int starts = 0;                    // counts system starts

//==============================================================================
// helper: attention blinker (let green status LED @0 attention blinking)
// - @ix=0: dark, @ix=1: status, @ix=2: red, @ix=3: green, @ix=4: blue
//==============================================================================

  static int blink(BL_ob *o, int ticks)     // attention blinker to be ticked
  {
    static BL_ms due = 0;                   // need for periodic action

    if (ix <= 1 || !bl_due(&due,T_BLINK))   // no blinking if @ix:off or not due
      return 0;                             // bye if LED off or not due

    if ( _bl_get(ATT_,(PMI)) ||             // no blinking in attention mode
         _bl_get(BUSY_,(PMI)) )             // no blinking during startup
      return 0;                             // bye if attention state

    static bool toggle;
    toggle = !toggle;

    if (toggle)
      return (bl_led(ix,1), bl_led(2+ix%3,0));    // flip LED pair
    else
      return (bl_led(ix,0), bl_led(2+ix%3,1));    // flip back LED pair
  }

//==============================================================================
// public app module interface
//==============================================================================
//
// M = main;  (L) := (bl_hwled);  (U) := (bl_up);  (D) := (bl_down);
// (N) := (bl_node);   (T) := (bl_top);
//
//                  +--------------------+
//                  |        app         |
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     INIT ->|       @ix,cnt      | init module, store <out> callback
// (M)->     TICK ->|  @ix,<BL_pace>,cnt | tick the module
// (M)->     TOCK ->|  @ix,<BL_pace>,cnt | tock the module
// (M)->      RUN ->|  <BL_run>,permill  | notify run monitoring record
//                  +--------------------+
//                  |       SWITCH:      | SWITCH: output interface
// (U)->      STS ->|       @ix,sts      | on/off status update of switch @ix
//                  +--------------------+
//                  |       GOOSRV:      | GOOSRV: ifc. (generic on/off server)
// (U)->      STS ->| @ix,<BL_goo>,onoff | on/off server status update
//                  +--------------------+
//                  |       GOOCLI:      | GOOCLI: ifc. (generic on/off client)
// (T)<-      SET <-| @ix,<BL_goo>,onoff | publish generic on/off SET command
// (T)<-      LET <-| @ix,<BL_goo>,onoff | publish generic on/off LET command
//                  +--------------------+
//                  |        NVM:        | NVM: interface (non volatile memory)
// (U)->    READY ->|       ready        | notify that NVM is ready
//                  +--------------------+
//                  |        GET:        | GET output interface
// (D)<-      ATT <-|                    | get attention status
// (D)<-      PRV <-|                    | get provision status
//                  +--------------------+
//                  |        LED:        | LED output interface
// (D)<-      SET <-|     @ix,onoff      | set LED @ix on/off state
// (D)<-   TOGGLE <-|        @ix         | toggle LED @ix state
//                  +--------------------+
///
//==============================================================================

  int app(BL_ob *o, int val)           // public APP module interface
  {
    static BL_oval D = bl_down;        // down gear
    static BL_oval T = bl_top;         // top gear

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        return 0;

      case BL_ID(_SYS,TICK_):
        return blink(o,val);           // tick blinker

      case BL_ID(_SYS,TOCK_):          // [SYS:TOCK @ix,cnt]
        if (val % 20 == 0)             // log every 20th tock
          LOGO(1,"I'm alive! ",o,val); // log to see we are alife
        return 0;                      // OK

      case BL_ID(_SYS,RUN_):
        LOGO(1,BL_C"(#)",o,val);
        return 0;

      case BL_ID(_SWITCH,STS_):
        LOGO(1,"@",o,val);
        if ( _bl_get(PRV_,(PMI)) )     // only if provisioned
        {
          BL_goo goo = {delay:150, tt:100};
          bl_log(1,BL_R "post [#GOOCLI:LET @%d,</%dms,&%dms>,%d] -> (app)",
 	               bl_ix(o), goo.tt,goo.delay, val);

          if (bl_ix(o) % 2 == 0)
            _bl_goolet(1,&goo,val,(PMI)); // [GONOFF:LET @1,&goo,val] -> (PMI)
          else
            _bl_gooset(1,&goo,val,(PMI)); // [GONOFF:LET @1,&goo,val] -> (PMI)
        }
        else
          _bl_led(ix,val,(PMI));       // switch LED @ix on/off
        return 0;                      // OK

      case BL_ID(_GOOSRV,STS_):        // generic on/off server status update
        LOGO(1,BL_R,o,val);
        if (bl_ix(o) == 1)
          _bl_led(ix,val,(D));         // switch LED @ix
        return 0;                      // OK

      case _BL_ID(_GOOCLI,SET_):       // generic on/off server client SET
      case _BL_ID(_GOOCLI,LET_):       // generic on/off server client LET
        return bl_out(o,val,(T));      // output to top gear

      case BL_ID(_NVM,READY_):
        LOGO(1,BL_M,o,val);
        starts = bl_recall(0);         // recall system starts from NVM @0
        ix = 2 + (starts % 3);         // map starts -> 2:4
        bl_store(0,++starts);          // store back incremented value at NVM @0
        LOG(1,BL_M "system start #%d",starts);
        return 0;

      case _BL_ID(_GET,ATT_):
      case _BL_ID(_GET,PRV_):
        bl_out(o,val,(D));             // post to down gear

      case _BL_ID(_LED,SET_):
      case _BL_ID(_LED,TOGGLE_):
        bl_out(o,val,(D));

      default:
        return BL_VOID;                // interaction not handeled
    }
  }

//==============================================================================
// main function
//==============================================================================

  void main(void)
  {
    bl_hello(VERBOSE,PROJECT);         // set verbose level, print hello message
    bl_install(bl_node);               // install bl_node in top gear
    bl_install(bl_spool);              // install bl_spool in top gear
    bl_run(app,10,1000,app);           // run app with 10/1000 ms tick/tock
  }
