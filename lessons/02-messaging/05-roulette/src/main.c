//==============================================================================
// main.c for 05-roulette (posting custom messages)
//==============================================================================

#include "bluccino.h"                  // bluccino swiss knife
#include "bl_util.h"                   // to access bl_rand() function

//==============================================================================
// app module (public module interface)
//==============================================================================
// (M) := main;
//                  +--------------------+
//                  |        app         | app module (high level)
//                  +--------------------+
//                  |       RANDOM:      | RANDOM input interface
// (M)->   RESULT ->|  @ix,<BL_txt>,val  | receive random stuff
//                  +--------------------+
//==============================================================================

  int app(BL_ob *o, int val)
  {
    static BL_txt color[3] = {BL_R,BL_G,BL_0};

    if (bl_is(o,_RANDOM,RESULT_))  // dispatch [RANDOM:RESULT @ix,<BL_txt>,val]
    {
      int ix = bl_ix(o);           // instance index
      BL_txt data = bl_data(o);
      bl_logo(2,BL_Y"app:",o,val);
      bl_log(1, "%snumber %d - %s wins", color[ix], val, data);

      bl_led(-1,0);  bl_led(2+ix,1); // after all LEDs off set LED @ix on
    }

    return 0;
  }

//==============================================================================
// wheel module (roulette wheel - public module interface)
//==============================================================================
// (M) := main;  (A) := app
//
//                  +--------------------+
//                  |        wheel       | app module (high level)
//                  +--------------------+
//                  |       BUTTON:      | BUTTON input interface
// (U)->    PRESS ->|        @ix         | button @ix clicked
//                  +--------------------+
//                  |       RANDOM:      | RANDOM output interface
// (A)<-   RESULT <-|  @ix,<BL_txt>,val  | receive random stuff
//                  +--------------------+
//==============================================================================

  int wheel(BL_ob *o, int val)
  {
    int ix;
    BL_txt data[3] = {"red","zero","black"};
    enum color {R,G,B} cix[] = {G,R,B,R,B,R,B,R,B,R,B, B,R,B,R,B,R,B,R,
                                  R,B,R,B,R,B,R,B,R,B, B,R,B,R,B,R,B,R};
    switch (bl_id(o))
    {
      case BL_ID(_BUTTON,PRESS_):
        bl_log(2,BL_C"wheel is spinning, ball rolling ...");
        return 0;

      case BL_ID(_BUTTON,RELEASE_):
        val = bl_rand(37);             // random value 0 <= val < 37
        ix = cix[val];                 // get actual color index
        bl_msg(app, _RANDOM,RESULT_, ix,data[ix],val);  // send message to app
        return 0;

      default:
        return -1;                     // bad command
    }
  }

//==============================================================================
// main function (set verbose level and run tick/tock engine)
//==============================================================================

  void main(void)
  {
    bl_hello(4,PROJECT" (press button)");    // verbose level 4, print hello msg
    bl_init(bl_gear,wheel);                 // init bluccino, msg's go to wheel
  }
