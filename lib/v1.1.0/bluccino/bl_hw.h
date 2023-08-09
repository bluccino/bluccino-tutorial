//==============================================================================
// bl_hw.h
// Bluccino HW core supporting basic functions for button & LED
//
// Created by Hugo Pristauz on 2022-Feb-18
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

#ifndef __BL_HW_H__
#define __BL_HW_H__

//==============================================================================
// public module interface
//==============================================================================

  int bl_hw(BL_ob *o, int val);        // HW core module interface

//==============================================================================
// [LED:op] message definition
// - [LED:SET @ix,onoff] set LED @ix on/off (i=0..4)
// - [LED:TOGGLE @ix] toggle LED @ix (i=0..4)
//==============================================================================

  #define LED_SET_ix_0_onoff      BL_ID(_LED,SET_)
  #define LED_TOGGLE_ix_0_0       BL_ID(_LED,TOGGLE_)

    // augmented messages

  #define _LED_SET_ix_0_onoff     _BL_ID(_LED,SET_)
  #define _LED_TOGGLE_ix_0_0      _BL_ID(_LED,TOGGLE_)

//==============================================================================
// [BUTTON:op] message definition
// - [BUTTON:PRESS @ix] button @ix pressed (rising edge)
// - [BUTTON:RELEASE @ix,ms] button @ix released after ms-time
// - [BUTTON:CLICK @ix,cnt] button @ix clicked (cnt: number of clicks)
// - [BUTTON:HOLD @ix,ms] button @ix held (ms: hold ms-time)
// - [BUTTON:CFG mask] config button module
// - [BUTTON:MS mask] set click/hold discrimination time
//==============================================================================

  #define BUTTON_PRESS_ix_0_0     BL_ID(_BUTTON,PRESS_)
  #define BUTTON_RELEASE_ix_0_ms  BL_ID(_BUTTON,RELEASE_)
  #define BUTTON_CLICK_ix_0_cnt   BL_ID(_BUTTON,CLICK_)
  #define BUTTON_HOLD_ix_0_ms     BL_ID(_BUTTON,HOLD_)
  #define BUTTON_CFG_0_0_mask     BL_ID(_BUTTON,CFG_)
  #define BUTTON_MS_0_0_ms        BL_ID(_BUTTON,MS_)

    // augmented messages

  #define _BUTTON_PRESS_ix_0_0    _BL_ID(_BUTTON,PRESS_)
  #define _BUTTON_RELEASE_ix_0_ms _BL_ID(_BUTTON,RELEASE_)
  #define _BUTTON_CLICK_ix_0_cnt  _BL_ID(_BUTTON,CLICK_)
  #define _BUTTON_HOLD_ix_0_ms    _BL_ID(_BUTTON,HOLD_)
  #define _BUTTON_CFG_0_0_mask    _BL_ID(_BUTTON,CFG_)
  #define _BUTTON_MS_0_0_ms       _BL_ID(_BUTTON,MS_)

//==============================================================================
// enable masks for button events
// - usage:
//     bl_cfg(bl_hw,_BUTTON,BL_PRESS)          // enable [BUTTON:PRESS]
//     bl_cfg(bl_hw,_BUTTON,BL_RELEASE)        // enable [BUTTON:RELEASE]
//     bl_cfg(bl_hw,_BUTTON,BL_EDGE)           // enable [BUTTON:PRESS/RELEASE]
//     bl_cfg(bl_hw,_BUTTON,BL_SWITCH)         // enable [BUTTON:SWITCH]
//     bl_cfg(bl_hw,_BUTTON,BL_CLICK)          // enable [BUTTON:CLICK]
//     bl_cfg(bl_hw,_BUTTON,BL_HOLD)           // enable [BUTTON:HOLD]
//
//     bl_cfg(bl_hw,_BUTTON,0xffff)            // enable all [BUTTON:] events
//     bl_cfg(bl_hw,_BUTTON,0x0000)            // disable all [BUTTON:] events
//     bl_cfg(bl_hw,_BUTTON,BL_PRESS|BL_RELEASE) // press/release events
//     bl_cfg(bl_hw,_BUTTON,BL_EDGE)           // same as above
//     bl_cfg(bl_hw,_BUTTON,BL_CLICK|BL_HOLD)  // click/hold events
//     bl_cfg(bl_hw,_BUTTON,BL_MULTI)          // same as above
//==============================================================================

  #define BL_PRESS   0x0001  // mask for [BUTTON:PRESS] events
  #define BL_RELEASE 0x0002  // mask for [BUTTON:RELEASE] events
  #define BL_EDGE    0x0003  // mask for [BUTTON:PRESS],[BUTTON:RELEASE]
  #define BL_SWITCH  0x0004  // mask for [BUTTON:SWITCH] events
  #define BL_CLICK   0x0008  // mask for [BUTTON:CLICK] events
  #define BL_HOLD    0x0010  // mask for [BUTTON:HOLD] events
  #define BL_TOCK    0x0020  // mask for [BUTTON:TOCK] events
  #define BL_MULTI   0x0018  // mask for [BUTTON:CLICK],[BUTTON:HOLD]
  #define BL_ALL     0xFFFF  // mask for all [BUTTON:] events
  #define BL_NONE    0x0000  // mask for no [BUTTON:] events

