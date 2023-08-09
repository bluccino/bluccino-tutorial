//==============================================================================
// blhook.h - hooking up in Zephyr Bluetooth mesh stack
//            Copyright(c): Bluenetics 2021, Hugo Pristauz (2021-Nov-16)
//==============================================================================

#ifndef __BLHOOK_H__
#define __BLHOOK_H__

#include <zephyr.h>
//#include <net/buf.h>
#include <bluetooth/conn.h>
#include <bluetooth/mesh.h>
#include "net.h"
#include "host/ecc.h"

//==============================================================================
// scanner callback & setting up a scanner hook
//==============================================================================

  typedef void (*BL_scan_cb)(uint8_t type, const bt_addr_le_t *addr,
					    	             int8_t rssi, struct net_buf_simple *buf);

//==============================================================================
// advertising callback & setting up an advertiser hook
//==============================================================================

  typedef void (*BL_adv_cb)(void *buf, uint8_t xmit);

//==============================================================================
// enum typedef for different kinds of hook setup
//==============================================================================

  typedef enum BL_hook
          { BL_HOOK_SCAN_IBEACON,      // setup scanner hook for iBeacon packets
            BL_HOOK_ADV_INTERVAL,      // setup advertiser hook for AFD interval
          } BL_hook;

  void bl_scan_hook(BL_scan_cb cb);    // set up a hook in scanner
  void bl_adv_hook(BL_adv_cb cb);      // set up a hook in advertiser

  void bl_hook(BL_hook kind);          // actual hook setup (different choices)

#endif // __BLHOOK_H__
