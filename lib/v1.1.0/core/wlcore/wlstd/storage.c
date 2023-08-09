//==============================================================================
// storage.c
// storage handler
//
// Created by Hugo Pristauz on 2022-Mar-24
// Copyright © 2022 Blunetics. All rights reserved.
//==============================================================================
// Bluetooth: Mesh Generic OnOff, Generic Level, Lighting & Vendor Models
// Copyright (c) 2018 Vikrant More
// SPDX-License-Identifier: Apache-2.0
//==============================================================================

  #include "ble_mesh.h"
  #include "bl_dcomp.h"
  #include "storage.h"

  #include "bluccino.h"

  #define PMI  storage                // public module interface
  #define sys_init  storage_sys_init  // avoid name clashes

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO  "storage:"

  #define LOG                     LOG_NVM
  #define LOGO(lvl,col,o,val)     LOGO_NVM(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_NVM(lvl,col,o,val)

//==============================================================================
// locals
//==============================================================================

  static uint8_t storage_id;
  uint8_t reset_counter;

    // the NVM cache

  static int nvm_cache[20];            // our addressable NVM cache memory
  static bool dirty = false;           // is nvm_cache dirty?
  static bool ready = false;           // is nvm cache ready?

//==============================================================================
// helper: settings init (initializes the zephyr settings subsystem)
// - we keep this function global in order to avoid multiple initializing
//   calls to settings_subsys_init()
// - we define this function weak to avoid collision with the same named
//   function in bl_hwnvm
//==============================================================================

  __weak int bl_settings_init(void)
  {
    static bool initialized = false;

    if (initialized)
      return 0;                        // already initialized

    LOG(4,BL_B "init settings subsystem (bl_storage)");

    int err = settings_subsys_init();
    if (err)
      bl_err(err,"settings subsys initialization");
    else
      initialized = true;

    return err;
  }

//==============================================================================
// helper: local save functions
//==============================================================================

  static void save_nvm_cache(void)
  {
    LOG(4,"store: @nvm_cache");
  	settings_save_one("ps/nvm", &nvm_cache, sizeof(nvm_cache));
  }

  static void save_reset_counter(void)
  {
    LOG(4,"store: @reset_counter,%d",reset_counter);
  	settings_save_one("ps/rc", &reset_counter, sizeof(reset_counter));
  }

  static void save_gen_def_trans_time_state(void)
  {
  	settings_save_one("ps/gdtt", &ctl->tt, sizeof(ctl->tt));
  }

  static void save_gen_onpowerup_state(void)
  {
  	settings_save_one("ps/gpo", &ctl->onpowerup, sizeof(ctl->onpowerup));

  	if (ctl->onpowerup == 0x02) {
  		save_on_flash(LAST_TARGET_STATES);
  	}
  }

  static void save_def_states(void)
  {
  	settings_save_one("ps/ld", &ctl->light->def, sizeof(ctl->light->def));
  	settings_save_one("ps/td", &ctl->temp->def, sizeof(ctl->temp->def));
  	settings_save_one("ps/dd", &ctl->duv->def, sizeof(ctl->duv->def));
  }

  static void save_lightness_last_state(void)
  {
  	settings_save_one("ps/ll", &ctl->light->last, sizeof(ctl->light->last));

  	LOG(4,"lightness last values saved");
  }

  static void save_last_target_states(void)
  {
  	settings_save_one("ps/llt", &ctl->light->target,
  			  sizeof(ctl->light->target));

  	settings_save_one("ps/tlt", &ctl->temp->target,
  			  sizeof(ctl->temp->target));

  	settings_save_one("ps/dlt", &ctl->duv->target,
  			  sizeof(ctl->duv->target));
  }

  static void save_lightness_range(void)
  {
  	ctl->light->range = (uint32_t) ((ctl->light->range_max << 16) |
  				     ctl->light->range_min);

  	settings_save_one("ps/lr", &ctl->light->range,
  			  sizeof(ctl->light->range));
  }

  static void save_temperature_range(void)
  {
  	ctl->temp->range = (uint32_t) ((ctl->temp->range_max << 16) |
  				    ctl->temp->range_min);

  	settings_save_one("ps/tr", &ctl->temp->range, sizeof(ctl->temp->range));
  }

