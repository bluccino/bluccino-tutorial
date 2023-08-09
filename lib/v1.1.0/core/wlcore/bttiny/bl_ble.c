//==============================================================================
// bl_ble.c
// BLE (GAP/GATT) functions
//
// Created by Hugo Pristauz on 2022-Aug-01
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_wl.h"
  #include "bl_ble.h"

//==============================================================================
// PMI definition and CORE level logging shorthands
//==============================================================================

  #define WHO  "bl_ble:"

  #define LOG                     LOG_BLE
  #define LOGO(lvl,col,o,val)     LOGO_BLE(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_BLE(lvl,col,o,val)

  #define PMI   bl_ble            // public module interface
  BL_PMI(PMI)                     // define static inline _bl_pmi() function

//==============================================================================
// module state
//==============================================================================

  typedef struct MY_state
          {
            bool advertising;
          } MY_state;

  static MY_state state = { advertising:false };
  static BL_ble *ptr_ble = NULL;  // remember reference of app's BLE recipe

//==============================================================================
// helper: log BLE receipe
// - usage: log_receipe(&ble)
//==============================================================================
#if (CFG_LOG_BLE)

  static void log_receipe(BL_ble *p)
  {
    int packet = 1;

    LOG(4,BL_Y"BLE receipe: <%s|%s>", BL_IDTXT(p->sid));

    LOG(4,"  ap: (advertising parameters)");
    LOG(4,"    options: 0x%02X", p->ap->options);
    LOG(4,"    interval: %d/%d ms",
          BL_GAP2MS(p->ap->interval_min), BL_GAP2MS(p->ap->interval_max));
    //LOG(4,"    peer: %s", bl_mac(p->ap->peer));

    bl_sleep(50); // slow down to give Segger RTT CPU for buffer printing

 	  for (int k=1; k <= 2; k++)  // two runs, 1st run for ad[], 2nd run for sd[]
    {
      BL_packet *pd = (k==1) ? p->ad : p->sd;
      int n = (k==1) ? p->alen : p->slen;
      int total = 0;

      for (int i=0; i < n; i++, packet++)
      {
        bl_fmt("  %s[%d]: ", k==1?"ad":"sd", i);
        BL_packet *q = pd + i;
        total += 2;                      // for type and length
        bl_fmt("%&type: 0x%02X #%02d |", q->type,q->data_len);

        BL_txt sep = " ";
        for (int j=0; j < q->data_len; j++)
        {
          total++;
          bl_fmt("%&%s%02X", sep, q->data[j]);
          sep = "-";
        }
        LOG(4,"%s",bl_fmt(NULL));
      }
      LOG(4,"%stotal %s payload: #%d bytes (max 31 bytes allowed)",
			    total>31 ? BL_R : BL_Y, k==1 ? "advertising":"scan response", total);
    }
  }

#else  // (CFG_BLE_LOG)
  #define log_receipe(p)     // empty
#endif // (CFG_LOG_BLE)
//==============================================================================
// access advertising data
// - usage:
//   AD_manu *pmanu = bl_adata(&ble,BT_DATA_MANUFACTURER_DATA,sizeof(AD_manu));
//   AD_srvc *psrvc = bl_adata(&ble,BT_DATA_SVC_DATA16, sizeof(AD_srvc));
//==============================================================================

  void *bl_adata(BL_ble *ble, BL_byte type, size_t size)
  {
    BT_data *p = ble->ad;

    for (int i=0; i < ble->alen; i++,p++)
    {
      if (p == 0 || p->type != type)
        continue;

      if (size != p->data_len)
        bl_err(-1,"bl_data() called with unsuitable size arg");
      else
        return (void*)p->data;
    }

    bl_err(-2,"bl_data(): data packet with given type not found");
    return NULL;
  }

//==============================================================================
// callback: Bluetooth ready
// - load settings (if enabled)
// - emit [BLE.READY] message if Bluetooth init was successful
//==============================================================================

  static void bt_ready(int err)
  {
    if (err)
    {
      bl_err(err,"Bluetooth not ready");
      return;
    }

    if (IS_ENABLED(CONFIG_SETTINGS))
    {
      LOG(4,"loading settings ...");
      settings_load();
    }

    _bl_pmi(_BLE,READY_, 0,ptr_ble,0);
  }

