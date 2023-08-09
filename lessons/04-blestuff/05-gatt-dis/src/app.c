//==============================================================================
// app.c
// app code for 06-gatt-bas peripheral
//
// Created by Hugo Pristauz on 2022-Aug-23
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "app.h"
  #include "bl_ble.h"
  #include "bl_bas.h"                  // battery service

//==============================================================================
// BLE service ID and PMI definition, logging shorthands
//==============================================================================

  #define WHO                     "app:"

  #define LOG                     LOG_APP
  #define LOGO(lvl,col,o,val)     LOGO_APP(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_APP(lvl,col,o,val)

  #define PMI                     app  // public module interface
  BL_PMI(PMI)                          // define static _bl_pmi() function

//==============================================================================
// app
//==============================================================================
// M = main;  U = bl_up;  D = bl_down;
//
//                  +--------------------+
//                  |        app         | application top module
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     INIT ->|        (to)        | init module, store (to) callback
// (M)->     TOCK ->|  @ix,<BL_pace>,cnt | tock module
//                  +--------------------+
//                  |        BLE:        | BLE input interface
// (U)->  CONNECT ->|      <BL_conn>     | notify connection event
// (U)->   DISCON ->|  <BL_conn>,reason  | notify disconnection event
// (U)->      MTU ->|       @rx,tx       | notify MTU update
//                  |....................| BLE output interface
// (D)<-   ENABLE <-|      <BL_ble>      | enable Bluetooth
// (D)<-    START <-|      <BL_ble>      | start advertising BLE service
//                  +--------------------+
//
//==============================================================================

  int app(BL_ob *o, int val)
  {
    static BL_ble ble;

    static BL_oval D = bl_down;

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        _bl_pmi(_BLE,ENABLE_, 0,&ble,0); // enable Bluetooth
        _bl_pmi(_BLE,START_, 0,&ble,0);
        return ble.err;

      case BL_ID(_SYS,TOCK_):
        if (bl_period(o,10000))
          LOG(1,"I'm alive!");
        return 0;

      case BL_ID(_BLE,CONNECT_):       // [BLE:CONNECT <BL_conn>]
      case BL_ID(_BLE,DISCON_):        // [BLE:DISCON  <BL_conn>,reason]
        bl_color(val?BL_0:BL_C);       // set log header color
        return 0;

      case BL_ID(_BLE,MTU_):           // [BLE:MTU  @rx,tx]
        LOGO(1,BL_M"::",o,val);
	      return 0;

      case _BL_ID(_BLE,ENABLE_):       // read my own Bluetooth MAC address
      case _BL_ID(_BLE,START_):        // start BLE advertising
        return bl_out(o,val,D);

      default:
        return BL_VOID;
    }
  }
