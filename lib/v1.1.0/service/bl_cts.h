//==============================================================================
// bl_cts.h
// current time BLE service setup
//
// Created by Hugo Pristauz on 2022-Aug-09
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

#ifndef __BL_CTS_H__
#define __BL_CTS_H__

  void cts_init(void);
  void cts_notify(void);

//==============================================================================
// public module interface (PMI)
//==============================================================================

  int bl_cts(BL_ob *o, int val);

#endif // __BL_CTS_H__
