//==============================================================================
// bl_wl.c
// multi model mesh demo based wireless (mesh) core
//
// Created by Hugo Pristauz on 2022-Jan-02
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================
// mcore derived from:
// Bluetooth: Mesh Generic OnOff, Generic Level, Lighting & Vendor Models
// Copyright (c) 2018 Vikrant More
// SPDX-License-Identifier: Apache-2.0
//==============================================================================

//==============================================================================
// include all C-sources of module BL_WL
//==============================================================================

  #include "bl_deco.c"                 // Bluccino log decoration
  #include "bl_mesh.c"
  #include "ble_mesh.c"
  #include "bl_dcomp.c"
  #include "notrans.c"
  #include "publisher.c"
  #include "state_binding.c"
  #include "storage.c"
  #include "transition.c"
  #include "bl_trans.c"
  #include "bl_reset.c"

//==============================================================================
// start actual BL_WL module
//==============================================================================

  #include <zephyr/drivers/gpio.h>

  #include "bluccino.h"

  #include "bl_core.h"
  #include "bl_dcomp.h"
  #include "bl_gonoff.h"
  #include "ble_mesh.h"
  #include "bl_reset.h"
  #include "bl_wl.h"

  #include "notrans.h"
  #include "state_binding.h"
  #include "storage.h"
  #include "transition.h"
  #include "publisher.h"

//==============================================================================
// CORE level logging shorthands
//==============================================================================

  #define WHO                     "bl_core:"

  #define LOG                     LOG_CORE
  #define LOGO(lvl,col,o,val)     LOGO_CORE(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_CORE(lvl,col,o,val)

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

#if defined(CONFIG_MCUMGR)
#include <mgmt/mcumgr/smp_bt.h>
#include "smp_svr.h"
#endif

static bool reset;

static void light_default_var_init(void)
{
	ctl->tt = 0x00;

	ctl->onpowerup = STATE_DEFAULT;

	ctl->light->range_min = LIGHTNESS_MIN;
	ctl->light->range_max = LIGHTNESS_MAX;
	ctl->light->last = LIGHTNESS_MAX;
	ctl->light->def = LIGHTNESS_MAX;
	ctl->light->target = ctl->light->def;

	ctl->temp->range_min = TEMP_MIN;
	ctl->temp->range_max = TEMP_MAX;
	ctl->temp->def = TEMP_MAX;
	ctl->temp->target = ctl->temp->def;

	ctl->duv->def = DELTA_UV_DEF;
	ctl->duv->target = ctl->duv->def;
}

/* This function should only get call after execution of settings_load() */
static void light_default_status_init(void)
{
	/* Retrieve Range of Lightness */
	if (ctl->light->range) {
		ctl->light->range_max = (uint16_t) (ctl->light->range >> 16);
		ctl->light->range_min = (uint16_t) ctl->light->range;
	}

	/* Retrieve Range of Temperature */
	if (ctl->temp->range) {
		ctl->temp->range_max = (uint16_t) (ctl->temp->range >> 16);
		ctl->temp->range_min = (uint16_t) ctl->temp->range;
	}

	ctl->light->last = constrain_lightness(ctl->light->last);
	ctl->light->def = constrain_lightness(ctl->light->def);
	ctl->light->target = constrain_lightness(ctl->light->target);

	ctl->temp->def = constrain_temperature(ctl->temp->def);
	ctl->temp->target = constrain_temperature(ctl->temp->target);

	ctl->temp->current = ctl->temp->def;
	ctl->duv->current = ctl->duv->def;

	switch (ctl->onpowerup) {
	case STATE_OFF:
		ctl->light->current = 0U;
		break;
	case STATE_DEFAULT:
		if (ctl->light->def == 0U) {
			ctl->light->current = ctl->light->last;
		} else {
			ctl->light->current = ctl->light->def;
		}
		break;
	case STATE_RESTORE:
		ctl->light->current = ctl->light->target;
		ctl->temp->current = ctl->temp->target;
		ctl->duv->current = ctl->duv->target;
		break;
	}

	ctl->light->target = ctl->light->current;
	ctl->temp->target = ctl->temp->current;
	ctl->duv->target = ctl->duv->current;
}

