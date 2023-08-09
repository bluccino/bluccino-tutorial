//==============================================================================
// bl_hwio.c
// digital/analog IO driver module
//
// Created by Hugo Pristauz on 2022-Oct-05
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================
// simplified access for nRF52832 DKs, nRF52840 DK and nRF52840 dongle
//==============================================================================

  #include "bluccino.h"
  #include "bl_hwio.h"

//==============================================================================
// logging shorthands & PMI definition
//==============================================================================

  #define WHO "bl_hwio"           // who is logging

  #define LOG                     LOG_IO
  #define LOGO(lvl,col,o,val)     LOGO_IO(lvl,col WHO ":",o,val)

  #define PMI bl_hwio
  BL_PMI(PMI)

//==============================================================================
// data structure for enhanced pin configuration
//==============================================================================

  typedef struct IO_cfg
          {
            BL_pin pin;
            GP_flags flags;                 // extension of .pin.io.dt_flags
            int val;                        // copy of output value (for read back)
            BL_oval isr;                    // "C" ISR
            BL_data module;                 // "C++" module callback
            BL_ob oo;
          } IO_cfg;

  static IO_cfg config[CFG_MAX_GPIO_PINS];  // pin configuration table
  static int count = -1;                    //  current pin count

//==============================================================================
// helper: reset I/O configuration table
// - usage: cnt = reset(false)    // cond. resets GPIO pin table (if never init)
//          cnt = reset(true)     // always reset (forced reset)
//==============================================================================

  static int reset(bool force)
  {
    if (force || count < 0)       // count < 0 means: module not initialized
    {
      LOG(5,"reset I/O configuration table");

      for (int i=0; i < BL_LEN(config); i++)
      {
        IO_cfg *cfg = config + i;
        cfg->pin.io.port = NULL;
        cfg->isr = NULL;
        cfg->module = NULL;
        cfg->val = cfg->flags = 0;
      }
      count = 0;
    }
    return count;
  }

//==============================================================================
// ISR: provide pin ISR callback
// - use `ctx` pointer to reconstruct config entry index `ix`
// - from `ix` construct pointer `cfg` to config entry
// - if ISR has been provided then call
//==============================================================================

  static inline IO_cfg *resolve(GP_ctx *ctx)
  {
    int ix = (IO_cfg*)ctx - (IO_cfg*)(&config[0].pin.ctx);
    return (config + ix);
  }

  static void isr_callback(GP_dev *dev, GP_ctx *ctx, GP_pins pins_)
  {
    IO_cfg *cfg = resolve(ctx);        // resolve pointer to config table
    bool val = bl_pin_get(&cfg->pin);  // I/O value

    if (cfg->isr)
      cfg->isr(&cfg->oo,val);          // emit event message (ISR)
    else
      _bl_pmi(_IO,ISR_, (cfg-config),cfg->module,val);  // ISR to submit work
  }

//==============================================================================
// helper: pin setup  // return ix>=0: OK, ix < 0: error
// - usage: ix = pin_mode(o,val,BL_DIGOUT);  setup input
//          ix = pin_mode(o,val,BL_DIGIN);   setup output
//==============================================================================

  static int pin_mode(BL_ob *o, int val, GP_flags flags)
  {
    reset(false);                    // conditional reset I/O configuration table

    int pin = bl_ix(o);                // pin number

    if (count >= BL_LEN(config))
      return bl_err(-1,"out of GPIO entries");  // increase CFG_MAX_GPIO_PINS

    if (pin < 0)
      return bl_err(-2,"pin index must be >= 0");

      // find proper port label depending on pin number

    BL_txt label = "???";

    if (pin < 100) label = CFG_PORT0_LABEL;
      else if (pin < 200) label = CFG_PORT1_LABEL;
      else if (pin < 300) label = CFG_PORT2_LABEL;
      else bl_err(-3,"pin number must be less than 300");

      // resolve device binding for port device

    GP_dev *port = device_get_binding(label);
    if (port == NULL)
    {
      bl_log(1,BL_R "error -4: cannot access port: %s",label);
      return -4;
    }

      // input data is good, so we can start pin config ...

    int ix = count++;                  // pin instance index
    IO_cfg *cfg = config + ix;         // pointer to config entry
    GP_io *io = &cfg->pin.io;          // pointing to GP_io field of table
    cfg->flags = (GP_flags)val|flags;  // store flags in BL_pin data structure

      // finally setup GP_io structure

    io->port = port;                   // copy device pointer
    io->pin = pin % 100;               // truncate pin number to 0..99
    io->dt_flags = BL_LO(cfg->flags);  // store lower byte of flags in dt_flags
    cfg->val = 0;                      // init read back value

    return ix;
  }

