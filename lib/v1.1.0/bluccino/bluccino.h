//==============================================================================
//  bluccino.h
//  Bluccino overall header
//
//  Created by Hugo Pristauz on 2021-11-06
//  Copyright © 2021 Bluenetics GmbH. All rights reserved.
//==============================================================================

#include "bl_clean.h"                  // makes concatenation of .c files easy

#ifndef __BLUCCINO_H__
#define __BLUCCINO_H__

  #ifndef CFG_BLUCCINO_RTL
    #define CFG_BLUCCINO_RTL   0       // no Bluccino real time log by default
  #endif

  #ifdef __cplusplus
    extern "C"
    {
  #endif

        // first batch of includes cannot refer to C++ base classes

      #include "bl_type.h"
      #include "bl_app.h"              // #include "config.h", "logging.h" ?
      #include "bl_symb.h"
      #include "bl_msg.h"
      #include "bl_log.h"
      #include "bl_time.h"

  #ifdef __cplusplus
    }

    #include "bl_class.h"              // Bluccino base class definitions

    #include "ifc/button.h"
    #include "ifc/digital.h"
    #include "ifc/io.h"
    #include "ifc/led.h"
    #include "ifc/nvm.h"
    #include "ifc/switch.h"

    extern "C"
    {
  #endif

        // second batch of includes may refer to Bluccino C++ base classes,
        // allowing definition of message and interface classes

      #include "bl_gear.h"
      #include "bl_run.h"
      #include "bl_lib.h"
      #include "bl_timer.h"
      #include "bl_work.h"
      #include "bl_sugar.h"

//==============================================================================
// footer
//==============================================================================

  #ifdef __cplusplus
    }
  #endif // __cplusplus

#endif // __BLUCCINO_H__
