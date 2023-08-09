//==============================================================================
// bl_wl.c
// tiny wireless BLE core
//
// Created by Hugo Pristauz on 2022-Aug-01
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================
//
// Bluetooth initializing
//
//  main       app        gear         bl_wl     bl_ble   ble_ready    bl_svc
//  (M)        (A)       (U)(D)         (W)       (B)        (R)        (S)
//   |          |         |  |           |         |          |          |
//   |   SYS.SVC (bl_svc) |  |           |         |          |          |
//   o---------------------->o---------->|  SYS.INIT (bl_wl)  |          |
//   |          |         |  |           o------------------------------>|
//   |          |         |  |           |         |          |          |
//   |          |         |  |           |        SYS.LIB <BL_lib>       |
//   |          |         |  |           |<------------------------------o
//   |          |         |  |           |         |          |          |
//   |          |         |  | +---------v-------+ |          |          |
//   |          |         |  | | register bl_svc | |          |          |
//   |          |         |  | +-----------------+ |          |          |
//   |          |         |  |           |         |          |          |
//   | SYS.INIT |         |  |           |         |          |          |
//   o--------->| BLE.ENABLE <BL_ble>    |         |          |          |
//   |          o<---------->o---------->o         |          |          |
//   |          |         |  ^           |         |          |          |
//   |          |         |  | +---------v-------+ |          |          |
//   |          |         |  | | setup BLE svc's | |          |          |
//   |          |         |  | +---------v-------+ |          |          |
//   |          |         |  |           |         |          |          |
//   |          |         |  |           | BLE.SETUP <BL_ble> |   +------v-----+
//   |          |         |  |           o<---------------------->|    update  |
//   |          |         |  |           |         |          |   | BLE recipe |
//   |          |         |  |           |         |          |   +------v-----+
//   |          |         |  |     +-----v-----+   |          |          |
//   |          |         |  |     |  take(S)  |   |          |          |
//   |          |         |  |     +-----v-----+   |          |          |
//   |          |         |  |           |         |          |          |
//   |          |         |  |           | BLE.ENABLE <BL_ble>|          |
//   |          |         |  |           o-------->|          |          |
//   |          |         |  |           | +-------v-------+  |          |
//   |          |         |  |           | | bt_enable(R)  |->|          |
//   |          |         |  |           | +-------v-------+  |          |
//   |          |         |  |           |         |          |          |
//   |          |         |  |           |         | BLE.READY|          |
//   |          |         |  |           |<--------o<---------o          |
//   |          |         |  |     +-----v-----+   |          |          |
//   |          |         |  |     |   give(S) |   |          |          |
//   |          |         |  |     +-----v-----+   |          |          |
//   |          |         |  |           |         |          |          |
//   |          |         |  o<----------o         |          |          |
//   |          |         |              |         |          |          |
//   |          |  BLE.START <BL_ble>    |         |          |          |
//   |          o----------------------->o-------->|          |          |
//   |          |         |  |           | +-------v-------+  |          |
//   |          |         |  |           | | start advert. |  |          |
//   |          |         |  |           | +---------------+  |          |
//   |          |         |  |           |         |          |          |
//
//==============================================================================

  #include "bluccino.h"
  #include "bl_wl.h"
  #include "bl_ble.h"

//==============================================================================
// CORE level logging shorthands
//==============================================================================

  #define WHO                     "bl_wl:"

  #define LOG                     LOG_CORE
  #define LOGO(lvl,col,o,val)     LOGO_CORE(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_CORE(lvl,col,o,val)

  #define PMI                     bl_wl
  BL_PMI(PMI)                     // define static inline _bl_pmi() function

//==============================================================================
// locals
//==============================================================================
#if (CFG_BLE_SYC_ENABLE)

    static K_SEM_DEFINE(bt_ready_sem,0,1);  // semaphore to wait on BT ready

#endif
//==============================================================================
// handler: [BLE:ENABLE <BL_ble>,mode]
// - first setup all registered BLE services
// - next step is to enable BLE
// - wait for semaphore take, which can happen once BT ready message has been
//   received from BLE module, and semaphore will be given
//==============================================================================

  static int ble_enable(BL_ob *o, int val)
  {
    BL_ble *ble = bl_data(o);
    int mode = val;

    _bl_pmi(_BLE,SETUP_, 0,ble,mode);  // setup all BLE services
    LOG(3,BL_C "setup of registered BLE services complete");

      // BLE module is advised to enable Bluetooth. Once Bluetooth is ready
      // the BLE module response by emitting a [BLE:READY] message to
      // the wireless core (to this module)

    int err = _bl_pmi(_BLE,ENABLE_, 0,bl_data(o),0);   // enable BLE

    if (err)
      return err;

      // now wait until BLE nodule sends the [BLE:READY] message to the wireless
      // core, which causes the message handler to give the semaphore, so we can
      // take the semaphore

    #if (CFG_BLE_SYC_ENABLE)
      LOG(5,"waiting for BT ready");
      err = k_sem_take(&bt_ready_sem,K_MSEC(1000)); // take with 1000 ms timeout
      if (err)
        return bl_err(err,"timeout when waiting for BLE ready");

      LOG(3,BL_C "Bluetooth is ready");
    #endif

    return err;   // err is equal 0 or error code for semaphore wait
  }