//==============================================================================
// callback: storage work handler
//==============================================================================

  static void storage_work_handler(struct k_work *work)
  {
  	switch (storage_id)
    {
    	case NVM_CACHE:
    		save_nvm_cache();
    		break;
    	case RESET_COUNTER:
    		save_reset_counter();
    		break;
    	case GEN_DEF_TRANS_TIME_STATE:
    		save_gen_def_trans_time_state();
    		break;
    	case GEN_ONPOWERUP_STATE:
    		save_gen_onpowerup_state();
    		break;
    	case DEF_STATES:
    		save_def_states();
    		break;
    	case LIGHTNESS_LAST_STATE:
    		save_lightness_last_state();
    		break;
    	case LAST_TARGET_STATES:
    		save_last_target_states();
    		break;
    	case LIGHTNESS_RANGE:
    		save_lightness_range();
    		break;
    	case TEMPERATURE_RANGE:
    		save_temperature_range();
    		break;
  	}
  }

  K_WORK_DEFINE(storage_work, storage_work_handler);

//==============================================================================
// helper: save on flash
//==============================================================================

  void save_on_flash(uint8_t id)
  {
  	storage_id = id;
    LOG(5,"save_on_flash: @%d",id);
  	k_work_submit(&storage_work);
  }

//==============================================================================
// helper: notify readiness
//==============================================================================

  static void nvm_ready_worker(struct k_work *work)
  {
		ready = true;
    LOG(4,BL_M "NVM is now ready");
    _bl_msg((bl_storage),_NVM,READY_, 0,NULL,ready);  // (BL_NVM) <- [NVM:READY ready]
  }

  K_WORK_DEFINE(nvm_ready_work, nvm_ready_worker);

  static void submit_nvm_ready(void)
  {
    k_work_submit(&nvm_ready_work);
  }

//==============================================================================
// helper: save settings on flash
//==============================================================================

  static int ps_set(const char *key, size_t len_rd,
  		  settings_read_cb read_cb, void *cb_arg)
  {
    ssize_t len = 0;
    int key_len;
    const char *next;

    key_len = settings_name_next(key, &next);
    LOG(5,"ps_set - key:%s, next: %s", key, next?next:"<NULL>");

    if (!next)
    {
      if (!strncmp(key, "nvm", key_len))
      {
        len = read_cb(cb_arg, &nvm_cache, sizeof(nvm_cache));
        submit_nvm_ready();
      }

      if (!strncmp(key, "rc", key_len))
      {
        len = read_cb(cb_arg, &reset_counter, sizeof(reset_counter));
        //LOG(4,"ps_set: reset counter = %d",reset_counter);
      }

      if (!strncmp(key, "gdtt", key_len))
      {
        len = read_cb(cb_arg, &ctl->tt, sizeof(ctl->tt));
        //LOG(4,"ps_set: tt = %d",ctl->tt);
      }

      if (!strncmp(key, "gpo", key_len))
        len = read_cb(cb_arg, &ctl->onpowerup, sizeof(ctl->onpowerup));

      if (!strncmp(key, "ld", key_len))
        len = read_cb(cb_arg, &ctl->light->def, sizeof(ctl->light->def));

      if (!strncmp(key, "td", key_len))
  	    len = read_cb(cb_arg, &ctl->temp->def, sizeof(ctl->temp->def));

      if (!strncmp(key, "dd", key_len))
  	    len = read_cb(cb_arg, &ctl->duv->def, sizeof(ctl->duv->def));

      if (!strncmp(key, "ll", key_len))
  	    len = read_cb(cb_arg, &ctl->light->last, sizeof(ctl->light->last));

      if (!strncmp(key, "llt", key_len))
  	    len = read_cb(cb_arg, &ctl->light->target, sizeof(ctl->light->target));

      if (!strncmp(key, "tlt", key_len))
  	    len = read_cb(cb_arg, &ctl->temp->target, sizeof(ctl->temp->target));

      if (!strncmp(key, "dlt", key_len))
  	    len = read_cb(cb_arg, &ctl->duv->target, sizeof(ctl->duv->target));

      if (!strncmp(key, "lr", key_len))
  	    len = read_cb(cb_arg, &ctl->light->range,  sizeof(ctl->light->range));

      if (!strncmp(key, "tr", key_len))
  	    len = read_cb(cb_arg, &ctl->temp->range, sizeof(ctl->temp->range));

      return (len < 0) ? len : 0;
    }

    return -ENOENT;
  }

//==============================================================================
// settings handler
//==============================================================================

  static struct settings_handler ps_settings =
         {
  	       .name = "ps",
  	       .h_set = ps_set,
         };

//==============================================================================
//------------------------------------------------------------------------------
//                                WORKERS
//------------------------------------------------------------------------------
//==============================================================================

