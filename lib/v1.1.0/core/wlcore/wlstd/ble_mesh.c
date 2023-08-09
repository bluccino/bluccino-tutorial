/* Bluetooth: Mesh Generic OnOff, Generic Level, Lighting & Vendor Models
 *
 * Copyright (c) 2018 Vikrant More
 *
 * SPDX-License-Identifier: Apache-2.0
 */

  #include "ble_mesh.h"
  #include "bl_dcomp.h"

  #include "bluccino.h"
  #include "bl_wl.h"

  #define PMI  ble_mesh                // public module interface
  #define sys_init  ble_mesh_sys_init  // avoid name clashes

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO                     "ble_mesh:"

  #define LOG                     LOG_CORE
  #define LOGO(lvl,col,o,val)     LOGO_CORE(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_CORE(lvl,col,o,val)
  #define ERR 1,BL_R

//==============================================================================
// overview code migration from original onoff_app sample to a bluccino core
//==============================================================================

#ifndef MIGRATION_STEP1
  #define MIGRATION_STEP1         1    // TODO introduce bl_core()
#endif
#ifndef MIGRATION_STEP2
  #define MIGRATION_STEP2         1    // TODO introduce bl_core()
#endif

//==============================================================================
// let's go ...
//==============================================================================

#ifdef OOB_AUTH_ENABLE

static int output_number(bt_mesh_output_action_t action, uint32_t number)
{
  #if MIGRATION_STEP2
   	LOG(1,BL_M "OOB Number: %u", number);
	#else
   	printk("OOB Number: %u\n", number);
	#endif
	return 0;
}

static int output_string(const char *str)
{
  #if MIGRATION_STEP2
  	LOG(1,BL_M "OOB String: %s", str);
	#else
  	printk("OOB String: %s\n", str);
	#endif
	return 0;
}

#endif

  static void prov_complete(uint16_t net_idx, uint16_t addr)
  {
    _bl_msg(ble_mesh,_MESH,PRV_, 0,NULL,1);     // (BL_WL) <- [#MESH,PRV 1]
  }

  static void prov_reset(void)
  {
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    _bl_msg(ble_mesh,_MESH,PRV_, 0,NULL,0);     // (BL_WL) <- [#MESH,PRV 0]
  }

//==============================================================================
// provisioning link open/close callbacks
//==============================================================================

  static void link_open(bt_mesh_prov_bearer_t bearer)
  {
    _bl_msg(ble_mesh,_MESH,ATT_, 0,NULL,1);     // (BL_WL) <- [#MESH,ATT 1]
  }

  static void link_close(bt_mesh_prov_bearer_t bearer)
  {
    _bl_msg(ble_mesh,_MESH,ATT_, 0,NULL,0);     // (BL_WL) <- [#MESH,ATT 0]
  }

//==============================================================================
// provisioning table
//==============================================================================

  static uint8_t dev_uuid[16] = { 0xdd, 0xdd };

  static const struct bt_mesh_prov prov = {
  	.uuid = dev_uuid,

  #ifdef OOB_AUTH_ENABLE

  	.output_size = 6,
  	.output_actions = BT_MESH_DISPLAY_NUMBER | BT_MESH_DISPLAY_STRING,
  	.output_number = output_number,
  	.output_string = output_string,

  #endif
    .link_open = link_open,              // to activate attention mode
    .link_close = link_close,            // to deactivate attention mode
  	.complete = prov_complete,
  	.reset = prov_reset,
  };

//==============================================================================
// callback: Bluetooth is ready
//==============================================================================

  void bt_ready(void)
  {
  	int err;
  	struct bt_le_oob oob;

  	LOG(4,BL_B "Bluetooth initialized");

  	err = bt_mesh_init(&prov, &comp);
  	if (err)
  	{
  	  bl_err(err,"initializing mesh failed");
  		return;
  	}

  	if (IS_ENABLED(CONFIG_SETTINGS)) {
  		settings_load();
  	}

  	  // use identity address as device UUID

  	if (bt_le_oob_get_local(BT_ID_DEFAULT, &oob))
    	LOG(1,BL_R "Identity Address unavailable");
  	else
    {
  		memcpy(dev_uuid, oob.addr.a.val, 6);
LOG(3,BL_B "device UUID: [%02X%02X:%02X%02X:%02X%02X]",
      dev_uuid[5],dev_uuid[4],dev_uuid[3],dev_uuid[2],dev_uuid[1],dev_uuid[0]);
    }

  	bt_mesh_prov_enable(BT_MESH_PROV_GATT | BT_MESH_PROV_ADV);
    LOG(4,BL_B"mesh initialized");
  }

//==============================================================================
// worker: init
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    LOG(3,BL_C "init BLE/Mesh...");

      // init Bluetooth subsystem

    int err = bt_enable(NULL);
    if (err)
    {
      bl_err(err,"Bluetooth init failed");
      return err;
    }

    bt_ready();
    return 0;                            // OK
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (!) := (<parent>);  (O) := (<parent>);  (#) := (BLE_MESH)
//                  +--------------------+
//                  |      BLE_MESH      | manage BLE/mesh
//                  +--------------------+
//                  |        SYS:        | SYS: public interface
// (!)->     INIT ->|      @ix,<out>     | init module, store <out> callback
//                  +--------------------+
//                  |       MESH:        | MESH: public interface
// (O)<-      PRV <-|       onoff        | provision on/off
// (O)<-      ATT <-|       onoff        | attention on/off
//                  +====================+
//                  |      #MESH:        | MESH: private interface
// (#)->      PRV ->|       onoff        | provision on/off
// (#)->      ATT ->|       onoff        | attention on/off
//                  +--------------------+
//
//==============================================================================

  int ble_mesh(BL_ob *o, int val)
  {
    static BL_oval W = bl_wl;          // out goes to BL_WL by default

    switch (bl_id(o))                  // dispatch message ID
    {
      case BL_ID(_SYS,INIT_):          // [SYS:INIT <out>]
        W = bl_cb(o,(W),WHO"(W)");     // store output callback
        return sys_init(o,val);        // delegate to sys_init() worker

      case _BL_ID(_MESH,PRV_):         // [#SET:PRV val]  (provision)
      case _BL_ID(_MESH,ATT_):         // [#SET:ATT val]  (attention)
//      LOGO(3,BL_G,o,val);
        return bl_out(o,val,(W));      // output to subscriber

      default:
        return -1;                     // bad input
    }
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
  #undef   sys_init
