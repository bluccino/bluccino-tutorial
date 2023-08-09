//==================================================================================================
// ifc/sys.h
// SYS interface class
//
// Created by Hugo Pristauz on 2022-Oct-15
// Copyright © 2022 Bluenetics. All rights reserved.
//==================================================================================================

#ifndef __IFC_SYS_H__
#define __IFC_SYS_H__

//==============================================================================
// config defaults
//==============================================================================

  #ifndef CFG_LOG_BL_SYS
    #define CFG_LOG_BL_SYS   0
  #endif

//==================================================================================================
// SYS message class definition
//==================================================================================================
//                  +--------------------+
//                  |        SYS:        | SYS input interface
//           INIT --|        (cb)        | system init, store callback
//           TICK --| @ix,<BL_pace>,cnt  | system tick
//           TOCK --| @ix,<BL_pace>,cnt  | system tock
//                  +--------------------+
//==================================================================================================

  class Sys_Init : public BlMsg
  {
    public:
      Sys_Init() : BlMsg(_SYS,INIT_,0,NULL,0) {}
      Sys_Init(BlMod &out) : BlMsg(_SYS,INIT_,0,&out,0) {}
  };

  class Sys_Tick : public BlMsg
  {
    public:
      Sys_Tick(BL_pace *pace) : BlMsg(_SYS,TICK_,0,pace,0) {}
      Sys_Tick(int ix,BL_pace *pace, int cnt) : BlMsg(_SYS,TICK_,ix,pace,cnt) {}
  };

  class Sys_Tock : public BlMsg
  {
    public:
      Sys_Tock(BL_pace *pace) : BlMsg(_SYS,TOCK_,0,pace,0) {}
      Sys_Tock(int ix,BL_pace *pace, int cnt) : BlMsg(_SYS,TOCK_,ix,pace,cnt) {}
  };

//==================================================================================================
// BlSys class
//==================================================================================================

  #undef SYS_INIT                            // avoid collision with Zephyr SYS_INIT macro

  class BlSys : public BlIfc
  {
    public:
      virtual int SYS_INIT(BlMod &module) { return BL_VOID2; }
      virtual int SYS_TICK(int id, BL_pace *data, int cnt) { return BL_VOID2; }
      virtual int SYS_TOCK(int id, BL_pace *data, int cnt) { return BL_VOID2; }

    public:
      BlSys() : BlIfc(_SYS) {}
      int dispatch(BlMsg &msg)
      {
        switch (msg.op)
        {
          case INIT_: return SYS_INIT(*((BlMod*)msg.data));
          case TICK_: return SYS_TICK(msg.ix, (BL_pace *)msg.data, msg.val);
          case TOCK_: return SYS_TOCK(msg.ix, (BL_pace *)msg.data, msg.val);
          default: return BL_VOID1;
        }
      }
  };

#endif // __IFC_SYS_H__
