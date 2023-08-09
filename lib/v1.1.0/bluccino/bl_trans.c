//==============================================================================
//  bl_trans.c
//  managing transitions
//
//  Created by Hugo Pristauz on 2022-06-13
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_trans.h"

//==============================================================================
// update transition
// - usage: bl_trans(&trans,&update) // update a transition
//==============================================================================

  int bl_trans(BL_trans *p, BL_trans *u)
  {
    if (p == NULL || u == 0)
      return -1;                       // expectations not met!

    memcpy(p,u,sizeof(BL_trans));
    return 0;
  }

//==============================================================================
// get current transition level
// - usage: bl_cur(&trans) // get current transition level
//==============================================================================

  int bl_cur(BL_trans *p)              // get current transition level
  {
    if (p == NULL)
      return 0;                        // expectations not met!

    BL_ms now = bl_ms();               // current time
    BL_ms dt = now - p->begin;

    if (dt > p->tt || p->tt == 0)
    {
      p->basis = p->target;
      return p->target;
    }

    int64_t delta = ((int64_t)p->target - p->basis) * dt / p->tt;
    return p->basis + (int)delta;
  }

//==============================================================================
// finish an active transition / ask for finishing
// - usage: done = bl_fin(&trans) // finish an active transition
//==============================================================================

  int bl_fin(BL_trans *p)              // finish transition, return onetime true
  {
    if (p == NULL)
      return 0;                        // expectations not met!

    if (p->basis != p->target && bl_cur(p) == p->target)
    {
      p->basis = p->target;
      return 1;                        // final target has been reached
    }

    return 0;                          // default return value
  }
