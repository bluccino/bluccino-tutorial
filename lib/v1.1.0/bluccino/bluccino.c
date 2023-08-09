//==============================================================================
//  bluccino.c
//  Bluccino overall source
//
//  Created by Hugo Pristauz on 2022-01-01
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================
//
// This is how bluccino intends standard pipeline setup
//
// main => bl_engine(app,tick_ms,tock_ms)
//  |
//  +- app                                     (application)
//  |   |<==================================|
// .|.......................................|...................................
//  |                                       |
//  +- bluccino ===========================>|  (Bluccino module)
//      |                                   |
//      +- bl_top =========================>|  (top gear)
//      |   |                               |
//      |   |<========================|     |  // top gear needs some messages
//      |                             |     |
//      +- bl_up =====================|====>|  (up gear)
//      |   |
//      |   |<==============================|
//      |                                   |
//      +- bl_down                          |  (down gear)
//          |                               |
// .........|...............................|...................................
//          |                               |
//          +- bl_core                      |
//
//==============================================================================

  #include "bluccino.h"

//==============================================================================
// include Bluccino standard C_modules
//==============================================================================

#ifndef __BLUCCINO_C__
#define __BLUCCINO_C__

  #include "bl_time.c"                 // Bluccino API stuff
  #include "bl_clean.h"

  #include "bl_log.c"                  // Bluccino (standard) logging stuff
  #include "bl_clean.h"

  #include "bl_gear.c"                 // Bluccino gear
  #include "bl_clean.h"

  #include "bl_run.c"                  // Bluccino engine (weak functions)
  #include "bl_clean.h"

  #include "bl_lib.c"                  // library registration primitives
  #include "bl_clean.h"

  #include "bl_timer.c"                // Bluccino timer support
  #include "bl_clean.h"

  #include "bl_work.c"                 // Bluccino work package submission
  #include "bl_clean.h"

  #include "bl_core.c"                 // Bluccino default core (weak functions)
  #include "bl_clean.h"

#endif // __BLUCCINO_C__
