//==============================================================================
// bl_dis.c
// BLE device information service setup
//
// Created by Hugo Pristauz on 2022-Aug-23
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_ble.h"
  #include "bl_dis.h"

//==============================================================================
// definition of PMI and logging shorthands
//==============================================================================

  #define PMI  bl_dis
  #define WHO "bl_dis:"

  #define LOG                     LOG_SVC
  #define LOGO(lvl,col,o,val)     LOGO_SVC(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_SVC(lvl,col,o,val)

//==============================================================================
// receipe definition
// - set up advertising data as follows:
//   1) advertising flags: general advertising, no BR/EDR support
//   2) device information service (DIS - 16 bit UUID)
//==============================================================================

  #define SID  BL_SID(DIS_)                      // Bluccino service ID

	  // build-up advertising data and scan response data from data packets

  static BT_data ad[] =
  {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_DIS_VAL)),
  };

    // composition of BLE receipe

  static BL_ble ble = {ap:BT_LE_ADV_CONN_NAME_AD, sid:SID,
                       ad:ad, alen:BL_LEN(ad)};

//==============================================================================
// handler: [BLE:SETUP @sid,<BL_ble>]  setup BLE service
//==============================================================================

  static int ble_setup(BL_ob *o, int val)
  {
    BL_ble *p = bl_blecopy(o,&ble);
    bl_log(4,BL_B "setup BLE Device Information Service");

    return (p->err = 0);               // setup completed without errors
  }

//==============================================================================
// public module interface (PMI): Device Information Service (DIS)
//==============================================================================
//
// (W) := bl_wl;
//                  +--------------------+
//                  |       bl_dis       | BLE battery service
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)->     INIT ->|        (cb)        | module init, store cb, register svc
//                  +--------------------+
//                  |        SVC:        | SVC input interface
// (W)->    SETUP ->|    @sid,<BL_ble>   | setup Simple Advertising BLE service
//                  +--------------------+
//
//==============================================================================

  int bl_dis(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,SID,NULL};
    static BL_oval W = NULL;           // wireless core

    if (bl_is(o,_SYS,INIT_))
    {
        LOG(3,BL_B"init bl_dis");
        W = bl_cb(o,W,WHO"(W)");
        return bl_lib(W,&lib);         // register service @ parent
    }
    else if (bl_ix(o) != SID)          // proper service ID provided?
      return -1;

      // messages are only dispatched for proper serviced ID

    switch (bl_id(o))
    {
      case BL_ID(_BLE,SETUP_):         // [BLE:SETUP @sid,<BL_ble>]
        return ble_setup(o,val);       // setup BLE service

      default:
        return -1;
    }
  }