//==============================================================================
// helper: dynamic add of chunk to BL_bt data
// - usage: static BL_BLE(bt_ad,array,2);
//          static BT_data flags = BT_DATA_BYTES(BT_DATA_FLAGS, BL_ADFLG_N);
//          static BT_data uuid  = BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa,0xfe);
//          bl_btadd(&bt_ad,NULL);     // reset bt_ad data (optional)
//          bl_btadd(&bt_ad,&flags);   // add flags Bluetooth data chunk
//          bl_btadd(&bt_ad,&uuid);    // add uuid Bluetooth data chunk
//==============================================================================
/*
  static int bl_btadd(BL_bt *bt, const BT_data *chunk)
  {
    if (chunk == 0)
    {
       bt->len = 0;                    // reset Bluetooth data
       return 0;
    }
    else if (bt->len < bt->max)
    {
       memcpy(bt->array+bt->len, chunk, sizeof(BT_data));
       bt->len++;
       return 0;
    }

    return bl_err(-1,"bl_btadd: cannot add to Bluetooth data");
  }
*/
//==============================================================================
// callbacks: authentication
//==============================================================================
#if (CONFIG_BT_PERIPHERAL && CONFIG_BT_SMP)               // defined in prj.conf

  static void auth_passkey_display(BL_conn *conn, unsigned int passkey)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG(1,BL_R "Passkey for %s: %06u", addr, passkey);
  }

  static void auth_cancel(BL_conn *conn)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG(1,BL_R "Pairing cancelled: %s", addr);
  }

  static struct bt_conn_auth_cb auth_cb_display =
  {
    passkey_display: auth_passkey_display,
    passkey_entry: NULL,
    cancel: auth_cancel,
  };

#endif
//==============================================================================
// gatt callback: MTU updated (`MTU` = maximum transmission unit)
// - MTU exchange: the exchange MTU request is used by the client to inform the
//   server of the client's max. receive MTU size and request the server to
//   respond with its maximum receive MTU size
// - the client RX MTU shall be greater than or equal to the default ATT_MTU
// - this request shall only be sent once during aconnection by the client
// - the client RX MTU parameter shall be sent to the maximum size of the attri-
//   bute protocol PDU that the client can receive
//==============================================================================
#if (CONFIG_BT_PERIPHERAL)

  static void mtu_updated(BL_conn *conn, BL_word tx, BL_word rx)
  {
    LOG(3,"updated MTU: TX: %d RX: %d bytes", tx, rx);
    _bl_msg(PMI, _BLE,MTU_, tx,NULL,rx);
  }

  static struct bt_gatt_cb gatt_callbacks = { att_mtu_updated:mtu_updated };

#endif
//==============================================================================
// callbacks: connected and disconnected callback
//==============================================================================
#if (CONFIG_BT_PERIPHERAL)

  static void connected(BL_conn *conn, BL_byte err)
  {
    if (err)
      bl_err(err,"connection failed");
    else
    {
      LOG(3,"connected");
      _bl_pmi(_BLE,CONNECT_, 0,conn,0);
    }
  }

  static void disconnected(BL_conn *conn, BL_byte reason)
  {
    LOG(3,"disconnected (reason 0x%02x)", reason);
    _bl_pmi(_BLE,DISCON_, 0,conn,reason);
  }

  BT_CONN_CB_DEFINE(conn_callbacks) =
    {connected:connected, disconnected:disconnected};

#endif
//==============================================================================
// handler: [BT:MAC <BL_mac>]   // get readable Bluetooth MAC address
//==============================================================================

  static int ble_mac(BL_ob *o, int val)
  {
    char *p = bl_data(o);
    BL_mac mac;

    BL_btad addr = {0};
    size_t count = 1;

    bt_id_get(&addr, &count);
    bt_addr_le_to_str(&addr, mac, sizeof(mac));

    memcpy(p,mac,sizeof(mac));
    return 0;
  }

//==============================================================================
// handler: [BLE.INIT <BL_ble>]        // init <BL_ble> data structure
//==============================================================================

  static int ble_init(BL_ob *o, int val)
  {
    BL_ble *p = ptr_ble = bl_data(o);  // remember app's BLE recipe reference
                                       // in ptr_ble, use p asa short hand
    if (p == NULL)
      return -1;

    LOG(5,"init BLE recipe ...");

    p->ap = NULL;                      // advertising parameters
    p->sp = NULL;                      // scannning parameters
    p->ad = p->sd = NULL;              // ad[] and sd[] data pointer
    p->alen = p->slen = 0;             // length of ad[], sd[]
    p->sid = 0;                        // clear service ID

    memcpy(p->mac,bl_mac(bl_me()),sizeof(BL_mac));
    p->err = 1;                        // not yet done

    return 0;
  }

