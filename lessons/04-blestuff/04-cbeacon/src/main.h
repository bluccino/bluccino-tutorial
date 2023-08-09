//==============================================================================
// main.h for 03-cbeacon demo (Eddystone beacon)
//==============================================================================
//
// Module hierarchy and pipeline setup
//
// main => bl_engine(app,tick_ms,tock_ms)
//  |
//  +- app                                     (application)
//  |   |<==================================|
// .|.......................................|...................................
//  |                                       |
//  +- bluccino ===========================>|  (Bluccino module)
//      |                                   |
//      +- bl_top =========================>|  (top gear)
//      |   |                               |
//      |   |<========================|     |  // top gear needs some messages
//      |                             |     |
//      +- bl_up =====================|====>|  (up gear)
//      |   |
//      |   |<==============================|
//      |                                   |
//      +- bl_down                          |  (down gear)
//          |                               |
// .........|...............................|...................................
//          |                               |
//          +- bl_core ====================>|  (core system)
//              |                           |
//              +- bl_wl ==================>|  (tiny BLE wireless core)
//              |  |  |<==============|     |
//              |  |                  |     |
//              |  +- bl_ble ========>|     |  (BLE module)
//              |  +- bl_eddy =======>|     |  (Eddystone BLE service)
//              |  +- bl_ibeacon ====>|     |  (iBeacon BLE service)
//              |  +- bl_bas ========>|     |  (battery BLE service)
//              |                           |
//              +- bl_hw ==================>|  (empty hardware core)
//
//==============================================================================
//
// Message Flow Chart: (note: cBeacon needs explicite registering @ bl_wl!)
//
// main            app              gear     cbeacon
// (M)             (A)             (D)(U)      (C)
//  |               |               |  |        |
//  |    +----------v----------+    |  |        |  // app defines statically
//  |    |  static BL_ble ble; |    |  |        |  // a data structure <ble>
//  |    +----------v----------+    |  |        |  // to hold all receipe
//  |               |               |  |        |
//  |               |               |  |        |  // +-----------------------+
//  |               |   SYS:INIT    |  |        |  // | explicite registering |
//  o------------------------------------------>|  // | of cbeacon @ bl_wl    |
//  |               |   (bl_wl)     |  |        |  // | (wireless core)       |
//  |               |               |  |        |  // +-----------------------+
//  |               |               |  |        |
//  |               |   SYS:INIT    |  |        |  // bl_engine() initializes
//  o------------------------------>|  |        |  // wireless core, which re-
//  |               |    (app)      |  |        |  // gisters cBeacon service
//  |               |               |  |        |
//  |   SYS:INIT    |               |  |        |  // bl_engine() inits app
//  o-------------->|  BLE:ENABLE   |  |        |
//  |    (NULL)     o-------------->|  |        |  // app asks for enabling
//  |               |    <ble>      |  |        |  // BLE system
//  |               |               |  |        |
//  |               |   BLE:READY   |  |        |  // BLE module notifies
//  |               |<-----------------o        |  // app that Bluetooth is
//  |               |               |  |        |  // now ready
//  |               |               |  |        |
//  |               |SERVICE:CBEACON|  |        |  // advise wireless core
//  |               o-------------->|  |        |  // to setup receipe <ble>
//  |               |     <ble>     |  |        |  // for cBeacon service
//  |               |               |  |        |
//  |               |  BLE:START    |  |        |  // start BLE system by
//  |               o-------------->|  |        |  // running advertiser
//  |               |    <ble>      |  |        |  // with <ble> receipe
//  |               |               |  |        |
//
//==============================================================================

#ifndef __MAIN_H__
#define __MAIN_H__

  #define BL_CL_TEXT {BL_CL_SYMBOLS}
  #define BL_OP_TEXT {BL_OP_SYMBOLS, "CBEACON"}

  typedef enum BL_cl {BL_CL_ENUMS} BL_cl;              // class tags
  typedef enum BL_op {BL_OP_ENUMS, CBEACON_} BL_op;    // opcode

#endif // __MAIN_H__