//==============================================================================
// bl_hwio.cpp
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
// C-include of bl_hwio.c
//==============================================================================

extern "C"
{
  #include "bl_hwio.c"
}

//==================================================================================================
// BlHwio handler methods: SYS message class
//==================================================================================================

	int BlHwio::SYS_INIT(BlMod &module)
  {
    return bl_msg((bl_hwio), _SYS,INIT_, 0,NULL,0);
  }

//==================================================================================================
// BlHwio handler methods: IO message class
//==================================================================================================

	int BlHwio::IO_GET(int ix)
  {
    return bl_msg((bl_hwio), _IO,GET_, ix,NULL,0);
  }

	int BlHwio::IO_SET(int ix, int val)
  {
    return bl_msg((bl_hwio), _IO,SET_, ix,NULL,val);
  }

	int BlHwio::IO_DEVICE(int ix, GP_dev **pdev)
  {
    return bl_msg((bl_hwio), _IO,DEVICE_, ix,pdev,0);
  }






  static int launch(BL_ob *o, int val)
  {
bl_log(1,BL_R"launch");
return 0;
    BlMod *module = (BlMod*)bl_data(o);
    BlMsg msg(o,val);
    module->input(msg);
  }

	int BlHwio::IO_ATTACH(int ix, BlMod &module, int flags)
  {
bl_log(1,BL_Y"setup launch");
   ix = bl_digin(13, BL_ACTLOW|BL_PULLUP);    // button input
   bl_attach(ix,launch,BL_EDGEBOTH|BL_DEBOUNCE);          // attach interrupt / ISR
return 0;

	  BL_data args[2] = {(BL_data)launch,(BL_data)(&module)};
	  return bl_msg((bl_hwio), _IO,ATTACH_, ix,(BL_data)args,(int)flags);
  }

//==================================================================================================
// BlHwio handler methods: DIGITAL message class
//==================================================================================================

  static BlMod *callbacks[20];

  static int oval_cb(BL_ob *o, int val)
  {
    int ix = bl_ix(o);
    bl_log(4,"[GPIO oval_cb @%d,%d]", ix,val);

    if (ix < 0 || ix >= (int)BL_LEN(callbacks) || !callbacks[ix])
      return bl_err(-1,"cannot access callback");

    BlMod cb = *(callbacks[ix]);
    return BlMsg(o,val) >> cb;
  }

	int BlHwio::DIGITAL_INPUT(int ix, BlMod *cb, int flags)
  {
    if (ix < 0 || ix >= (int)BL_LEN(callbacks))
      return bl_err(-1,"out of callback table entries");

//  bl_log(4,"[DIGITAL:INPUT @%d,(cb),%08X]", ix,flags);
    return bl_msg((bl_hwio), _DIGITAL,INPUT_, ix, (BL_data)oval_cb, flags);
  }

	int BlHwio::DIGITAL_OUTPUT(int ix, int flags)
  {
    return bl_msg((bl_hwio), _DIGITAL,OUTPUT_, ix,NULL,flags);
  }

//==================================================================================================
// create a BlHwio instance
//==================================================================================================

  BlHwio blHwio("hwio");                         // instance of class BlHwio
