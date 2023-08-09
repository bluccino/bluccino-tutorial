//==============================================================================
// nandsvc.c
// BLE NAND service
//
// Created by Hugo Pristauz on 2022-Aug-23
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_ble.h"
  #include "nandsvc.h"

//==============================================================================
// definition of PMI and logging shorthands
//==============================================================================

  #define PMI  nandsvc
  #define WHO "nandsvc:"

  #define LOG                     LOG_SVC
  #define LOGO(lvl,col,o,val)     LOGO_SVC(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_SVC(lvl,col,o,val)

//==============================================================================
// BAS module state
//==============================================================================

  static TP_nand state = { in1:0, in2:0, out:1 };

//==============================================================================
// callback: input 1 read
// - is called if central requests to read characteristics
//==============================================================================

  static ssize_t read_in1(BL_conn *conn, BL_attr *attr, void *buf,
                          BL_u16 len, BL_u16 offset)
  {
	  BL_u8 val = state.in1;

	  return bt_gatt_attr_read(conn, attr, buf, len, offset, &val, sizeof(val));
  }

//==============================================================================
// callback: input 2 read
// - is called if central requests to read characteristics
//==============================================================================

  static ssize_t read_in2(BL_conn *conn, BL_attr *attr, void *buf,
                          BL_u16 len, BL_u16 offset)
  {
	  BL_u8 val = state.in2;

	  return bt_gatt_attr_read(conn, attr, buf, len, offset, &val, sizeof(val));
  }

//==============================================================================
// callback: output read
// - is called if central requests to read characteristics
//==============================================================================

  static ssize_t read_out(BL_conn *conn, BL_attr *attr, void *buf,
                          BL_u16 len, BL_u16 offset)
  {
	  BL_u8 val = state.out;
	  return bt_gatt_attr_read(conn, attr, buf, len, offset, &val, sizeof(val));
  }

//==============================================================================
// callback:
//==============================================================================

  static void ccc_change(BL_attr *attr, BL_u16 value)
  {
	  ARG_UNUSED(attr);

	  bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);
	  LOG(3,"NAND Notifications %s", notif_enabled ? "enabled" : "disabled");
  }

//==============================================================================
// service: define GATT battery service
//==============================================================================

  typedef struct bt_uuid_128 BL_uuid128;

  #define UUID128(id) \
          BT_UUID_128_ENCODE(0x88000088, 0x0088, id, 0x8800, 0x880088008800)

  #define UUID_NAND        UUID128(0x0A00)
  #define UUID_IN1         UUID128(0x0A10)
  #define UUID_IN2         UUID128(0x0A20)
  #define UUID_OUT         UUID128(0x0A30)

    // define UUIDs for GATT service and GATT characteristics

  static BL_uuid128 uuid_nand = BT_UUID_INIT_128(UUID_NAND);  // service UUID
  static BL_uuid128 uuid_in1  = BT_UUID_INIT_128(UUID_IN1);   // service UUID
  static BL_uuid128 uuid_in2  = BT_UUID_INIT_128(UUID_IN2);   // service UUID
  static BL_uuid128 uuid_out  = BT_UUID_INIT_128(UUID_OUT);   // service UUID

    // define GATT service

  BT_GATT_SERVICE_DEFINE(nand,                     // define NAND service
    BT_GATT_PRIMARY_SERVICE(&uuid_nand),           // BLE service UUID (predef.)

      // 1st characteristics: input 1

    BT_GATT_CHARACTERISTIC(                       // 1st characteristic
      &uuid_in1.uuid,                             // characteristic UUID
      BL_CHRC_RN,                                 // read/notify properties
      BL_PERM_R,                                  // read permissions
      read_in1,                                   // read callback
      NULL,                                       // write callback (not set)
      &state.in1),                                // data reference
/*
    BT_GATT_CCC(ccc_change,                       // client chrc. config change
      BL_PERM_RW),                                // read/write permissions
*/
      // 2nd characteristics: input 2

    BT_GATT_CHARACTERISTIC(                       // 2nd characteristic
      &uuid_in2.uuid,                             // characteristic UUID
      BL_CHRC_RN,                                 // read/notify properties
      BL_PERM_R,                                  // read permissions
      read_in2,                                   // read callback
      NULL,                                       // write callback (not set)
      &state.in2),                                // data reference
/*
    BT_GATT_CCC(ccc_change,                       // client chrc. config change
      BL_PERM_RW),                                // read/write permissions
*/
      // 3rd characteristics: output

    BT_GATT_CHARACTERISTIC(                       // 3rd characteristic
      &uuid_out.uuid,                             // characteristic UUID
      BL_CHRC_RN,                                 // read/notify properties
      BL_PERM_R,                                  // read permissions
      read_out,                                   // read callback
      NULL,                                       // write callback (not set)
      &state.out),                                // data reference

      // 4th characteristics: ccc

    BT_GATT_CCC(ccc_change,                       // client chrc. config change
      BL_PERM_RW),                                // read/write permissions

  );

