//==============================================================================
//  bl_lib.c
//  Bluccino library/service/mesh model registration primitives
//
//  Created by Hugo Pristauz on 2022-08-14
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_lib.h"

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO "bl_lib:"           // who is logging?

  #define LOG                     LOG_LIB
  #define LOGO(lvl,col,o,val)     LOGO_LIB(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_LIB(lvl,col,o,val)

//==============================================================================
// link registry node into registry list
// - usage: static BL_lib *list = NULL;
//          bl_reglink(o,&list);   // link registry node into registry list
//==============================================================================

  int bl_reglink(BL_ob *o, BL_lib **plist)
  {
    BL_lib *reg = (BL_lib*)bl_data(o);

    if (reg)
    {
      LOG(4,"register service/module <%s|%s>",BL_IDTXT(reg->id));
      reg->next = *plist;  *plist = reg;
    }
    return 0;
  }

//==============================================================================
// iterate through list of library modules
// - usage: static BL_lib *list = NULL;
//          bl_iter(o,val,list);       // iterate through list of libraries
//==============================================================================

  int bl_iter(BL_ob *o, int val, BL_lib *list)
  {
    int rv = BL_VOID;                  // interface not handeled by default
    int count = 0;

    for (;list; list = list->next)
    {
      if (list->module)
      {
        int err = list->module(o,val); // execute registered library module
        if (err != BL_VOID)            // last come overrides
        {
          rv = err;  count++;
        }
      }
    }
    return (count == 1) ? rv : BL_VOID;
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
