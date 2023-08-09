//==============================================================================
// bl_ble.h
// BLE declarations
//
// Created by Hugo Pristauz on 2022-Aug-01
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================
// Advertising Type
//


// Advertising Flags:

//   [ADV:MODE "C"] // 0x00: Scannable & connectable advertising    (BL_ADTYP_C)
//   [ADV:MODE "D"] // 0x01: Directed connectable advertising       (BL_ADTYP_D)
//   [ADV:MODE "S"] // 0x02: Non-connectable and scannable advert.  (BL_ADTYP_S)
//   [ADV:MODE "N"] // 0x03: Non-connectable and non-scannable adv. (BL_ADTYP_N)
//   [ADV:MODE "R"] // 0x04: Add. adv data request by active scanner(BL_ADTYP_R)
//   [ADV:MODE "X"] // 0x05: Extended adv., see adv. properties.    (BL_ADTYP_X)
//
// Advertising Flags:
//
//   [ADV:MODE "l"]   // 0x01: LE Limited Discoverable Mode (BT_LE_AD_LIMITED)
//   [ADV:MODE "g"]   // 0x02: LE General Discoverable Mode (BT_LE_AD_GENERAL)
//   [ADV:MODE "n"]   // 0x04: BR/EDR Not Supported         (BT_LE_AD_NO_BREDR)
//   [ADV:MODE "c"]   // 0x08: Controller: simultaneous LE & BR/EDR
//   [ADV:MODE "h"]   // 0x10: Host: simultaneous LE & BR/EDR
//
//==============================================================================

#ifndef __BL_BLE_H__
#define __BL_BLE_H__

	#include <zephyr/types.h>
	#include <stddef.h>
	#include <zephyr/sys/printk.h>
	#include <zephyr/sys/util.h>

	#include <zephyr/bluetooth/bluetooth.h>
	#include <zephyr/bluetooth/hci.h>

  #include <zephyr/settings/settings.h>

  #include <zephyr/bluetooth/conn.h>
  #include <zephyr/bluetooth/uuid.h>
  #include <zephyr/bluetooth/gatt.h>

//==============================================================================
// BLE Logging
//==============================================================================

#ifndef CFG_LOG_BLE
  #define CFG_LOG_BLE    0           // no BLE logging by default on
#endif

#if (CFG_LOG_BLE)
  #define LOG_BLE(l,f,...)    BL_LOG(CFG_LOG_BLE-1+l,f,##__VA_ARGS__)
  #define LOGO_BLE(l,f,o,v)   bl_logo(CFG_LOG_BLE-1+l,f,o,v)
#else
  #define LOG_BLE(l,f,...)    {}     // empty
  #define LOGO_BLE(l,f,o,v)   {}     // empty
#endif

//==============================================================================
// config defaults
//==============================================================================

#ifndef CONFIG_BT_PERIPHERAL           // defined in prj.conf
  #define CONFIG_BT_PERIPHERAL 0       // by default 0 if not def'ed in prj.conf
#endif

#ifndef CONFIG_BT_SMP
  #define CONFIG_BT_SMP 0
#endif

#ifndef CFG_BLE_SYC_ENABLE
  #define CFG_BLE_SYC_ENABLE    1      // syncronous protocol for BLE enable
#endif

//==============================================================================
// Advertising Type:
//   [ADV:MODE "C"]   // 0x00: Scannable and connectable advertising
//   [ADV:MODE "D"]   // 0x01: Directed connectable advertising
//   [ADV:MODE "S"]   // 0x02: Non-connectable and scannable advertising
//   [ADV:MODE "N"]   // 0x03: Non-connectable and non-scannable advertising
//   [ADV:MODE "R"]   // 0x04: Additional adv. data requested by active scanner
//   [ADV:MODE "X"]   // 0x05: Extended adv., see advertising properties
//==============================================================================

  #define BL_ADTYP_C      BT_GAP_ADV_TYPE_ADV_IND          // 0x00
  #define BL_ADTYP_D      BT_GAP_ADV_TYPE_ADV_DIRECT_IND   // 0x01
  #define BL_ADTYP_S      BT_GAP_ADV_TYPE_ADV_SCAN_IND     // 0x02
  #define BL_ADTYP_N      BT_GAP_ADV_TYPE_ADV_NONCONN_IND  // 0x03
  #define BL_ADTYP_R      BT_GAP_ADV_TYPE_SCAN_RSP         // 0x04
  #define BL_ADTYP_X      BT_GAP_ADV_TYPE_EXT_ADV          // 0x05

