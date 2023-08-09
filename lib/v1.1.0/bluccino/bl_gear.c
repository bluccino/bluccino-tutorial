//==============================================================================
//  bl_gear.c
//  Bluccino gear modules
//
//  Created by Hugo Pristauz on 2022-01-01
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_core.h"

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO                     "bl_gear:"

  #define LOG                     LOG_GEAR
  #define LOGO(lvl,col,o,val)     LOGO_GEAR(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_GEAR(lvl,col,o,val)

//==============================================================================
// event message output (message emission of a module)
// - usage: bl_out(o,val,(to))   // output to given module
// - important note: class tags are automatically un-augmented before posting
// - note: bl_gear.c provides a weak implementation (redefinition possible)
//==============================================================================

  __weak int bl_out(BL_ob *o, int val, BL_oval to)
  {
    if (!to)                           // is a valid <out> callback provided?
      return 0;

      // augmented class tag? (aug bit set) => need to duplicate object with
      // de-augmented class tag before forwarding

    if ( !BL_ISAUG(o->cl) )            // easy for un-augmented class tags!
      return to(o,val);                // forward event message to subscriber
    else                               // forward with un-augmented class tag
      return bl_msg((to), BL_UNAUG(o->cl),o->op, bl_ix(o),o->data,val);
  }

//==============================================================================
// augmented event message output (message emission of a module)
// - usage: _bl_out(o,val,(to))   // output to given module
// - important note: class tags are automatically augmented before posting
// - note: bl_gear.c provides a weak implementation (redefinition possible)
//==============================================================================

  __weak int _bl_out(BL_ob *o, int val, BL_oval to)
  {
    if (!to)                       // is a valid <out> callback provided?
      return 0;

      // augmented class tag? (aug bit set) => need to duplicate object with
      // de-augmented class tag before forwarding

    if ( BL_ISAUG(o->cl) )             // easy for un-augmented class tags!
      return to(o,val);            // forward event message to subscriber
    else                               // forward with un-augmented class tag
      return bl_msg((to), BL_AUG(o->cl),o->op, bl_ix(o),o->data,val);
  }

//==============================================================================
// auxillary emission function
// - used by bl_top to output to app module
// - all messages except [SYS:] messages to be forwarded to app
//==============================================================================
/* obsoleted
  __weak int bl_emit(BL_ob *o, int val)
  {
    static BL_oval O = NULL;
    if (o->cl == _SYS)
    {
      if (o->op == INIT_)
        O = (BL_oval)o->data;
      return 0;
    }

    return (O) ? (O)(o,val) : 0;
  }
*/
//==============================================================================
// message downward posting to lower level / driver layer (default/__weak)
// - bl_down() is defined as weak and can be overloaded
// - by default all messages posted to BL_DOWN are forwarded to BL_CORE
// - note: bl_gear.c provides a weak implementation (redefinition possible)
//==============================================================================

  __weak int bl_down(BL_ob *o, int val)
  {
    bool nolog = bl_is(o,_LED,SET_) && bl_ix(o) == 0;
    nolog = nolog || (o->cl == _SYS);

    if ( !nolog )
      LOG0(3,"down:",o,val);           // not suppressed messages are logged

    if (bl_is(o,_SYS,INIT_))
    {
      LOG(3,BL_C "init down gear");
    }

    return bl_core(o,val);             // forward down to BL_CORE module
  }

//==============================================================================
// message upward posting to API layer (default/__weak)
// - bl_up() is defined as weak and can be overloaded
// - by default all messages posted to BL_UP are forwarded to BL_TOP
// - note: bl_gear.c provides a weak implementation (redefinition possible)
//==============================================================================

  __weak int bl_up(BL_ob *o, int val)
  {
    static BL_oval A = NULL;           // outputs to app by default
    static BL_oval T = bl_top;         // top gear

    if (!bl_is(o,_SYS,INIT_))
    {
      LOG0(3,"up:",o,val);
    }

		switch (bl_id(o))
		{
      case BL_ID(_SYS,INIT_):          // [SYS:INIT <out>]
        LOG(3,BL_C "init up gear");
        A = bl_cb(o,(A),"bl_up:(A)");  // store output callback
			  return 0;

      case BL_ID(_MESH,ATT_):          // mesh attention
      case BL_ID(_MESH,PRV_):          // mesh provision
      case BL_ID(_RESET,DUE_):         // reset timer due
        return bl_fwd(o,val,(T));      // forward to top gear

			default:
        return bl_out(o,val,(A));      // output to app by default
		}
  }

