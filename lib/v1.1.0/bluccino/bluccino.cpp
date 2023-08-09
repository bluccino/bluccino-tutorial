//==============================================================================
//  bluccino.cpp
//  bluccino class implementation
//
//  Created by Hugo Pristauz on 2021-11-21
//  Copyright © 2021 Bluenetics GmbH. All rights reserved.
//==============================================================================

  #include "bluccino.h"

//==============================================================================
// include Bluccino .c files (these are a lot ...)
//==============================================================================

  extern "C"
  {
    #include "bluccino.c"
  }

//==============================================================================
// method: Bluccino hello
//==============================================================================

  void Bluccino::hello(int verbose, BL_txt msg)
  {
    bl_hello(verbose,msg);
  }

//==============================================================================
// method: Bluccino init with callback object (BlMod) reference
// - usage: App app("app");
//          init(app);  // app is a module wgich catches all up events
//==============================================================================

  static BlMod *pwhen = NULL;

  static int when(BL_ob *o, int val)
  {
    BlMsg msg(o,val);

//  if (msg.op != TICK_) msg.logo(1,BL_R "callback");

    if (pwhen) return msg >> pwhen;
      else return -1;
  }

  int Bluccino::init(BlMod &cb)
  {
    pwhen = &cb;
    return bl_init(bl_gear,when);
  }

//==============================================================================
// method: Bluccino init
//==============================================================================

  int Bluccino::init()                 // init gear, no notifications
  {
    return bl_init(bl_gear, NULL);
  }

  int Bluccino::init(BL_oval module)   // init C-module, no notifications
  {
    return bl_init(module, NULL);
  }

  int Bluccino::init(BL_oval module,BL_oval cb)  // init C-module with notify cb
  {
    return bl_init(module, cb);
  }

  void Bluccino::sleep(BL_ms ms)
  {
    return bl_sleep(ms);
  }

//==============================================================================
// method: Bluccino engine
// - we set a pointer (pmodule) to the module being passed by ::engine()
// - then we call bl_engine(&proxy,tick_ms,tock_ms) with &proxy being the
//   pointer to an Oval module (acting as a proxy)
// - when a callback is invoked by lower layers, it is forwarded to the
//   proxy module, which finally forwards the event (via `forward`) to the app
//==============================================================================

  static BlMod *forward = NULL;

  static int proxy(BL_ob *o, int val)
  {
    BlMsg msg(o,val);

//  if (msg.op != TICK_) msg.logo(1,BL_R "proxy");

    if (forward)
      return msg >> forward;
    else
      return -1;
  }

  void Bluccino::engine(BlMod &app, int tick_ms, int tock_ms)
  {
    forward = &app;
    bl_engine(proxy, tick_ms, tock_ms);
  }

//==============================================================================
// BlDown method implementation
//==============================================================================

  int BlDown::input(BlMsg &msg)
  {
//  msg.logo(2,BL_G"Down::");
    return bl_msg((bl_down), msg.cl,msg.op, msg.ix,msg.data,msg.val);
  }

//==============================================================================
// class instanciation
//==============================================================================

  Bluccino bl;                         // Bluccino swiss knife
  BlDown blDown("blDown");             // down gear