//==============================================================================
// Advertising Flags:
//   [ADV:MODE "l"]   // 0x01: LE Limited Discoverable Mode         (BL_ADFLG_L)
//   [ADV:MODE "g"]   // 0x02: LE General Discoverable Mode         (BL_ADFLG_G)
//   [ADV:MODE "n"]   // 0x04: BR/EDR Not Supported                 (BL_ADFLG_N)
//   [ADV:MODE "c"]   // 0x08: Controller: simultaneous LE & BR/EDR (BL_ADFLG_C)
//   [ADV:MODE "h"]   // 0x10: Host: simultaneous LE & BR/EDR       (BL_ADFLG_H)
//==============================================================================

  #define BL_ADFLG_L      BT_LE_AD_LIMITED                 // 0x01
  #define BL_ADFLG_G      BT_LE_AD_GENERAL                 // 0x02
  #define BL_ADFLG_N      BT_LE_AD_NO_BREDR                // 0x04
  #define BL_ADFLG_C      0x08                             // 0x08
  #define BL_ADFLG_H      0x10                             // 0x10

//==============================================================================
// properties of GATT characteristics
//==============================================================================

  #define BL_CHRC_R   BT_GATT_CHRC_READ
  #define BL_CHRC_W   BT_GATT_CHRC_WRITE
  #define BL_CHRC_N   BT_GATT_CHRC_NOTIFY
  #define BL_CHRC_I   BT_GATT_CHRC_INDICATE

  #define BL_CHRC_RW  (BL_CHRC_R | BL_CHRC_W)
  #define BL_CHRC_RN  (BL_CHRC_R | BL_CHRC_N)
  #define BL_CHRC_WN  (BL_CHRC_W | BL_CHRC_N)
  #define BL_CHRC_RWN (BL_CHRC_R | BL_CHRC_W | BL_CHRC_N)

  #define BL_PERM_R   BT_GATT_PERM_READ
  #define BL_PERM_W   BT_GATT_PERM_WRITE
  #define BL_PERM_N   BT_GATT_PERM_NOTIFY
  #define BL_PERM_I   BT_GATT_PERM_INDICATE

  #define BL_PERM_RW  (BL_PERM_R | BL_PERM_W)
  #define BL_PERM_RN  (BL_PERM_R | BL_PERM_N)
  #define BL_PERM_WN  (BL_PERM_W | BL_PERM_N)
  #define BL_PERM_RWN (BL_PERM_R | BL_PERM_W | BL_PERM_N)

//==============================================================================
// scan and advertising parameters
//==============================================================================

  #define BL_MS2GAP(ms) ((ms*16)/10)   // convert ms to gap (win/interval) units
  #define BL_GAP2MS(gap) ((gap*10)/16) // convert ms to gap (win/interval) units

//==============================================================================
// typedefs
//==============================================================================

    // short hands for Bluetooth address (Bluetooth MAC address)

  typedef bt_addr_le_t BL_btad;             // Bluetooth addr
  typedef char BL_mac[BT_ADDR_LE_STR_LEN];  // Bluetooth MAC address

//==============================================================================
// work structure for dynamic advertising setup of ad[] data or sd[] data
// - usage: static struct bt_data array[3];
//          static BL_bt ad = {array,0,BL_LEN(array)}
//==============================================================================

  typedef struct bt_data          BT_data;   // Bluetooth data
  typedef const struct bt_data    BL_packet; // Bluetooth (adv) data packet

  typedef struct bt_le_adv_param  BL_apar;   // Bluetooth advertising parameters
  typedef struct bt_le_scan_param BL_spar;   // Bluetooth sacanning parameters

  typedef struct BL_bt
          {
            struct bt_data *array;     // points to Bluetooth data
            int len;                   // current length of array
            int max;                   // max length of ad[] or sd[] array
          } BL_bt;

    // macro for compact declaration of BL_bt structure

  #define BL_BT(name,array,max) \
          struct bt_data array[max]; \
          static BL_bt name = {array,0,BL_LEN(array)};

  typedef struct bt_conn                 BL_conn; // Bluetooth connection data
  typedef const struct bt_gatt_attr      BL_attr; // GATT attribute
  typedef struct bt_gatt_indicate_params BL_gip;  // GATT indicate parameters

//==============================================================================
// BLE data definition (completely defines how to run the advertiser)
// - usage:
//   BL_apar *ap = BT_LE_ADV_NCONN_IDENTITY;
//   BL_id sid = BL_ID(_SVC,FOO_);
//   BL_ble ble = {ap:ap, ad:ad,alen:BL_LEN(ad), sd:sd,slen:BL_LEN(sd),sid:foo};
//==============================================================================

  typedef struct BL_ble
          {
            BL_apar *ap;               // pointer to advertising parameters
            BT_data *ad;               // advertising data (array)
            int alen;                  // length of advertising data (array)
            BT_data *sd;               // scan response data (array)
            int slen;                  // length of scan response data (array)
            BL_id sid;                 // service ID

            BL_spar *sp;               // pointer to scan parameters
            int err;                   // error code during setup
            BL_mac mac;                // mac address of advertiser
         } BL_ble;