//==============================================================================
// helper: get pin instance index
// - usage: ix = pin_ix(o) // ix < 0: error
//==============================================================================

  static int pin_ix(BL_ob *o)
  {
    int ix = bl_ix(o);                // pin instance index

    if (ix < 0 || ix >= count)
      return bl_err(-1,"bad pin instance index");
    else
      return ix;
  }

//==============================================================================
// handler: [DIGITAL:OUTPUT @pin,flags]  // config pin as digital output
// - usage: ix = bl_msg((bl_gpio), _DIGITAL,OUTPUT_, 17,cb,flags);
// - @ix: pin number (LED@1: @17:P0.17, LED@2:P0.18, LED@3:P0.19, LED@4:P0.20)
// - flags:
// - return @ix (instance index)
//==============================================================================

  static int digital_output(BL_ob *o, int val)
  {
    int ix = pin_mode(o,val,BL_DIGOUT);           // pin instance index
    if (ix >= 0)
    {
      IO_cfg *cfg = config + ix;
 	    GP_io *io = &cfg->pin.io;

      LOG(5,"configure pin @%d.%d as output (flags 0x%04X)",
        bl_ix(o)/100, io->pin, cfg->flags);
      gpio_pin_configure(io->port, io->pin, cfg->flags);

      if (cfg->flags & BL_INIT_HIGH)
        cfg->val = 1;
    }

    return ix;
  }

//==============================================================================
// handler: [DIGITAL:INPUT @pin,(cb),flags] // config pin as digital input
// - usage: ix = bl_msg((bl_gpio), _DIGITAL,INPUT_, 24,cb,flags);
//          ix = bl_msg((bl_gpio), _DIGITAL,INPUT_, 24,NULL,0);
//          ix = bl_msg((bl_gpio), _DIGITAL,INPUT_, 24,cb,GP_DEBOUNCE);
// - @ix: pin number (@24:P0.24, @113:P1.13, @205:P2.05)
// - (cb): callback (BL_oval type)
// - flags:
// - return @ix (instance index)
//==============================================================================

  static int digital_input(BL_ob *o, int val)
  {
    int ix = pin_mode(o,val,BL_DIGIN);      // pin instance index
    if (ix >= 0)
    {
      IO_cfg *cfg = config + ix;
 	    GP_io *io = &cfg->pin.io;

      LOG(5,"configure pin @%d.%d as input (flags 0x%04X)",
        bl_ix(o)/100, io->pin, cfg->flags);

      gpio_pin_configure(io->port, io->pin, cfg->flags);
    }
    return ix;
  }

//==============================================================================
// handler: [IO:GET @ix] // get digital input or read analog value from pin
// - for digital inputs: return value is 0/1
// - for analog inputs: return value in mV
//==============================================================================

  static int io_get(BL_ob *o, int val)
  {
    int ix = pin_ix(o);                 // pin instance index
    if (ix < 0) return ix;              // return error code

    IO_cfg *cfg = config + ix;
    GP_io *io = &cfg->pin.io;

    if (cfg->flags & BL_DIGOUT)
      val = cfg->val;                  // read back value
    else
      val = gpio_pin_get(io->port,io->pin);

    LOG(5, BL_C "bl_hwio << [IO:GET @%d] = %d",io->pin,val);
    return val;
  }

//==============================================================================
// handler: [IO:SET @ix,val] // get digital input or read analog value from pin
// - for digital inputs: return value is 0/1
// - for analog inputs: return value in mV
//==============================================================================

  static int io_set(BL_ob *o, int val)
  {
    LOG(5,"bl_hwio << [IO:SET @%d,%d]",bl_ix(o),val);

    int ix = pin_ix(o);                // pin instance index
    if (ix < 0) return ix;

    IO_cfg *cfg = config + ix;
    GP_io *io = &cfg->pin.io;

    if (cfg->flags & BL_DIGOUT)
      cfg->val = val;                  // return read back value

    gpio_pin_set(io->port,io->pin,val);
    return 0;
  }

