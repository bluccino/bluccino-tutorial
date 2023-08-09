//==============================================================================
//  bl_symb.h
//  Bluccino message symbol definitions
//
//  Created by Hugo Pristauz on 2022-02-22
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_SYMB_H__
#define __BL_SYMB_H__

  #include "bl_defs.h"

//==============================================================================
// define BL_CL_TEXT if application did not define yet
//==============================================================================
 #ifndef BL_CL_TEXT

  #define BL_CL_TEXT   {BL_CL_SYMBOLS}

  typedef enum BL_cl                 // class tag
          {
            BL_CL_ENUMS,
          } BL_cl;                   // class tag

 #endif // BL_CL_TEXT
//==============================================================================
// define BL_OP_TEXT if application did not define yet
//==============================================================================
#ifndef BL_OP_TEXT

  #define BL_OP_TEXT {BL_OP_SYMBOLS}

  typedef enum BL_op
          {
            BL_OP_ENUMS
          } BL_op;

  #endif // BL_OP_TEXT
//==============================================================================
// message object & message callback definition
//==============================================================================

  typedef struct BL_ob
          {
            BL_cl cl;                  // class tag
            BL_op op;                  // opcode
            int ix;                    // instance index
            const void *data;          // pointer to data
          } BL_ob;

    // define "OVAL" function interface and callback type

  typedef int (*BL_oval)(BL_ob *o, int val);

    // initializer for BL_ob

  #define  BL_OB(cl,op,ix,data) {cl,op,ix,data}       // unaugmented initializer
  #define _BL_OB(cl,op,ix,data) {BL_AUG(cl),op,ix,data} // augmented initializer

//==============================================================================
// syntactic sugar: get/set message class tag
// - usage: cl = bl_cl(o);  bl_set_cl(o,cl)
//==============================================================================

  static inline BL_cl bl_cl(BL_ob *o)
  {
    return o->cl;
  }

  static inline void bl_set_cl(BL_ob *o, BL_cl cl)
  {
    o->cl = cl;
  }

//==============================================================================
// syntactic sugar: get message opcode
// - usage: op = bl_op(o);
//==============================================================================

  static inline BL_op bl_op(BL_ob *o)
  {
    return o->op;
  }

  static inline void bl_set_op(BL_ob *o, BL_op op)
  {
    o->op = op;
  }

//==============================================================================
// syntactic sugar: compound message identifier
// - usage: bl_id(o)                   // same as BL_PAIR(o->cl,o->op)
//==============================================================================

  static inline BL_id bl_id(BL_ob *o)
  {
    return BL_ID(o->cl,o->op);
  }

//==============================================================================
// syntactic sugar: get/set instance index
// - usage: ix = bl_ix(o);  bl_set_ix(o,ix);
//==============================================================================

  static inline int bl_ix(BL_ob *o)
  {
    return o->ix;
  }

  static inline void bl_set_ix(BL_ob *o, int ix)
  {
    o->ix = ix;
  }

//==============================================================================
// syntactic sugar: get/set data reference of message object
// - usage: MY_data *p = bl_data(o);  bl_set_data(o,data)
//==============================================================================

  static inline void *bl_data(BL_ob *o)
  {
    return (void*)o->data;
  }

  static inline void bl_set_data(BL_ob *o, BL_data data)
  {
    o->data = data;
  }

//==============================================================================
// library register node type definition
//==============================================================================

  typedef struct BL_lib                // library register node
          {
            BL_oval module;            // module to be registered in library
            int id;                    // unique module id
            struct BL_lib *next;       // pointer to next library register node
          } BL_lib;

//==============================================================================
// undef anti-recursion symbol (__BL_SYMB_H__) if suppression is active
//==============================================================================
#endif // __BL_SYMB_H__
