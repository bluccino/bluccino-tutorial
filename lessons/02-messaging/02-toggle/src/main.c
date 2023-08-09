//==============================================================================
// main.c for 02-toggle (tiny LED toggle control)
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
// toggle.b (event message flow)
//
//  (A) := app;  (D) := bl_down;  (U) := bl_up;
//
//  (U) -> [BUTTON:PRESS @ix] -> (A);
//  (A) -> [LED:SET @ix,onoff[@ix]] -> (D);
//
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
// (D)<-   TOGGLE <-|        @-1         | toggle all LEDs
//                  +--------------------+
//==============================================================================

  int app(BL_ob *o, int val)
  {
    static BL_oval D = bl_down;                       // down gear

    switch (bl_id(o))
    {
      case BL_ID(_BUTTON,PRESS_):
        return bl_msg((D), _LED,TOGGLE_, -1,NULL,0);  // toggle all LEDs

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
    bl_engine(app,10,1000);                 // run 10ms/1000ms tic/toc engine
  }
