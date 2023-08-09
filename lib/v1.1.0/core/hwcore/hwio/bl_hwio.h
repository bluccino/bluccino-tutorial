//==================================================================================================
// bl_hwio.h
// digital/analog IO driver module
//
// Created by Hugo Pristauz on 2022-Oct-05
// Copyright © 2022 Bluenetics. All rights reserved.
//==================================================================================================
// usage: (C)
//   int ix = bl_digin(13,0);                    // standard P0.13 config (button @1)
//   int val = bl_ioget(ix);                     // get digital input value
//
//   int ox = bl_digout(17,0);                   // standard P0.17 config (button @1)
//   int val = bl_ioset(ox,1);                   // set digital output value to 1
//==================================================================================================

#ifndef __BL_IO_H__
#define __BL_IO_H__

#ifdef __cplusplus
  extern "C" {
#endif

  #include "bl_gpio.h"

//==================================================================================================
// IO Logging
//==================================================================================================

#ifndef CFG_LOG_IO
  #define CFG_LOG_IO    0           // IO logging is by default off
#endif

#if (CFG_LOG_IO)
  #define LOG_IO(l,f,...)    BL_LOG(CFG_LOG_IO-1+l,f,##__VA_ARGS__)
  #define LOGO_IO(l,f,o,v)   bl_logo(CFG_LOG_IO-1+l,f,o,v)
#else
  #define LOG_IO(l,f,...)    {}     // empty
  #define LOGO_IO(l,f,o,v)   {}     // empty
#endif

//==================================================================================================
// config defaults
//==================================================================================================

  #ifndef CFG_MAX_GPIO_PINS
    #define CFG_MAX_GPIO_PINS      10       // max 10 pin definitions by default
  #endif

  #ifndef CFG_PORT0_LABEL
    #define CFG_PORT0_LABEL   "GPIO_0"      // used by nRF52832 DK, nRF52840 DK
  #endif

  #ifndef CFG_PORT1_LABEL
    #define CFG_PORT1_LABEL   "GPIO_1"
  #endif

  #ifndef CFG_PORT2_LABEL
    #define CFG_PORT2_LABEL   "GPIO_2"
  #endif

//==================================================================================================
// PMI: bl_hwio
//==================================================================================================

  int bl_hwio(BL_ob *o, int val);

//==================================================================================================
// syntactic sugar: define pin as digital output (message sent to down gear)
// - usage: ix = bl_digout(@pin,flags) // general form (sent to down gear)
//          ix = bl_digout(17,0)       // standard P0.17 output (led @1)
//          ix = bl_digout(109,0)      // pin P1.09 standard output
//          ix = bl_digout(17,GP_ACTIVE_LOW)
//          ix = bl_digout(17,GP_OPEN_DRAIN)
//          ix = bl_digout(17,GP_OPEN_SOURCE|GP_ACTIVE_LOW)
// - usage: ix = _bl_digout(@pin,flags,(to))  // augmented general form
//==================================================================================================

  static inline int bl_digout(int pin, GP_flags flags)
  {
    return bl_msg((bl_down), _DIGITAL,OUTPUT_, pin,NULL,(int)flags);
  }

  static inline int _bl_digout(int pin, GP_flags flags, BL_oval to)
  {
    return _bl_msg((to), _DIGITAL,OUTPUT_, pin,NULL,(int)flags);
  }

//==================================================================================================
// syntactic sugar: define pin as digital input (message sent to down gear)
// - usage: ix = bl_digin(@pin,flags)  // general form (sent to down gear)
//          ix = bl_digin(13,0)        // standard P0.13 input (button@1)
//          ix = bl_digin(105,0)       // pin P1.05 standard input
//          ix = bl_digin(13,GP_ACTIVE_LOW)
//          ix = bl_digin(13,GP_PULL_UP)
//          ix = bl_digin(13,GP_PULL_UP|GP_ACTIVE_LOW)
// - usage: ix = _bl_digin(@pin,flags,(to))  // augmented general form
//==================================================================================================

  static inline int bl_digin(int pin, GP_flags flags)
  {
    return bl_msg((bl_down), _DIGITAL,INPUT_, pin,NULL,(int)flags);
  }

  static inline int _bl_digin(int pin, GP_flags flags, BL_oval to)
  {
    return _bl_msg((to), _DIGITAL,INPUT_, pin,NULL,(int)flags);
  }

//==================================================================================================
// syntactic sugar: get value of pin (message sent to down gear)
// - usage: val = bl_ioget(pin)        // get pin value (digital or analog)
// -        val = _bl_ioget(pin,(PMI)) // get pin value (digital or analog)
//==================================================================================================

  static inline int bl_ioget(int pin)
  {
    return bl_msg((bl_down), _IO,GET_, pin,NULL,0);
  }

  static inline int _bl_ioget(int pin, BL_oval to)
  {
    return _bl_msg((to), _IO,GET_, pin,NULL,0);
  }

//==================================================================================================
// syntactic sugar: set value of pin (message sent to down gear)
// - usage: val = bl_ioset(ix,val)        // set pin value (dig. or analog)
// -        val = _bl_ioset(ix,val,(PMI)) // set pin value (dig. or analog)
//==================================================================================================

  static inline int bl_ioset(int pin, int val)
  {
    return bl_msg((bl_down), _IO,SET_, pin,NULL,val);
  }

  static inline int _bl_ioset(int pin, int val, BL_oval to)
  {
    return _bl_msg((to), _IO,SET_, pin,NULL,val);
  }

//==================================================================================================
// syntactic sugar: attach ISR (interrupt service routine) to input pin
// - usage: bl_attach(ix,isr,flags)                // attach isr to pin @ix
// -        bl_attach2(ix,isr,&module,flags,(to))  // attach (isr,module) to pin @ix
// - there are two function pointers coming in the BL_data args[2] field
// - args[0]: (void *)isr => cast to BL_oval isr
// - args[1]: (void *)module => store to BL_data module
// - interrupt will send [IO:ISR ix,&module,val] to isr() 
//==================================================================================================

  static inline int bl_attach(int ix,BL_oval isr,GP_flags flags)
  {
    BL_data args[2] = {(BL_data)isr,(BL_data)NULL};
    return bl_msg((bl_down), _IO,ATTACH_, ix,(BL_data)args,(int)flags);
  }

  static inline int bl_attach2(int ix,BL_oval isr,void *module,GP_flags flags)
  {
	  BL_data args[2] = {(BL_data)isr,(BL_data)module};
	  return bl_msg((bl_down), _IO,ATTACH_, ix,(BL_data)args,(int)flags);
  }

//==================================================================================================
// public C++ module interface for BlHwio
//==================================================================================================
// - usage: (C++)
//   BlHwio &io = blHwio;                        // need a module where messages are being posted to
//   int ix = Digital_Input(13,0) >> io;         // standard P0.13 config (button @1)
//   int val = Io_Get(ix) >> io;                 // get digital input value
//
//   int ox = Digital_Output(17,0) >> io;        // standard P0.17 config (LED @1)
//   int val = Io_Set(ox,1) >> io;               // set digital output value to 1
//==================================================================================================
#ifdef __cplusplus

  #include "ifc/sys.h"
  #include "ifc/digital.h"
  #include "ifc/io.h"

  class BlHwio : public BlMod, BlSys, BlDigital, BlIo
  {                                    // (D) := bl_down;
    public:                            //                +--------------------+
      BlHwio(BL_txt t) : BlMod(t) {}   //                |       BlHwio       |
    private:                           //                +--------------------+
      BlSys &SYS = *this;              //                |        SYS:        | SYS input interface
      In SYS_INIT(BlMod &module);      // (D)->   INIT ->|      (module)      | system init
    private:                           //                +--------------------+
      BlDigital &DIGITAL = *this;      //                |      DIGITAL:      | DIGITAL input ifc.
      In DIGITAL_INPUT(int pin,        // (D)->  INPUT ->| @pin,<BlMod>,flags | use I/O pin as
        BlMod *cb, int flags);         //                |                    |   digital input
      In DIGITAL_OUTPUT(int pin,       // (D)-> OUTPUT ->|     @pin,flags     | use I/O pin as
        int flags);                    //                |                    |   digital output
    private:                           //                +--------------------+
      BlIo &IO = *this;                //                |         IO:        | IO input interface
      In IO_GET(int ix);               // (D)->    GET ->|         ix         | get I/O value
      In IO_SET(int ix, int val);      // (D)->    SET ->|       ix,val       | set I/O value
      In IO_DEVICE(int ix,GP_dev **d); // (D)-> DEVICE ->|     @ix,&<GP_dev>  | retrieve I/O device
      In IO_ATTACH(int ix,BlMod &isr,  // (D)-> ATTACH --|   @ix,(isr),flags  | attach interrupt
                   int flags);         //                |                    |
                                       //                +--------------------+
	  public:
		  In input(BlMsg &msg)
      {
        switch (msg.cl)
        {
          case _SYS:     return msg >> SYS;
          case _DIGITAL: return msg >> DIGITAL;
          case _IO:      return msg >> IO;
          default: return BL_VOID;
        }
      }
  };

  extern BlHwio blHwio;

#endif // __cplusplus
//==============================================================================
// footer
//==============================================================================

#ifdef __cplusplus
  }
#endif

#endif // __BL_IO_H__