//==============================================================================
// BL_TOP: input a message to Bluccino API
// - top gear's role is to distribute messages between app level modules
// - any unaugmented message posted to the top gear is posted to the app level
// - augmented messages posted to the top gear should be forwarded to the down
//   gear
// - bl_top() is defined as weak and can be overloaded
//==============================================================================
//
// (A) := (app);  (M) := (main);  (*) := (<any>)
//
//                  +--------------------+
//                  |       bl_top       | Bluccino top gear
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     INIT ->|       <out>        | init module, store <out> callback
//                  +--------------------+
//                  |      <OTHER>:      | <OTHER> input/output interface
// (*)->        * ->|        ...         | any other input event message will
// (A)<-        * <-|        ...         | be forwarded to app
//                  +--------------------+
//
//==============================================================================

  __weak int bl_top(BL_ob *o, int val)
  {
    static BL_lib *libs = NULL;        // pointer to library module list

      //if a library module is requested to register in the top gear
      // we will register it in the libs module list and return

    if (bl_is(o,_SYS,INIT_))
    {
      LOG(3,BL_C "init top gear");
    }
    else if (bl_is(o,_SYS,LIB_))       // [SYS:LIB @ix,<BL_lib>]
      return bl_reglink(o,&libs);      // link lib registry node in libs list

      // otherwise iterate through every library module in the libs list
      // and let them occasionally handle the message

    return bl_iter(o,val,libs);        // iterate through library module list
  }

//==============================================================================
// PMI: bl_gear (Bluccino gear)
//==============================================================================
//
// (M) := (MAIN); (D) := (BL_DOWN); (T) := (BL_TOP);  (A) := (APP)
//
//                  +--------------------+
//                  |       bl_gear      |
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (M)->     INIT ->|       <out>        | init module, store <out> callback
// (M)->     TICK ->|      @ix,cnt       | tick module
// (M)->     TOCK ->|      @ix,cnt       | tock module
// (M)->      OUT ->|       <out>        | set <out> callback
//                  +--------------------+
//                  |        SYS:        | SYS output interface
// (D)<-     INIT <-|       <out>        | init module, store <out> callback
// (D)<-     TICK <-|      @ix,cnt       | tick module
// (D)<-     TOCK <-|      @ix,cnt       | tock module
//                  +--------------------+
//
//==============================================================================

  int bl_gear(BL_ob *o, int val)
  {
    static BL_oval A = NULL;           // output to app module
    static BL_oval D = bl_down;        // output to down gear
    static BL_oval T = bl_top;         // output to top gear
    static BL_oval U = bl_up;          // output to up gear

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):          // [SYS:INIT <out>]
        LOG(2,BL_B "init gear");
        A = bl_cb(o,(A),WHO"(A)");     // store output callback

          // first init emitter (bl_emit), since down gear can send early
          // messages which requires bl_top to be able to forward messages
          // to app via bl_emit();

//      bl_init(bl_emit,(A));          // output non [SYS:] message to app

          // after that we initialize up gear, down gear and top gear
          // in exactly this order.

        bl_init((U),(A));              // init up gear, output to app
        bl_init((D),(U));              // init down gear, output to up gear
        bl_init((T),(A));              // init top gear, output to app
        return 0;

      case BL_ID(_SYS,TICK_):
      case BL_ID(_SYS,TOCK_):
        bl_fwd(o,val,(D));             // tick/tock down gear
        bl_fwd(o,val,(T));             // tick/tock top gear
        return 0;

      case BL_ID(_SYS,OUT_):
        A = bl_cb(o,(A),WHO"(A)");     // store output callback
        return 0;

      default:
        return -1;                     // bad command
    }
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
