//==============================================================================
// nandsvc.h
// BLE NAND service
//
// Created by Hugo Pristauz on 2022-Aug-23
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

#ifndef __NANDSVC_H__
#define __NANDSVC_H__

//==============================================================================
// state data structure <MY_nand> of battery service
//==============================================================================

  typedef struct TP_nand
          {
            BL_u8 in1;
            BL_u8 in2;
            BL_u8 out;
          } TP_nand;

//==============================================================================
// public module interface (PMI): BLE battery service
//==============================================================================

  int nandsvc(BL_ob *o, int val);

#endif // __NANDSVC_H__
