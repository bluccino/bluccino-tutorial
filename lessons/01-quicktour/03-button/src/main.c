//==============================================================================
// main.c for 03-button (LED toggle control by subscribing to button events)
//==============================================================================

  #include "bluccino.h"                // access bluccino stuff

  int app(BL_ob *o, int val)           // app function, handling all messages
  {                                    // which are forwarded by Bluccino layer
    if ( bl_is(o,_BUTTON,PRESS_) )     // dispatch [BUTTON:PRESS sts] event
    {
      bl_logo(1,"app:",o,val);         // log the [BUTTON:PRESS sts]
      bl_led(bl_ix(o),-1);             // toggle LED with corresponding index
    }
    return 0;                          // OK
  }

  void main(void)
  {
    bl_hello(1,PROJECT " (click any of the buttons)");
    bl_init(bl_gear,app);              // run tick/tock engine, cb's go to app
  }
