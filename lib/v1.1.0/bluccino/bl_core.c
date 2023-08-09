//==============================================================================
// bl_core.c (weak Bluccino core - used as the default)
// Bluccino default core, supporting hardware (HW) and wireless (WL) core
//
// Created by Hugo Pristauz on 2022-Apr-02
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_hw.h"                   // hardware core
  #include "bl_wl.h"                   // wireless core

//==============================================================================
// CORE level logging shorthands
//==============================================================================

  #define LOG                     LOG_CORE
  #define LOGO(lvl,col,o,val)     LOGO_CORE(lvl,col"core:",o,val)
  #define LOG0(lvl,col,o,val)     LOGO_CORE(lvl,col,o,val)

//==============================================================================
// hardware core (weak defaults - public module interface)
//==============================================================================
//
// (B) := bl_hwbut;  (L) := bl_hwled;  (D) := bl_down;  (U) := bl_up
//
//                  +--------------------+
//                  |       bl_hw        |
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (D)->     INIT ->|       <out>        | init module, store <out> callback
// (D)->     TICK ->|       @ix,cnt      | tick the module
// (!)->      CFG ->|        mask        | config module
//                  |        SYS:        | SYS output interface
// (L,B)<-   INIT <-|       <out>        | init module, store <out> callback
// (L,B)->   TICK <-|       @ix,cnt      | tick the module
// (B)<-      CFG <-|        mask        | config module
//                  +--------------------+
//                  |        LED:        | LED: input interface
// (D)->      SET ->|      @ix,onoff     | set LED @ix on/off (i=0..4)
// (D)->   TOGGLE ->|                    | toggle LED @ix (i=0..4)
//                  |        LED:        | LED: output interface
// (L)<-      SET <-|      @ix,onoff     | set LED @ix on/off (i=0..4)
// (L)<-   TOGGLE <-|                    | toggle LED @ix (i=0..4)
//                  +--------------------+
//                  |       BUTTON:      | BUTTON output interface
// (U)<-    PRESS <-|        @ix,0       | button @ix pressed (rising edge)
// (U)<-  RELEASE <-|        @ix,ms      | button release after elapsed ms-time
// (U)<-    CLICK <-|       @ix,cnt      | button @ix clicked (cnt: nmb. clicks)
// (U)<-     HOLD <-|       @ix,time     | button @ix held (time: hold time)
// (B)<-      CFG <-|        mask        | config button event mask
//                  |       BUTTON:      | BUTTON input interface
// (B)->    PRESS ->|        @ix,1       | button @ix pressed (rising edge)
// (B)->  RELEASE ->|        @ix,ms      | button release after elapsed ms-time
// (B)->    CLICK ->|       @ix,cnt      | button @ix clicked (cnt: nmb. clicks)
// (B)->     HOLD ->|       @ix,time     | button @ix held (time: hold time)
// (D)->      CFG ->|        mask        | config button event mask
//                  +--------------------+
//                  |       SWITCH:      | SWITCH: output interface
// (U)<-      STS <-|       @ix,sts      | on/off status update of switch @ix
//                  |       SWITCH:      | SWITCH: input interface
// (B)->      STS ->|       @ix,sts      | on/off status update of switch @ix
//                  +--------------------+
//
//==============================================================================

  __weak int bl_hwbut(BL_ob *o, int val) { return -1; }
  __weak int bl_hwled(BL_ob *o, int val) { return -1; }
  __weak int bl_hwnvm(BL_ob *o, int val) { return -1; }

  __weak int bl_hw(BL_ob *o, int val)
  {
    switch (o->cl)
    {
      case _SYS:
        bl_hwbut(o,val);
        bl_hwled(o,val);
        bl_hwnvm(o,val);
        return 0;                      // OK

      case _LED:
        return bl_hwled(o,val);

      case _BUTTON:
			case _SWITCH:
        return bl_hwbut(o,val);

      case _NVM:
        return bl_hwnvm(o,val);

      default:
        return -1;
    }
  }

