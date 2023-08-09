//==============================================================================
// bl_bas.h
// battery BLE service setup
//
// Created by Hugo Pristauz on 2022-Aug-11
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

#ifndef __BL_BAS_H__
#define __BL_BAS_H__

//==============================================================================
// state data structure <BL_bas> of battery service
//==============================================================================

  typedef struct BL_bas
          {
            BL_byte percent;           // battery level in percent;
          } BL_bas;

//==============================================================================
// public module interface (PMI): BLE battery service 
//==============================================================================

  int bl_bas(BL_ob *o, int val);

#endif // __BL_BAS_H__
