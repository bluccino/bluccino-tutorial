//==============================================================================
//  bl_lisp.h
//  LISP like structures and primitives to deal with dynamic lists
//
//  Created by Hugo Pristauz on 2022-10-27
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_LISP_H__
#define __BL_LISP_H__

//==============================================================================
// config defaults
//==============================================================================

#ifndef CFG_LISP_NB_SYMBOLS
  #define CFG_LISP_NB_SYMBOLS 100      // max number of symbols
#endif

#ifndef CFG_LISP_NB_GLUES
  #define CFG_LISP_NB_GLUES 1000       // max number of glues
#endif

#ifndef CFG_LISP_DEBUG
  #define CFG_LISP_DEBUG   0
#endif

//==============================================================================
// LISP references and LISP glues
// - uint16_t representation
// - always > 0 for valid reference (ref = 0 means NIL)
// - glues are pairs of LISP references
//==============================================================================

  typedef uint16_t BL_lisp;            // index reference for symbols and glues

  typedef struct BL_glue               // LISP glue
  {
    BL_lisp ref_car;                   // car reference
    BL_lisp ref_cdr;                   // cdr reference
  } BL_glue;

//==============================================================================
// THE lisp heap (symbols and glues)
// - free_glue tells reference in glues of next free glue (must be initialized
//   with 1 to avoid return of NIL reference in 1st allocation)
// - free_symbol tells reference in symbols of next free symbol slot (must be
//   initialized with 1 to avoid return of NIL reference in 1st allocation)
//==============================================================================

  typedef struct BL_lheap
  {
    BL_txt symbols[CFG_LISP_NB_SYMBOLS];
    BL_glue glues[CFG_LISP_NB_GLUES];
    BL_lisp free_symbol;
    BL_lisp free_glue;
  } BL_lheap;

    // initializer macro (note: init free ref's wirth 1 !!!)

  #define BL_LHEAP()  {symbols:{"NIL"},glues:{{0,0}},free_symbol:1,free_glue:1}

  extern BL_lheap bl_lheap;             // global LISP (symbol & glue) heap

  #define BL_NIL  0x0000
  #define BL_BAD  0xFFFF

//==============================================================================
// init lisp data structures (optional at beginning)
// usage: err = bl_lisp_init(void);
//==============================================================================

  static inline int bl_lisp_init(void)
  {
    bl_lheap.free_symbol = bl_lheap.free_glue = 1;
    return 0;
  }

//==============================================================================
// LISP primitives CAR and CDR
// - usage: ref = bl_car(glue_ref)
//          ref = bl_cdr(glue_ref)
//==============================================================================

  static inline BL_lisp bl_car(BL_lisp ref)
  {
    if (ref < 0 || ref >= bl_lheap.free_glue) return BL_BAD;
    return bl_lheap.glues[ref].ref_car;
  }

  static inline BL_lisp bl_cdr(BL_lisp ref)
  {
    return bl_lheap.glues[ref].ref_cdr;
  }

//==============================================================================
// construct glue node
// - usage: ref = bl_cons(car_ref,cdr_ref)
//==============================================================================

  BL_lisp bl_cons(BL_lisp ref_car, BL_lisp ref_cdr);

//==============================================================================
// get symbol reference (note: symbol references have HSBit set)
// - usage: ref = bl_sym("app") // importannt: arg must be ptr to static string
// - lookup symbol in symbol table, and if found, return symbol reference
// - if not found in symbol table, first add symbol, then return symbol ref
//==============================================================================

  BL_lisp bl_sym(BL_txt sym);

//==============================================================================
// get name (text) of a symbol
// - usage: txt = bl_name(ref)
//==============================================================================

  BL_txt bl_name(BL_lisp ref);

//==============================================================================
// check if reference refers to NIL, to a list (glue) or refers to a symbol
// - usage: ok = is_nil(lref)
//          ok = is_nil(lref)
//          ok = bl_islist(list_ref)
//          ok = blissym(sym_ref)
//==============================================================================

  static inline bool bl_isnil(BL_lisp ref)
  {
    return (ref == 0);
  }

  static inline bool bl_iserr(BL_lisp ref)
  {
    return (ref == BL_BAD);
  }

  static inline bool bl_islist(BL_lisp ref)
  {
    return (ref == 0) ? false : ((ref & 0x8000) == 0);
  }

  static inline bool bl_issym(BL_lisp ref)
  {
    return (ref == 0) ? false : ((ref & 0x8000) != 0);
  }

//==============================================================================
// LISP primitives SETCAR! and SETCDR!
// - usage: ref = bl_setcar(glue_ref,item_ref)
//          ref = bl_setcdr(glue_ref,item_ref)
//==============================================================================

  static inline BL_lisp bl_setcar(BL_lisp glue, BL_lisp item)
  {
    if (!bl_islist(glue)) return BL_BAD;
    bl_lheap.glues[glue].ref_car = item;
    return glue;
  }

  static inline BL_lisp bl_setcdr(BL_lisp glue, BL_lisp item)
  {
    if (!bl_islist(glue)) return BL_BAD;
    bl_lheap.glues[glue].ref_cdr = item;
    return glue;
  }

//==============================================================================
// get tail glue of a list
// - usage: tail_ref = bl_tail(lref);
//==============================================================================

  BL_lisp bl_tail(BL_lisp list);

//==============================================================================
// append to list
// - usage: lref = bl_append(lref,item_ref);   // add to list end
//==============================================================================

  BL_lisp bl_append(BL_lisp list,BL_lisp item);

//==============================================================================
// retrieve from an assoc table a pair/list with associated key
// - usage: pair = ef_assoc(key,table) // pair is (key . val), NIL if not found
// - table: ((A ...) (8 ...) ("q" ...)) - keys: A,8,"q"
//==============================================================================

  BL_lisp bl_assoc(BL_lisp key, BL_lisp table);

//==============================================================================
// log lisp expression
// - usage: err = bl_prtl(dir_ref);
//          err = bl_logl(1,"directory:",dir_ref);
//==============================================================================
#if (CFG_LISP_DEBUG)
  int bl_prtl(BL_lisp ref);
  int bl_logl(int lvl,BL_txt txt,BL_lisp ref);
#else
  #define bl_logl(lvl,txt,ref)         // expand to nothing
#endif

#endif // __BL_LISP_H__
