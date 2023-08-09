//==============================================================================
//  bl_gear.h
//  Bluccino gear
//
//  Created by Hugo Pristauz on 2021-11-06
//  Copyright © 2021 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_GEAR_H__
#define __BL_GEAR_H__

//==============================================================================
// GEAR Logging
//==============================================================================

#ifndef CFG_LOG_GEAR
  #define CFG_LOG_GEAR    0       // GEAR logging is by default off
#endif

#if (CFG_LOG_GEAR)
  #define LOG_GEAR(l,f,...)       BL_LOG(CFG_LOG_GEAR-1+l,f,##__VA_ARGS__)
  #define LOGO_GEAR(l,f,o,v)      bl_logo(CFG_LOG_GEAR-1+l,f,o,v)
#else
  #define LOG_GEAR(l,f,...)       {}     // empty
  #define LOGO_GEAR(l,f,o,v)      {}     // empty
#endif

//==============================================================================
// [SYS:op] message definition
// - [SYS:INIT cb] inits module, stores output callback
// - [SYS:TICK @ix,cnt] ticks module (@ix: tick ID, cnt: tick counter)
// - [SYS:TOCK @ix,cnt] tocks module (@ix: tock ID, cnt: tock counter)
//==============================================================================

  #define SYS_INIT_0_cb_0   BL_ID(_SYS,INIT_) // [SYS:INIT cb] init module
  #define SYS_TICK_ix_BL_pace_cnt BL_ID(_SYS,TICK_) // [SYS:TICK @ix,cnt]
  #define SYS_TOCK_ix_BL_pace_cnt BL_ID(_SYS,TOCK_) // [SYS:TOCK @ix,cnt]

    // augmented messages

  #define _SYS_INIT_0_cb_0   _BL_ID(_SYS,INIT_) // [#SYS:INIT cb] init module
  #define _SYS_TICK_ix_BL_pace_cnt _BL_ID(_SYS,TICK_) // [#SYS:TICK @ix,cnt]
  #define _SYS_TOCK_ix_BL_pace_cnt _BL_ID(_SYS,TOCK_) // [#SYS:TOCK @ix,cnt]

//==============================================================================
// event message output (message emission of a module)
// - usage: bl_out(o,val,(to))  // output to given module
// - important note: class tags are automatically un-augmented before posting
// - note: bl_gear.c provides a weak implementation (redefinition possible)
//==============================================================================

  int bl_out(BL_ob *o, int val, BL_oval to);

//==============================================================================
// augmented event message output (message emission of a module)
// - usage: _bl_out(o,val,(to))   // output to given module
// - important note: class tags are automatically augmented before posting
// - note: bl_gear.c provides a weak implementation (redefinition possible)
//==============================================================================

  int _bl_out(BL_ob *o, int val, BL_oval to);

//==============================================================================
// auxillary emission function
// - used by bl_top to output to app module
// - all messages except [SYS:] messages to be forwarded to app
//==============================================================================

// int bl_emit(BL_ob *o, int val);      // output non [SYS:] message to app

//==============================================================================
// gear upward/downward and top gear interface
// - note: bl_gear.c provides a weak implementation (redefinition possible)
//==============================================================================

  int bl_top(BL_ob *o, int val);       // top gear
  int bl_up(BL_ob *o, int value);      // upward gear
  int bl_down(BL_ob *o, int value);    // downward gear

//==============================================================================
// PMI: bl_gear (Bluccino gear)
// - initializing and ticking bl_down, bl_up, bl_top and bl_core
//==============================================================================

  int bl_gear(BL_ob *o, int val);

#endif // __BL_GEAR_H__
