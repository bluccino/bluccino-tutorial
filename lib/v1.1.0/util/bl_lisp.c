//==============================================================================
//  bl_lisp.c
//  LISP like structures and primitives to deal with dynamic lists
//
//  Created by Hugo Pristauz on 2022-10-27
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

  #include <string.h>

  #include "bluccino.h"
  #include "bl_lisp.h"

//==============================================================================
// THE lisp heap (symbols and glues)
// - free_glue tells reference in glues of next free glue (must be initialized
//   with 1 to avoid return of NIL reference in 1st allocation)
// - free_symbol tells reference in symbols of next free symbol slot (must be
//   initialized with 1 to avoid return of NIL reference in 1st allocation)
//==============================================================================

  BL_lheap bl_lheap = BL_LHEAP();       // global LISP data structure

//==============================================================================
// construct glue node
// - usage: ref = bl_cons(car_ref,cdr_ref)
//==============================================================================

  BL_lisp bl_cons(BL_lisp ref_car, BL_lisp ref_cdr)
  {
    if (bl_lheap.free_glue >= BL_LEN(bl_lheap.glues))
    {
      bl_err(-1,"out of glues");
      return BL_BAD;
    }

    bl_lheap.glues[bl_lheap.free_glue].ref_car = ref_car;
    bl_lheap.glues[bl_lheap.free_glue].ref_cdr = ref_cdr;
    return bl_lheap.free_glue++;
  }

//==============================================================================
// get symbol reference (note: symbol references have HSBit set)
// - usage: ref = bl_sym("app") // importannt: arg must be ptr to static string
// - lookup symbol in symbol table, and if found, return symbol reference
// - if not found in symbol table, first add symbol, then return symbol ref
//==============================================================================

  BL_lisp bl_sym(BL_txt sym)
  {
    for (int ref=0; ref < BL_LEN(bl_lheap.symbols); ref++)
      if (!strcmp(bl_lheap.symbols[ref],sym)) // if string match
        return (ref | 0x8000);               // found => return symbol reference

      // not found - do we have still a free slot

    if (bl_lheap.free_symbol >= BL_LEN(bl_lheap.symbols))
    {
      bl_err(-1,"out of glues");
      return BL_BAD;
    }

      // add symbol to symbol table and return symbol reference

    BL_lisp ref = bl_lheap.free_symbol++;
    bl_lheap.symbols[ref] = sym;
    return (ref | 0x8000);                    // return symbol reference
  }

//==============================================================================
// get name (text) of a symbol
// - usage: txt = bl_name(ref)
//==============================================================================

  BL_txt bl_name(BL_lisp ref)
  {
    if (ref == 0 )
      return "NIL";
    else if ((ref & 0x8000) == 0)
      return "*ERR*";
    else
      return bl_lheap.symbols[ref & ~0x8000];
  }

//==============================================================================
// get tail glue of a list
// - usage: tail_ref = bl_tail(list_ref);
//==============================================================================

  BL_lisp bl_tail(BL_lisp list)
  {
    BL_lisp tail = BL_NIL;

    for (; bl_islist(list); list = bl_cdr(list))
      tail = list;

    return tail;
  }

//==============================================================================
// append to list
// - usage: list_ref = bl_append(list_ref,item_ref);   // add to list end
// - note: bl_append() does not add to non-proper lists, e.g. to (A B . C)
//==============================================================================

  BL_lisp bl_append(BL_lisp list,BL_lisp item)
  {
    if (bl_isnil(list))                // trivial append operation?
      return bl_cons(item,BL_NIL);

    BL_lisp tail = bl_tail(list);
    if (!bl_islist(tail) || !bl_isnil(bl_cdr(tail)))
    {
      bl_err(-1,"cannot append to non proper list");
      return BL_BAD;
    }

    bl_setcdr(tail,bl_cons(item,0));
    return list;
  }

//==============================================================================
// retrieve from an assoc table a pair/list with associated key
// - usage: pair = ef_assoc(key,table) // pair is (key . val), NIL if not found
// - table: ((A ...) (8 ...) ("q" ...)) - keys: A,8,"q"
//==============================================================================

  BL_lisp bl_assoc(BL_lisp key, BL_lisp table)
  {
    while (bl_islist(table))
    {
      BL_lisp pair = bl_car(table);
      table = bl_cdr(table);

      if (bl_islist(pair) && bl_car(pair) == key)
        return pair;
    }
    return BL_NIL;
  }

//==============================================================================
// print lisp expression
// - usage: err = bl_prtl(ref);
//==============================================================================
#if (CFG_LISP_DEBUG)

  int bl_prtl(BL_lisp ref)
  {
    if (bl_isnil(ref))
    {
      bl_prt("NIL");  return 0;
    }
    else if (bl_iserr(ref))
	  {
		  bl_prt("*ERR*");  return 0;
	  }
    else if (bl_issym(ref))
    {
      bl_prt("%s",bl_name(ref));
      return 0;
    }
    else if (bl_islist(ref)) // list
    {
      BL_txt sep = "";
      bl_prt("(");

      for (;bl_islist(ref); ref = bl_cdr(ref))
      {
bl_sleep(5);
        BL_lisp ref_car = bl_car(ref);
        BL_lisp ref_cdr = bl_cdr(ref);

        bl_prt("%s",sep);  sep = " ";

        if (bl_issym(ref_cdr) )
        {
          bl_prtl(ref_car);
          bl_prt(" . %s",bl_name(ref_cdr));
          break;
        }
        bl_prtl(ref_car);
      }
      bl_prt(")");
    }
    else
      bl_prt("???");

    return 0;
  }

#endif
//==============================================================================
// log lisp expression
// - usage: err = bl_logl(1,"this is text: ",ref);
//==============================================================================
#if (CFG_LISP_DEBUG)

  int bl_logl(int lvl,BL_txt txt,BL_lisp ref)
  {
    bl_log(lvl,"");
    bl_prt("%s",txt);
    bl_prtl(ref);
    bl_prt("\n"BL_0);
    return 0;
  }

#endif
