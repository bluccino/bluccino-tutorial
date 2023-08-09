//==============================================================================
// bl_cts.c
// current time BLE service setup
//
// Created by Hugo Pristauz on 2022-Aug-09
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

#include <errno.h>
#include <sys/byteorder.h>

  #include "bluccino.h"
  #include "bl_ble.h"
  #include "bl_cts.h"

//==============================================================================
// definition of PMI and logging shorthands
//==============================================================================

  #define PMI  bl_cts
  #define WHO "bl_cts:"

  #define LOG                     LOG_SVC
  #define LOGO(lvl,col,o,val)     LOGO_SVC(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_SVC(lvl,col,o,val)

//==============================================================================
// BAS module state
//==============================================================================

  static uint8_t ct[10];
  static uint8_t ct_update;

//==============================================================================
// callbacks
//==============================================================================

	static void ct_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
	{
		/* TODO: Handle value */
	}

	static ssize_t read_ct(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			       void *buf, uint16_t len, uint16_t offset)
	{
		const char *value = attr->user_data;

		return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
					 sizeof(ct));
	}

	static ssize_t write_ct(struct bt_conn *conn, const struct bt_gatt_attr *attr,
				const void *buf, uint16_t len, uint16_t offset,	uint8_t flags)
	{
		uint8_t *value = attr->user_data;

		if (offset + len > sizeof(ct))
			return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

		memcpy(value + offset, buf, len);
		ct_update = 1U;

		return len;
	}

//==============================================================================
// define GATT service
//==============================================================================

  #define SID       BL_ID(_CTS,SVC_)            // Bluccino service ID

  /* Current Time Service Declaration */
  BT_GATT_SERVICE_DEFINE(cts_cvs,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_CTS),

    BT_GATT_CHARACTERISTIC(BT_UUID_CTS_CURRENT_TIME,
      BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			read_ct,
      write_ct,
      ct),

    BT_GATT_CCC(ct_ccc_cfg_changed,
      BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
  );

//==============================================================================
// generate current time
//==============================================================================

	static void generate_current_time(uint8_t *buf)
	{
		uint16_t year;

		/* 'Exact Time 256' contains 'Day Date Time' which contains
		 * 'Date Time' - characteristic contains fields for:
		 * year, month, day, hours, minutes and seconds.
		 */

		year = sys_cpu_to_le16(2015);
		memcpy(buf,  &year, 2); /* year */
		buf[2] = 5U; /* months starting from 1 */
		buf[3] = 30U; /* day */
		buf[4] = 12U; /* hours */
		buf[5] = 45U; /* minutes */
		buf[6] = 30U; /* seconds */

		/* 'Day of Week' part of 'Day Date Time' */
		buf[7] = 1U; /* day of week starting from 1 */

		/* 'Fractions 256 part of 'Exact Time 256' */
		buf[8] = 0U;

		/* Adjust reason */
		buf[9] = 0U; /* No update, change, etc */
	}

//==============================================================================
// handler: [SERVICE:CTS]  // setup CTS service receipe
//==============================================================================

  int svc_cts(BL_ob *o, int val)
  {
    bl_err(-1,"setup of CTS receipe not yet supported");
    return -1;
  }

//==============================================================================
// handler: [SERVICE:SIMU @sid]
// - Current Time Service updates only when time is changed
//==============================================================================

  int svc_update(BL_ob *o, int val)
  {
    LOGO(4,"simu:",o,val);

    if (ct_update)
    {
      ct_update = 0U;
      bt_gatt_notify(NULL, &cts_cvs.attrs[1], &ct, sizeof(ct));
    }
    return 0;
  }

//==============================================================================
// handler: [SYS:INIT (cb)]
// - simulate current time for Current Time Service
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    generate_current_time(ct);
    return 0;
  }

//==============================================================================
// Current Time Service (CTS)
//==============================================================================
//
// (W) := bl_wl;
//                  +--------------------+
//                  |       bl_cts       | Current Time Service (CTS)
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)->     INIT ->|        (cb)        | init module, store (cb) callback
//                  +--------------------+
//                  |      SERVICE:      | SERVICE input interface
// (W)->      CTS ->|      <BL_ble>      | setup CTS service
//                  +--------------------+
//
//==============================================================================

  int bl_cts(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,SID,NULL};
    static BL_oval W = NULL;           // wireless core

    if (bl_is(o,_SYS,INIT_))
    {
      LOG(3,BL_B "init bl_cts");
      W = bl_cb(o,W,WHO"(W)");
      bl_lib(W,&lib);                  // register service @ parent
      return sys_init(o,val);          // delegate to sys_init() handler
    }
    else if (bl_ix(o) != SID)          // proper service ID provided?
      return -1;

      // messages are only dispatched for proper serviced ID

    switch (bl_id(o))
    {
      case BL_ID(_SVC,CTS_):           // setup CTS service
        return svc_cts(o,val);

      case BL_ID(_SVC,UPDATE_):        // update service data
        return svc_update(o,val);

      default:
        return -1;
    }
  }
