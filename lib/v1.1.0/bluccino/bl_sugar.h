//==============================================================================
//  bl_sugar.h
//  syntactic sugars
//
//  Created by Hugo Pristauz on 2022-04-03
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_SUGAR_H__
#define __BL_SUGAR_H__

  #include "bl_hw.h"

//==============================================================================
// get module property (use opcodes for property names)
// - usage: val = bl_get(op)           // post to down gear
//          val = _bl_get(op,(PMI))    // augmented version
//==============================================================================

  static inline int bl_get(BL_op op)
  {
    return bl_msg((bl_down),_GET,op, 0,NULL,0);       // post to down gear
  }

  static inline int _bl_get(BL_op op, BL_oval to)
  {
    return _bl_msg((to),_GET,op, 0,NULL,0);           // post augmented
  }

//==============================================================================
// set module property (use opcodes for property names)
// - usage: bl_set(op,val)             // post to down gear
//          _bl_set(op,val,(PMI))      // augmented version
//==============================================================================

  static inline int bl_set(BL_op op, int val)
  {
    return bl_msg((bl_down),_SET,op, 0,NULL,val);     // post to down gear
  }

  static inline int _bl_set(BL_op op, int val, BL_oval to)
  {
    return _bl_msg((to),_SET,op, 0,NULL,val);         // post augmented
  }

//==============================================================================
// syntactic sugar: set LED @ix on/off or togggle LED @ix (@ix: 0..4)
// - usage: bl_led(id,val)          // val 0:off, 1:on, -1:toggle
//          _bl_led(id,val,(PMI))   // augmented version
//==============================================================================

  static inline int bl_led(int id, int val)
  {
    BL_id mid = (val >= 0 ? LED_SET_ix_0_onoff : LED_TOGGLE_ix_0_0);
    return bl_post((bl_down),mid, id,NULL, val<0?0:val);
  }

  static inline int _bl_led(int id, int val, BL_oval to)
  {
    BL_id mid = (val >= 0 ? LED_SET_ix_0_onoff : LED_TOGGLE_ix_0_0);
    return _bl_post((to),mid, id,NULL, val<0?0:val);
  }

//==============================================================================
// syntactic sugar: check if message is a button press message ([BUTTON:PRESS])
// - usage: pressed = bl_pressed(o)
//==============================================================================

  static inline bool bl_pressed(BL_ob *o)
  {
    return  (bl_id(o) == BUTTON_PRESS_ix_0_0);
  }

//==============================================================================
// syntactic sugar: check if message is a button release msg ([BUTTON:RELEASE])
// - usage: released = bl_released(o)
//==============================================================================

  static inline bool bl_released(BL_ob *o)
  {
    return  (bl_id(o) == BUTTON_RELEASE_ix_0_ms);
  }

//==============================================================================
// syntactic sugar: check if message is a switch status update ([SWITCH:STS])
// - usage: switched = bl_switched(o)
//==============================================================================

  static inline bool bl_switched(BL_ob *o)
  {
    return  (bl_id(o) == SWITCH_STS_ix_0_sts);
  }

//==============================================================================
// syntactic sugar: bl_load (load data from NVM)
// - usage: bl_load("count",&count,sizeof(count))          // post to down gear
//         _bl_load("count",&count,sizeof(count),(PMI))    // augmented version
//==============================================================================

  static inline int bl_load(BL_txt key, void *data, size_t size)
  {
    BL_tray tray = {data:data, size:size, key:key};
    return bl_post((bl_down), NVM_LOAD_0_BL_tray_0, 0,&tray,0);
  }

  static inline int _bl_load(BL_txt key, void *data, size_t size, BL_oval to)
  {
    BL_tray tray = {data:data, size:size, key:key};
    return _bl_post((to), NVM_LOAD_0_BL_tray_0, 0,&tray,0);
  }

