//==============================================================================
// main.c for 04-cbeacon sample (Eddystone beacon)
//==============================================================================

  #include "bluccino.h"
  #include "bl_ble.h"
  #include "cbeacon.h"

//==============================================================================
// BLE service ID and PMI definition, logging shorthands
//==============================================================================

	#define PMI                     app  // public module interface
	BL_PMI(PMI)                          // define static _bl_pmi() function

//==============================================================================
// our BLE receipe for the beacon
//==============================================================================

    static BL_ble ble;

//==============================================================================
// handler: [BUTTON:PRESS @ix]
//==============================================================================

  static int button_press(BL_ob *o, int val)
  {
    size_t size = sizeof(AD_cbeacon);

      // access manufacturing data (as part of advertising data)

    AD_cbeacon *p = bl_adata(&ble,BT_DATA_MANUFACTURER_DATA,size);

    if (p)
    {
      p->major_lo = p->major_lo+1;  // increment major
      p->minor_lo = p->minor_lo+1;  // increment minor

        // retrieve current <major|minor>

      BL_word major = BL_HBLB(p->major_hi,p->major_lo);
      BL_word minor = BL_HBLB(p->minor_hi,p->minor_lo);

      bl_log(1,BL_M "new beacon ID: <0x%04X|0x%04X>",major,minor);

        // increment minor and update advertiser

      _bl_msg(PMI, _BLE,UPDATE_, 0,&ble,0);
    }
    return 0;
  }

//==============================================================================
// app
//==============================================================================
//
// M := main;  U := bl_up;  D := bl_down;
//
//                  +--------------------+
//                  |        app         | application top module
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     INIT ->|        (to)        | init module, store (to) callback
//                  +--------------------+
//                  |       BUTTON:      | BUTTON input interface
// (M)->    PRESS ->|         @ix        | notify button @ix press
//                  +--------------------+
//                  |        BLE:        | BLE output interface
// (D)<-   ENABLE <-|      <BL_ble>      | enable Bluetooth
// (D)<-    START <-|      <BL_ble>      | start BLE advertising
// (D)<-   UPDATE <-|      <BL_ble>      | update advertising data
//                  +--------------------+
//
//==============================================================================

  int app(BL_ob *o, int val)
  {
    static BL_oval D = bl_down;

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        bl_log(1,BL_B"init app");
        _bl_msg(PMI, _BLE,ENABLE_, 0,&ble,0);  // enable Bluetooth
        _bl_pmi(_BLE,START_, 0,&ble,0);     // start BLE advertising
        return ble.err;

      case BL_ID(_SYS,TOCK_):
        if (bl_period(o,10000) || val == 0)
          bl_log(1,"press button to change beacon ID");
        return 0;

      case BL_ID(_BUTTON,PRESS_):
        return button_press(o,val);        // delegate to button_press() handler

      case _BL_ID(_BLE,ENABLE_):            // enable Bluetooth
      case _BL_ID(_BLE,START_):             // start BLE advertising
      case _BL_ID(_BLE,UPDATE_):            // update advertiser data
        return bl_out(o,val,D);

      default:
        return -1;
    }
  }

//==============================================================================
// main function
// - set verbose level and print hello message
// - run init/tick/tock engine
//==============================================================================

  void main(void)
  {
    bl_hello(4,PROJECT);                 // verbose level 4, print hello message
    bl_service(cbeacon);                 // register cBeacon service in WL core
    bl_engine(app,10,1000);              // run 10ms/1000ms tick/tock engine
  }
