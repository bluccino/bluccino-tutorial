//==============================================================================
// bl_node.h
// mesh node house keeping (startup, provision, attention)
//
// Created by Hugo Pristauz on 2022-Feb-21
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

#ifndef __BL_NODE_H__
#define __BL_NODE_H__

//==============================================================================
// NODE Logging
//==============================================================================

#ifndef CFG_LOG_NODE
    #define CFG_LOG_NODE    1           // NODE logging is by default on
#endif

#if (CFG_LOG_NODE)
    #define LOG_NODE(l,f,...)    BL_LOG(CFG_LOG_NODE-1+l,f,##__VA_ARGS__)
    #define LOGO_NODE(l,f,o,v)   bl_logo(CFG_LOG_NODE-1+l,f,o,v)
#else
    #define LOG_NODE(l,f,...)    {}     // empty
    #define LOGO_NODE(l,f,o,v)   {}     // empty
#endif

//==============================================================================
// message definitions
//==============================================================================
//
//                  +--------------------+
//                  |        NODE:       | NODE interface
//          READY --|        &int        | returns node's ready state
//                  +--------------------+
//                  |       STATE:       | STATE interface
//            ATT --|        &int        | get node's attention state
//            PRV --|        &int        | get node's provision state
//                  +--------------------+
//                  |        GET:        | GET interface (obsolete !!!)
//            ATT --|                    | get node's attention status
//            PRV --|                    | get node's provision status
//                  +--------------------+
///
//==============================================================================

  #define NODE_READY_0_BL_pint_0   BL_ID(_NODE,READY_)
  #define STATE_ATT_0_BL_pint_0    BL_ID(_STATE,ATT_)
  #define STATE_PRV_0_BL_pint_0    BL_ID(_STATE,PRV_)

    // augmented messages

  #define _NODE_READY_0_BL_pint_0  _BL_ID(_NODE,READY_)
  #define _STATE_ATT_0_BL_pint_0   _BL_ID(_STATE,ATT_)
  #define _STATE_PRV_0_BL_pint_0   _BL_ID(_STATE,PRV_)

//==============================================================================
// public module interface
//==============================================================================

  int bl_node(BL_ob *o, int val);

//==============================================================================
// syntactic sugar: is node ready? (startup sequence completed?)
// - usage: ok = bl_ready()        // sends message to top gear
//          ok = _bl_ready(PMI)    // PMI needs to output message to top gear
//==============================================================================

  static inline int bl_ready(void)
  {
    int val;
    bl_post((bl_top), _NODE_READY_0_BL_pint_0, 0,&val,0);
    return val;
  }

  static inline int _bl_ready(BL_oval to)
  {
    int val;
    _bl_post((to), _NODE_READY_0_BL_pint_0, 0,&val,0);
    return val;
  }

//==============================================================================
// syntactic sugar: get attention state
// - usage: ok = bl_att()        // sends message to down gear
//          ok = _bl_att(PMI)    // PMI needs to output message to down gear
//==============================================================================

  static inline int bl_att(void)
  {
    return bl_post((bl_down), _GET_ATT_0_0_0, 0,NULL,0);
  }

  static inline int _bl_att(BL_oval to)
  {
    return _bl_post((to), _GET_ATT_0_0_0, 0,NULL,0);
  }

//==============================================================================
// syntactic sugar: get provision state
// - usage: ok = bl_prv()        // sends message to down gear
//          ok = _bl_prv(PMI)    // PMI needs to output message to down gear
//==============================================================================

  static inline int bl_prv(void)
  {
    return bl_post((bl_down), _GET_PRV_0_0_0, 0,NULL,0);
  }

  static inline int _bl_prv(BL_oval to)
  {
    return _bl_post((to), _GET_PRV_0_0_0, 0,NULL,0);
  }

#endif // __BL_NODE_H__
