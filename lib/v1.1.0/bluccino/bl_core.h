//==============================================================================
// bl_core.h
// Bluccino default core, supporting hardware (HW) and wireless (WL) core
//
// Created by Hugo Pristauz on 2022-Apr-02
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================
//
// Module Hierarchie
//
// +- BL_CORE
//    +- BL_HW (hardware core)
//    |  +- BL_HWLED
//    |  +- BL_HWBUT
//    +- BL_WL (wireless core)
//       +- BL_COMP (mesh device composition)
//       +- BL_NVM  (non volatile memory)
//
//==============================================================================
// System Message Flow Diagrams
//==============================================================================
//
// Message Flow Diagram: Initializing
//
//     BL_DOWN        BL_CORE         BL_HW          BL_WL          BL_UP
//       (v)            (#)            (H)            (W)            (^)
//        |              |              |              |              |
//        |  [SYS:INIT]  |              |              |              |
//        o------------->|  [SYS:INIT]  |              |              |
//        |              o------------->|  [SYS:INIT]  |              |
//        |              o---------------------------->|              |
//        |              |              |              |              |
//
// Message Flow Diagram: Ticking
//
//     BL_DOWN        BL_CORE         BL_HW          BL_WL          BL_UP
//       (v)            (#)            (H)            (W)            (^)
//        |              |              |              |              |
//        |  [SYS:TICK]  |              |              |              |
//        o------------->|  [SYS:TICK]  |              |              |
//        |              o------------->|  [SYS:TICK]  |              |
//        |              o---------------------------->|              |
//        |              |              |              |              |
//
// Message Flow Diagram: Tocking
//
//     BL_DOWN         BL_HW        BL_HWBUT       BL_HWLED         BL_UP
//       (v)            (#)            (B)            (L)            (^)
//        |              |              |              |              |
//        |  [SYS:TOCK]  |              |              |              |
//        o------------->|  [SYS:TOCK]  |              |              |
//        |              o------------->|  [SYS:TOCK]  |              |
//        |              o---------------------------->|              |
//        |              |              |              |              |
//
//==============================================================================
// Hardware Core Message Flow Diagrams
//==============================================================================
//
// Message Flow Diagram: Setting LED ON/OFF State
//
//     BL_DOWN        BL_CORE         BL_HW          BL_WL          BL_UP
//       (v)            (#)            (H)            (W)            (^)
//        |              |              |              |              |
//        |  [LED:SET]   |              |              |              |
//        o------------->|              |              |              |
//        |   @ix,onoff  | [LED:ONOFF]  |              |              |
//        |              o------------->|              |              |
//        |              |  @ix,onoff   |              |              |
//        |              |              |              |              |
//
// Message Flow Diagram: Button Configuration
//
//     BL_DOWN        BL_CORE         BL_HW          BL_WL          BL_UP
//       (v)            (#)            (H)            (W)            (^)
//        |              |              |              |              |
//        | [BUTTON:CFG] |              |              |              |
//        o------------->|              |              |              |
//        |     flags    | [BUTTON:CFG] |              |              |
//        |              o------------->|              |              |
//        |              |    flags     |              |              |
//        |              |              |              |              |
//
// Message Flow Diagram: Button Click (Button Release within Grace Time)
//
//   BL_DOWN          BL_CORE           BL_HW            BL_WL            BL_UP
//     (v)              (#)              (H)              (W)              (^)
//      |                |                |                |                |
//      |                |        +-------v-------+        |                |
//      |                |        | press button  |        |                |
//      |                |        | time[@ix] = 0 |        |                |
//      |                |        +-------v-------+        |                |
//      |                |                |                |                |
//      |                | [BUTTON:PRESS] |                |                |
//      |                |<---------------o                |                |
//      |                |      @ix,0     |                | [BUTTON:PRESS] |
//      |                o------------------------------------------------->|
//      |                | [BUTTON:CLICK] |                |     @ix,0      |
//      |                |<---------------o                |                |
//      |                |      @ix,0     |                | [BUTTON:CLICK] |
//      |                o------------------------------------------------->|
//      |                |                |                |     @ix,0      |
//      |                |        +-------v--------+       |                |
//      |                |        | onoff = !onoff |       |                |
//      |                |        | (toggle switch)|       |                |
//      |                |        +-------v--------+       |                |
//      |                |                |                |                |
//      :                :                :                :                :
//      |                |                |                |                |
//      |                |                - now < grace    |                |
//      |                |        +-------v--------+       |                |
//      |                |        | release button |       |                |
//      |                |        |ms=now-time[@ix]|       |                |
//      |                |        +-------v--------+       |                |
//      |                |[BUTTON:RELEASE]|                |                |
//      |                |<---------------o                |                |
//      |                |      @ix,ms    |                |[BUTTON:RELEASE]|
//      |                o------------------------------------------------->|
//      |                |                |                |                |
//      |                |       +--------v--------+       |                |
//      |                |       | ms < grace time |       |                |
//      |                |       | we had 1 click  |       |                |
//      |                |       |     cnt = 1     |       |                |
//      |                |       +--------v--------+       |                |
//      |                | [BUTTON:CLICK] |                |     @ix,ms     |
//      |                |<---------------o                |                |
//      |                |     @ix,cnt    |                | [BUTTON:CLICK] |
//      |                o------------------------------------------------->|
//      |                |                |                |    @ix,cnt     |
//      |                |                |                |                |
//
// Message Flow Diagram: Button Hold (Button Release Exceeds Grace Time)
//
//   BL_DOWN          BL_CORE           BL_HW            BL_WL            BL_UP
//     (v)              (#)              (H)              (W)              (^)
//      |                |                |                |                |
//      |                |        +-------v-------+        |                |
//      |                |        | press button  |        |                |
//      |                |        | time[@ix] = 0 |        |                |
//      |                |        +-------v-------+        |                |
//      |                | [BUTTON:PRESS] |                |                |
//      |                |<---------------o                |                |
//      |                |      @ix,0     |                | [BUTTON:PRESS] |
//      |                o------------------------------------------------->|
//      |                | [BUTTON:CLICK] |                |     @ix,0      |
//      |                |<---------------o                |                |
//      |                |      @ix,0     |                | [BUTTON:CLICK] |
//      |                o------------------------------------------------->|
//      |                |                |                |     @ix,0      |
//      |                |        +-------v--------+       |                |
//      |                |        | onoff = !onoff |       |                |
//      |                |        | (toggle switch)|       |                |
//      |                |        +-------v--------+       |                |
//      |                |                |                |                |
//      :                :                :                :                :
//      |                |                |                |                |
//      |                |                - now == grace   |                |
//      |                | [BUTTON:HOLD]  |                |                |
//      |                |<---------------o                |                |
//      |                |     @ix,0      |                | [BUTTON:HOLD]  |
//      |                o------------------------------------------------->|
//      |                |                |                |     @ix,0      |
//      |                |        +-------v--------+       |                |
//      |                |        | release button |       |                |
//      |                |        |ms=now-time[@ix]|       |                |
//      |                |        +-------v--------+       |                |
//      |                |                |                |                |
//      |                |[BUTTON:RELEASE]|                |                |
//      |                |<---------------o                |                |
//      |                |      @ix,ms    |                |[BUTTON:RELEASE]|
//      |                o------------------------------------------------->|
//      |                |                |                |                |
//      |                | [BUTTON:HOLD]  |                |     @ix,ms     |
//      |                |<---------------o                |                |
//      |                |     @ix,ms     |                | [BUTTON:HOLD]  |
//      |                o------------------------------------------------->|
//      |                |                |                |     @ix,0      |
//      |                |                |                |                |
//
//==============================================================================
// Wireless Core Message Flow Diagrams
//==============================================================================
//
// Message Flow Diagram: Attention State on/off
//
//   BL_DOWN          BL_CORE           BL_HW            BL_WL            BL_UP
//     (v)              (#)              (H)              (W)              (^)
//      |                |                |                |                |
//      |                | []             |                |                |
//      |                |<--------------------------------o                |
//      |                |                |                |                |
//      |                |                |                |                |
//      |                |                |                |                |
//      |                |                |                |                |
//==============================================================================

