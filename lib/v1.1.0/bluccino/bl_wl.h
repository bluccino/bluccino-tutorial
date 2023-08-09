//==============================================================================
// bl_hw.h
// Bluccino wireless core supporting mesh model access, reset logic and NVM
//
// Created by Hugo Pristauz on 2022-Feb-18
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

#ifndef __BL_WL_H__
#define __BL_WL_H__

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
// (O)<-      PRV <-|       onoff        | provision on/off
// (O)<-      ATT <-|       onoff        | attention on/off
//                  |....................|
//                  |       MESH:        | MESH lower interface
// (B)->      PRV ->|       onoff        | provision on/off
// (B)->      ATT ->|       onoff        | attention on/off
//                  +--------------------+
//                  |       RESET:       | RESET: public interface
// (O)<-      DUE <-|                    | reset timer is due
// (!)->      INC ->|         ms         | inc reset counter & set due timer
// (!)->      PRV ->|                    | unprovision node
//                  +--------------------+
//                  |        NVM:        | NVM: upper interface
// (O)<-    READY <-|       ready        | notification that NVM is now ready
// (!)->    STORE ->|      @ix,val       | store value in NVM at location @ix
// (!)->   RECALL ->|        @ix         | recall value in NVM at location @ix
// (!)->     SAVE ->|                    | save NVM cache to NVM
//                  |....................|
//                  |        NVM:        | NVM: lower interface
// (N)->    READY ->|       ready        | notification that NVM is now ready
// (N)<-    STORE <-|      @ix,val       | store value in NVM at location @ix
// (N)<-   RECALL <-|        @ix         | recall value in NVM at location @ix
// (N)<-     SAVE <-|                    | save NVM cache to NVM
//                  +====================+
//                  |       #SET:        | SET: private interface
// (#)->      PRV ->|       onoff        | provision on/off
// (#)->      ATT ->|       onoff        | attention on/off
//                  +--------------------+
//                  |      #RESET:       | RESET: private interface
// (#)->      DUE ->|                    | reset timer is due
//                  +--------------------+
//
//==============================================================================

  int bl_wl(BL_ob *o, int val);        // HW core module interface

//==============================================================================
// syntactic sugar: store value to non volatile memory (NVM)
// - usage: _NVM_STORE_(id,val,(bl_down)) // store value at NVM location @ix
//==============================================================================

  static inline int _NVM_STORE_(int id, int val, BL_oval to)
  {
    return bl_msg((to), _NVM,STORE_, id,NULL,val);
  }

//==============================================================================
// syntactic sugar: recall value from non volatile memory (NVM)
// - usage: val = _NVM_RECALL_(id,(bl_down)) // recall val from NMV loc. @ix
//==============================================================================

  static inline int _NVM_RECALL_(int id, BL_oval to)
  {
    return bl_msg((to), _NVM,RECALL_, id,NULL,0);
  }

#endif // __BL_HW_H__
