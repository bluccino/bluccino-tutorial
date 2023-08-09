//==============================================================================
// main.c for 08-attention sample
//==============================================================================

#include "bluccino.h"

  static BL_txt msg1 = "I'm a (provisoned) mesh node";
  static BL_txt msg2 = "I'm a (unprovisoned) mesh device";

  static bool attention = 0;           // local copy of attention state
  static bool provision = 0;           // local copy of provision state

//==============================================================================
// app module (public module interface)
//==============================================================================

  static int app(BL_ob *o, int val)    // OVAL interface of app module
  {
    switch (bl_id(o))                  // dispatch message ID
    {
      case SYS_INIT_0_cb_0:            // [SYS:INIT <cb>]
        bl_log(1,"here we go ...");    // init log message
        return 0;                      // OK

      case SYS_TOCK_ix_BL_pace_cnt:    // [SYS:TOCK @ix,cnt]
        if (bl_period(o,5000))
           bl_log(1,"%s",provision?msg1:msg2);

        if (attention)
          bl_log(1,BL_M "attention state");
        return 0;                      // OK

      case MESH_PRV_0_0_sts:           // [MESH:PRV sts] update provision status
        provision = val;               // store a copy updated provision state
        bl_led(1,provision);           // show provision status at LED @1
        return 0;                      // OK

      case MESH_ATT_0_0_sts:           // [MESH:ATT sts] update attention status
        attention = val;               // store a copy updated attention state
        bl_led(2,attention);           // show attention status at LED @2
        return 0;

      default:
        return -1;                     // bad args
    }
  }

//==============================================================================
// main function
// - set verbose level and print hello message
// - run init/tick/tock engine
//==============================================================================

  void main(void)
  {
    bl_hello(5,PROJECT);               // set verbose level, show project title
    bl_engine(app,10,1000);            // run 10/1000ms tick/tock engine
  }
