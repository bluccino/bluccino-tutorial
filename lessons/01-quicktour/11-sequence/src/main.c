//==============================================================================
// main.c for 11-sequence
//==============================================================================

  #include "bluccino.h"                // access to Bluccino stuff

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO                     "main:"

  #define LOG                     LOG_MAIN
  #define LOGO(lvl,col,o,val)     LOGO_MAIN(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_MAIN(lvl,col,o,val)

//==============================================================================
// public module interface for a sequence module (note: OVAL interface!)
// - usage: bl_msg((sequence), _SYS,RUN_, 0,NULL,0)  // run sequence
// - sequence consists of 3 parts with delay 2s and 3s in between
// - the sequence is
//==============================================================================
//
// (*) := (any)
//                  +--------------------+
//                  |      sequence      |
//                  +--------------------+
//                  |        SYS:        | BUTTON input interface
// (*)->    RUN   ->|        @ix         | run sequence part @ix
//                  +--------------------+
//
//==============================================================================

  int sequence(BL_ob *o, int val)      // app module (with OVAL interface)
  {
    BL_ob oo = {_TASK,NEXT_,0,NULL};
    static BL_timer tim = BL_TIMER(sequence);

    switch (bl_id(o))
    {
      case BL_ID(_TASK,INIT_):         // init state machine
      case BL_ID(_TASK,NEXT_):         // state transition
        break;                         // run state machine ...

      default:
        return -1;                     // bad message
    }

    int state = val;                   // rename for clearer meaning
    switch (state)
    {
      case 0:                          // initial state
        bl_sleep(5);                   // sleep to avoid screwing up Segger RTT
        LOG(1,"task %d ...",state);    // we start in state 0
        state++;                       // next state
        bl_timer(&tim,&oo,state,-2000);// start single timer @ +2s
        return 0;

      case 1:                          // initial state
        LOG(1,"task %d ...",state);    // we are in state 1
        state++;                       // next state
        bl_timer(&tim,&oo,state,-5000);// start single timer @ +5s
        return 0;

      case 2:                          // initial state
        LOG(1,"task %d ...",state);    // we  are in state 2
        LOG(1,"stop");                 // time to stop now!
        return 0;

      default:
        return -1;                     // message not dispatched
    }
  }

//==============================================================================
// test: testing Bluccino work package submission to work horse task
// - note: work must have static storage implementation !!!
// - message objects can be implemented on dynamic (stack) storage
//==============================================================================

  int test(BL_ob *o, int val)
  {
    static BL_work work = BL_WORK(sequence);
    BL_ob oo = {_TASK,INIT_,88,"let's go ..."};  // @88 is just any @ix for demo

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        val = 0;                                 // start with initial state = 0
        LOGO(1,BL_G "bl_submit(&work,&oo,val)",&oo,val);
        return bl_submit(&work,&oo,val);         // submit work

      default:
        return -1;
    }
  }

//==============================================================================
// main function
// - set verbose level and print hello message
// - init bluccino module with all event output going to app() module
//==============================================================================

  void main(void)
  {
    bl_hello(5,PROJECT);
    bl_engine(test,10,1000);           // run test() on base of Bluccino engine
  }
