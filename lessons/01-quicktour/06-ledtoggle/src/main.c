//==============================================================================
// main.c for 06-ledtoggle (tiny LED toggle control by buttons)
//==============================================================================
// App Description
// - all LEDs are off at the beginning
// - any button press activites blinking of first LED for 5s
// - any further button press activates blinking of next LED for 10s
//
// Lessons to Learn
// - learn how to setup a standard Bluccino framework, reacting on standard
//   timing events (tick/tock), button events and controlling the board's LEDs
// - demonstration of role of main() - Bluccino main()'s have usually 2-3 lines
// - show standard implementation of an APP module with dispatcher and workers
// - demonstrate how module interfaces (of app) should be documented
// - demonstrate how worker functions should be documented
//==============================================================================

  #include "bluccino.h"
  #include "bl_hw.h"

  static int id = -1;                  // id of blinking LED (id=-1: invalid)
  static bool enable = false;          // blinking enabled?
  static BL_ms due = 0;                // due time for turning off blinker

//==============================================================================
// worker: button press (handles [BUTTON:PRESS] events)
// - turns off current selected LED @ix
// - cycle increments @ix (1->2->3->4 -> 1->2->...) to select next LED @ix
// - enable blinking of selected LED @ix, and setting up a timeout for blinking
//==============================================================================

  static int button_press(BL_ob *o, int val)
  {
    bl_led(id,0);                      // turn off current LED @ix
    id = 1 + (id  % 4);                // cycle id through 1,2,3,4 -> 1,2,...
    due = bl_ms() + 5000;              // due at time (now + 5s)
    enable = true;                     // blinker enabled
    return 0;                          // OK
  }

//==============================================================================
// worker: system tick (handles [SYS:TICK] events) - tick() is called every 10ms
// - is passive if current LED @ix is zero
// - otherwise toggles LED every 80-th tick (every 800ms, as tick period = 10ms)
//==============================================================================

  static int sys_tick(BL_ob *o, int val)
  {
    if (enable && bl_ms() >= due)      // if blinker is enabled and we are due
    {
      bl_led(id,0);                    // turn off current LED @ix
      enable = false;                  // disable blinker
    }
    return 0;                          // OK
  }

//==============================================================================
// worker: system tock (handles [SYS:TOCK] events)
// - behaves passive if blinking not enabled (enable=false)
// - otherwise toggles LED every 5-th tock (every 500ms, as tock period = 100ms)
//==============================================================================

  static int sys_tock(BL_ob *o, int val)
  {
    if (val % 5 == 0 && enable)        // every 5-th tick (500ms), if enabled
      bl_led(id,-1);                   // toggle selected LED
    return 0;                          // OK
  }

//==============================================================================
// worker: system init (handles [SYS:INIT <out>] events)
// - prints a log so we can cross check whether APP module is initialized
// - init id with value 0 (next LED selection to be @1) (note: blinker disabled)
// - <out> argument is ignored, since APP does not emit messages
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    bl_log(2,BL_B "init app");         // event message log in blue
    id = 0;                            // init LED @ix=0 => starts with LED @1
    bl_log(1,"=> click button to cycle to next LED to be toggled!");
    return 0;                          // OK
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (M) := (main);  (U) := (bl_up)
//                  +--------------------+
//                  |        app         |
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     INIT ->|       <cb>         | init module, store output callback
// (M)->     TICK ->|       @ix,cnt      | system tick
// (M)->     TOCK ->|       @ix,cnt      | system tock
//                  +--------------------+
//                  |       BUTTON:      | BUTTON output interface
// (U)->    PRESS ->|        @ix,1       | button @ix pressed (rising edge)
//                  +--------------------+
//
//==============================================================================

  int app(BL_ob *o, int val)           // app receiving event messages
  {
    switch (bl_id(o))                  // dispatch message ID
    {
      case SYS_INIT_0_cb_0:            // [SYS:INIT <cb>] => init module
        return sys_init(o,val);        // forward to sys_init() worker

      case SYS_TICK_ix_BL_pace_cnt:          // [SYS:TICK @ix,cnt] => handle sys tick
        return sys_tick(o,val);        // forward to sys_tick() worker

      case SYS_TOCK_ix_BL_pace_cnt:          // [SYS:TOCK @ix,cnt] => handle sys tick
        return sys_tock(o,val);        // forward to tock() worker

      case BUTTON_PRESS_ix_0_0:        // [BUTTON:PRESS @ix] => putton pressed
        return button_press(o,val);    // forward to button_press() worker

      default:
        return 0;                      // OK - ignore anything else
    }
  }

//==============================================================================
// main program
// - sets verbose level, prints hello message
// - and runs app with init/tick/tock using tick/tock/engine
//==============================================================================

  void main(void)
  {
    bl_hello(2,PROJECT " (click button to cycle to next LED to be toggled)");
    bl_cfg(bl_down,_BUTTON,BL_PRESS);  // configure only [BUTTON:PRESS] events
    bl_engine(app,10,100);             // run APP with 10/100ms ticks/tocks
  }
