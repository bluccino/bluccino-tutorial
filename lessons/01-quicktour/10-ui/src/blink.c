//==============================================================================
// blink.c - blink module
//==============================================================================
// module blinker (PMI)
// - [LED:BLINK @0,"s",0]              // status LED @0 on
// - [LED:BLINK @0,"-",0]              // status LED off
// - [LED:BLINK @0,"s-",500]           // blinking status LED
// - [LED:BLINK @0,"r",0]              // RGB LED red on
// - [LED:BLINK @0,"-",0]              // RGB LED off
// - [LED:BLINK @0,"rrr---:r-",500]    // init pattern "rrr---",then repeat "r-"
// - [LED:BLINK @0,"",0]               // blinker off
//==============================================================================

  #include "bluccino.h"                // access bluccino stuff
  #include "blink.h"                   // blinker module

//==============================================================================
// shorthands
//==============================================================================

  #define PMI blink                    // public module interface
  BL_PMI(PMI)                          // define static inline fct. _bl_pmi()

//==============================================================================
// locals
//==============================================================================

  static BL_txt pattern = NULL;
  static BL_txt p = NULL;
  static int period = 0;               // zero means: blinker is inactive
  static char previous = 0;

//==============================================================================
// helper: set led
//==============================================================================

  static int led(int ix, int onoff)
  {
    return _bl_pmi(_LED,SET_, ix,NULL,onoff);
  }

//==============================================================================
// helper: control leds (according to pattern character code)
//==============================================================================

   static char led_control(char c)
   {
     switch (c)
     {
       case 's': led(-1,0); led(1,1); break;     // status LED on
       case 'r': led(-1,0); led(2,1); break;     // red LED on
       case 'g': led(-1,0); led(3,1); break;     // green LED on
       case 'b': led(-1,0); led(4,1); break;     // blue LED on
       case 'w': led(-1,1); break;               // white LED (RGB) on
       default:  led(-1,0); break;               // all LEDs off
     }
     return c;
   }

//==============================================================================
// handler: [LED:BLINK @prio,"pattern",period]
//==============================================================================

  static int led_blink(BL_ob *o, int val)
  {
    if (bl_data(o) == 0)
      return bl_err(-EINVAL,"invalid pattern (NULL)");

    p = pattern = bl_data(o);
    period = p ? val : 0;
    previous = 0;
    bl_log(2,BL_Y"set blink pattern \"%s\",%d",pattern?pattern:"", period);

      // zero period means inactive blinker but 1-time immediate LED control

    if (!period)
    {
      led_control(*p);
      bl_log(2,BL_Y "blinker inactive");
    }
    return 0;
  }

//==============================================================================
// handler: [SYS:TICK (cb)]
//==============================================================================

  static int sys_tick(BL_ob *o, int val)
  {
	  if (period && p && bl_period(o,period))
	  {
	    bl_log(5,BL_G"p:\"%s\", pattern:\"%s\", period:%d", p,pattern,period);
	    if (*p == ':')               // pattern separator
	    {
	      p = pattern = p+1;         // set p and pattern to repeat part
	      bl_log(5,BL_C"p:\"%s\", pattern:\"%s\", period:%d",p,pattern,period);
	    }

	    if (!*p)
	    {
	      p = pattern;               // load repeat pattern
	      if (!*p)                   // is there an effective pattern?
	      {
	        bl_log(2,BL_Y "deactivate blinker");
	        period = 0;              // deactivate blinker if no pattern
	      }
	    }

	      // process current character of blink pattern

	    char c = *p++;               // current character of pattern
	    if (c != previous)           // is there a change?
	      previous = led_control(c); // control LED level
	  }
    return 0;
  }

//==============================================================================
// A = app;  D = bl_down;  M = main;
//                  +--------------------+
//                  |       BLINK        |
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (A)->     INIT ->|       <out>        | init module, ignore <out> arg
// (A)->     TICK ->|      @ix,cnt       | receive system ticks (50ms period)
// (M)->  INSTALL ->|                    | advise to self-install @ top gear
//                  |....................| SYS output interface
// (T)<-      LIB <-|      <BL_lib>      | register library
//                  +--------------------+
//                  |       LED:         | LED input interface
// (A)->    BLINK ->|  @ix,"pattern",ms  | set blink pattern
//                  |....................| LED output interface
// (D)<-      SET <-|     @ix,onoff      | set on/off status of LED @ix
//                  +--------------------+
//==============================================================================

  int blink(BL_ob *o, int val)
  {
    static BL_lib lib = {PMI,BL_ID(_LIB,NODE_),NULL};  // lib ID <LIB:NODE>
    static BL_oval D = bl_down;
    static BL_oval T = bl_top;

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        return 0;

      case BL_ID(_SYS,TICK_):
        return sys_tick(o,val);        // delegate to handler

     case BL_ID(_SYS,INSTALL_):        // advise to self register @ top gear
			 bl_log(3,"install `blink` in top gear");
			 return bl_lib((T),&lib);        // register bl_node as lib @ top  gear

      case BL_ID(_LED,BLINK_):
        return led_blink(o,val);       // delegate to handler

      case _BL_ID(_LED,SET_):
        return bl_out(o,val,(D));

      default:
        return BL_VOID;
    }
  }
