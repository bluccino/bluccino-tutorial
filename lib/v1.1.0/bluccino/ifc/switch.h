//==================================================================================================
// ifc/switch.h
// SWITCH interface class
//
// Created by Hugo Pristauz on 2022-Oct-15
// Copyright © 2022 Bluenetics. All rights reserved.
//==================================================================================================

#ifndef __IFC_SWITCH_H__
#define __IFC_SWITCH_H__

//==============================================================================
// config defaults
//==============================================================================

  #ifndef CFG_LOG_BL_SWITCH
    #define CFG_LOG_BL_SWITCH   0
  #endif

//==================================================================================================
// SWITCH message class definition
//==================================================================================================
//                  +--------------------+
//                  |       SWITCH:      | SWITCH input interface
//            STS --|       @ix,sts      | on/off status update of switch @ix
//                  +--------------------+
//==================================================================================================

  class Switch_Sts : public BlMsg
  {
    public:
      Switch_Sts(int ix, int sts) : BlMsg(_SWITCH,STS_,ix,sts) {}
  };

//==================================================================================================
// BlSwitch class (button interface)
//==================================================================================================

  class BlSwitch : public BlIfc
  {
    public:
      virtual int SWITCH_STS(int ix, int sts) { return BL_VOID2; }

    public:
      BlSwitch() : BlIfc(_SWITCH) {}
      int dispatch(BlMsg &msg)
      {
        switch (msg.op)
        {
          case STS_: return SWITCH_STS(msg.ix, msg.val);
          default: return BL_VOID1;
        }
      }
  };

#endif // __IFC_SWITCH_H__
