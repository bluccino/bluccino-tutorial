//==============================================================================
//  bl_deco.h
//  log (time stamp) decoration according to mesh attention/provision state
//
//  Created by Hugo Pristauz on 2022-06-16
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_DECO_H__
#define __BL_DECO_H__

//==============================================================================
// message definitions
//==============================================================================
//
//                  +--------------------+
//                  |       MESH:        | MESH input interface
//            ATT --|        sts         | notiy attention status
//            PRV --|        sts         | notiy provision status
//                  +--------------------+
//                  |       STATE:       | STATE input interface
//            ATT --|        &int        | retrieve attention status
//            PRV --|        &int        | retrieve provision status
//                  +--------------------+
//
//==============================================================================

  #define MESH_ATT_0_0_sts       BL_ID(_MESH,ATT_)
  #define MESH_PRV_0_0_sts       BL_ID(_MESH,PRV_)
  #define STATE_ATT_0_BL_pint_0  BL_ID(_STATE,ATT_)
  #define STATE_PRV_0_BL_pint_0  BL_ID(_STATE,PRV_)

    // augmented messages

  #define _MESH_ATT_0_0_sts      _BL_ID(_MESH,ATT_)
  #define _MESH_PRV_0_0_sts      _BL_ID(_MESH,PRV_)
  #define _STATE_ATT_0_BL_pint_0 _BL_ID(_STATE,ATT_)
  #define _STATE_PRV_0_BL_pint_0 _BL_ID(_STATE,PRV_)

//==============================================================================
// public module interface
//==============================================================================

  int bl_deco(BL_ob *o, int val);

#endif // __BL_DECO_H__
