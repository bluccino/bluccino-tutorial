//==============================================================================
//  bl_rtos.h
//  realtime OS dependent includes
//
//  Created by Hugo Pristauz on 2021-12-11
//  Copyright © 2021 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_RTOS_H__
#define __BL_RTOS_H__

  #ifdef ZEPHYR
    #ifndef __ZEPHYR__
      #define __ZEPHYR__
    #endif
  #endif

  #ifndef __ZEPHYR__
    #define __NRF_SDK__ 1
  #endif

//==============================================================================
// Zephyr support
// - bl_printf(): driver level Bluccino printf() function - with Zephyr RTOS
//   this function is equivalent with printk function
//==============================================================================

#ifdef __ZEPHYR__

  #include <zephyr/kernel.h>
  #include <zephyr/sys/util.h>
  #include <zephyr/sys/printk.h>
  #include <zephyr/irq.h>              // access irq_lock() and irq_unlock()

  #include <inttypes.h>

  #ifndef __struct
    #define __struct   struct __attribute((packed))   // for packed structures
  #endif

  #define bl_printf          printk
  #define BL_SLEEP(ms)       k_msleep(ms)
  #define BL_VPRINTF(...)    // empty

#endif // __ZEPHYR__

//==============================================================================
// Nordic SDK support
// - obsoleted in Bluccino V1.0.9 or higher
//==============================================================================
/*
#ifdef __NRF_SDK__

  #include <nrf_error.h>
  #include <nrf_delay.h>     // "nrf_delay.h"
  #include "log.h"
  #include "timer.h"

  #define __weak             __WEAK

  #ifndef __struct
    #define __struct   struct __attribute((packed))   // for packed structures
  #endif

  #define bl_prt             bl_printf
  #define printk(...)        bl_printf(__VA_ARGS__)

  #define BL_SLEEP(ms)       nrf_delay_ms(ms)
//  #define BL_VPRINTF(...)    SEGGER_RTT_vprintf(...)
  #define BL_VPRINTF         SEGGER_RTT_vprintf

  int BL_VPRINTF(unsigned, const char *, va_list *);
  void bl_printf(const char * format, ...);

#endif
*/
//==============================================================================
//
//==============================================================================

#endif // __BL_RTOS_H__
