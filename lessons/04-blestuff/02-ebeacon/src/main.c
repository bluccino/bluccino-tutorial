//==============================================================================
// main.c for 02-ebeacon sample (Eddystone beacon)
//==============================================================================

  #include "bluccino.h"
  #include "bl_eddy.h"
  #include "app.h"

//==============================================================================
// main function
// - set verbose level and print hello message
// - register BLE service
// - run init/tick/tock engine
//==============================================================================

  void main(void)
  {
    bl_hello(4,PROJECT);                // verbose level 4, print hello message
    bl_service(bl_eddy);                // register Eddystone service in WL core
    bl_engine(app,10,1000);             // run 10ms/1000ms tick/tock engine
  }
