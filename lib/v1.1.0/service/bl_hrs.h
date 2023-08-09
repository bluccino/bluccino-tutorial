//==============================================================================
// bl_hrs.h
// heart rate BLE service setup
//
// Created by Hugo Pristauz on 2022-Aug-11
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

#ifndef __BL_HRS_H__
#define __BL_HRS_H__

//==============================================================================
// notify heart rate measurement (send GATT notify to all current subscribers)
// - usage: err = notify(heartrate) // heartrate: measurement in beats/minute
//==============================================================================

  int bt_hrs_notify(uint16_t heartrate);

//==============================================================================
// public module interface (PMI)
//==============================================================================

  int bl_hrs(BL_ob *o, int val);

#endif // __BL_HRS_H__
