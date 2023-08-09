//==============================================================================
// cbeacon.h
// setup/provide custom cBeacon service
//
// Created by Hugo Pristauz on 2022-Aug-14
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

#ifndef __BL_CBEACON_H__
#define __BL_CBEACON_H__

  typedef __struct AD_cbeacon
            {
              BL_u8   uuid_lo;           // Eddystone UUID low byte
              BL_u8   uuid_hi;           // Eddystone UUID high byte
              BL_u8   major_lo;          // major (low byte)
              BL_u8   major_hi;          // major (high byte)
              BL_u8   minor_lo;          // minor (low byte)
              BL_u8   minor_hi;          // minor (high byte)
              BL_s8   nhops;             // number of hops
              BL_s8   rssi0;             // calibrated TX power at 0m
              BL_u8   bflags_lo;         // flags (low byte)
              BL_u8   bflags_hi;         // flags (high byte)
            } AD_cbeacon;

//==============================================================================
// Eddystone beacon BLE service (public module interface)
//==============================================================================

  int cbeacon(BL_ob *o, int val);

#endif // __BL_CBEACON_H__
