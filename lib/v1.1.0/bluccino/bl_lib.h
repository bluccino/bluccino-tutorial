//==============================================================================
//  bl_lib.h
//  Bluccino library/service/mesh model registration primitives
//
//  Created by Hugo Pristauz on 2022-08-14
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_LIB_H__
#define __BL_LIB_H__

//==============================================================================
// register library module
// - usage: static BL_lib reg = {module,BL_ID(_SVC,SVC1_),NULL};
//          bl_lib(library,&reg);   // register module as part of library @ix
//==============================================================================

  static inline int bl_lib(BL_oval to, BL_lib *registry)
  {
    return bl_msg((to), _SYS,LIB_, 0,registry,0);
  }

//==============================================================================
// link registry node into registry list
// - usage: static BL_lib *list = NULL;
//          bl_reglink(o,&list);   // link registry node into registry list
//==============================================================================

  int bl_reglink(BL_ob *o, BL_lib **plist);

//==============================================================================
// iterate through list of library modules
// - usage: static BL_lib *list = NULL;
//          bl_iter(o,val,list);       // iterate through list of libraries
//==============================================================================

  int bl_iter(BL_ob *o, int val, BL_lib *list);

#endif // __BL_LIB_H__
