//==================================================================================================
// ifc/digital.h
// DIGITAL interface class
//
// Created by Hugo Pristauz on 2022-Oct-15
// Copyright © 2022 Bluenetics. All rights reserved.
//==================================================================================================
// - usage: (C++)
//   BlHwio &io = blHwio;                        // need a module where messages are being posted to
//   int ix = Digital_Input(13,0) >> io;         // standard P0.13 config (button @1)
//   int val = Io_Get(ix) >> io;                 // get digital input value
//
//   int ox = Digital_Output(17,0) >> io;        // standard P0.17 config (LED @1)
//   int val = Io_Set(ox,1) >> io;               // set digital output value to 1
//==================================================================================================

#ifndef __IFC_DIGITAL_H__
#define __IFC_DIGITAL_H__

  #undef SYS_INIT                                // avoid collision with Zephyr SYS_INIT macro

//==============================================================================
// config defaults
//==============================================================================

  #ifndef CFG_LOG_BL_DIGITAL
    #define CFG_LOG_BL_DIGITAL   0
  #endif

//==================================================================================================
// DIGITAL message class definition
//==================================================================================================
//                  +--------------------+
//                  |      DIGITAL:      | DIGITAL interface
//          INPUT --|  @pin,*BlMod,flags | setup pin @ix as a digital input, events go to BlMod
//         OUTPUT --|     @pin,flags     | setup pin @ix as a digital output
//                  +--------------------+
//==================================================================================================

  class Digital_Input : public BlMsg
  {
    public: Digital_Input(int pin, BlMod *cb, int flags) : BlMsg(_DIGITAL,INPUT_,pin,cb,flags) {}
    public: Digital_Input(int pin, int flags) : BlMsg(_DIGITAL,INPUT_,pin,NULL,flags) {}
  };

  class Digital_Output : public BlMsg
  {
    public: Digital_Output(int pin, int flags) : BlMsg(_DIGITAL,OUTPUT_, pin,NULL,flags) {}
  };

//==================================================================================================
// BlDigital class
//==================================================================================================

  class BlDigital : public BlIfc
  {
    public:
      virtual int DIGITAL_INPUT(int pin, BlMod *cb, int flags) { return BL_VOID2; }
      virtual int DIGITAL_OUTPUT(int pin, int flags) { return BL_VOID2; }

    public:
      BlDigital() : BlIfc(_DIGITAL) {}
      int dispatch(BlMsg &msg)
      {
        switch (msg.op)
        {
          case INPUT_: return DIGITAL_INPUT(msg.ix, (BlMod*)msg.data, msg.val);
          case OUTPUT_: return DIGITAL_OUTPUT(msg.ix, msg.val);
          default: return BL_VOID1;
        }
      }
  };

#endif // __IFC_DIGITAL_H__
