//==============================================================================
//  bl_rtl.h
//  Bluccino real time logging (supporting dongle logging)
//
//  Created by Chintan parmar on 2022-07-01
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_RTL_H__
#define __BL_RTL_H__

  #include <stdio.h>

//==============================================================================
// RTL methods: allocate an RTL segment for sprintf
// - usage:
//     int id = bl_rtl_alloc()    // allocate an RTL segment
//     if (id)
//     {
//       BL_byte off = 0;              // print offset in buffer (initially 0)
//       off += sprintf(bl_rtl(id,off),"stuff (id:%d)",id);
//       off += sprintf(bl_rtl(id,off),"\n");
//       bl_rtl_submit(id,off);        // submit segment to RTL FIFO
//     }
//==============================================================================

  int bl_rtl_alloc(void);
  void bl_rtl_submit(int id, BL_byte off);
  char *bl_rtl(int id,BL_byte off);  // actual buffer pointer for sprintf()

//==============================================================================
// debug tracing to be executed for given log level?
//==============================================================================

  //bool bl_rtl_dbg(int lvl);

//==============================================================================
// RTL methods: allocate an RTL segment for sprintf
// - usage:
//     int id = bl_rtl_get()      // get (allocate) an RTL segment
//     BL_byte off = 0;                // print offset in buffer (initially 0)
//     off += sprintf(bl_rtl_buf(id,off),"stuff (id:%d)",id);
//     off += sprintf(bl_rtl_buf(id,off),"\n");
//     bl_rtl_put(id,off);             // put segment into print fifo
//==============================================================================

  #define BL_RTL_SUSPEND 1000     // indicates to suspend RTL segment submission

  #define BL_LOG(lvl,fmt,...)                                             \
    do                                                                    \
    {                                                                     \
      if (bl_dbg(lvl))                                                    \
      {                                                                   \
        int id = bl_now(lvl+BL_RTL_SUSPEND);                              \
        if (id)                                                           \
        {                                                                 \
          BL_byte off = sprintf(bl_rtl(id,0), fmt BL_0, ##__VA_ARGS__);   \
          if (*fmt) off += sprintf(bl_rtl(id,off), "\n");                 \
          bl_rtl_submit(id,off);                                          \
        }                                                                 \
      }                                                                   \
    } while(0)

  #define BL_PRT(fmt,...)                                                 \
    do                                                                    \
    {                                                                     \
      int id = bl_rtl_alloc();                                            \
      if (id)                                                             \
      {                                                                   \
        BL_byte off = sprintf(bl_rtl(id,0), fmt BL_0, ##__VA_ARGS__);     \
        bl_rtl_submit(id,off);                                            \
      }                                                                   \
    } while(0)

  #define bl_log(l,f,...)  BL_LOG(l,f,##__VA_ARGS__)  // always enabled
  #define bl_prt(l,f,...)  BL_PRT(l,f,##__VA_ARGS__)  // always enabled

#endif // __BL_RTL_H__
