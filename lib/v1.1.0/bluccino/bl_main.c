//==============================================================================
// bl_main.c skeleton for BLE/mesh applications
//==============================================================================

  #include "bluccino.h"

  #define VERSION  "blmain v0.0.0"
  #define VERBOSE  4                   // verbose level for application

//==============================================================================
// MAIN level logging shorthands
//==============================================================================

  #define LOG                     LOG_MAIN
  #define LOGO(lvl,col,o,val)     LOGO_MAIN(lvl,col"main:",o,val)
  #define LOG0(lvl,col,o,val)     LOGO_MAIN(lvl,col,o,val)

//==============================================================================
// tick function (sends tick messages to all modules which have to be ticked)
//==============================================================================

  static int tick(BL_ob *o, int val)   // system ticker: ticks all subsystems
  {
     LOGO(4,BL_Y,o,val);               // log to see we are alife
     return 0;
  }

//==============================================================================
// tock function (sends tock messages to all modules which have to be tocked)
//==============================================================================

  static int tock(BL_ob *o, int val)   // system tocker: tocks all subsystems
  {
     LOGO(1,BL_Y,o,val);               // log to see we are alife
     return 0;
  }

//==============================================================================
// app init
// - 1) init all modules of app (note: Bluccino init is done in main engine!)
// - 2) setup connections: all outputs of SOS module have to go to LED module
//==============================================================================

  static void init(void)               // init all modules
  {
  }

//==============================================================================
// main engine
// - calling Bluccino init and app init() function
// - periodic (100ms)  calls of app tick() function
// - periodic (5000ms) calls of app tock() function
//==============================================================================

  void main(void)
  {
    bl_hello(VERBOSE,VERSION);

    bl_init(bl_gear,NULL);            // Bluccino init
    init();                            // app init

    int tocks = 0;                     // tock counter
    for(int ticks=0;;ticks++)          // loop generating (approx) 500ms ticks
    {
      bl_tick(tick,0,ticks);           // app tick

      if (ticks % 50 == 0)             // app tock is 50 times slower
        bl_tock(tock,1,tocks++);       // app tock

      bl_sleep(100);                   // sleep 100 ms
    }
  }
