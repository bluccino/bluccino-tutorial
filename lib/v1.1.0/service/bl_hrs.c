//==============================================================================
// bl_hrs.c
// heart rate BLE service setup
//
// Created by Hugo Pristauz on 2022-Aug-11
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include <init.h>

  #include "bluccino.h"
  #include "bl_ble.h"
  #include "bl_hrs.h"

//==============================================================================
// definition of PMI and logging shorthands
//==============================================================================

  #define PMI  bl_hrs
  #define WHO "bl_hrs:"

  #define LOG                     LOG_SVC
  #define LOGO(lvl,col,o,val)     LOGO_SVC(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_SVC(lvl,col,o,val)

//==============================================================================
// config defaults
//==============================================================================

  #ifndef CONFIG_BT_HRS_DEFAULT_PERM_RW_AUTHEN
    #define CONFIG_BT_HRS_DEFAULT_PERM_RW_AUTHEN 0
  #endif
  #ifndef CONFIG_BT_HRS_DEFAULT_PERM_RW_ENCRYPT
    #define CONFIG_BT_HRS_DEFAULT_PERM_RW_ENCRYPT 0
  #endif
  #ifndef CONFIG_BT_HRS_DEFAULT_PERM_RW
    #define CONFIG_BT_HRS_DEFAULT_PERM_RW 0
  #endif

//==============================================================================
// definition og GATT characteristics
//==============================================================================

  #define GATT_PERM_READ_MASK     (BT_GATT_PERM_READ | \
	         BT_GATT_PERM_READ_ENCRYPT | \
				   BT_GATT_PERM_READ_AUTHEN)

  #define GATT_PERM_WRITE_MASK    (BT_GATT_PERM_WRITE | \
				   BT_GATT_PERM_WRITE_ENCRYPT | \
				   BT_GATT_PERM_WRITE_AUTHEN)

  #define HRS_GATT_PERM_DEFAULT (						\
	        CONFIG_BT_HRS_DEFAULT_PERM_RW_AUTHEN ?				\
	          (BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN) :	\
	        CONFIG_BT_HRS_DEFAULT_PERM_RW_ENCRYPT ?				\
	          (BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT) :	\
	          (BT_GATT_PERM_READ | BT_GATT_PERM_WRITE))			\

//==============================================================================
// HRS callbacks
//==============================================================================

  static uint8_t hrs_blsc;

  static void hrmc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
  {
    ARG_UNUSED(attr);
    bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

    LOG(3,BL_M"HRS notifications %s", notif_enabled ? "enabled" : "disabled");
  }

  static ssize_t read_blsc(struct bt_conn *conn, const struct bt_gatt_attr *attr,
	  		   void *buf, uint16_t len, uint16_t offset)
  {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &hrs_blsc,
                             sizeof(hrs_blsc));
  }

//==============================================================================
// Heart Rate Service Declaration
//==============================================================================

  #define SID       BL_ID(_SVC,HRS_)   // Bluccino service ID

  BT_GATT_SERVICE_DEFINE(hrs_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_HRS),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_MEASUREMENT, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(hrmc_ccc_cfg_changed,
		    HRS_GATT_PERM_DEFAULT),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_BODY_SENSOR, BT_GATT_CHRC_READ,
			       HRS_GATT_PERM_DEFAULT & GATT_PERM_READ_MASK,
			       read_blsc, NULL, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_CONTROL_POINT, BT_GATT_CHRC_WRITE,
			       HRS_GATT_PERM_DEFAULT & GATT_PERM_WRITE_MASK,
			       NULL, NULL, NULL),
  );

//==============================================================================
// init HRS
//==============================================================================

  static int hrs_init(const struct device *dev)
  {
    ARG_UNUSED(dev);
    hrs_blsc = 0x01;
    return 0;
  }

//==============================================================================
// notify HRS
//==============================================================================

  int bt_hrs_notify(uint16_t heartrate)
  {
    int rc;
    static uint8_t hrm[2];

    hrm[0] = 0x06; /* uint8, sensor contact */
    hrm[1] = heartrate;

    rc = bt_gatt_notify(NULL, &hrs_svc.attrs[1], &hrm, sizeof(hrm));

    return rc == -ENOTCONN ? 0 : rc;
  }

//==============================================================================
// system initialize
//==============================================================================

  SYS_INIT(hrs_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

//==============================================================================
// public module interface (PMI)
//==============================================================================

  int bl_hrs(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,SID,NULL};
    static BL_oval W = NULL;           // wireless core

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        LOG(3,BL_B"init bl_hrs");
        W = bl_cb(o,W,WHO"(W)");
        return bl_lib(W,&lib);         // register service @ parent

      default:
        return -1;
    }
  }
