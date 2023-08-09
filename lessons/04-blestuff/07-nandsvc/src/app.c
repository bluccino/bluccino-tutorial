//==============================================================================
// app.c for 07-nandsvc sample (NAND service peripheral)
//==============================================================================

  #include "bluccino.h"
  #include "app.h"
  #include "bl_ble.h"
  #include "nandsvc.h"

//==============================================================================
// BLE service ID and PMI definition, logging shorthands
//==============================================================================

  #define WHO  "app:"

  #define LOG                     LOG_APP
  #define LOGO(lvl,col,o,val)     LOGO_APP(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_APP(lvl,col,o,val)

  #define PMI                     app  // public module interface
  BL_PMI(PMI)                          // define static _bl_pmi() function

//==============================================================================
// helper: battery drain simulation
//==============================================================================

  static void signal_change(void)
  {
    TP_nand nand;
    _bl_pmi(_SVC,GET_, BL_SID(NAND_),&nand,0);

    nand.in1 = nand.in2 = nand.out;
    nand.out = !(nand.in1 && nand.in2);

    _bl_pmi(_SVC,SET_, BL_SID(NAND_),&nand,0);
  }

//==============================================================================
// handler: [SYS:TOCK @ix,<BL_pace>,cnt] // battery drain simulation
//==============================================================================

  static int sys_tock(BL_ob *o, int val)
  {
    if (bl_period(o,5000))             // every 5s we simulate a battery drain
      signal_change();                 // changing signal levels
    return 0;
  }

//==============================================================================
// app
//==============================================================================
//
// M = main;  U = bl_up;  D = bl_down;  T = bl_top;
//
//                  +--------------------+
//                  |        app         | application top module
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     INIT ->|        (to)        | init module, store (to) callback
// (M)->     TOCK ->|  @ix,<BL_pace>,cnt | system tock
//                  +--------------------+
//                  |        BLE:        | BLE input interface
// (U)->  CONNECT ->|      <BL_conn>     | notify connection event
// (U)->   DISCON ->|  <BL_conn>,reason  | notify disconnection event
//                  |....................| BLE output interface
// (D)<-   ENABLE <-|      <BL_ble>      | enable Bluetooth
// (D)<-    START <-|      <BL_ble>      | start BLE advertising
// (T)<-  CONNECT <-|      <BL_conn>     | notify connection event
// (T)<-   DISCON <-|  <BL_conn>,reason  | notify disconnection event
//                  +--------------------+
//                  |        SVC:        | SVC output interface
// (D)<-      GET <-|   @sid,<BL_data>   | get BLE service data
// (D)<-      SET <-|   @sid,<BL_data>   | set BLE service data
//                  +--------------------+
//
//==============================================================================

  int app(BL_ob *o, int val)
  {
    static BL_ble ble;
    static BL_oval D = bl_down;
    static BL_oval T = bl_top;

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        bl_log(1,BL_B"init app");
        _bl_pmi(_BLE,ENABLE_, 0,&ble,0);         // enable Bluetooth
        return _bl_pmi(_BLE,START_, 0,&ble,0);   // start BLE advertising

      case BL_ID(_SYS,TOCK_):
        return sys_tock(o,val);        // enable Bluetooth

      case _BL_ID(_BLE,ENABLE_):       // enable BLE (Bluetooth) system
      case _BL_ID(_BLE,START_):        // start BLE advertising
        return bl_out(o,val,D);        // enable BLE service

      case BL_ID(_BLE,CONNECT_):       // BLE service connection
      case BL_ID(_BLE,DISCON_):        // BLE service disconnection
        return bl_out(o,val,T);        // forward to top gear

      case _BL_ID(_SVC,GET_):          // [SVC.GET @sid,<BL_data>]
      case _BL_ID(_SVC,SET_):          // [SVC.SET @sid,<BL_data>]
        return bl_out(o,val,D);        // interact with BLE service module

      default:
        return BL_VOID;
    }
  }