//==============================================================================
// syntactic sugar: bl_save (save data to NVM)
// - usage: bl_save("count",&count,sizeof(count))          // post to down gear
//         _bl_save("count",&count,sizeof(count),(PMI))    // augmented version
//==============================================================================

  static inline int bl_save(BL_txt key, void *data, size_t size)
  {
    BL_tray tray = {data:data, size:size, key:key};
    return bl_post((bl_down), NVM_SAVE_0_BL_tray_0, 0,&tray,0);
  }

  static inline int _bl_save(BL_txt key, void *data, size_t size, BL_oval to)
  {
    BL_tray tray = {data:data, size:size, key:key};
    return _bl_post((to), NVM_SAVE_0_BL_tray_0, 0,&tray,0);
  }

//==============================================================================
// syntactic sugar: store value to non volatile memory (NVM)
// - usage: bl_store(id,val)           // post to down gear
//         _bl_store(id,val,(PMI))     // augmented version
//==============================================================================

  static inline int bl_store(int id, int val)
  {
    return bl_msg(bl_down,_NVM,STORE_, id,NULL,val);
  }

  static inline int _bl_store(int id, int val, BL_oval to)
  {
    return _bl_msg((to),_NVM,STORE_, id,NULL,val);
  }

//==============================================================================
// syntactic sugar: recall value from non volatile memory (NVM)
// - usage: val = bl_recall(id)        // recall value from NMV location @ix
//          val = _bl_recall(id,(PMI)) // augmented version
//==============================================================================

  static inline int bl_recall(int id)
  {
    return bl_msg(bl_down, _NVM,RECALL_, id,NULL,0);
  }

  static inline int _bl_recall(int id, BL_oval to)
  {
    return bl_msg((to), _NVM,RECALL_, id,NULL,0);
  }

//==============================================================================
// syntactic sugar: register BLE service in wireless core
// - usage: bl_service(service)      // [SYS:SERVICE (service)] -> bl_down
//          _bl_service(service,to)  // [SYS:SERVICE (service)] -> to
//==============================================================================

  static int inline bl_service(BL_oval service)   // register service
  {
    return bl_msg(bl_down, _SYS,SVC_, 0,(BL_data)service,0);
  }

  static int inline _bl_service(BL_oval service,BL_oval to) // register service
  {
    return _bl_msg(to, _SYS,SVC_, 0,(BL_data)service,0);
  }

//==============================================================================
// syntactic sugar: install lib module in top gear (advise module to register)
// - usage: bl_install(module)      // [SYS:INSTALL] -> bl_top
//==============================================================================

  static int inline bl_install(BL_oval module)   // install module in top gear
  {
    return bl_msg(module, _SYS,INSTALL_, 0,NULL,0);
  }

//==============================================================================
// [MESH:] message definitions
// - [MESH:PRV sts]  update mesh provision status
// - [MESH:ATT sts]  update mesh attention status
//==============================================================================

  #define MESH_PRV_0_0_sts       BL_ID(_MESH,PRV_)
  #define MESH_ATT_0_0_sts       BL_ID(_MESH,ATT_)
  #define GET_ATT_0_0_0          BL_ID(_GET,ATT_)
  #define GET_PRV_0_0_0          BL_ID(_GET,PRV_)
  #define STATE_ATT_0_BL_pint_0  BL_ID(_STATE,ATT_)
  #define STATE_PRV_0_BL_pint_0  BL_ID(_STATE,PRV_)

    // augmented messages

  #define _MESH_PRV_0_0_sts      _BL_ID(_MESH,PRV_)
  #define _MESH_ATT_0_0_sts      _BL_ID(_MESH,ATT_)
  #define _GET_ATT_0_0_0         _BL_ID(_GET,ATT_)
  #define _GET_PRV_0_0_0         _BL_ID(_GET,PRV_)
  #define _STATE_ATT_0_BL_pint_0 _BL_ID(_STATE,ATT_)
  #define _STATE_PRV_0_BL_pint_0 _BL_ID(_STATE,PRV_)

#endif // __BL_SUGAR_H__
