//==============================================================================
// bl_bas.c
// battery BLE service setup
//
// Created by Hugo Pristauz on 2022-Aug-11
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include <errno.h>
  #include <zephyr/init.h>
  #include <zephyr/sys/__assert.h>
  #include <stdbool.h>

  #include "bluccino.h"
  #include "bl_ble.h"
  #include "bl_bas.h"

//==============================================================================
// definition of PMI and logging shorthands
//==============================================================================

  #define PMI  bl_bas
  #define WHO "bl_bas:"

  #define LOG                     LOG_SVC
  #define LOGO(lvl,col,o,val)     LOGO_SVC(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_SVC(lvl,col,o,val)

//==============================================================================
// BAS module state
//==============================================================================

  static BL_bas battery = { percent:100U };

//==============================================================================
// callback: battery level read
// - is called if central requests to read characteristics
//==============================================================================

  static ssize_t batlvl_read(struct bt_conn *conn,
               BL_attr *attr, void *buf, uint16_t len, uint16_t offset)
  {
	  uint8_t lvl8 = battery.percent;

	  return bt_gatt_attr_read(conn, attr, buf, len, offset, &lvl8, sizeof(lvl8));
  }

//==============================================================================
// callback:
//==============================================================================

  static void batlvl_ccc_change(const struct bt_gatt_attr *attr,
	            uint16_t value)
  {
	  ARG_UNUSED(attr);

	  bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);
	  LOG(3,"BAS Notifications %s", notif_enabled ? "enabled" : "disabled");
  }

//==============================================================================
// service: define GATT battery service
//==============================================================================

  BT_GATT_SERVICE_DEFINE(bas,                     // define battery service
    BT_GATT_PRIMARY_SERVICE(BT_UUID_BAS),         // BLE service UUID (predef.)

      // 1st characteristics

    BT_GATT_CHARACTERISTIC(                       // first characteristic
      BT_UUID_BAS_BATTERY_LEVEL,                  // characteristic UUID
      BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,    // read/notify properties
      BT_GATT_PERM_READ,                          // read permissions
      batlvl_read,                                // read callback
      NULL,                                       // write callback (not set)
      &battery.percent),                          // data reference

      // 2nd characteristics

    BT_GATT_CCC(batlvl_ccc_change,                // client chrc. config change
      BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),    // read/write permissions
  );

//==============================================================================
// receipe definition
// - set up advertising data as follows:
//   1) advertising flags: general advertising, no BR/EDR support
//   2) battery service (BAS - 16 bit UUID)
//==============================================================================

  #define SID BL_SID(BAS_)                       // Bluccino service ID

	  // build-up advertising data and scan response data from data packets

  static BT_data ad[] =
  {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
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
    bl_log(4,BL_B "setup BLE Battery Service");

    return (p->err = 0);               // setup completed without errors
  }

//==============================================================================
// handler: [SVC:GET @sid,<BL_bas>] get battery service data
//==============================================================================

  static int svc_get(BL_ob *o, int val)
  {
    BL_bas *p = bl_data(o);
    LOG(4,"get: @<%s|%s>,%d", BL_IDTXT(bl_ix(o)), battery.percent);
	  return p->percent = battery.percent;
  }

//==============================================================================
// handler: [SVC:SET @sid,<BL_bas>] set battery service data
//==============================================================================

  static int svc_set(BL_ob *o, int val)
  {
    BL_bas *p = bl_data(o);
    int rc;

    if (p->percent > 100U)
	    return -EINVAL;

    battery.percent = p->percent;
    LOG(4,BL_M"set: @<%s|%s>,%d (GATT notify: 0x%02X)",
              BL_IDTXT(bl_ix(o)), battery.percent, battery.percent);

    rc = bt_gatt_notify(NULL, &bas.attrs[1],
                        &battery.percent, sizeof(battery.percent));

    return rc == -ENOTCONN ? 0 : rc;
  }

//==============================================================================
// system init (not sure whether we need this stuff)
//==============================================================================
/*
  static int bas_init(const struct device *dev)
  {
	  ARG_UNUSED(dev);
	  return 0;
  }

  SYS_INIT(bas_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
*/
//==============================================================================
// public module interface (PMI): Battery Service (simple BLE service)
//==============================================================================
//
// (W) := bl_wl;
//                  +--------------------+
//                  |       bl_bas       | BLE battery service
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)->     INIT ->|        (cb)        | module init, store cb, register svc
//                  +--------------------+
//                  |        SVC:        | SVC input interface
// (W)->      GET ->|    @sid,<BL_bas>   | get BAS service data
// (W)->      SET ->|    @sid,<BL_bas>   | set BAS service data (for simulation)
// (W)->    SETUP ->|    @sid,<BL_ble>   | setup Simple Advertising BLE service
//                  +--------------------+
//
//==============================================================================

  int bl_bas(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,SID,NULL};
    static BL_oval W = NULL;           // wireless core

    if (bl_is(o,_SYS,INIT_))
    {
        LOG(3,BL_B"init bl_bas");
        W = bl_cb(o,W,WHO"(W)");
        return bl_lib(W,&lib);         // register service @ parent
    }
    else if (bl_ix(o) != SID)          // proper service ID provided?
      return BL_VOID;

      // messages are only dispatched for proper serviced ID

    switch (bl_id(o))
    {
      case BL_ID(_SVC,GET_):           // [SVC:GET @sid,<BL_bas>]
        return svc_get(o,val);         // delegate to svc_get() handler

      case BL_ID(_SVC,SET_):           // [SVC:SET @sid,<BL_bas>]
        return svc_set(o,val);         // delegate to svc_set() handler

      case BL_ID(_BLE,SETUP_):         // [BLE:SETUP @sid,<BL_ble>]
        return ble_setup(o,val);      // setup BLE service

      default:
        return -1;
    }
  }
