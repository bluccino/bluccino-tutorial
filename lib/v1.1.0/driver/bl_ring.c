//==============================================================================
// bl_ring.c 
// ring buffer support
//
// Created by Hugo Pristauz on 2022-Aug-06
// Copyright © 2022 Blunetics. All rights reserved.
//==============================================================================
// Howto:
//
//   BL_byte rxbuf[128];
//   BL_ring rxring;
//
//   bl_ring_init(&rxring,&rxbuf,sizeof(rxbuf));
//
//     // put into receiver ring buffer
//     // note: bl_ring_put() would ignore for true bl_ring_full()
//
//   for (BL_byte i=0; i < 30; i++)
//      if ( !bl_ring_full(&rxring) )
//          bl_ring_put(&rxring,i);
//
//     // get from receiver ring buffer
//     // note: // bl_ring_get() would receive 0xff for false bl_ring_avail()
//
//   BL_byte byte;
//   for (BL_byte i=0; i < 50; i++)
//      if ( bl_ring_avail(&rxring) )
//          byte = bl_ring_get(&rxring);
//
//==============================================================================

  #include <assert.h>

  #include "bluccino.h"
  #include "bl_ring.h"

//==============================================================================
// check if ring buffer is full
//==============================================================================

  bool bl_ring_full(BL_ring *r)
  {
    BL_byte avail;

    bl_irq(0);                         // disable IRQ;
    avail = r->avail;
    bl_irq(1);                         // enable IRQ;

    return (avail >= r->size);
  }

//==============================================================================
// put byte into ring buffer
//==============================================================================

  void bl_ring_put(BL_ring *r, BL_byte byte)     // put byte into ring buffer
  {                                              // ignore if full
    if ( bl_ring_full(r) )
    {
        r->overflow++;                 // count overflow (health impacted)
        return;
    }

    r->buf[r->iput] = byte;           // put byte into ring buffer at head position
    bl_irq(0);  // disable IRQ;
    r->avail++;
    bl_irq(1);  // enable IRQ;

    r->iput = (r->iput + 1) % r->size;
  }

//==============================================================================
// check for available byte(s) in ring buffer
//==============================================================================

  bool bl_ring_avail(BL_ring *r)       // is a byte available in ring buffer?
  {
    BL_byte avail;
    bl_irq(0);  // disable IRQ;
    avail = r->avail;
    bl_irq(1);  // enable IRQ;
    return avail > 0;                  // return availability status
  }

//==============================================================================
// get byte from ring buffer
//==============================================================================

  BL_byte bl_ring_get(BL_ring *r)      // get byte from ring buffer
  {
    BL_byte byte = 0xFF;               // default init (if no byte available)
    if ( !bl_ring_avail(r) )
        r->underflow++;                // count underflows (health impact)
    else
    {
        byte = r->buf[r->iget];       // take byte at tail position
        bl_irq(0);  // disable IRQ;
        r->avail--;
        bl_irq(1);  // enable IRQ;
        r->iget = (r->iget + 1) % r->size;
    }
    return byte;
  }

//==============================================================================
// init ring buffer
//==============================================================================

  void bl_ring_init(BL_ring *r, BL_byte *buf, BL_byte size) // init ring buffer
  {
    assert(size > 0);
    r->iput = r->iget = r->avail = 0;
    r->underflow = 0;                  // health counter (for underflow)
    r->overflow = 0;                   // health counter (for overflow)
    r->buf = buf;
    r->size = size;
    r->avail = 0;
  }
