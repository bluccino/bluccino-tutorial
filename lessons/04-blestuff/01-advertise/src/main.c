//==============================================================================
// main.c for 01-advertise sample (simple advertising)
//==============================================================================

  #include "bluccino.h"
  #include "app.h"
  #include "advsvc.h"

//==============================================================================
// main function
// - set verbose level and print hello message
// - run init/tick/tock engine
//==============================================================================

  void main(void)
  {
    bl_hello(5,PROJECT);                 // verbose level 4, print hello message
    bl_service(advsvc);                  // register ADV service in WL core
    bl_engine(app,10,1000);              // run 10ms/1000ms tick/tock engine
  }