//==============================================================================
// wireless core (weak defaults)
//==============================================================================
//
// (D) := (bl_down);  (U) := (bl_up);
//
//                  +--------------------+
//                  |       BL_WL        | wireless core module
//                  +--------------------+
//                  |        SYS:        | SYS: public interface
// (D)->     INIT ->|        <cb>        | init module, store output callback
// (D)->     TICK ->|       @ix,cnt      | tick the module
// (D)->     TOCK ->|       @ix,cnt      | tock the module
//                  +--------------------+
//                  |        SET:        | SET: public interface
// (U)<-      PRV <-|       onoff        | provision on/off
// (U)<-      ATT <-|       onoff        | attention on/off
//                  +--------------------+
//                  |       RESET:       | RESET input interface
// (D)->      INC ->|         ms         | inc reset counter & set due timer
// (D)->      PRV ->|                    | unprovision node
//                  +--------------------+
//                  |       RESET:       | RESET output interface
// (U)<-      DUE <-|                    | reset timer is due
//                  +--------------------+
//                  |        NVM:        | NVM input interface
// (D)->    STORE ->|      @ix,val       | store value in NVM at location @ix
// (D)->   RECALL ->|        @ix         | recall value in NVM at location @ix
// (D)->     SAVE ->|                    | save NVM cache to NVM
//                  +--------------------+
//                  |        NVM:        | NVM output interface
// (U)<-    READY <-|       ready        | notification that NVM is now ready
//                  +--------------------+
//                  |      GONOFF:       | GONOFF input interface (gen. on/off)
// (D)->      SET ->|  @ix,val,<BL_goo>  | publish [GONOFF:SET] mesh message
// (D)->      LET ->|  @ix,val,<BL_goo>  | publish [GONOFF:LET] mesh message
// (D)->      GET ->|  @ix,val,<BL_goo>  | publish [GONOFF:GET] mesh message
//                  +--------------------+
//                  |      GONOFF:       | GONOFF output interface (gen. on/off)
// (U)<-      STS <-|  @ix,val,<BL_goo>  | notify [GONOFF:STS] status update
//                  +--------------------+
//                  |      GLEVEL:       | GLEVEL input interface (gen. level)
// (D)->      SET ->|  @ix,val,<BL_glv>  | publish [GLEVEL:SET] mesh message
// (D)->      LET ->|  @ix,val,<BL_glv>  | publish [GLEVEL:LET] mesh message
// (D)->      GET ->|  @ix,val,<BL_glv>  | publish [GLEVEL:GET] mesh message
//                  +--------------------+
//                  |      GLEVEL:       | GLEVEL output interface (gen. level)
// (U)<-      STS <-|  @ix,val,<BL_glv>  | notify GLEVEL status update
//                  +--------------------+
//
//==============================================================================

  __weak int bl_wl(BL_ob *o, int val) { return -1; }