//==============================================================================
// public module interface
//==============================================================================
//
// U = bl_up;  D = bl_down;  B = bl_ble
//
//                  +--------------------+
//                  |        bl_wl       | wireless BLE core
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (D)->     INIT ->|        (to)        | init module, store (to) callback
// (D)->     TICK ->|       @ix,cnt      | tick the module
// (D)->     TOCK ->|       @ix,cnt      | tock the module
// (D)->      LIB ->|       <BL_lib>     | add library/service
// (D)->      SVC ->|      (service)     | register BLE service
//                  +--------------------+
//                  |        BLE:        | BLE input interface
// (D)->   ENABLE ->|      <BL_ble>      | enable Bluetooth
// (D)->   UPDATE ->|      <BL_ble>      | update advertising data
// (D)->      MAC ->|      <BL_mac>      | get Bluetooth mac address
//                  |....................| BLE output interface
// (B)<-   ENABLE <-|      <BL_ble>      | enable Bluetooth
// (U)<-    READY <-|      <BL_ble>      | report Bluetooth to be ready
// (*)<-    SETUP <-|    <BL_ble>,mode   | setup all registered BLE services
// (B)<-   UPDATE <-|      <BL_ble>      | update advertising data
// (U)<-  CONNECT <-|      <BL_conn>     | notify connection event
// (U)<-   DISCON <-|  <BL_conn>,reason  | notify disconnection event
// (U)<-      MTU <-|       @rx,tx       | notify MTU update
//                  +--------------------+
//
//==============================================================================

  int bl_wl(BL_ob *o, int val)
  {
    static BL_oval U = bl_up;          // up gear
    static BL_oval B = bl_ble;         // BLE module

      // Next stuff all deals with service registering and service execution.
      // Services have to be dispatched before standard message ID dispatching

    static BL_lib *services = NULL;

    if (bl_cl(o) == _SVC)              // all SVC: class tags
      return bl_iter(o,val,services);  // iterate through services list
    else if (bl_id(o) == BL_ID(_BLE,SETUP_))
      return bl_iter(o,val,services);  // iterate through services list
    else if (bl_is(o,_SYS,LIB_))       // [SYS:LIB @ix,<BL_lib>]
      return bl_reglink(o,&services);  // register library in services list
    else if (bl_is(o,_SYS,SVC_))       // [SYS:SVC (service)]
    {
      LOG(5,"service register request");
      return bl_init(o->data,PMI);     // init service to register here at PMI
    }

      // dispatch in the usual way

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):          // [SYS:INIT (to)]
        LOG(2,BL_B "init tiny BLE wireless core");
        U = bl_cb(o,(U),WHO"(U)");     // store output callback
        return bl_init(B,PMI);         // init BLE sub module

      case BL_ID(_SYS,TICK_):          // [SYS:TICK @0,<BL_pace>,cnt]
        return 0;                      // OK - nothing to tick/tock

      case BL_ID(_SYS,TOCK_):          // [SYS:TOCK @0,<BL_pace>,cnt]
        bl_fwd(o,val,B);               // tock BLE module
        return 0;

      case BL_ID(_BLE,ENABLE_):        // D -> [BLE:ENABLE]
        return ble_enable(o,val);      // delegate to handler

      case BL_ID(_BLE,START_):         // D -> [BLE:START <BL_ble>]
      case BL_ID(_BLE,STOP_):          // D -> [BLE:STOP <BL_ble>]
      case BL_ID(_BLE,MAC_):           // D -> [BLE:MAC <BL_mac>]
      case BL_ID(_BLE,UPDATE_):        // D -> [BLE:UPDATE <BL_ble>]
        return bl_fwd(o,val,B);        // forward to bl_ble

      case BL_ID(_BLE,READY_):         // [BLE:READY <BL_ble>]
        #if (CFG_BLE_SYC_ENABLE)       //  syncronous protocl for BLE enable
          k_sem_give(&bt_ready_sem);   // give BT-ready-semaphore
          return 0;
        #else
          return bl_out(o,val,U);      // forward to up gear
        #endif

      case _BL_ID(_BLE,SETUP_):        // [BLE.SETUP 0,<BL_ble>]
        bl_msg((B), _BLE,SETUP_, 0,bl_data(o),val);
        for (BL_lib *p=services; p; p = p->next) // setup registered BLE svc's
			    bl_msg((p->module), _BLE,SETUP_, p->id,bl_data(o),val);

      case BL_ID(_BLE,CONNECT_):       // [BLE:CONNECT <BL_conn>]
      case BL_ID(_BLE,DISCON_):        // [BLE:DISCON  <BL_conn>,reason]
      case BL_ID(_BLE,MTU_):           // [BLE:MTU  @rx,NULL,tx]
        return bl_out(o,val,U);        // forward to up gear

      case _BL_ID(_BLE,ENABLE_):       // [BLE:ENABLE <BL_ble>]
        return bl_out(o,val,B);        // forward to bl_ble module

      default:
        return -1;                     // bad input
    }
  }

//==============================================================================
// cleanup
//==============================================================================

  #undef WHO