//==============================================================================
// get my own Bluetooth MAC address
// - usage: pbtad = bl_me();
//==============================================================================

  static inline const BL_btad *bl_me(void)
  {
    static BL_btad addr = {0};
    size_t count = 1;

    bt_id_get(&addr, &count);
    return &addr;
  }

//==============================================================================
// convert BT address to MAC address text for printing
// - usage: pmac = bl_mac(&addr);
//          pmac = bl_mac(bl_me())
//==============================================================================

  static inline BL_txt bl_mac(const BL_btad *addr)
  {
    static BL_mac mac;
    bt_addr_le_to_str(addr, mac, sizeof(mac));
    return mac;
  }

//==============================================================================
// copy <bL_ble> data (might be statically def'ed) to msg. obj's data reference
// - usage: bl_blecopy(o,&ble);  // copy <BL_ble> data into obj's data reference
//==============================================================================

  static inline BL_ble *bl_blecopy(BL_ob *o, BL_ble *p)
  {
    BL_ble *data = bl_data(o);

    if (data == 0)    // error: no <BL_ble> data provided
    {
      bl_err(-1,"bl_blecopy: no <BL_ble> data provided");
      return NULL;
    }

    memcpy(data ,p, sizeof(BL_ble));
    return data;
  }

//==============================================================================
// add <bL_ble> (BLE recipe) data to message object's data reference
// - usage: bl_bleadd(o,&ble);  // copy <BL_ble> data into obj's data reference
//==============================================================================

  static inline BL_ble *bl_bleadd(BL_ob *o, BL_ble *p)
  {
    BL_ble *data = bl_data(o);

    if (data == 0)    // error: no <BL_ble> data provided
    {
      bl_err(-1,"bl_bleadd: no <BL_ble> data provided");
      return NULL;
    }

    data->ap = p->ap;                  // pointer to advertising parameters

    data->ad = p->ad;                  // advertising data (array)
    data->alen =  p->alen;             // length of advertising data (array)

    data->sd = p->sd;                  // scan response data (array)
    data->slen = p->slen;              // length of scan response data (array)

    data->sid = p->sid;                // service ID

    data->sp = p->sp;                  // pointer to scan parameters

    return data;
  }

//==============================================================================
// access advertising data
// - usage:
//   AD_manu *pmanu = bl_adata(&ble,BT_DATA_MANUFACTURER_DATA,sizeof(AD_manu));
//   AD_srvc *psrvc = bl_adata(&ble,BT_DATA_SVC_DATA16, sizeof(AD_srvc));
//==============================================================================

  void *bl_adata(BL_ble *ble, BL_byte type, size_t size);

//==============================================================================
// BLE interaction interface
//==============================================================================
//                  +--------------------+
//                  |        BLE:        | BLE interface
//           INIT --|      <BL_ble>      | init advertising BLE service
//          SETUP --|    @sid,<BL_ble>   | setup BLE service
//         ENABLE --|      <BL_ble>      | enable Bluetooth
//          START --|      <BL_ble>      | start advertising BLE service
//           STOP --|      <BL_ble>      | stop advertising BLE service
//          READY --|                    | report Bluetooth to be ready
//        CONNECT --|      <BL_conn>     | notify connection event
//         DISCON --|  <BL_conn>,reason  | notify disconnection event
//            MTU --|       @rx,tx       | notify MTU update
//            MAC --|      <BL_mac>      | get Bluetooth mac address
//                  +--------------------+
//==============================================================================

  typedef int (*_BLE_INIT_)(BL_ble *ble);    // init advertising BLE service
  typedef int (*_BLE_SETUP_)(int sid, BL_ble *ble);  // setup BLE service
  typedef int (*_BLE_ENABLE_)(BL_ble *ble);  // enable Bluetooth
  typedef int (*_BLE_START_)(BL_ble *ble);   // start advertising BLE service
  typedef int (*_BLE_STOP_)(BL_ble *ble);    // stop advertising BLE service
  typedef int (*_BLE_READY_)(void);          // report Bluetooth to be ready
  typedef int (*_BLE_CONNECT_)(BL_conn *conn);      // notify connection event
  typedef int (*_BLE_DISCON_)(BL_conn *conn,int reason);  // notify discon event
  typedef int (*_BLE_MTU_)(int rx,int tx);   // notify MTU update
  typedef int (*_BLE_MAC_)(BL_mac *ble);     // get Bluetooth mac address

  typedef struct
          {
            _BLE_INIT_    INIT;
            _BLE_SETUP_   SETUP;
            _BLE_ENABLE_  ENABLE;
            _BLE_START_   START;
            _BLE_STOP_    STOP;
            _BLE_READY_   READY;
            _BLE_CONNECT_ CONNECT;
            _BLE_DISCON_  DISCON;
            _BLE_MTU_     MTU;
            _BLE_MAC_     MAC;
          } _BLE_;

//==============================================================================
// public module interfaces for bl_gap and bl_gatt
//==============================================================================

  int bl_ble(BL_ob *o, int val);

#endif // __BL_BLE_H__
