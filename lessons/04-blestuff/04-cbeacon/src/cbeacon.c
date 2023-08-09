//==============================================================================
// cbeacon.c
// setup/provide custom cBeacon service
//
// Created by Hugo Pristauz on 2022-Aug-14
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_ble.h"
  #include "cbeacon.h"

//==============================================================================
// PMI definition and logging shorthands
//==============================================================================

  #define PMI                      cbeacon
  #define WHO                     "cbeacon:"

  #define LOG                     LOG_CORE
  #define LOGO(lvl,col,o,val)     LOGO_CORE(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_CORE(lvl,col,o,val)

//==============================================================================
// define advertising & scan response data based on the Eddystone specification:
// - https://github.com/google/eddystone/blob/master/protocol-specification.md
// - https://github.com/google/eddystone/tree/master/eddystone-url
//
// nRF Connect app scanner will display:
//   04-edi-beacon
//   Services: Eddystone
//   Data: http://www.bluccino.org
//   Frame Type: 0x10 (URL)
//   Service Data: Eddystone
//   0x08-2E-6F-6E-69-63-61-6C-62-00-00-10
//   Tx Power: 0 dBm
//==============================================================================

  #define EDDY_CID      0xFEAA         // Eddystone company identifier
  #define BNX_CID       0x07C6         // Bluenetics company identifier
  #define TEST_CID      0xFFFF         // Test company identifier

//==============================================================================

  #define SID         BL_ID(_SVC,CBEACON_)   // Bluccino service ID
  #define CID         BNX_CID
  #define UUID        0xFFFE                 // custom service UUID

  #define MAJOR       5
  #define MINOR       0x8888
  #define BFLAGS      0x0000

  static AD_cbeacon ad_manu =
         {
           uuid_lo:  BL_LB(CID),
           uuid_hi:  BL_HB(CID),
           major_lo: BL_LB(MAJOR),
           major_hi: BL_HB(MAJOR),
           minor_lo: BL_LB(MINOR),
           minor_hi: BL_HB(MINOR),
           nhops:    0,
           rssi0:    -60,
           bflags_lo: BL_LB(BFLAGS),
           bflags_hi: BL_HB(BFLAGS),
         };

    // build-up advertising data and scan response data from data packets

  static BT_data ad[] =                // advertising data
         {
           BT_DATA_BYTES(BT_DATA_FLAGS,BT_LE_AD_NO_BREDR),
           BT_DATA_BYTES(BT_DATA_UUID16_ALL,BL_LB(UUID),BL_HB(UUID)),
           BT_DATA(BT_DATA_MANUFACTURER_DATA, &ad_manu, sizeof(ad_manu)),
         };

  static BT_data sd[] =                // scan response data
         {
           BT_DATA(BT_DATA_NAME_COMPLETE,PROJECT,BL_LEN(PROJECT)),
         };

    // setup BLE service receipe to prepare for advertising start

  static BL_ble ble = {ap:BT_LE_ADV_NCONN_IDENTITY, sid:SID,
                       ad:ad, alen:BL_LEN(ad), sd:sd, slen:BL_LEN(sd)};

//==============================================================================
// handler: [BLE:SETUP sid,<BL_ble>] setup/return Eddystone service
//==============================================================================

  static int ble_setup(BL_ob *o, int val)
  {
    BL_ble *p = bl_blecopy(o,&ble);
    LOG(4,BL_B "setup (custom) cBeacon BLE service");

    return (p->err = 0);               // setup completed without errors
  }

//==============================================================================
// Eddystone beacon BLE service
//==============================================================================
//
// (W) := bl_wl;
//                  +--------------------+
//                  |      cbeacon       | Eddystone beacon BLE service setup
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)->     INIT ->|        (cb)        | module init, store cb, register svc
//                  +--------------------+
//                  |      SERVICE:      | SERVICE input interface
// (W)->  CBEACON ->|      <BL_ble>      | setup Eddystone beacon BLE service
//                  +--------------------+
//
//==============================================================================

  int cbeacon(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,SID,NULL};
    static BL_oval W = NULL;             // wireless core

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        LOG(3,BL_B"init cbeacon");
        W = bl_cb(o,W,WHO"(W)");
        return bl_lib(W,&lib);           // register service @ parent

      case BL_ID(_BLE,SETUP_):           // setup Eddystone BLE service
        return ble_setup(o,val);

      default:
        return 1;                        // error: cannot provide
    }
  }
