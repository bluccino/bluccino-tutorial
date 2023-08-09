//==================================================================================================
// ifc/io.h
// IO interface class
//
// Created by Hugo Pristauz on 2022-Oct-15
// Copyright © 2022 Bluenetics. All rights reserved.
//==================================================================================================
// - usage: BlHwio &io = blHwio;                 // need a module where messages are being posted to
//          int ix = Digital_Input(13,0) >> io;  // standard P0.13 config (button @1)
//          int val = Io_Get(ix) >> io;          // get digital input value
//
//          int ox = Digital_Output(17,0) >> io; // standard P0.17 config (LED @1)
//          int val = Io_Set(ox,1) >> io;        // set digital output value to 1
//==================================================================================================

#ifndef __IFC_IO_H__
#define __IFC_IO_H__

  extern "C"
  {
    #include "bl_gpio.h"
  }

//==============================================================================
// config defaults
//==============================================================================

  #ifndef CFG_LOG_BL_IO
    #define CFG_LOG_BL_IO   0
  #endif

//==================================================================================================
// IO message class definition
//==================================================================================================
//                  +--------------------+
//                  |         IO:        | IO input interface
//            GET --|         ix         | get I/O value
//            SET --|       ix,val       | set I/O value
//         DEVICE --|     @ix,&<GP_dev>  | retrieve I/O device pointer
//         ATTACH --|   @ix,(isr),flags  | attach interrupt service routine
//                  +--------------------+
//==================================================================================================

  class Io_Get : public BlMsg
  {
    public: Io_Get(int ix) : BlMsg(_IO,GET_,ix,NULL,0) {}
  };

  class Io_Set : public BlMsg
  {
    public: Io_Set(int ix, int val) : BlMsg(_IO,SET_,ix,NULL,val) {}
  };

  class Io_Device : public BlMsg
  {
    public: Io_Device(int ix, GP_dev **dev) : BlMsg(_IO,DEVICE_,ix,dev,0) {}
  };

  class Io_Attach : public BlMsg
  {
    public: Io_Attach(int ix, BlMod &isr, int flags) : BlMsg(_IO,ATTACH_, ix,&isr,flags) {}
  };

//==================================================================================================
// BlIo class
//==================================================================================================

  class BlIo : public BlIfc
  {
    public:
      virtual int IO_GET(int ix) { return BL_VOID2; }
      virtual int IO_SET(int ix, int val) { return BL_VOID2; }
      virtual int IO_DEVICE(int ix, GP_dev **dev) { return BL_VOID2; }
      virtual int IO_ATTACH(int ix, BlMod &isr, int flags) { return BL_VOID2; }

    public:
      BlIo() : BlIfc(_IO) {}
      int dispatch(BlMsg &msg)
      {
        switch (msg.op)
        {
          case GET_: return IO_GET(msg.ix);
          case SET_: return IO_SET(msg.ix, msg.val);
          case DEVICE_: return IO_DEVICE(msg.ix, (GP_dev**)msg.data);
          case ATTACH_: return IO_ATTACH(msg.ix, *((BlMod*)msg.data), msg.val);
          default: return BL_VOID1;
        }
      }
  };

#endif // __IFC_IO_H__