//==============================================================================
// handler: [BLE.SETUP <BL_ble>]       // setup common part o BLE recipe
//==============================================================================

//  static BL_apar *ap = BT_LE_ADV_NCONN_IDENTITY;

  static int ble_setup(BL_ob *o, int val)
  {
    if (ble_init(o,val))               // init BLE receipe
      return -1;                       // bad reference of recipe data

    LOG(5,"setup common part of BLE recipe (mode %d)",val);
/*
    BL_ble *p = bl_data(o);

      // set advertising parameters;
      // for mode 0: use default parameters of first registered service
      // for mode > 0: set specific ADV parameters

    int mode = val;                    // rename `val` arg
    switch (mode)
    {
      case 1: p->ap = BT_LE_ADV_NCONN_IDENTITY;  break;
      case 2: p->ap = BT_LE_ADV_CONN_NAME;       break;
      case 3: p->ap = BT_LE_ADV_CONN_NAME_AD;    break;
      default: break;
    }
*/
    return 0;
  }

//==============================================================================
// handler: [ADV:UPDATE] // update advertiser
//==============================================================================

  static int ble_update(BL_ob *o, int val)
  {
    BL_ble *p = bl_data(o);            // points to BLE receipe

    if (state.advertising && p != 0)
    {
      log_receipe(p);                  // log BLE receipe

      LOG(4,"updating advertising data");
      int err = bt_le_adv_update_data(p->ad, p->alen, p->sd, p->slen);

      if (err)
        return bl_err(err,"advertiser update failed");
    }
    return 0;
  }

//==============================================================================
// handler: [BLE.START <BL_ble>] // start BLE service
//==============================================================================

  static int ble_start(BL_ob *o, int val)
  {
    BL_ble *p = bl_data(o);          // points to BLE receipe

    if (p == NULL)
      return bl_err(-1,"bad <BL_ble> data");

    if (p->alen == 0 && p->slen == 0)
      return bl_err(-2,"bad <BL_ble> data");

      // log BLE receipe

    log_receipe(p);

      // register GATT callbacks

    #if (CONFIG_BT_PERIPHERAL)                            // defined in prj.conf
      LOG(4,"register GATT callbacks");
      bt_gatt_cb_register(&gatt_callbacks);
    #endif
        // register SM (security manager) callbacks

    #if (CONFIG_BT_PERIPHERAL && CONFIG_BT_SMP)           // defined in prj.conf
      bt_conn_auth_cb_register(&auth_cb_display);
    #endif

    LOG(3,BL_C "Bluetooth device name: %s",bt_get_name());

      // run BLE service by starting the advertiser

    int err = bt_le_adv_start(p->ap, p->ad,p->alen, p->sd,p->slen);
    state.advertising = (err == 0);
    if (err)
      return bl_err(err,"advertising failed to start");

    memcpy(p->mac,bl_mac(bl_me()),sizeof(BL_mac));
    LOG(3,BL_B "advertising of BLE service <%s|%s> started @ %s",
                  BL_IDTXT(p->sid),p->mac);

     return err;
  }

//==============================================================================
// handler: [BLE.ENABLE <BL_ble>]      // enable BLE service
//==============================================================================

  static int ble_enable(BL_ob *o, int val)
  {

    LOG(4,BL_B "enable Bluetooth ...");
    int err = bt_enable(bt_ready);

    return bl_err(err,"Bluetooth init failed");
  }

