//==============================================================================
// main.c for 02-blink (tiny LED toggle control)
//==============================================================================

  #include "bluccino.h"                // access Bluccino stuff

  void main(void)
  {
    bl_hello(4,PROJECT);               // set verbose level, print hello message
    bl_init(bl_gear,NULL);             // init gear, no interest in callbacks

    for(;;bl_sleep(1000))              // forever (sleep 1000ms in between)
      bl_led(1,-1);                    // toggle LED @1 status
  }
