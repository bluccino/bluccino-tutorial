//==============================================================================
// bl_eddy.c
// setup/provide Eddystone beacon service
//
// Created by Hugo Pristauz on 2022-Aug-07
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_ble.h"
  #include "bl_eddy.h"

//==============================================================================
// PMI definition and logging shorthands
//==============================================================================

  #define PMI                      bl_eddy
  #define WHO                     "bl_eddy:"

  #define LOG                     LOG_SVC
  #define LOGO(lvl,col,o,val)     LOGO_SVC(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_SVC(lvl,col,o,val)

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
//
// Here is the alternative how to setup the svc packet:
//
//   static BL_packet svc   = BT_DATA_BYTES(BT_DATA_SVC_DATA16,
//                            0xaa, 0xfe, // Eddystone UUID (low,high byte)
//                            0x10,       // Eddystone-URL frame type
//                            -40,        // Calibrated Tx power at 0m
//                            0x00,       // URL Scheme Prefix http://www.
//                            'b','l','u','e','n','e','t','i','c','s',
//                            7);        // .biz(5), .gov(6), .com(7), .org(8)
//
//==============================================================================

  #define LB_HB(cid)  BL_LB(CID),BL_HB(CID)      // a comma separated pair

  #define SID         BL_SID(EDDY_)    // Bluccino service ID
  #define CID         0xFEAA           // Eddystone company ID
  #define UIID        CID              // service UUID, same as CID

    // type definition for Eddystone beacon advertising service data

  typedef __struct AD_svc              // Eddystone service data
          {
            BL_u8   uuid_lo;           // Eddystone UUID low byte
            BL_u8   uuid_hi;           // Eddystone UUID high byte
            BL_u8   uft;               // Eddystone URL frame type
            BL_s8   rssi0;             // calibrated TX power at 0m
            BL_u8   prefix;            // URL scheme prefix http://www.
            BL_u8   url[9];            // URL
            BL_u8   suffix;            // .biz(5), .gov(6), .com(7), .org(8)
          } AD_svc;

    // setup actual Eddystone beacon data

  static AD_svc ad_svc =               // Eddystone service data setup
          {
            uuid_lo: BL_LB(CID),       // Eddystone UUID low byte
            uuid_hi: BL_HB(CID),       // Eddystone UUID high byte
            uft: 0x10,                 // URL frame type
            rssi0: -40,                // calibrated TX power at 0m
            prefix: 0,                 // URL scheme prefix
            url: {'b','l','u','c','c','i','n','o','.'},
            suffix: 8,                 // .org
          };

    // setup advertising packets for Eddystone beacon

  static BL_packet flags = BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR);
  static BL_packet uuid  = BT_DATA_BYTES(BT_DATA_UUID16_ALL, LB_HB(UUID));
  static BL_packet svc   = BT_DATA(BT_DATA_SVC_DATA16, &ad_svc, sizeof(ad_svc));

   // setup scan response packet for Eddystone beacon

  static BL_packet rsp = BT_DATA(BT_DATA_NAME_COMPLETE,PROJECT,BL_LEN(PROJECT));

    // build-up advertising data and scan response data from data packets

  static BT_data ad[] = {flags,uuid,svc};
  static BT_data sd[] = {rsp};

    // setup BLE service receipe data to prepare for advertising start

  static BL_ble ble = {ap:BT_LE_ADV_NCONN_IDENTITY, sid:SID,
                       ad:ad, alen:BL_LEN(ad), sd:sd, slen:BL_LEN(sd)};

//==============================================================================
// handler: [BLE:SETUP @sid,<BL_ble>]  setup BLE service
//==============================================================================

  static int ble_setup(BL_ob *o, int val)
  {
    BL_ble *p = bl_blecopy(o,&ble);
    LOG(4,BL_B "setup Eddystone BLE service");

    return (p->err = 0);               // setup completed without errors
  }

//==============================================================================
// Eddystone beacon BLE service
//==============================================================================
//
// (W) := bl_wl;
//                  +--------------------+
//                  |      bl_eddy       | Eddystone beacon BLE service setup
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)->     INIT ->|        (cb)        | module init, store cb, register svc
//                  +--------------------+
//                  |        BLE:        | BLE input interface
// (W)->    SETUP ->|    @sid,<BL_ble>   | setup Eddystone beacon BLE service
//                  +--------------------+
//
//==============================================================================

  int bl_eddy(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,SID,NULL};
    static BL_oval W = NULL;             // wireless core

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        LOG(3,BL_B"init bl_eddy");
        W = bl_cb(o,W,WHO"(W)");
        return bl_lib(W,&lib);           // register service @ parent

      case BL_ID(_BLE,SETUP_):           // [BLE:SETUP @sid,<BL_ble>]
        if (bl_ix(o) == SID)
           return ble_setup(o,val);      // setup BLE service
        return BL_VOID;

      default:
        return 1;                        // error: cannot provide
    }
  }