//==============================================================================
// public module interface
// - usage: to setup for Edistone beacon
//   [ADV:MODE "n"]   // BR/EDR Not Supported (BT_LE_AD_LIMITED)
//==============================================================================
// Advertising Type:
//   [ADV:MODE "C"] // 0x00: Scannable & connectable advertising    (BL_ADTYP_C)
//   [ADV:MODE "D"] // 0x01: Directed connectable advertising       (BL_ADTYP_D)
//   [ADV:MODE "S"] // 0x02: Non-connectable and scannable advert.  (BL_ADTYP_S)
//   [ADV:MODE "N"] // 0x03: Non-connectable and non-scannable adv. (BL_ADTYP_N)
//   [ADV:MODE "R"] // 0x04: Add. adv data request by active scanner(BL_ADTYP_R)
//   [ADV:MODE "X"] // 0x05: Extended adv., see adv. properties.    (BL_ADTYP_X)
//
// Advertising Flags:
//   [ADV:MODE "l"]   // 0x01: LE Limited Discoverable Mode (BT_LE_AD_LIMITED)
//   [ADV:MODE "g"]   // 0x02: LE General Discoverable Mode (BT_LE_AD_GENERAL)
//   [ADV:MODE "n"]   // 0x04: BR/EDR Not Supported         (BT_LE_AD_NO_BREDR)
//   [ADV:MODE "c"]   // 0x08: Controller: simultaneous LE & BR/EDR
//   [ADV:MODE "h"]   // 0x10: Host: simultaneous LE & BR/EDR
//
//==============================================================================
//
// W := bl_wl
//
//                  +--------------------+
//                  |       bl_ble       | BLE core module
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)->     INIT ->|        (to)        | init module, store (to) callback
// (W)->     TICK ->|       @ix,cnt      | tick the module
// (W)->     TOCK ->|       @ix,cnt      | tock the module
//                  +--------------------+
//                  |        BLE:        | BLE input interface
// (W)->     INIT ->|      <BL_ble>      | init advertising BLE service
// (W)->    SETUP ->|   <BL_ble>,mode    | setup common part of BLE recipe
// (W)->   ENABLE ->|   <BL_ble>,mode    | enable Bluetooth
// (W)->    START ->|      <BL_ble>      | start advertising BLE service
// (W)->      MAC ->|      <BL_mac>      | get Bluetooth mac address
//                  |....................| BLE output interface
// (W)<-    READY <-|      <BL_ble>      | report Bluetooth to be ready
// (W)<-  CONNECT <-|      <BL_conn>     | notify connection event
// (W)<-   DISCON <-|  <BL_conn>,reason  | notify disconnection event
// (W)<-      MTU <-|       @rx,tx       | notify MTU update
//                  +--------------------+
//                  |      SERVICE:      | SERVICE input interface
// (W)->        * <-|      <BL_ble>      | init service
//                  +--------------------+
//                  |        ADV:        | ADV public interface
// (W)->   UPDATE ->|                    | update advertiser
// (W)->     MODE ->|       flags        | setup ADV mode data (flags & type)
// (W)->     UUID ->|        uuid        | setup ADV UUID data
// (W)->      SVC ->|        svc         | setup ADV SVC data
//                  +--------------------+
//
//==============================================================================

  int bl_ble(BL_ob *o, int val)
  {
    static BL_oval W = bl_wl;          // wireless core

    if (bl_cl(o) == _SVC)
      return ble_init(o,val);          // init <BL_ble> data structure

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):          // [SYS:INIT (to)]
        W = bl_cb(o,(W),WHO"(W)");     // store output callback
        LOG(3,BL_B "init bl_ble");
        return 0;

      case BL_ID(_SYS,TICK_):          // [SYS:TICK @0,<BL_pace>,cnt]
      case BL_ID(_SYS,TOCK_):          // [SYS:TOCK @0,<BL_pace>,cnt]
        return 0;                      // OK - nothing to tick/tock

      case BL_ID(_BLE,INIT_):          // [BLE.INIT <BL_ble>] <- D
        return ble_init(o,val);        // init <BL_ble> structure

      case BL_ID(_BLE,ENABLE_):        // [BLE.ENABLE] <- D
        return ble_enable(o,val);

      case BL_ID(_BLE,SETUP_):         // [BLE:SETUP <BL_ble>,mode]
        return ble_setup(o,val);       // delegate to handler

      case _BL_ID(_BLE,READY_):        // [BLE:READY]
      case _BL_ID(_BLE,CONNECT_):      // [BLE:CONNECT <BL_conn>]
      case _BL_ID(_BLE,DISCON_):       // [BLE:DISCON  <BL_conn>,reason]
      case _BL_ID(_BLE,MTU_):          // [BLE:MTU  @rx,NULL,tx]
        return bl_out(o,val,W);        // report that Bluetooth is ready

      case BL_ID(_BLE,MAC_):           // [BLE.MAC <BL_mac>]
        return ble_mac(o,val);         // forward to ble_mac() handler

      case BL_ID(_BLE,START_):         // start BLE service
        return ble_start(o,val);

      case BL_ID(_BLE,UPDATE_):        // update advertiser
			  return ble_update(o,val);      // delegate to ble_update() handler

      default:
        return -1;                     // bad input
    }
  }

//==============================================================================
// cleanup
//==============================================================================

  #undef WHO