//==============================================================================
// worker: save NVM cache to NVM
// - save operation will be only executed if <data> equals NULL
//==============================================================================

  static int nvm_save(BL_ob *o, int val)
  {
    if (o->data == NULL)
    {
      save_nvm_cache();
      return 0;                        // OK
    }

    return -1;                         // otherwise return error
  }

//==============================================================================
// worker: store value in NVM
//==============================================================================

  static int nvm_store(BL_ob *o, int val)
  {
      if (bl_ix(o) < 0 || bl_ix(o) >= BL_LEN(nvm_cache))
        return -1;                   // bad storage ID

      LOG(4,"store @%d: %d",bl_ix(o),val);

      if (nvm_cache[bl_ix(o)] != val)
      {
        //bl_feed();                   // feed watchdog
        nvm_cache[bl_ix(o)] = val;
        dirty = true;                  // cache is now dirty
      }
      //bl_nvmdrv_write(id,value);
      return 0;                        // OK
  }

//==============================================================================
// worker: recall value from NVM
//==============================================================================

  static int nvm_recall(BL_ob *o, int val)
  {
      if (bl_ix(o) < 0 || bl_ix(o) >= BL_LEN(nvm_cache))
        return 0;

      val = nvm_cache[bl_ix(o)];
      LOG(4,"recall @%d: %d",bl_ix(o),val);
      return val;
  }

//==============================================================================
// worker: system tock
//==============================================================================

  static int sys_tock(BL_ob *o, int val)
  {
    if (val % 2 == 0)                  // every 2 seconds
    {
      if (!ready && val >= 4)          // if not ready in 4 seconds
      {
        LOG(4,BL_R "reset NVM cache (since wasn't ready within 4s)");

        for (int i=0; i<BL_LEN(nvm_cache); i++)
          nvm_cache[i] = 0;            // init NVM cache

        ready = true;
        dirty = true;

        submit_nvm_ready();            // notify higher levels
      }

      if (ready && dirty)
      {
        LOG(4,"backup NVM cache ...");
        save_nvm_cache();
        dirty = false;
      }
    }
    return 0;                          // OK
  }

//==============================================================================
// worker: system init
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    LOG(3,BL_C "init NVM");

    int err = bl_settings_init();
    if (err)
    {
      bl_err(err,"settings_subsys_init failed");
      return err;
    }

    err = settings_register(&ps_settings);
    if (err)
    {
      bl_err(err,"ps_settings_register failed");
      return err;
    }

    return 0;
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (W) := (BL_WL)
//                  +--------------------+
//                  |     bl_storage     | non volatile memory access
//                  +--------------------+
//                  |        SYS:        | SYS: public interface
// (W)->     INIT ->|       @ix,cnt      | init module, store <out> callback
// (W)->     TOCK ->|       @ix,cnt      | tock the module
//                  +--------------------+
//                  |        NVM:        | NVM input interface
// (W)->    STORE ->|      @ix,val       | store value in NVM at location @ix
// (W)->   RECALL ->|        @ix         | recall value in NVM at location @ix
// (W)->     SAVE ->|                    | save NVM cache to NVM
//                  |....................|
//                  |        NVM:        | NVM output interface
// (W)<-    READY <-|       ready        | notification that NVM is now ready
//                  +--------------------+
//
//==============================================================================

  int bl_storage(BL_ob *o, int val)
  {
    static BL_oval W = bl_wl;

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):        // [SYS:INIT <out>]
        W = bl_cb(o,(W),WHO"(W)");   // store output callback
        return sys_init(o,val);      // forward to sys_init() worker

      case BL_ID(_SYS,TOCK_):        // [SYS:TOCK @0,cnt]
        return sys_tock(o,val);      // forward to sys_tock() worker

      case BL_ID(_NVM,STORE_):       // [NVM:STORE @ix,val]
        return nvm_store(o,val);     // delegate to nvm_store() worker

      case BL_ID(_NVM,RECALL_):      // val = [NVM:RECALL @ix]
        return nvm_recall(o,val);    // delegate to nvm_recall() worker

      case BL_ID(_NVM,SAVE_):        // [NVM:STORE @ix,val]
        return nvm_save(o,val);      // delegate to nvm_save() worker

      case _BL_ID(_NVM,READY_):      // [#NVM:READY]
        return bl_out(o,val,(W));    // [NVM:READY] -> (W)

      case NVM_SUPPORT_0_0_0:
        LOG(4,BL_B "NVM supported by bl_storage");
        return 1;                    // yes, NVM functionality available

      default:
        return -1;                   // bad input
    }
  }

//==============================================================================
// cleanup
//==============================================================================

  #include "bl_clean.h"
  #undef   sys_init