//==============================================================================
// public module interface
//==============================================================================
//
// (H) := (BL_HW);  (W) := (BL_WL);  (D) := (BL_DOWN);  (U) := (BL_UP)
//
//                  +--------------------+
//                  |      BL_CORE       |
//                  +--------------------+
// (D)->          ->|        SYS:        | SYS (system) input interface
// (H,W)<-        <-|        SYS:        | SYS (system) output interface
//                  +--------------------+
// (D)->          ->|        LED:        | LED input interface
// (H)<-          <-|        LED:        | LED output interface
//                  +--------------------+
// (U)->          ->|       BUTTON:      | BUTTON input interface
// (H)<-          <-|       BUTTON:      | BUTTON output interface
//                  +--------------------+
// (U)<-          <-|       SWITCH:      | SWITCH interface (output only)
//                  +--------------------+
// (U)<-          <-|        MESH:       | MESH output interface (to reset node)
// (W)->          ->|        MESH:       | MESH input interface (to reset node)
//                  +--------------------+
// (D)->          ->|       RESET:       | RESET input interface (to reset node)
// (W)<-          <-|       RESET:       | RESET output interface (to reset node)
//                  +--------------------+
// (D)->          ->|        NVM:        | NVM input ifc. (non volatile memory)
// (W)<-          <-|        NVM:        | NVM output ifc. (non volatile memory)
//                  +--------------------+
// (D)->          ->|       CONFIG:      | CONFIG interface (config client)
// (W)<-          <-|       CONFIG:      | CONFIG interface (config client)
//                  +--------------------+
// (D)->          ->|       HEALTH:      | HEALTH interface (health client)
// (W)<-          <-|       HEALTH:      | HEALTH interface (health client)
//                  +--------------------+
// (D)->          ->|       GONOFF:      | GONOFF interface (generic on/off cli)
// (W)<-          <-|       GONOFF:      | GONOFF interface (generic on/off cli)
//                  +--------------------+
// (W)<-          <-|       GLEVEL:      | GLEVEL interface (generic level cli)
// (D)->          ->|       GLEVEL:      | GLEVEL interface (generic level cli)
//                  +--------------------+
// (U)<-          <-|       GLEVEL:      | GLEVEL interface (generic level srv)
// (W)->          ->|       GLEVEL:      | GLEVEL interface (generic level srv)
//                  +--------------------+
//
//==============================================================================
// - important note: all messages invoked at bl_core() are intended to go down to
//   bl_core's sub-modules !!!
// - don't post up-stream messages to bl_core, as they have to go directly to
//   bl_up()
//==============================================================================

  __weak int bl_core(BL_ob *o, int val)
  {
    static BL_oval N = NULL;           // NVM module

    switch (o->cl)
    {
      case _SYS:                       // SYSTEM interface
        if (bl_is(o,_SYS,SVC_))
          return bl_wl(o,val);         // services only to register at bl_wl

        if (bl_is(o,_SYS,INIT_))
        {
          LOG(3,BL_B "init core ..."); // init Bluccino core

	  int avail = bl_post((bl_hw),NVM_SUPPORT_0_0_0, 0,NULL,0);
	  //LOG(3,BL_C "NVM %shandeled by bl_hw",avail>=0?"":"not ");
	  if (avail >= 0)
            N = bl_hw;                 // NVM is handeled by HW core
          else
          {
            avail = bl_post((bl_wl),NVM_SUPPORT_0_0_0, 0,NULL,0);
            //LOG(3,BL_C "NVM %shandeled by bl_wl",avail>=0?"":"not ");
            N = (avail>=0) ? bl_wl:NULL; // NVM is handeled by WL core
          }
        }
        bl_hw(o,val);                  // forward to hardware core
        bl_wl(o,val);                  // forward to wireless core

        return 0;                      // OK

      case _SET:                       // SET interface
        bl_hw(o,val);                  // forward to hardware core
        bl_wl(o,val);                  // forward to wireless core
        return 0;                      // OK

      case _GET:                       // GET interface
        return bl_wl(o,val);           // forward to wireless core

      case _LED:                       // LED interface
      case _BUTTON:                    // BUTTON interface
      case _SWITCH:                    // SWITCH interface
        return bl_hw(o,val);           // forward to hardware core

      case _RESET:                     // RESET interface (reset mesh node)
        return bl_wl(o,val);           // forward to wireless core

      case _NVM:                       // NVM interface (non volatile memory)
        return bl_out(o,val,(N));      // forward to wireless or hardware core

      case _CFGCLI:                    // config client
      case _HEACLI:                    // health client
      case _GOOCLI:                    // generic on/off client
      case _GLVCLI:                    // generic level client
        return bl_wl(o,val);           // forward to wireless core

      case _BLE:                       // config client
      case _ADV:                       // health client
      case _SVC:                       // config client
        return bl_wl(o,val);           // forward to wireless core

      default:
        bl_out(o,val,bl_wl);           // output to wireless core
        bl_out(o,val,bl_hw);           // forward to hardware core
        return 0;                      // supported by default
    }
  }

//==============================================================================
// cleanup
//==============================================================================

  #include "bl_clean.h"
