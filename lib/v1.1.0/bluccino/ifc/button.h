//==================================================================================================
// ifc/button.h
// BUTTON interface class
//
// Created by Hugo Pristauz on 2022-Oct-15
// Copyright © 2022 Bluenetics. All rights reserved.
//==================================================================================================

#ifndef __IFC_BUTTON_H__
#define __IFC_BUTTON_H__

//==============================================================================
// config defaults
//==============================================================================

  #ifndef CFG_LOG_BL_BUTTON
    #define CFG_LOG_BL_BUTTON   0
  #endif

//==================================================================================================
// BUTTON message class definition
//==================================================================================================
//                  +--------------------+
//                  |      BUTTON:       | BUTTON input interface
//          PRESS --|        @ix         | button @ix pressed (rising edge)
//        RELEASE --|       @ix,ms       | button @ix released after ms-time
//          CLICK --|       @ix,cnt      | button @ix clicked (cnt: number of clicks)
//           HOLD --|       @ix,ms       | button @ix held (ms: hold ms-time)
//            CFG --|        mask        | config button module
//             MS --|         ms         | set click/hold discrimination time
//                  +--------------------+
//==================================================================================================

  class Button_Press : public BlMsg
  {
    public:
      Button_Press(int ix) : BlMsg(_BUTTON,PRESS_, ix,0,0) {}
  };

  class Button_Release : public BlMsg
  {
    public:
      Button_Release(int ix, int ms) : BlMsg(_BUTTON,RELEASE_, ix,0,ms) {}
  };

  class Button_Click : public BlMsg
  {
    public:
      Button_Click(int ix, int cnt) : BlMsg(_BUTTON,CLICK_, ix,0,cnt) {}
  };

  class Button_Hold : public BlMsg
  {
    public:
      Button_Hold(int ix, int ms) : BlMsg(_BUTTON,HOLD_, ix,0,ms) {}
  };

  class Button_Cfg : public BlMsg
  {
    public:
      Button_Cfg(int mask) : BlMsg(_BUTTON,CFG_, 0,0,mask) {}
  };

  class Button_Ms : public BlMsg
  {
    public:
      Button_Ms(int ms) : BlMsg(_BUTTON,MS_, 0,0,ms) {}
  };

//==================================================================================================
// BlButton class
//==================================================================================================

  class BlButton : public BlIfc
  {
    public:
      virtual int BUTTON_PRESS(int ix) { return BL_VOID2; }
      virtual int BUTTON_RELEASE(int ix,int ms) { return BL_VOID2; }
      virtual int BUTTON_CLICK(int ix,int cnt) { return BL_VOID2; }
      virtual int BUTTON_HOLD(int ix,int ms) { return BL_VOID2; }
      virtual int BUTTON_CFG(int mask) { return BL_VOID2; }
      virtual int BUTTON_MS(int ms) { return BL_VOID2; }

    public:
      BlButton() : BlIfc(_BUTTON) {}
      int dispatch(BlMsg &msg)
      {
        switch (msg.op)
        {
          case PRESS_:   return BUTTON_PRESS(msg.ix);
          case RELEASE_: return BUTTON_RELEASE(msg.ix,msg.val);
          case CLICK_:   return BUTTON_CLICK(msg.ix,msg.val);
          case HOLD_:    return BUTTON_HOLD(msg.ix,msg.val);
          case CFG_:     return BUTTON_CFG(msg.val);
          case MS_:      return BUTTON_MS(msg.val);
          default: return BL_VOID1;
        }
      }
  };

#endif // __IFC_BUTTON_H__