//==============================================================================
// handler: [IO:DEVICE @ix,&<GP_dev>]  // retrieve I/O device pointer
//==============================================================================

  static int io_device(BL_ob *o, int val)
  {
    GP_dev **pdev = (GP_dev**)bl_data(o);
    int ix = pin_ix(o);                // pin instance index
    if (ix < 0) return ix;

    IO_cfg *cfg = config + ix;
    *pdev = cfg->pin.io.port;          // return I/O port device pointer

    return 0;
  }

//==============================================================================
// handler: [IO:ATTACH @ix,(isr),flags] // attach interrupt service routine
// - there are two function pointers coming in the BL_data args[2] field
// - args[0]: (void *)isr => cast to BL_oval isr
// - args[1]: (void *)module => store to BL_data module
//==============================================================================

  static int io_attach(BL_ob *o, int val)
  {
    BL_data *args = (BL_data*)bl_data(o);
    BL_oval isr = (BL_oval)args[0];     // "C" ISR
    BL_data module = args[1];           // "C++"" module callback
    int ix = pin_ix(o);                 // pin instance index

    if (ix < 0) return ix;

    IO_cfg *cfg = config + ix;
    LOG(5,"attach ISR to pin @%d", bl_ix(o));

    int err = bl_pin_attach(&cfg->pin, (GP_flags)val, isr_callback);
    bl_err(err,"cannot attach interrupt to pin");

      // setup a message object for [IO:ISR @ix,(port),val] event ...
      // and copy this object into the config table

    BL_ob oo = BL_OB(_IO,ISR_,ix,cfg->pin.io.port);
    cfg->oo = oo;                      // copy messaging object
    cfg->isr = isr;                    // store provided ISR function
    cfg->module = module;              // store C++module callback

    return err;
  }

//==============================================================================
// handler: [SYS:INIT (cb))] // module init
// - resets GPIO pin table
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    return reset(true);                // forced reset of I(O config table)
  }

//==============================================================================
//
// (H) := bl_hw;  (U) := bl_up
//
//                  +--------------------+
//                  |      bl_hwio       |
//                  +--------------------+
//                  |        SYS:        |
// (D)->     INIT ->|        (cb)        | system init
//                  +--------------------+
//                  |      DIGITAL:      | DIGITAL input interface
// (D)->    INPUT ->|   @pin,(cb),flags  | setup pin @ix as a digital input
// (D)->   OUTPUT ->|     @pin,flags     | setup pin @ix as a digital output
//                  +--------------------+
//                  |      ANALOG:       | ANALOG input interface
// (D)->    INPUT ->|   @ix,(cb),flags   | setup pin @ix as an analog input
// (D)->   OUTPUT ->|   @ix,(cb),flags   | setup pin @ix as an analog output
//                  +--------------------+
//                  |         IO:        | IO input interface
// (D)->      GET ->|         @ix        | digital or analog get
// (D)->      SET ->|      @ix,val       | digital or analog set
// (D)->   DEVICE ->|     @ix,&<GP_dev>  | retrieve I/O device pointer
// (D)->   ATTACH ->|  @ix,&(isr),flags  | attach interrupt service routine
//                  |....................|
//                  |         IO:        | IO output interface
//            ISR --|       @ix,sts      | offload from ISR by submitting work
// (U)<-      STS <-|       @ix,sts      | notify pin level change
//                  +--------------------+
//
//==============================================================================

  int bl_hwio(BL_ob *o, int val)
  {
    static BL_work work = BL_WORK(PMI);   // worker calls PMI
    static BL_ob oo = BL_OB(BL_AUG(_IO),STS_, 0,NULL);
    static BL_oval U = NULL;              // most likely up gear

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        LOG(4,BL_B "init bl_hwio");
        U = bl_cb(o,(U),WHO"(U)");     // store callback
        return sys_init(o,val);

      case BL_ID(_DIGITAL,INPUT_):  return digital_input(o,val);
      case BL_ID(_DIGITAL,OUTPUT_): return digital_output(o,val);
      case BL_ID(_IO,GET_):         return io_get(o,val);
      case BL_ID(_IO,SET_):         return io_set(o,val);
      case BL_ID(_IO,DEVICE_):      return io_device(o,val);
      case BL_ID(_IO,ATTACH_):      return io_attach(o,val);

      case _BL_ID(_IO,STS_):        return bl_out(o,val,(U));
      case _BL_ID(_IO,ISR_):
        bl_set_ix(&oo,bl_ix(o));      // update current I/O index
        return bl_submit(&work,&oo,val);
      default:                      return BL_VOID;
    }
  }

//==================================================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==================================================================================================

  #include "bl_clean.h"
