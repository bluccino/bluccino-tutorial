//==============================================================================
// main.c - Bluetooth mesh client/server app (both GOOSET & GOOLET in use)
//==============================================================================
//
//  (U) := (bl_up);  (D) := (bl_down)
//                   +--------------------+
//                   |        app         |
//                   +--------------------+
//                   |      SWITCH:       |  SWITCH interface
//  (U)->      STS ->|      @ix,val       |  receive switch @ix status
//                   +--------------------+
//                   |      GOOCLI:       |  GOOCLI ifc. (generic on/off client)
//  (D)<-      SET <-|  @ix,<BL_goo>,val  |  publish generic on/off SET message
//  (D)<-      LET <-|  @ix,<BL_goo>,val  |  publish generic on/off LET message
//                   +--------------------+
//                   |      GOOSRV:       |  GOOSRV ifc. (generic on/off server)
//  (U)->      STS ->|  @ix,<BL_goo>,val  |  recieve generic on/off SET message
//                   +--------------------+
//                   |        LED:        |  LED interface
//  (D)<-      SET <-|      @ix,val       |  set LED @ix on/off
//                   +--------------------+
//
//==============================================================================

  #include "bluccino.h"                     // access bluccino stuff
  #include "bl_gonoff.h"                    // generic on/off model

  int app(BL_ob *o, int val)                // public APP module interface
  {
    if (bl_is(o,_SWITCH,STS_))              // switch status update
    {
      if (bl_ix(o) == 1 || bl_ix(o) == 3)         // if SWITCH @1 or SWITCH @3
        bl_gooset(bl_ix(o),NULL,val);          // send generic on/off SET message
      else                                  // otherwise
        bl_goolet(bl_ix(o),NULL,val);          // send generic on/off LET message
    }
    else if (bl_is(o,_GOOSRV,STS_))         // generic on/off srv status update
    {
      BL_goo *g = bl_data(o);
      bl_log(1,BL_R "receive [GOOSRV:STS @%d,<#%d,/%dms,&%dms,(%s)>,%d]",
                  bl_ix(o),  g->tid,g->tt,g->delay,g->acked?"SET":"LET",val);
      bl_led(bl_ix(o),val);                    // turn LED @ix on/off
    }
    return 0;                               // OK
  }

  void main(void)
  {
    bl_hello(4,PROJECT);                    // set verbose level, hello message
    bl_init(bl_gear,app);                  // init bluccino, output to app()
  }
