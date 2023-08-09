//==============================================================================
// main.c for 09-clisrv (Bluetooth mesh client/server app)
//==============================================================================

  #include "bluccino.h"                     // access bluccino stuff
  #include "bl_gonoff.h"                    // generic on/off mesh models

//==============================================================================
//
// (U) := (bl_up);  (D) := (bl_down)
//
//                  +--------------------+
//                  |        app         |
//                  +--------------------+
//                  |      SWITCH:       |  SWITCH interface
// (U)->      STS ->|      @ix,val       |  receive switch @ix status
//                  +--------------------+
//                  |      GOOSRV:       |  GOOSRV input interface
// (U)->      STS ->|  @ix,<BL_goo>,val  |  recieve generic on/off status update
//                  |....................|
//                  |     #GOOCLI:       |  GOOCLI output interface
// (D)<-      SET <-|  @ix,<BL_goo>,val  |  publish generic on/off SET message
//                  +--------------------+
//                  |        LED:        |  LED interface
// (D)<-      SET <-|      @ix,val       |  set LED @ix on/off
//                  +--------------------+
//
//==============================================================================

  int app(BL_ob *o, int val)           // public APP module interface
  {
    switch (bl_id(o))
    {
      case SWITCH_STS_ix_0_sts:        // [SWITCH:STS @ix,sts]
        return bl_gooset(1,NULL,val);  // send generic on/off SET message

      case GOOSRV_STS_ix_BL_goo_sts:   // [GONOFF:STS @ix,<BL_goo>,sts]
        return bl_led(bl_ix(o),val);      // turn LED @ix on/off

      default:
        return 0;                      // OK
    }
  }

//==============================================================================
// main function
// - set verbose level and print hello message
// - run init/tick/tock engine
//==============================================================================

  void main(void)
  {
    bl_hello(4,PROJECT);               // set verbose level, show project title
    bl_engine(app,10,1000);            // run 10/1000ms tick/tock engine
  }
