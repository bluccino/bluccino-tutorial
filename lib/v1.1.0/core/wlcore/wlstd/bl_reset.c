//==============================================================================
//  bl_reset.c
//  core reset module
//
//  Created by Hugo Pristauz on 2022-06-16
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================
//
//  bl_node        gear             bl_wl         bl_reset          timer
//   (N)          (D)(T)(U)          (W)             (R)             (T)
//    |            |  |  |            |               |               |
//    |            |  |  | [SYS:INIT] |               |               |
//    |            o----------------->|  [SYS:INIT]   |               |
//    |            |  |  |            o-------------->|               |
//    |            |  |  |            |               |               |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |  | if (counter > 3):       |  |
//    |            |  |  |            |  |   reset mesh node       |  |
//    |            |  |  |            |  |   counter=1, save @ NVM |  |
//    |            |  |  |            |  | else:                   |  |
//    |            |  |  |            |  |   counter++, save @ NVM |  |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |               |               |
//    |            |  |  |            |               | [TIMER:START] |
//    | [SYS:INIT] |  |  |            |               o-------------->|
//    |<--------------o  |            |               |    7000ms     |
//    |            |  |  |            |  [RESET:INC]  |               |
//    o---------------------------------------------->|               |
//    |            |  |  |            |    5000ms     | [TIMER:START] |
//    |            |  |  |            |               o-------------->|
//    |            |  |  |            |               |    5000ms     |
//    |            |  |  |            |               |               - after 5s
//    |            |  |  |            |               |               |
//  ---------------------------------------------------------------------
//     here the user could potentially decide for a power interruption
//               (which would terminate the event flow)
//  ---------------------------------------------------------------------
//    |            |  |  |            |               |               |
//    :            :  :  :            :               :               :
//    |            |  |  |            |               |               |
//  ---------------------------------------------------------------------
//       here the user could optionally press a button (first time)
//  ---------------------------------------------------------------------
//    |            |  |  |            |               |               |
//    | [BUTTON:PRESS]|<-o            |               |               |
//    |<--------------o  |            |               |               |
//    |            |  |  |            |  [RESET:INC]  |               |
//    o<--------------------------------------------->|               |
//    |            |  |  |            |    5000 ms    |               |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |  |  counter++, save @ NVM  |  |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |               | [TIMER:START] |
//    |            |  |  |            |               o-------------->|
//    |            |  |  |            |               |    5000ms     |
//    |            |  |  |            |               |               |
//  ---------------------------------------------------------------------
//       here the user could optionally press a button (second time)
//  ---------------------------------------------------------------------
//    |            |  |  |            |               |               |
//    | [BUTTON:PRESS]|<-o            |               |               |
//    |<--------------o  |            |               |               |
//    |            |  |  |            |  [RESET:INC]  |               |
//    o<--------------------------------------------->|               |
//    |            |  |  |            |    5000 ms    |               |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |  |  counter++, save @ NVM  |  |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |               | [TIMER:START] |
//    |            |  |  |            |               o-------------->|
//    |            |  |  |            |               |    5000ms     |
//    |            |  |  |            |               |               |
//  ---------------------------------------------------------------------
//       here the user could optionally press a button (third time)
//  ---------------------------------------------------------------------
//    |            |  |  |            |               |               |
//    | [BUTTON:PRESS]|<-o            |               |               |
//    |<--------------o  |            |               |               |
//    |            |  |  |            |  [RESET:PRV]  |               |
//    o<--------------------------------------------->|               |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |  |  counter=0, save @ NVM  |  |
//    |            |  |  |            |  |    unprovision node     |  |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |               |               |
//    :            :  :  :            :               :               :
//    |            |  |  |            |               |               |
//  ---------------------------------------------------------------------
//       otherwise this will happen in case of no user interaction
//  ---------------------------------------------------------------------
//    |            |  |  |            |               |               |
//    :            :  :  :            :               :               :
//    |            |  |  |            |               |               |
//    |            |  |  |            |               | [TIMER:FIRE]  |
//    |            |  |  |            |               |<--------------o
//    |            |  |  |            |               |               |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |  |  counter=0, save @ NVM  |  |
//    |            |  |  |            |  +------------v------------+  |
//    |            |  |  |            |               |               |
//    |            |  |  |            |  [RESET:DUE]  |               |
//    |            |  |  |RESET:DUE]  |<--------------o               |
//    |            |  |  |<-----------o               |               |
//    |  [RESET:DUE]  |<-o            |               |               |
//    |<--------------o  |            |               |               |
//    |            |  |  |            |               |               |
//
//==============================================================================

  #include "bluccino.h"
  #include "bl_reset.h"

  #define PMI bl_reset            // public module interface
  #define sys_init reset_sys_init // avoid collisions

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO "bl_reset:"         // who is logging

  #define LOG                     LOG_RESET
  #define LOGO(lvl,col,o,val)     LOGO_RESET(lvl,col WHO,o,val)

