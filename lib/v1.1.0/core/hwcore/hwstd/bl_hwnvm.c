//==============================================================================
// bl_hwnvm.c
// non volatile memory (NVM) handler
//
// Created by Hugo Pristauz on 2022-Mar-24
// Copyright © 2022 Blunetics. All rights reserved.
//==============================================================================
// Bluetooth: Mesh Generic OnOff, Generic Level, Lighting & Vendor Models
// Copyright (c) 2018 Vikrant More
// SPDX-License-Identifier: Apache-2.0
//==============================================================================

  #include <zephyr/settings/settings.h>

  #include "bluccino.h"
  #include "bl_hwnvm.h"

  #define PMI  bl_hwnvm

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO  "bl_hwnvm:"

  #define LOG                     LOG_NVM
  #define LOGO(lvl,col,o,val)     LOGO_NVM(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_NVM(lvl,col,o,val)

//==============================================================================
// typedef: internal type for fetching data
//==============================================================================

  typedef struct BL_fetch              // direct immediate value
  {
    size_t len;
    void *dest;
    uint8_t ok;
  } BL_fetch;

  typedef settings_read_cb BL_read;    // shorthand for settings read callback

//==============================================================================
// helper: settings init (initializes the zephyr settings subsystem)
// - we keep this function global in order to avoid multiple initializing
//   calls to settings_subsys_init()
//==============================================================================

  int bl_settings_init(void)
  {
    static bool initialized = false;

    if (initialized)
      return 0;                        // already initialized

    LOG(4,BL_B "init settings subsystem");

    int err = settings_subsys_init();
    if (err)
      bl_err(err,"settings subsys initialization");
    else
      initialized = true;

    return err;
  }

//==============================================================================
// helper: direct loader (immediate values)
//==============================================================================

  static int loader(BL_txt key, size_t len, BL_read read, void *arg, void *par)
  {
    BL_fetch *pval = (BL_fetch *)par;
    const char *next;
    size_t length;
    int err;

    length = settings_name_next(key, &next);

    if (length == 0)
    {
      if (len == pval->len)
      {
        err = read(arg, pval->dest, len);
        if (err >= 0)
        {
          pval->ok = 1;
          //bl_log(1,"load: OK");
          return 0;
        }

        bl_err(err,"NVM item load failed");
        return err;
      }
      return -EINVAL;
    }

    return 0;
  }

//==============================================================================
// helper: load NVM data
// - usage: err = load("value", &value, sizeof(value))
//==============================================================================

  static int load(BL_txt key, void *dest, size_t len)
  {
    int err;
    BL_fetch fetch;

    fetch.ok = 0;
    fetch.len = len;
    fetch.dest = dest;

    err = settings_load_subtree_direct(key, loader, (void *)&fetch);
    if (err == 0)
    {
      if (!fetch.ok)
        err = -ENOENT;
    }

    return err;
  }

//==============================================================================
// helper: save NVM data
// - usage: err = save("value", &value, sizeof(value))
//==============================================================================

  static int save(BL_txt key, BL_data data, size_t size)
  {
    bl_log(4,BL_G"saving \"%s\" setting (%d bytes)", key, (int)size);

    int err = settings_save_one(key, (const void *)data, size);
    bl_err(err,"nvm_save() failed");

    return err;
  }

//==============================================================================
// worker: load from NVM using BL_nvm structure
// - usage: FD_nvm nvm = {key, &value, sizeof(value)};
//          err = bl_msg((fd_nvm), _NVM,LOAD_, 0,&nvm,0);
//==============================================================================

  static int nvm_load(BL_ob *o, int val)
  {
    BL_tray *p = bl_data(o);
    return load(p->key, p->data, p->size);
  }

//==============================================================================
// worker: save to NVM using BL_nvm structure
// - usage: FD_nvm nvm = {key, &value, sizeof(value)};
//          err = bl_msg((fd_nvm), _NVM,SAVE_, 0,&nvm,0);
//==============================================================================

  static int nvm_save(BL_ob *o, int val)
  {
    BL_tray *p = bl_data(o);
    return save(p->key, p->data, p->size);
  }

//==============================================================================
// worker: system init
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    LOG(4,BL_B "init NVM");
    int err = bl_settings_init();

    if (err)
    {
      bl_err(err,"settings subsys initialization");
      return err;
    }

    bl_log(5,BL_B "init NVM: OK");
    return err;
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (H) := (bl_hw);  (U) := (bl_up);
//
//                  +--------------------+
//                  |      bl_hwnvm      |
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (H)->     INIT ->|        <cb>        | init module, store <out> callback
//                  +--------------------+
//                  |        NVM:        | NVM input interface
// (H)->     LOAD ->|      <BL_tray>      | load NVM data
// (H)->     SAVE ->|      <BL_tray>      | save NVM data
// (H)->  SUPPORT ->|                    | is NVM functionality supported?
//                  |....................|
//                  |        NVM:        | NVM output interface
// (U)<-    READY <-|       ready        | notification that NVM is now ready
//                  +--------------------+
//
//==============================================================================

   int bl_hwnvm(BL_ob *o, int val)
   {
     static BL_oval U = NULL;

     switch (bl_id(o))
     {
       case SYS_INIT_0_cb_0:
         U = bl_cb(o,(U),WHO"(U)");    // store output callback
         return sys_init(o,val);       // delegate to sys_init() worker

       case NVM_LOAD_0_BL_tray_0:       // [NVM:LOAD <BL_tray>]
         return nvm_load(o,val);       // delegate to nvm_load() worker

       case NVM_SAVE_0_BL_tray_0:       // [NVM:SAVE <BL_tray>]
         return nvm_save(o,val);       // delegate to nvm_save() worker

       case NVM_SUPPORT_0_0_0:
         LOG(4,BL_B "NVM supported by bl_hwnvm");
         return 1;                     // yes, NVM functionality available

      case NVM_READY_0_0_sts:
        return bl_out(o,val,(U));      // [NVM:READY] -> (U)

      default:
         return 0;
     }
   }

//==============================================================================
// cleanup
//==============================================================================

  #include "bl_clean.h"
