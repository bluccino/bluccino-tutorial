//==============================================================================
// app.c for 01-advertise sample (simple advertising)
//==============================================================================

  #include "bluccino.h"
  #include "app.h"
  #include "bl_ble.h"

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
// app
//==============================================================================
//
// (M) := main;  (U) := bl_up;  (D) := bl_down;
//
//                  +--------------------+
//                  |        app         | application top module
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     INIT ->|        (to)        | init module, store (to) callback
// (M)->     TOCK ->|  @ix,<BL_pace>,cnt | system tock
//                  +--------------------+
//                  |        BLE:        | BLE output interface
// (D)<-   ENABLE <-|      <BL_ble>      | enable Bluetooth
// (D)<-    START <-|      <BL_ble>      | start BLE advertising
//                  +--------------------+
//
//==============================================================================

  int app(BL_ob *o, int val)
  {
    static BL_ble ble;
    static BL_oval D = bl_down;

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        bl_log(1,BL_B"init app");
        _bl_pmi(_BLE,ENABLE_, 0,&ble,0); // enable Bluetooth
        _bl_pmi(_BLE,START_, 0,&ble,0);  // start BLE advertising
        return ble.err;

      case BL_ID(_SYS,TOCK_):
        if (bl_period(o,10000))          // every 10 seconds
          bl_log(1,"I'm alive!");        // live signal
        return 0;                        // enable Bluetooth

      case _BL_ID(_BLE,ENABLE_):         // enable BLE (Bluetooth) system
      case _BL_ID(_BLE,START_):          // start BLE advertising
        return bl_out(o,val,D);

      default:
        return -1;
    }
  }
