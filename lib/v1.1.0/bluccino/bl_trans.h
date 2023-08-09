//==============================================================================
//  bl_trans.h
//  managing transitions
//
//  Created by Hugo Pristauz on 2022-06-13
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_TRANS_H__
#define __BL_TRANS_H__


//==============================================================================
// update transition
// - usage: bl_trans(&trans,&update) // update a transition
//==============================================================================

  int bl_trans(BL_trans *p, BL_trans *u);

//==============================================================================
// get current transition level
// - usage: bl_cur(&trans) // get current transition level
//==============================================================================

  int bl_cur(BL_trans *p);             // get current transition level

//==============================================================================
// finish an active transition / ask for finishing
// - usage: done = bl_fin(&trans) // finish an active transition
//==============================================================================

  int bl_fin(BL_trans *p);             // finish transition, return onetime true

#endif // __BL_TRANS_H__
