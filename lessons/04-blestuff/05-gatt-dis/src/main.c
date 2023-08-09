//==============================================================================
// main.c - generic main module
//==============================================================================

  #include "bluccino.h"
  #include "app.h"
  #include "bl_dis.h"

//==============================================================================
// main function
// - set verbose level and print hello message
// - run init/tick/tock engine
//==============================================================================

  void main(void)
  {
    bl_hello(4,PROJECT);                 // verbose level 4, print hello message
    bl_service(bl_dis);                  // install DIS service in WL core
    bl_engine(app,10,1000);              // run 10ms/1000ms tick/tock engine
  }
