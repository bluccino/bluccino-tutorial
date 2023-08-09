//==============================================================================
// advsvc.c
// setup/provide a simple advertising service
//
// Created by Hugo Pristauz on 2022-Aug-15
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_ble.h"
  #include "advsvc.h"

//==============================================================================
// PMI definition and logging shorthands
//==============================================================================

  #define PMI                      advsvc
  #define WHO                     "advsvc:"

  #define LOG                     LOG_CORE
  #define LOGO(lvl,col,o,val)     LOGO_CORE(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_CORE(lvl,col,o,val)

//==============================================================================
// define advertising & scan response data based on the Eddystone specification:
//==============================================================================

  #define SID         BL_SID(ADV_)               // Bluccino service ID
  #define CID         0xFFFF                     // test company ID

  #define UUID \
    BT_UUID_128_ENCODE(0x88000088, 0x0088, 0x0000, 0x8800, 0x880088008800)

  static BT_data ad[] =
         {
           BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
           BT_DATA_BYTES(BT_DATA_UUID128_ALL,UUID),
         };

  static BT_data sd[] =
         {
           BT_DATA(BT_DATA_NAME_COMPLETE,PROJECT,BL_LEN(PROJECT)),
         };

    // setup BLE service receipe to prepare for advertising start

  static BL_ble ble = {ap:BT_LE_ADV_NCONN_IDENTITY, sid:SID,
                       ad:ad, alen:BL_LEN(ad), sd:sd, slen:BL_LEN(sd)};

//==============================================================================
// handler: [BLE:SETUP @sid,<BL_ble>]  setup BLE service
//==============================================================================

  static int ble_setup(BL_ob *o, int val)
  {
    BL_ble *p = bl_bleadd(o,&ble);
    bl_log(4,BL_B "setup Simple Advertising BLE service");

    return (p->err = 0);               // setup completed without errors
  }

//==============================================================================
// Simple Advertising BLE service
//==============================================================================
//
// (W) := bl_wl;
//                  +--------------------+
//                  |       advsvc       | Simple Advertising BLE service setup
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)->     INIT ->|        (cb)        | module init, store cb, register svc
//                  +--------------------+
//                  |        BLE:        | BLE input interface
// (W)->    SETUP ->|   @sid,<BL_ble>    | setup Simple Advertising BLE service
//                  +--------------------+
//
//==============================================================================

  int advsvc(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,SID,NULL};  // library registering data block
    static BL_oval W = NULL;             // wireless core

    if (bl_is(o,_SYS,INIT_))
    {
      bl_log(3,BL_B"init advsvc");
      W = bl_cb(o,W,WHO"(W)");           // store address of wireless core
      return bl_lib(W,&lib);             // register service @ wireless core
    }
    else if (bl_ix(o) != SID)            // proper service ID provided?
      return -1;

      // messages are only dispatched for proper serviced ID

    switch (bl_id(o))
    {
      case BL_ID(_BLE,SETUP_):           // [BLE:SETUP @sid,<BL_ble>]
        if (bl_ix(o) == SID)
          ble_setup(o,val);              // setup BLE service
        return 0;

      default:
        return 1;                        // error: do not handle message
    }
  }
