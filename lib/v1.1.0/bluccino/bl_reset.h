//==============================================================================
//  bl_reset.h
//  core reset module
//
//  Created by Hugo Pristauz on 2022-06-16
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_RESET_H__
#define __BL_RESET_H__

//==============================================================================
// RESET Logging
//==============================================================================

  #ifndef CFG_LOG_RESET
    #define CFG_LOG_RESET    1           // RESET logging is by default on
  #endif

  #if (CFG_LOG_RESET)
    #define LOG_RESET(l,f,...)    BL_LOG(CFG_LOG_RESET-1+l,f,##__VA_ARGS__)
    #define LOGO_RESET(l,f,o,v)   bl_logo(CFG_LOG_RESET-1+l,f,o,v)
  #else
    #define LOG_RESET(l,f,...)    {}     // empty
    #define LOGO_RESET(l,f,o,v)   {}     // empty
  #endif

//==============================================================================
// message definitions
//==============================================================================
//
//                  +--------------------+
//                  |       RESET:       | RESET interface
//            INC --|         ms         | set due timer, return ++counter
//            PRV --|                    | unprovision node
//            DUE --|                    | reset timer is due
//                  +--------------------+
//
//==============================================================================

  #define RESET_INC_0_0_ms   BL_ID(_RESET,INC_)
  #define RESET_PRV_0_0_0    BL_ID(_RESET,PRV_)
  #define RESET_DUE_0_0_0    BL_ID(_RESET,DUE_)

    // augmented messages

  #define _RESET_INC_0_0_ms  _BL_ID(_RESET,INC_)
  #define _RESET_PRV_0_0_0   _BL_ID(_RESET,PRV_)
  #define _RESET_DUE_0_0_0   _BL_ID(_RESET,DUE_)

//==============================================================================
// public module interface
//==============================================================================

  int bl_reset(BL_ob *o, int val);

#endif // __BL_RESET_H__