//==============================================================================
// receipe definition
// - set up advertising data as follows:
//   1) advertising flags: general advertising, no BR/EDR support
//   2) battery service (BAS - 16 bit UUID)
//==============================================================================

  #define SID BL_SID(NAND_)                       // Bluccino service ID

	  // build-up advertising data and scan response data from data packets

  static BT_data ad[] =
  {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, UUID_NAND),
  };
    // composition of BLE receipe

  static BL_ble ble = {ap:BT_LE_ADV_CONN_NAME, sid:SID,
                       ad:ad, alen:BL_LEN(ad)};

//==============================================================================
// handler: [BLE:SETUP @sid,<BL_ble>]  setup BLE service
//==============================================================================

  static int ble_setup(BL_ob *o, int val)
  {
    BL_ble *p = bl_blecopy(o,&ble);
    bl_log(4,BL_B "setup NAND BLE service");

    return (p->err = 0);               // setup completed without errors
  }

//==============================================================================
// handler: [SVC:GET @sid,<BL_bas>] get NAND data
//==============================================================================

  static int svc_get(BL_ob *o, int val)
  {
    TP_nand *p = bl_data(o);
    *p = state;

    LOG(4,"get: @<%s|%s>, %d & %d = %d ", BL_IDTXT(bl_ix(o)),
          state.in1, state.in2, state.out);
    return 0;
  }

//==============================================================================
// handler: [SVC:SET @sid,<BL_bas>] set NAND data
//==============================================================================

  static int svc_set(BL_ob *o, int val)
  {
    TP_nand *p = bl_data(o);

    int rc;

    state = *p;
    LOG(4,BL_M"set: @<%s|%s>, !(%d & %d) = %d (GATT notify)",
          BL_IDTXT(bl_ix(o)), state.in1, state.in2, state.out);

    rc = bt_gatt_notify(NULL, &nand.attrs[1], &state.in1, sizeof(state.in1));
    rc = bt_gatt_notify(NULL, &nand.attrs[2], &state.in2, sizeof(state.in2));
    rc = bt_gatt_notify(NULL, &nand.attrs[3], &state.out, sizeof(state.out));

    return rc == -ENOTCONN ? 0 : rc;
  }

//==============================================================================
// public module interface (PMI): Battery Service (simple BLE service)
//==============================================================================
//
// (W) := bl_wl;
//                  +--------------------+
//                  |       nandsvc       | BLE battery service
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)->     INIT ->|        (cb)        | module init, store cb, register svc
//                  +--------------------+
//                  |        BLE:        | BLE input interface
// (W)->    SETUP ->|    @sid,<BL_ble>   | setup Simple Advertising BLE service
//                  +--------------------+
//
//==============================================================================

  int nandsvc(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,SID,NULL};
    static BL_oval W = NULL;           // wireless core

    if (bl_is(o,_SYS,INIT_))
    {
        LOG(3,BL_B"init nandsvc");
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

      case BL_ID(_SVC,GET_):           // [SVC:GET @sid,<BL_bas>]
        return svc_get(o,val);         // delegate to svc_get() handler

      case BL_ID(_SVC,SET_):           // [SVC:SET @sid,<BL_bas>]
        return svc_set(o,val);         // delegate to svc_set() handler

       default:
        return -1;
    }
  }