#ifndef __BL_CORE_H__
#define __BL_CORE_H__

//==============================================================================
// public module interface
//==============================================================================
//
// (B) := (BL_HWBUT);  (L) := (BL_HWLED);  (v) := (BL_DOWN);  (^) := (BL_UP)
//
//                  +--------------------+
//                  |      BL_CORE       |
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (v)->     INIT ->|       <out>        | init module, store <out> callback
// (v)->     TICK ->|       @ix,cnt      | tick the module
// (!)->      CFG ->|        mask        | config module
//                  |        SYS:        | SYS output interface
// (L,B)<-   INIT <-|       <out>        | init module, store <out> callback
// (L,B)->   TICK <-|       @ix,cnt      | tick the module
// (B)<-      CFG <-|        mask        | config module
//                  +--------------------+
//                  |        LED:        | LED: input interface
// (v)->      SET ->|      @ix,onoff     | set LED @ix on/off (i=0..4)
// (v)->   TOGGLE ->|                    | toggle LED @ix (i=0..4)
//                  |        LED:        | LED: output interface
// (L)<-      SET <-|      @ix,onoff     | set LED @ix on/off (i=0..4)
// (L)<-   TOGGLE <-|                    | toggle LED @ix (i=0..4)
//                  +--------------------+
//                  |       BUTTON:      | BUTTON output interface
// (^)<-    PRESS <-|        @ix,0       | button @ix pressed (rising edge)
// (^)<-  RELEASE <-|        @ix,ms      | button release after elapsed ms-time
// (^)<-    CLICK <-|       @ix,cnt      | button @ix clicked (cnt: nmb. clicks)
// (^)<-     HOLD <-|       @ix,time     | button @ix held (time: hold time)
// (B)<-      CFG <-|        mask        | config button event mask
//                  |       BUTTON:      | BUTTON input interface
// (B)->    PRESS ->|        @ix,1       | button @ix pressed (rising edge)
// (B)->  RELEASE ->|        @ix,ms      | button release after elapsed ms-time
// (B)->    CLICK ->|       @ix,cnt      | button @ix clicked (cnt: nmb. clicks)
// (B)->     HOLD ->|       @ix,time     | button @ix held (time: hold time)
// (v)->      CFG ->|        mask        | config button event mask
//                  +--------------------+
//                  |       SWITCH:      | SWITCH: output interface
// (^)<-      STS <-|       @ix,sts      | on/off status update of switch @ix
//                  |       SWITCH:      | SWITCH: input interface
// (B)->      STS ->|       @ix,sts      | on/off status update of switch @ix
//                  +--------------------+
//
//==============================================================================

  int bl_core(BL_ob *o, int val); // public module interface

#endif // __BL_CORE_H__