//==============================================================================
// timer: reset counter timer fires
//==============================================================================

  static void timer_fire(struct k_timer *dummy)
  {
    reset_counter = 0U;
    save_on_flash(RESET_COUNTER);
    LOG(3,BL_M "reset counter set to zero");

    _bl_post((PMI),_RESET_DUE_0_0_0, 0,NULL,0);
  }

  K_TIMER_DEFINE(timer, timer_fire, NULL);

//==============================================================================
// worker: unprovision mesh node
//==============================================================================

  static int reset_prv(BL_ob *o, int val) // uprovision mesh node
  {
    reset_counter = 0U;
    save_on_flash(RESET_COUNTER);

    LOG(3,BL_B "unprovision node");
    bt_mesh_reset();
    return 0;                               // OK
  }

//==============================================================================
// worker: increment reset counter, set due timer
// - set due timer, return reset counter after increment
//==============================================================================

  static int reset_inc(BL_ob *o, int ms)
  {
    reset_counter++;
    LOG(3,BL_M "reset counter: %d", reset_counter);

    k_timer_start(&timer, K_MSEC(ms<1000?1000:ms), K_NO_WAIT);

    save_on_flash(RESET_COUNTER);
    return reset_counter;             // return counter value after increment
  }

//==============================================================================
// worker: system init
// - implements 'short time multireset bt mesh unprovisioning' functionality
// 1) in case of reset counter overflow at value 4:
//    - clear reset counter and save on NVM
//    - take off node from BT mesh
// 2) otherwise increment reset counter and save on flash
// 3) start timer with 7s timeout
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
/*
    if (reset_counter >= 4U)
    {
      reset_counter = 0U;
      LOG(1,BL_M "BT Mesh reset");
      bt_mesh_reset();
    }
    else
    {
      LOG(1,BL_M "reset counter -> %d", reset_counter);
      reset_counter++;
    }
*/
    save_on_flash(RESET_COUNTER);

      // finally start reset counter timer

    k_timer_start(&timer, K_MSEC(7000), K_NO_WAIT);
    return 0;
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (W) := (bl_wl);
//                  +--------------------+
//                  |      bl_reset      | reset module
//                  +--------------------+
//                  |        SYS:        | SYS: public interface
// (W)->     INIT ->|       @ix,cnt      | init module, store <out> callback
//                  +--------------------+
//                  |       RESET:       | RESET input interface
// (W)->      INC ->|         ms         | set due timer, return ++counter
// (W)->      PRV ->|                    | unprovision node
//                  |....................|
//                  |      #RESET:       | RESET output interface
// (W)<-      DUE <-|                    | reset timer is due
//                  +--------------------+
//
//==============================================================================

  int bl_reset(BL_ob *o, int val)
  {
    static BL_oval W = bl_wl;          // wireless core

    switch (bl_id(o))
    {
      case SYS_INIT_0_cb_0:
        W = bl_cb(o,(W),"rct:(W)");
        return sys_init(o,val);        // delegate to sys_init() worker

      case RESET_PRV_0_0_0:            // unprovision node
        return reset_prv(o,val);       // delegate to reset_prv() worker

      case RESET_INC_0_0_ms:
        return reset_inc(o,val);       // delegate to reset_inc() worker

      case _RESET_DUE_0_0_0:
        return bl_out(o,val,(W));      // output to wireless core

      default:
        return -1;                     // bad input
    }
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
  #undef   sys_init
