//==============================================================================
//  bl_app.h
//  app specific includes
//
//  Created by Hugo Pristauz on 2021-11-06
//  Copyright © 2021 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_APP_H__
#define __BL_APP_H__

  #ifdef BL_INCLUDE          // BL_INCLUDE expected to be like "main.h"
    #include "bl_defs.h"     // need defs of BL_CL_SYMBOLS,BL_CL_ENUMS,...
    #include BL_INCLUDE      // auto-include header, included by "bluccino.h"
  #endif

#endif // __BL_APP_H__
