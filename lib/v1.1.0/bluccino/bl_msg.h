//==============================================================================
// bl_msg.h
// Bluccino messaging
//
// Created by Hugo Pristauz on 2022-Apr-03
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

#ifndef __BL_MSG_H__
#define __BL_MSG_H__

#include "bl_app.h"
#include "bl_log.h"
#include "bl_symb.h"
#include "bl_type.h"
#include "bl_vers.h"

  extern bool bl_attention;            // attention mode
  extern bool bl_provisioned;          // provisioned mode

//==============================================================================
// syntactic sugar: is event of given <class,opcode>?
// - usage: bl_is(o,_SYS,TOCK_)
//==============================================================================

  static inline bool bl_is(BL_ob *o, BL_cl cl, BL_op op)
  {
    return ( bl_id(o) == BL_ID(cl,op) );
  }

//==============================================================================
// post general message [CL:OP @ix,<data>,val] to module
// - usage: bl_post((to),mid,id,data,val)
//          bl_post((to),SYS_INIT_0_cb_0, 0,cb,0)
//==============================================================================

  static inline int bl_post(BL_oval to, BL_id mid, int id,BL_data data,int val)
  {
    BL_ob oo = {BL_CL(mid),BL_OP(mid),id,data};
    return to(&oo,val);            // post message to module interface
  }

//==============================================================================
// post general augmented message [#CL:OP @ix,<data>,val] to module
// - usage: _bl_post((to),mid,id,data,val)
//          _bl_post((to),SYS_INIT_0_cb_0, 0,cb,0)
//==============================================================================

  static inline int _bl_post(BL_oval to, BL_id mid, int id,BL_data data,int val)
  {
    BL_ob oo = {BL_AUG(BL_CL(mid)),BL_OP(mid),id,data};
    return to(&oo,val);            // post augmented message to module interface
  }

//==============================================================================
// post general message [CL:OP @ix,<data>,val] to module
// - usage: bl_msg(module,cl,op,id,data,val)
//==============================================================================

  static inline
    int bl_msg(BL_oval to, BL_cl cl, BL_op op, int id, BL_data data, int val)
  {
    BL_ob oo = {cl,op,id,data};
    return to(&oo,val);            // post message to module interface
  }

//==============================================================================
// post general augmented message [CL:OP @ix,<data>,val] to module
// - usage: _bl_msg(module,cl,op,id,data,val)
//==============================================================================

  static inline
    int _bl_msg(BL_oval to, BL_cl cl, BL_op op, int id, BL_data data, int val)
  {
    BL_ob oo = {BL_AUG(cl),op,id,data};// augmented class tag
    return to(&oo,val);            // post message to module interface
  }

//==============================================================================
// syntactic sugar: event message forwading
// - similar function to bl_out() without un-augmenting feature and NULL check
// - usage: bl_fwd(o,val,(to))      // forward to given module
//         _bl_fwd(o,val,(PMI))     // augmented version
//==============================================================================

  static inline int bl_fwd(BL_ob *o, int val, BL_oval to)
  {
    return to(o,val);     // it's PURE syntactic sugar
  }

  static inline int _bl_fwd(BL_ob *o, int val, BL_oval to)
  {
    return _bl_msg((to), o->cl,o->op, bl_ix(o),o->data,val);   // augmented message
  }

//==============================================================================
// syntactic sugar: ping a module
// - usage: bl_ping(module,"hello!")
//==============================================================================

  static inline BL_txt bl_ping(BL_oval module, BL_txt msg)
  {
    BL_ob oo = {_SYS,PING_,0,msg};
    module(&oo,0);                     // send [SYS:PING <msg>] to MODULE
    return (BL_txt)oo.data;
  }

//==============================================================================
// syntactic sugar: ping a module
// - usage: bl_pong(o,msg)
//==============================================================================

  static inline int bl_pong(BL_ob *o, BL_txt msg)
  {
    o->data = msg;
    return 0;
  }

//==============================================================================
// bl_cfg (syntactic sugar to config a given module)
// - usage: bl_cfg(module,mask)  // (<module>) <- [SYS:CFG mask]
//==============================================================================

  static inline int bl_cfg(BL_oval module, BL_cl cl, BL_word mask)
  {
    return bl_msg(module,cl,CFG_, 0,NULL,(int)mask);
  }

//==============================================================================
// send message to public module interface (PMI)
// - usage: BL_PMI(my_module)
// - 1) defines an external declaration for PMI function:
//      int my_module(BL_ob *p, int val);
// - 2) defines a static function to post an augmented message to PMI:
//      static int _bl_pmi(BL_cl cl, BL_op op, int ix, BL_data *data, int val)
//      {
//        return _bl_msg((my_module), cl,op, ix,data,val);
//      }
//==============================================================================

  #define BL_PMI(pmi)                                                         \
    int pmi(BL_ob *o, int val);                                               \
    static int inline _bl_pmi(BL_cl cl,BL_op op,int ix,BL_data data,int val)  \
    {                                                                         \
      return _bl_msg((pmi), cl,op, ix,data,val);                              \
    }                                                                         \

#endif // __BL_MSG_H__
