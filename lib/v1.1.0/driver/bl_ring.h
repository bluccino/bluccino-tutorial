//==============================================================================
// bl_ring.h
// ring buffer support
//
// Created by Hugo Pristauz on 2022-Aug-06
// Copyright © 2022 Blunetics. All rights reserved.
//==============================================================================

#ifndef __BL_RING_H__
#define __BL_RING_H__

//  #include <stdbool.h>
//  #include <stdint.h>
  #include "bl_type.h"

//==============================================================================
// config defaults
//==============================================================================

  #ifndef CFG_RINGBUF_SIZE
    #define CFG_RINGBUF_SIZE  256      // default ring buffer size = 256 bytes
  #endif

//==============================================================================
// data structure
//==============================================================================

  typedef struct BL_ring
  {
      BL_byte iput;                    // put index
      BL_byte iget;                    // get index
      BL_byte avail;                   // number of bytes in ring buffer

      BL_byte *buf;                    // ring buffer memory
      BL_byte size;

      BL_word underflow;               // underflow error count
      BL_word overflow;                // overflow error count
  } BL_ring;

//==============================================================================
// put byte into ring buffer
//==============================================================================

  bool bl_ring_full(BL_ring *r);                 // is ring buffer full?
  void bl_ring_put(BL_ring *r, BL_byte byte);    // put byte into ring buffer

//==============================================================================
// get byte from ring buffer
//==============================================================================

  bool bl_ring_avail(BL_ring *r);      // is a byte available in ring buffer?
  BL_byte bl_ring_get(BL_ring *r);     // get byte from ring buffer

//==============================================================================
// ring buffer init
//==============================================================================

  void bl_ring_init(BL_ring *r, BL_byte *buf, BL_byte size);

#endif // __BL_RING_H__
