//==============================================================================
// main.c for 04-light (messaging implementation basics)
//==============================================================================
//
// light control
//       +-------------+                         +-------------+
//       |             |       LIGHT:LEVEL       |             |
//       |    main     |------------------------>|     app     |
//       |             |      @ix,"room",val     |             |
//       +-------------+                         +-------------+
//
//                   main                        app
//                   (M)                         (A)
//                    |        LIGHT:LEVEL        |
//                    o-------------------------->|
//                    |      @ix,"room",val       |
//                    |                           |
//
//==============================================================================
// light.b (light control)
//
//  (M) := main;  (A) := app;
//
//  (M) -> [LIGHT:LEVEL @ix,"room",level] -> (A);
//
//==============================================================================

#include "bluccino.h"                  // bluccino swiss knife

//==============================================================================
// app module (public module interface)
//==============================================================================
// (M) := main;
//                  +--------------------+
//                  |        app         | app module (high level)
//                  +--------------------+
//                  |       LIGHT:       | LIGHT input interface
// (M)->    LEVEL ->|   @ix,"room",val   | light level control
//                  +--------------------+
//==============================================================================

  int app(BL_ob *o, int val)
  {
    if (bl_is(o,_LIGHT,LEVEL_))        // dispatch [LIGHT:LEVEL @ix,"room",val]
    {
      BL_txt room = bl_data(o);        // name of room
      int ix = bl_ix(o);               // index of luminaire
      bl_log(1, "set %s light level @%d: %d", room, ix, val);
      bl_led(ix, val>0);               // set LED @ix on (val>0) or off (val==0)
    }

    return 0;
  }

//==============================================================================
// main function (set verbose level and run tick/tock engine)
//==============================================================================

  void main(void)
  {
    bl_hello(4,PROJECT);               // verbose level 4, print hello msg
    bl_init(bl_gear,app);             // init bluccino, notifications go to app

    BL_ob oo = {_LIGHT,LEVEL_,1,"kitchen"};

    while (1)
    {
      app(&oo,7500);                   // [LIGHT:LEVEL @1,"kitchen",7500] -> app
      bl_sleep(2000);                  // sleep 2000 ms
      app(&oo,0);                      // [LIGHT:LEVEL @1,"kitchen",0] -> app
      bl_sleep(2000);                  // sleep 2000 ms
    }
  }
