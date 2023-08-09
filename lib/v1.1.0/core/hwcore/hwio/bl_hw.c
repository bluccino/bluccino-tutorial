//==============================================================================
// bl_hw.c
// tiny Bluccino HW core supporting GPIO access
//
// Created by Hugo Pristauz on 2022-Oct-04
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include "bluccino.h"

    // C-file includes

  #include "bl_hwio.c"

//==============================================================================
//
// (B) := bl_hwbut;  (L) := bl_hwled;  (D) := bl_down;  (U) := bl_up
//
//                  +--------------------+
//                  |       bl_hw        |
//                  +--------------------+
//                  |        PIN:        | PIN input interface
//           DEVICE |     "name",pin     | use GPIO device (return instance idx)
//             MODE |       @ix,mode     | set pin mode
//           ATTACH |   @ix,(cb),flags   | attach interrupt routine
//              GET |         @ix        | digital get
//              SET |       @ix,onoff    | digital set
//             READ |         @ix        | analog read (0..10000)
//            WRITE |       @ix,val      | analog write (0..10000)
//                  +--------------------+
//
//==============================================================================

  int bl_hw(BL_ob *o, int val)         // HW core module interface
  {
    static BL_oval I = bl_hwio;        // HW-I/O driver

    switch (bl_cl(o))                  // dispatch message class tag
    {
      case _SYS:
      case _DIGITAL:
      case _ANALOG:
      case _IO:
        return bl_fwd(o,val,(I));      // forward to I/O driver

      default:
        return BL_VOID;                // unhandeled event
    }
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
