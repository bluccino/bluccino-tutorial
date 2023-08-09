//==============================================================================
// main.c for 10-ui
//==============================================================================

  #include <zephyr/sys/reboot.h>
  #include "bluccino.h"                // access bluccino stuff
  #include "blink.h"

//==============================================================================
// shorthands
//==============================================================================

  BL_PMI(app)                          // define static inline fct. _bl_pmi()

//==============================================================================
// locals
//==============================================================================

  static int color = 4;                // instance index of red LED

//==============================================================================
// helper: get LED pattern with selected color
//==============================================================================

  static BL_txt pattern(int flashes)
  {
    static BL_txt red   = "r--r--r----------";
    static BL_txt green = "g--g--g----------";
    static BL_txt blue  = "b--b--b----------";

    flashes = BL_SAT(flashes,1,3);
    switch (color)
    {
      case 2:  return red   + (3-flashes)*3;  break;
      case 3:  return green + (3-flashes)*3;  break;
      case 4:  return blue  + (3-flashes)*3;  break;
      default: return "";
    }
  }

//==============================================================================
// handler: [BUTTON:TOCK tocks]
//==============================================================================

  static int button_tock(BL_ob *o, int val)
  {
    static int tocks = 0;              // counter for button tocks

    if (bl_ix(o) != 1)                      // ignore all buttons, except button @1
      return 0;

    bl_log(1,BL_M "button tock: %d",val);

		if (val == 0)
		{
      if (tocks < 6)
      {
        bl_log(1,BL_B "no system reboot");
        _bl_pmi(_LED,BLINK_, 0,"",0);
      }
      else if (tocks >= 6 && tocks < 11)
      {
        bl_log(1,BL_R "actual system reboot ...");
        sys_reboot(SYS_REBOOT_COLD);
      }
		  else if (tocks >= 11)
      {
        bl_log(1,BL_R "actual factory reboot ...");
        sys_reboot(SYS_REBOOT_COLD);
      }

      _bl_pmi(_LED,BLINK_, 0,"",0);    // blinker and LEDs off
		}

		tocks = val;

		if (val == 1)
		  _bl_pmi(_LED,BLINK_,0,"sssssssss-",100);
		else if (val == 4)
		  _bl_pmi(_LED,BLINK_,0,"ssss-",100);
		else if (val == 6)
    {
      bl_log(1,BL_R "performing system reset ...");
		  _bl_pmi(_LED,BLINK_,0,"wwwwwwwww-",100);
    }
		else if (val == 9)
		  _bl_pmi(_LED,BLINK_,0,"wwww-",100);
		else if (val == 11)
		{
      int starts = 0;                  // counts system starts
      int color = 4;                   // re-initialize blue color index

      bl_save("starts", &starts, sizeof(starts));
      bl_save("color", &color, sizeof(color));

		  bl_log(1,BL_R "performing factory reset ...");
		  _bl_pmi(_LED,BLINK_,0,"r-",150); // blinker (all LEDs) off
		}

		return 0;
  }

//==============================================================================
// handler: [BUTTON:CLICK @ix,n]
//==============================================================================

  static int button_click(BL_ob *o, int val)
  {
    if (bl_ix(o) == 1)             // only react on button @1
    {
      bl_log(1,BL_G "app: button @%d, %d clicks",bl_ix(o),val);
      _bl_pmi(_LED,BLINK_, 0,pattern(val),150);
    }

    if (val < 0)
    {
      color = 2 + (color % 3);
      bl_save("color", &color, sizeof(color));
      bl_log(1,BL_C "app: click/hold event: %d => next LED color",val);
      _bl_pmi(_LED,BLINK_, 0,pattern(1),150);
    }

    return 0;
  }

