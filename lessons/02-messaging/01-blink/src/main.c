//==============================================================================
// main.c for 01-blink (tiny LED blink control)
//==============================================================================

  #include "bluccino.h"                // access Bluccino stuff

  void main(void)
  {
    bl_bench(4,PROJECT,NULL);          // verbose level 4, print hello msg
    BL_ob oo = BL_OB(_LED,TOGGLE_,-1,NULL);

    for ( ;; bl_sleep(1000))
      bl_down(&oo,0);                  // send [LED:TOGGLE @-1] to down gear
  }