//==============================================================================
// [SYS:CFG mask] message definition
// - [SYS:CFG mask]  config module
//==============================================================================

  #define SYS_CFG_0_0_mask        BL_ID(_SYS,CFG_)

    // augmented messages

  #define _SYS_CFG_0_0_mask       _BL_ID(_SYS,CFG_)

//==============================================================================
// [SWITCH:STS @ix,sts] message definition
// - [SWITCH:STS @ix,sts] on/off status update of switch @ix
//==============================================================================

  #define SWITCH_STS_ix_0_sts     BL_ID(_SWITCH,STS_)

    // augmented messages

  #define _SWITCH_STS_ix_0_sts    _BL_ID(_SWITCH,STS_)

//==============================================================================
// [NVM:] non volatile memory (NVM) message definitions
// - [NVM:LOAD <BL_nvm>] load nvm data
// - [NVM:SAVE <BL_nvm>] load nvm data
// - [NVM:READY sts] notification that NVM is now ready
// - [NVM:STORE @ix,val] store value in NVM at location @ix
// - [NVM:RECALL @ix] recall value in NVM at location @ix
// - [NVM:AVAIL] is NVM functionality available? (return ok value >= 0)
//==============================================================================

  #define NVM_LOAD_0_BL_tray_0    BL_ID(_NVM,LOAD_)
  #define NVM_SAVE_0_BL_tray_0    BL_ID(_NVM,SAVE_)
  #define NVM_STORE_ix_0_val      BL_ID(_NVM,STORE_)
  #define NVM_RECALL_ix_0_0       BL_ID(_NVM,RECALL_)
  #define NVM_READY_0_0_sts       BL_ID(_NVM,READY_)
  #define NVM_SUPPORT_0_0_0       BL_ID(_NVM,SUPPORT_)
  #define NVM_CLEAR_0_0_0         BL_ID(_NVM,CLEAR_)

    // augmented messages

  #define _NVM_LOAD_0_BL_tray_0   _BL_ID(_NVM,LOAD_)
  #define _NVM_SAVE_0_BL_tray_0   _BL_ID(_NVM,SAVE_)
  #define _NVM_STORE_ix_0_val     _BL_ID(_NVM,STORE_)
  #define _NVM_RECALL_ix_0_0      _BL_ID(_NVM,RECALL_)
  #define _NVM_READY_0_0_sts      _BL_ID(_NVM,READY_)
  #define _NVM_SUPPORT_0_0_0      _BL_ID(_NVM,SUPPORT_)
  #define _NVM_CLEAR_0_0_0        _BL_ID(_NVM,CLEAR_)
/*
#ifdef __cplusplus

  class Nvm_Load : public BlMsg
  {
    private:
      BL_tray tray;

    public:
      Nvm_Load(BL_tray *ptray) : BlMsg(_NVM,LOAD_,0,&tray,0)
      {
        tray = *ptray;
      }

      Nvm_Load(BL_txt key, void *data, size_t size) : BlMsg(_NVM,LOAD_)
      {
        tray.data = data; tray.size = size; tray.key = key;
        data = &tray;
      }
  };

  class Nvm_Save : public BlMsg
  {
    private:
      BL_tray tray;

    public:
      Nvm_Save(BL_tray *ptray) : BlMsg(_NVM,SAVE_,0,&tray,0)
      {
        tray = *ptray;
      }

      Nvm_Save(BL_txt key, void *data, size_t size) : BlMsg(_NVM,SAVE_)
      {
        tray.data = data; tray.size = size; tray.key = key;
        data = &tray;
      }
  };

  class Nvm_Store : public BlMsg
  {
    public:
      Nvm_Store(int ix, int val) : BlMsg(_NVM,STORE_, ix,0,val) {}
  };

  class Nvm_Recall : public BlMsg
  {
    public:
      Nvm_Recall(int ix) : BlMsg(_NVM,RECALL_, ix,0,0) {}
  };

  class Nvm_Support : public BlMsg
  {
    public:
      Nvm_Support() : BlMsg(_NVM,SUPPORT_, 0,0,0) {}
  };

#endif
*/
//==============================================================================
// footer
//==============================================================================

#endif // __BL_HW_H__
