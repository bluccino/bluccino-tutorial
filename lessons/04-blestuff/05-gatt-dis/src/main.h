//==============================================================================
// main.h for 05-gatt-dis sample (battery service)
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
//              |  +- bl_dis ========>|     |  (device information service)
//              |                           |
//              +- bl_hw ==================>|  (empty hardware core)
//
//==============================================================================
//
// Message Flow Chart: (note: battery service bl_bas is auto-registered by
//                            tiny Bluetooth wireless core)
//
//   main                 app                   gear
//   (M)                  (A)                  (D)(U)
//    |                    |                    |  |
//    |         +----------v----------+         |  | // app defines statically
//    |         |  static BL_ble ble; |         |  | // a data structure <ble>
//    |         +----------v----------+         |  | // to hold all receipe
//    |                    |                    |  |
//    |                    |      SYS:INIT      |  | // bl_engine() initializes
//    o---------------------------------------->|  | // wireless core, which re-
//    |                    |        (app)       |  | // gisters advertising svc.
//    |                    |                    |  |
//    |      SYS:INIT      |                    |  | // bl_engine() inits app
//    o------------------->|     BLE:ENABLE     |  |
//    |       (NULL)       o------------------->|  | // app asks for enabling
//    |                    |       <ble>        |  | // BLE system
//    |                    |                    |  |
//    |                    |      BLE:READY     |  | // BLE module notifies
//    |                    |<----------------------o // app that Bluetooth is
//    |                    |                    |  | // now ready
//    |         +----------v----------+         |  |
//    |         | @sid = BL_SID(DIS_) |         |  | // define BLE service ID
//    |         +----------v----------+         |  |
//    |                    |                    |  |
//    |                    |   SERVICE:SETUP    |  | // advise wireless core
//    |                    o------------------->|  | // to setup receipe <ble>
//    |                    |     @sid,<ble>     |  | // for @sid BLE service
//    |                    |                    |  |
//    |                    |     BLE:START      |  | // start BLE system by
//    |                    o------------------->|  | // running advertiser
//    |                    |       <ble>        |  | // with <ble> receipe
//    |                    |                    |  |
//
//==============================================================================