void update_vnd_led_gpio(void)
{
#ifndef ONE_LED_ONE_BUTTON_BOARD
/*
	gpio_pin_set(led_device[1], DT_GPIO_PIN(DT_ALIAS(led1), gpios),
		     vnd_user_data.current == STATE_ON);
*/
#endif
}

  void update_led_gpio(void)
  {
  }

  void update_light_state(void)
  {
	  update_led_gpio();

	  if (ctl->transition->counter == 0 || reset == false)
    {
		  reset = true;
		  k_work_submit(&no_transition_work);
	  }
  }

//==============================================================================
// worker: system init (fomer main())
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    static BL_oval B = ble_mesh;       // Bluetooth BLE/mesh module
    static BL_oval D = bl_deco;        // decorate log lines
    static BL_oval R = bl_reset;       // reset module
    static BL_oval S = bl_storage;     // NVM storage module
    static BL_oval W = bl_wl;          // wireless core

    light_default_var_init();

    #if defined(CONFIG_MCUMGR)
      smp_svr_init();
    #endif

      // decorate module (coloring log time stamps) to be registered in top gear

    bl_install((D));                   // install bl_deco in top gear

      // NVM module and BLE/mesh module to be registered in wireless core

    bl_init((S),(W));                  // init NVM, output => bl_wl()
    bl_init((B),(W));                  // output of BLE_MESH goes to here!

    light_default_status_init();

    update_light_state();

    bl_init((R),(W));

    #if defined(CONFIG_MCUMGR)
      // init Bluetooth mcumgr transport
      smp_bt_register();

      k_timer_start(&smp_svr_timer, K_NO_WAIT, K_MSEC(1000));
    #endif
    return 0;                          // OK
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (N) := (BL_NVM);  (!) := (<parent>); (O) := (<out>); (B) := (BLE_MESH);
//                  +--------------------+
//                  |        BL_WL       | wireless core
//                  +--------------------+
//                  |        SYS:        | SYS: public interface
// (!)->     INIT ->|       @ix,cnt      | init module, store <out> callback
// (!)->     TICK ->|       @ix,cnt      | tick the module
// (!)->     TOCK ->|       @ix,cnt      | tock the module
//                  +--------------------+
//                  |       MESH:        | MESH upper interface
// (U)<-      PRV <-|       onoff        | provision on/off
// (U)<-      ATT <-|       onoff        | attention on/off
//                  |....................|
//                  |       MESH:        | MESH lower interface
// (B)->      PRV ->|       onoff        | provision on/off
// (B)->      ATT ->|       onoff        | attention on/off
//                  +--------------------+
//                  |       RESET:       | RESET input interface
// (!)->      INC ->|         ms         | inc reset counter & set due timer
// (!)->      PRV ->|                    | unprovision node
// (R)->      DUE ->|                    | reset timer is due
//                  |....................|
//                  |      #RESET:       | RESET output interface
// (R)<-      INC <-|         ms         | inc reset counter & set due timer
// (R)<-      PRV <-|                    | unprovision node
// (U)<-      DUE <-|                    | reset timer is due
//                  +--------------------+
//                  |        NVM:        | NVM: upper interface
// (U)<-    READY <-|       ready        | notification that NVM is now ready
// (!)->    STORE ->|      @ix,val       | store value in NVM at location @ix
// (!)->   RECALL ->|        @ix         | recall value in NVM at location @ix
// (!)->     SAVE ->|                    | save NVM cache to NVM
//                  |....................|
//                  |        NVM:        | NVM: lower interface
// (N)->    READY ->|       ready        | notification that NVM is now ready
// (N)<-    STORE <-|      @ix,val       | store value in NVM at location @ix
// (N)<-   RECALL <-|        @ix         | recall value in NVM at location @ix
// (N)<-     SAVE <-|                    | save NVM cache to NVM
//                  +--------------------+
//                  |      GOOCLI:       | GOOCLI input interface
// (!)->      SET ->| @ix,<BL_goo>,onoff | acknowledged generic on/off set
// (!)->      LET ->| @ix,<BL_goo>,onoff | unacknowledged generic on/off set
// (!)->      GET ->|        @ix         | request generic on/off server status
//                  +--------------------+
//                  |      GOOSRV:       | GOOSRV output interface
// (U)<-      STS <-|  @ix,<BL_goo>,sts  | notify server status change
//                  +--------------------+
//                  |        GET:        | GET input interface
// (!)->      ATT ->|                    | gett node's attention status
// (!)->      PRV ->|                    | gett node's provision status
//                  +--------------------+
//
//==============================================================================

  int bl_wl(BL_ob *o, int val)
  {
    static bool att = false;           // attention
    static bool prv = false;           // provision

    static BL_oval R = bl_reset;       // reset module
    static BL_oval S = bl_storage;     // NVM storage module
    static BL_oval U = bl_up;          // up gear

    switch (bl_id(o))
    {
      case SYS_INIT_0_cb_0:            // [SYS:INIT <out>]
        LOG(2,BL_B "init standard wireless core");
        U = bl_cb(o,(U),WHO"(U)");     // store output callback
        bl_init(bl_dcomp,bl_wl);       // output of BL_DEVCMP goes to here!
        return sys_init(o,val);        // forward to sys_init() worker

      case SYS_TICK_ix_BL_pace_cnt:    // [SYS:TICK @0,cnt]
        return 0;                      // OK - nothing to tick/tock

      case SYS_TOCK_ix_BL_pace_cnt:    // [SYS:TICK @0,cnt]
        return bl_fwd(o,val,(S));      // bl_storage module to be tocked

      case MESH_PRV_0_0_sts:           // (B)->[MESH:PRV sts]->(O)  (provision)
        LOGO(3,"(#)",o,val);
        prv = val;
        return bl_out(o,val,(U));      // output to subscriber

      case MESH_ATT_0_0_sts:           // (B)->[MESH:ATT sts]->(O)  (attention)
        LOGO(3,"(#)",o,val);
        att = val;
        return bl_out(o,val,(U));      // output to subscriber

      case RESET_INC_0_0_ms:           // cnt = [RESET:INC <ms>]
      case RESET_PRV_0_0_0:
        return bl_fwd(o,val,(R));      // forward to bl_reset module

      case RESET_DUE_0_0_0:            // (O)<-[#RESET:DUE] reset timer is due
        return bl_out(o,val,(U));      // output message (strip off aug bit)

      case GOOCLI_LET_ix_BL_goo_onoff:
      case GOOCLI_SET_ix_BL_goo_onoff:
      case GOOCLI_GET_ix_0_0:
        return bl_pub(o,val);          // publish [GOOCLI:LET]/[GOOCLI:SET] msg

      case GOOSRV_STS_ix_BL_goo_sts:
        return bl_out(o,val,(U));      // notify [GOOSRV:STS]

      case NVM_READY_0_0_sts:          // [NVM:READY] notify that NVM is ready
        return bl_out(o,val,(U));      // output [NVM:READY] to subscriber

      case NVM_STORE_ix_0_val:
      case NVM_RECALL_ix_0_0:
      case NVM_SAVE_0_BL_tray_0:
      case NVM_LOAD_0_BL_tray_0:
      case NVM_SUPPORT_0_0_0:
        return bl_fwd(o,val,(S));      // forward to bl_storage module

      case GET_ATT_0_0_0:
        return att;                    // return attention state

      case GET_PRV_0_0_0:
        return prv;                    // return provision state

      default:
        return -1;                     // bad input
    }
  }