//==============================================================================
// handler: [SWITCH:STS sts]
//==============================================================================

  static int switch_sts(BL_ob *o, int val)
  {
    if (bl_ix(o) > 1)
    {
      bl_log(1,BL_G "app: switch @%d, status:%d",bl_ix(o),val);
      if (val == 0)                // switch @ix is off
        _bl_pmi(_LED,BLINK_, 0,"-",0);   // turn all LEDs off
      else                         // switch @ix is on
        switch (bl_ix(o))
        {
          case 2: _bl_pmi(_LED,BLINK_, 0,"r",0);  break;
          case 3: _bl_pmi(_LED,BLINK_, 0,"g",0);  break;
          case 4: _bl_pmi(_LED,BLINK_, 0,"b",0);  break;
          default: break;
        }
    }
    return 0;
  }

//==============================================================================
// handler: [SYS:INIT (cb)]
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
     static int starts = 0;             // counts system starts
    bl_led(0,0);                       // turn status LED initially off

      // activate (3x all LEDs @ period 1s) startup blinking

    _bl_pmi(_LED,BLINK_, 0,"wwwww-----wwwww-----wwwww-----:ss------------",100);

      // load system starts counter from NVM

    bl_load("starts", &starts, sizeof(starts));
    starts++;                          // increment number of system starts
    bl_log(1,BL_M "system start: #%d",starts);

      // save system starts counter to NVM

    bl_save("starts", &starts, sizeof(starts));

      // load LED id

    bl_load("color", &color, sizeof(color));
    bl_log(1,BL_R "loaded LED @ix = %d from NVM",color);

    return 0;                          // OK
  }

//==============================================================================
// module app (PMI)
//==============================================================================
//
// M := main;  U := bl_up;  T := bl_top;
//                  +--------------------+
//                  |        APP         |
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     INIT ->|        (cb)        | init module, ignore <out> arg
//                  +--------------------+
//                  |      BUTTON:       | BUTTON input interface
// (U)->    PRESS ->|        @ix,0       | button press at time 0
// (U)->  RELEASE ->|        @ix,ms      | button release after ms
// (U)->    CLICK ->|        @ix,n       | number of button clicks
// (U)->     TOCK ->|       @ix,ms       | button tock event, every 1000 ms
//                  +--------------------+
//                  |       LED:         | LED output interface
// (T)<-    BLINK <-|  @ix,"pattern",ms  | set blink pattern
//                  +--------------------+
//
//==============================================================================

  int app(BL_ob *o, int val)           // app function, handling all messages
  {                                    // which are forwarded by Bluccino layer
    static BL_oval T = bl_top;         // top gear

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):          // dispatch [SYS:INIT] event
        return sys_init(o,val);        // delegate to init handler

      case BL_ID(_BUTTON,PRESS_):      // dispatch [BUTTON:PRESS] event
        bl_logo(1,"app:ignore",o,val);
        return 0;

      case BL_ID(_BUTTON,RELEASE_):    // dispatch [BUTTON:RELEASE] event
        bl_logo(1,"app:ignore",o,val);
        return 0;

      case BL_ID(_BUTTON,CLICK_):      // dispatch [BUTTON:PRESS] event
			  return button_click(o,val);    // delegate to handler

       case BL_ID(_BUTTON,HOLD_):       // dispatch [BUTTON:HOLD] event
        if (val == 0)
        {
          bl_log(1,BL_G "app: button @%d hold => all LEDs off",bl_ix(o));
          _bl_pmi(_LED,BLINK_,0,"",0); // blinker (all LEDs) off
        }
			  return 0;

      case BL_ID(_BUTTON,TOCK_):       // dispatch [BUTTON:TOCK] event
        return button_tock(o,val);     // delegate to handler

      case BL_ID(_SWITCH,STS_):        // dispatch [SWITCH:STS sts]
        return switch_sts(o,val);

     case _BL_ID(_LED,BLINK_):        // set blink pattern
        return bl_out(o,val,(T));

      default:
        return BL_VOID;
    }
  }

//==============================================================================
// main entry point
// - set verbose level, print hello message and run init/tick/tock engine
//==============================================================================

  void main(void)
  {
    bl_hello(3,PROJECT " (click any of the buttons)");
    bl_install(blink);                 // install blink module in top gear libs
    bl_engine(app,50,1000);            // run tick/tock engine, cb's go to app
  }
