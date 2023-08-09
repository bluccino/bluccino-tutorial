//==============================================================================
// main.c for 06-sos (tiny LED blink control)
//==============================================================================
//
//                  +-----------+                  +-----------+
//     BUTTON:PRESS |           |    LED:BLINK     |           |   LED:SET
// (U)------------->|    app    |----------------->|    led    |------------>(D)
//         @ix      |           | @ix,"pattern",ms |           |   @ix,onf
//                  +-----------+                  +-----------+
//
//==============================================================================

  #include "bluccino.h"                // access Bluccino stuff

  int app(BL_ob *o, int val);

//==============================================================================
// led module (public module interface)
// - activate blinker by setting pattern != 0, and ms > 0; deactivate by ms = 0
//==============================================================================
// (A) := app;  (D) := bl_down;
//                  +--------------------+
//                  |        led         | led module (high level)
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (A)->     TICK ->|       @ix,cnt      | tick the module
//                  +--------------------+
//                  |        LED:        | LED: input interface
// (A)->    BLINK ->|  @ix,"pattern",ms  | set blink pattern and ms-period
//                  |....................|
// (D)<-      SET <-|      @ix,onoff     | set LED @ix on/off (i=0..4)
//                  +--------------------+
//
//==============================================================================

  int led(BL_ob *o, int val)
  {
    static BL_oval D = bl_down;        // down gear

    static const char *pattern = NULL;
    static const char *p = NULL;       // current pointer to blink pattern
    static int ms = 0;                 // blink ms-period
    static int ix = 0;                 // instance ID of LED to blink

    switch (bl_id(o))
    {
      case BL_ID(_SYS,TICK_):
        if (p && pattern && ms > 0 && bl_period(o,ms))
        {
          p = *p ? p : pattern;        // occasionally start pattern again
          bool onoff = (*p++ == '*');
          bl_msg((D), _LED,SET_, ix,NULL,onoff);
        }
        return 0;

      case BL_ID(_LED,BLINK_):         // set blink pattern and ms-period
        ix = bl_ix(o);                 // store instance index
        p = pattern = bl_data(o);      // init current pointer with pattern begin
        ms = val;                      // set ms-period
        bl_msg((D), _LED,SET_, -1,NULL,0);  // turn all LEDs off
        return 0;

      default:
        return -1;
    }
  }

//==============================================================================
// app module (public module interface)
// - dependig on switch @ix status (0/1) de/activate blinker @ix (send ms=0/200)
//==============================================================================
// (M) := main;  (B) := button;  (L) := led;  (D) := bl_down
//                  +--------------------+
//                  |        app         | app module (high level)
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     TICK ->|       @ix,cnt      | tick the module
//                  |....................|
// (L)<-     TICK <-|       @ix,cnt      | tick the module
//                  +--------------------+
//                  |        LED:        | LED: input interface
// (L)<-    BLINK <-|  @ix,"pattern",ms  | set blink sequence and ms-period
//                  +--------------------+
//                  |       BUTTON:      | BUTTON: input interface
// (U)->    PRESS ->|        @ix         | receive button @ix press event
//                  +--------------------+
//==============================================================================

  int app(BL_ob *o, int val)
  {
    static BL_oval L = led;            // led module

    static int ms = 0;
    BL_txt sos = "*-*-*---***-***-***---*-*-*---------";

    switch (bl_id(o))
    {
      case BL_ID(_SYS,TICK_):
        return bl_fwd(o,val,(L));

      case BL_ID(_BUTTON,PRESS_):
        ms = ms ? 0 : 200;                               // toggle on/off
        return bl_msg((L), _LED,BLINK_, bl_ix(o),sos,ms);

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
