//==============================================================================
// bl_ibeacon.c
// setup/provide iBeacon service
//
// Created by Hugo Pristauz on 2022-Aug-07
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_wl.h"
  #include "bl_ble.h"
  #include "bl_ibeacon.h"

//==============================================================================
// PMI definition and logging shorthands
//==============================================================================

  #define PMI                      bl_ibeacon
  #define WHO                     "bl_ibeacon:"

  #define LOG                     LOG_SVC
  #define LOGO(lvl,col,o,val)     LOGO_SVC(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_SVC(lvl,col,o,val)

//==============================================================================
// define advertising & scan response data based on the iBeacon specification:
//==============================================================================

  #define SID       BL_ID(_SVC,IBEACON_)   // Bluccino service ID

  #ifndef IBEACON_RSSI
    #define IBEACON_RSSI 0xc8
  #endif

  static char dev[] = PROJECT;      // device name

    // Set iBeacon demo advertisement data. These values are for
    // demonstration only and must be changed for production environments!
    //
    // UUID:  18ee1516-016b-4bec-ad96-bcb96d166e97
    // Major: 0
    // Minor: 0
    // RSSI:  -56 dBm

  static BL_packet flags = BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR);
  static BL_packet manu  = BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA,
                           0x4c, 0x00,                          // Apple
                           0x02, 0x15,                          // iBeacon
                           0x18, 0xee, 0x15, 0x16,              // UUID[15..12]
                           0x01, 0x6b,                          // UUID[11..10]
                           0x4b, 0xec,                          // UUID[9..8]
                           0xad, 0x96,                          // UUID[7..6]
                           0xbc, 0xb9, 0x6d, 0x16, 0x6e, 0x97,  // UUID[5..0]
                           0x00, 0x00,                          // Major L/H
                           0x00, 0x00,                          // Minor L/H
                           IBEACON_RSSI);              // Calibrated RSSI @ 1m

    // setup scan response chunks for iBeacon

  static BL_packet rsp = BT_DATA(BT_DATA_NAME_COMPLETE,dev,BL_LEN(dev));

    // build-up advertising data and scan response data from data lumps

  static BT_data ad[] = {flags,manu};
  static BT_data sd[] = {rsp};

    // setup BLE service data to prepare for advertising start

  static BL_ble ble = {ap:BT_LE_ADV_NCONN_IDENTITY, sid:SID,
                       ad:ad, alen:BL_LEN(ad), sd:sd, slen:BL_LEN(sd)};

//==============================================================================
// handler: setup/return iBeacon service
//==============================================================================

  static int ble_setup(BL_ob *o, int val)
  {
    BL_ble *p = bl_data(o);

    if (p == 0)
    {
      bl_err(-1,"bl_ibeacon: no <BL_ble> data provided");
      return (p->err = -1);            // error: no <BL_ble> data provided
    }

    LOG(4,BL_B "setup iBeacon BLE service");
    memcpy(p,&ble,sizeof(BL_ble));

    return (p->err = 0);               // setup completed without errors
  }

//==============================================================================
// iBeacon BLE service
//==============================================================================
//
// (W) := bl_wl;
//                  +--------------------+
//                  |     bl_ibeacon     | iBeacon BLE service setup
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)->     INIT ->|        (cb)        | init module, store (cb) callback
//                  +--------------------+
//                  |        BLE:        | BLE input interface
// (W)->    SETUP ->|      <BL_ble>      | setup iBeacon BLE service
//                  +--------------------+
//
//==============================================================================

  int bl_ibeacon(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,SID,NULL};
    static BL_oval W = bl_wl;            // wireless core

    if (bl_is(o,_SYS,INIT_))
    {
        LOG(3,BL_B"init bl_ibeacon");
        W = bl_cb(o,W,WHO"(W)");         // store output callback
        return bl_lib(W,&lib);           // register service @ parent
    }
    else if (bl_ix(o) != SID)            // proper service ID provided?
      return BL_VOID;

      // from here execution is granted only for matching service ID

    switch (bl_id(o))
    {
      case BL_ID(_BLE,SETUP_):           // setup iBeacon BLE service
        return ble_setup(o,val);

      default:
        return 1;                        // error: cannot provide
    }
  }
