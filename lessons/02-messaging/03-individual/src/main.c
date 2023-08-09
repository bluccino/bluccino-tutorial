//==============================================================================
// main.c for 03-individual (individual LED toggle control)
//==============================================================================
//
// toggle control
//                           +-------------+
//             BUTTON:PRESS  |             |     LED:SET
//       (U)---------------->|     app     |---------------->(D)
//                 @ix       |             |    @ix,onoff
//                           +-------------+
//
//           bl_up                 app                 bl_down
//            (U)                  (A)                  (D)
//             |                    |                    |
//             |    BUTTON:PRESS    |                    |
//             o------------------->|                    |
//             |         @ix        |                    |
//             |          +---------v---------+          |
//             |          | onoff[@ix]        |          |
//             |          |    = !onoff[@ix]  |          |
//             |          +---------v---------+          |
//             |                    |      LED:SET       |
//             |                    o------------------->|
//             |                    |   @ix,onoff[@ix]   |
//             |                    |                    |
//
//==============================================================================
/*
// toggle.b

  (A) := app;  (D) := bl_down;  (U) := bl_up;

  (U) -> [BUTTON:PRESS @ix] -> (A)
      -> {
           // onoff[@ix] = ! onoff[@ix];
         }
      -> [LED:SET @ix,onoff[@ix]] -> (D);
*/
//==============================================================================

  #include "bluccino.h"                // access Bluccino stuff

//==============================================================================
// app module (public module interface)
// - dependig on switch @ix status (0/1) de/activate blinker @ix (send ms=0/200)
//==============================================================================
// (D) := bl_down;  (U) := bl_up;
//                  +--------------------+
//                  |        app         | app module (high level)
//                  +--------------------+
//                  |       BUTTON:      | BUTTON: input interface
// (U)->    PRESS ->|         @ix        | receive button @ix press event
//                  +--------------------+
//                  |        LED:        | LED: input interface
// (D)<-      SET <-|      @ix,onoff     | set LED @ix on/off (i=0..4)
//                  +--------------------+
//==============================================================================

  int app(BL_ob *o, int val)
  {
    static BL_oval D = bl_down;          // down gear
    static bool onoff[5] = {0,0,0,0,0};  // on/off status per instance

    switch (bl_id(o))
    {
      case BL_ID(_BUTTON,PRESS_):
        onoff[bl_ix(o)] = !onoff[bl_ix(o)];
        return bl_msg((D), _LED,SET_, bl_ix(o),NULL,onoff[bl_ix(o)]);

      default:
        return -1;
    }
  }

//==============================================================================
// main function (set verbose level and run tick/tock engine)
//==============================================================================

  void main(void)
  {
    bl_hello(4,PROJECT" (press button!)");  // verbose level 4, print hello msg
    bl_engine(app,10,1000);                 // setup 10ms/1000ms tic/toc engine
  }
