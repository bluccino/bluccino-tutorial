//==============================================================================
// main.c for 08-spool (Bluetooth mesh client/server app)
//==============================================================================

  #include "bluccino.h"                     // access bluccino stuff
  #include "bl_gonoff.h"                    // generic on/off mesh models
  #include "bl_spool.h"                      // mesh publisher
  #include "bl_node.h"                      // mesh node house keeping
  #include "bl_trans.h"

  #define PMI  app                          // public module interface
  int app(BL_ob *o, int val);               // forward declaration of PMI

//==============================================================================
// defining our transition object
//==============================================================================

  static BL_trans trans = {0,0,0,0};

//==============================================================================
// syntactic sugar
// - usage: ok = ready()       // is node ready (startup sequence completed)?
//          ok = prv()         // is node provisioned
//          led(int,onf)       // set LED @ix on/off
//          rgb(onf)           // set LED @2,@3,@4 all on or off
//==============================================================================

  static int ready(void) { return _bl_ready(PMI); }
  static int prv(void) { return _bl_prv(PMI);}
  static int led(int id, int val) { return _bl_led(id,val,(PMI)); }
  static int rgb(int onf) { led(2,onf); led(3,onf); led(4,onf); return 0; }

//==============================================================================
// handler: generic on/off status update
//==============================================================================

  static int gonoff_sts(BL_ob *o, int val)
  {
    BL_goo *g = bl_data(o);
    bl_log(1,BL_R "receive [GOOSRV:STS @%d,<#%d,/%dms,&%dms,(%s)>,%d]",
                  bl_ix(o),  g->tid,g->tt,g->delay,g->acked?"SET":"LET",val);

    bl_trans(&trans,&g->trans);        // update transition in trans. object
    return 0;                          // OK
  }

//==============================================================================
// handler: switch status update
// - with any switch status change (@ix does not matter) forward switch status
//   value with 1s delay to mesh generic on/off client
// - use a BL_goo data structure to define delay
//==============================================================================

  static int switch_sts(BL_ob *o, int val)
  {
    if ( ready() )                      // is node ready (startup completed)?
    {
      if ( prv() )                      // go via mesh if provisioned
      {
        BL_goo goo = {delay:150, tt:100};
        bl_log(1,BL_R "post [#GOOCLI:LET @%d,</%dms,&%dms>,%d] -> (app)",
	         bl_ix(o), goo.tt,goo.delay, val);
        _bl_goolet(1,&goo,val,(PMI));   // [GONOFF:LET @1,&goo,val] -> (PMI)
      }
      else                              // otherwise direct LED control
        rgb(val);                       // turn all RGB LEDs on/off
    }

    return 0;
  }

//==============================================================================
// handler: system tick (manages transition)
//==============================================================================

  static int sys_tick(BL_ob *o, int val)
  {
    if (bl_period(o,50))                    // every 50 ms
    {
      if (bl_fin(&trans))
        rgb(trans.target);                  // turn all RGB LEDs on/off
    }
    return 0;                               // OK
  }

//==============================================================================
//
// (U) := (bl_up);  (D) := (bl_down)
//
//                  +--------------------+
//                  |        APP         |
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
// (D)<-   TOGGLE <-|        @ix         |  set LED @ix on/off
//                  +--------------------+
//                  |       #NODE:       | NODE output interface
// (T)<-    READY <-|                    | returns node's ready status
//                  +--------------------+
//                  |        #GET:       | GET output interface
// (T)<-      PRV <-|                    | returns node's ready status
//                  +--------------------+
//
//==============================================================================

  int app(BL_ob *o, int val)                // public APP module interface
  {
    static BL_oval D = bl_down;             // down gear
    static BL_oval T = bl_top;              // top gear

    switch (bl_id(o))
    {
      case SYS_INIT_0_cb_0:
        return 0;

      case SYS_TICK_ix_BL_pace_cnt:
        return sys_tick(o,val);             // delegate to sys_tick() handler

      case SWITCH_STS_ix_0_sts:
        return switch_sts(o,val);           // delegate to switch_sts() handler

      case GOOSRV_STS_ix_BL_goo_sts:
        return gonoff_sts(o,val);           // delegate to gonoff_sts() handler

      case _GOOCLI_LET_ix_BL_goo_onoff:
        return bl_out(o,val,T);             // output to top gear (-> bl_spool)

      case _LED_SET_ix_0_onoff:
      case _LED_TOGGLE_ix_0_0:
        return bl_out(o,val,D);             // output to down gear

      case _NODE_READY_0_BL_pint_0:
        return bl_out(o,val,T);             // is node ready (startup complete)?

      case _GET_PRV_0_0_0:                  // [#GET:PRV]
      case _GET_ATT_0_0_0:                  // [#GET:ATT]
        return bl_out(o,val,D);             // output to down gear

      default:
        return 0;                           // OK
    }
  }

//==============================================================================
// main function
// - set verbose level and print hello message
// - run init/tick/tock engine
//==============================================================================

  void main(void)
  {
    bl_hello(VERBOSE,PROJECT);         // set verbose level, show project title
    bl_install(bl_node);               // install bl_node in top gear
    bl_install(bl_spool);              // install bl_spool in top gear
    bl_cfg(bl_down,_BUTTON,BL_SWITCH|BL_HOLD); // [BUTTON:SWITCH/HOLD] events
    bl_engine(app,2,1000);             // run 2/1000ms tick/tock engine
  }
