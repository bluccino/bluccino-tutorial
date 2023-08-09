//==================================================================================================
// ifc/led.h
// LED interface class
//
// Created by Hugo Pristauz on 2022-Oct-15
// Copyright © 2022 Bluenetics. All rights reserved.
//==================================================================================================

#ifndef __IFC_LED_H__
#define __IFC_LED_H__

//==============================================================================
// config defaults
//==============================================================================

  #ifndef CFG_LOG_BL_LED
    #define CFG_LOG_BL_LED   0
  #endif

//==================================================================================================
// SYS message class definition
//==================================================================================================
//                  +--------------------+
//                  |        LED:        | LED input interface
//            SET --|       @ix,on       | set LED @ix on/off (i=0..4)
//         TOGGLE --|        @ix         | toggle LED @ix (i=0..4)
//                  +--------------------+
//==================================================================================================

  class Led_Set : public BlMsg
  {
    public:
      Led_Set(int ix, int on) : BlMsg(_LED,SET_,ix,on) {}
  };

  class Led_Toggle : public BlMsg
  {
    public:
      Led_Toggle(int ix) : BlMsg(_LED,TOGGLE_,ix) {}
  };

//==================================================================================================
// BlLed class
//==================================================================================================

  class BlLed : public BlIfc
  {
    public:
      virtual int LED_SET(int ix, int on) { return BL_VOID2; }
      virtual int LED_TOGGLE(int ix) { return BL_VOID2; }

    public:
      BlLed() : BlIfc(_LED) {}
      int dispatch(BlMsg &msg)
      {
        switch (msg.op)
        {
          case SET_: return LED_SET(msg.ix,msg.val);
          case TOGGLE_: return LED_TOGGLE(msg.ix);
          default: return BL_VOID1;
        }
      }
  };

#endif // __IFC_LED_H__
