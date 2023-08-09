//==============================================================================
//  bl_deco.c
//  log (time stamp) decoration according to mesh attention/provision state
//
//  Created by Hugo Pristauz on 2022-06-16
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_deco.h"

//==============================================================================
// PMI and logging shorthands
//==============================================================================

  #define PMI                     bl_deco
  #define WHO                     "bl_deco:"

  #define LOG                     LOG_GEAR
  #define LOGO(lvl,col,o,val)     LOGO_GEAR(lvl,col WHO,o,val)

//==============================================================================
// public module interface
//==============================================================================
//
// (A) := (app);  (M) := (main);  (W) := (bl_wl);  (*) := (<any>)
//
//                  +--------------------+
//                  |       bl_deco      | log decoration
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (W)-> REGISTER ->|                    | advise to self-register @ top gear
//                  +--------------------+
//                  |        BLE:        | BLE input interface
// (T)->  CONNECT ->|      <BL_conn>     | notify connection event
// (T)->   DISCON ->|  <BL_conn>,reason  | notify disconnection event
//                  +--------------------+
//                  |       MESH:        | MESH input interface
// (T)->      ATT ->|        sts         | notiy attention status
// (T)->      PRV ->|        sts         | notiy provision status
//                  +--------------------+
//                  |        GET:        | GET input interface
// (T)->      ATT ->|        sts         | retrieve attention status
// (T)->      PRV ->|        sts         | retrieve provision status
//                  +--------------------+
//
// decorate loging according to [MESH:ATT] and [MESH:PRV] events
//==============================================================================

  int bl_deco(BL_ob *o, int val)
  {
    static BL_oval T = bl_top;         // top gear

    static BL_lib lib = {PMI,BL_ID(_LIB,DECO_),NULL};  // lib ID <LIB:DECO>
    static bool att = false;
    static bool prv = false;

    int *pint;                         // work pointer to int data

    switch (bl_cl(o))                  // fast check of supported interfaces
    {
      case _SYS:                       // SYS interface is supported
      case _BLE:                       // BLE interface is supported
      case _MESH: break;               // MESH interface is supported
      case _GET:                       // GET interface is supported
        break;
      default:
        return -1;                     // other interfaces not supported
    }

    switch (bl_id(o))                  // dispatch event
    {
      case BL_ID(_SYS,INSTALL_):       // install bl_deco @ top gear
        LOG(3,"install bl_deco in top gear");
        bl_lib(T,&lib);                // register bl_deco as lib @ top  gear
        return 0;

      case BL_ID(_BLE,CONNECT_):
        bl_color(BL_C);                // indicate connection by log head color
        return 0;

      case BL_ID(_BLE,DISCON_):
        bl_color(BL_0);                // indicate a disconn. by log head color
        return 0;

      case BL_ID(_MESH,ATT_):          // attention state changed
        att = (val != 0);
        bl_decorate(att,prv);
        LOG(2,BL_G "bl_top: attention %s",val?"on":"off");
        return 0;

      case BL_ID(_MESH,PRV_):          // provision state changed
        prv = (val != 0);
        bl_decorate(att,prv);
        LOG(2,BL_M"bl_top: node %sprovision",val?"":"un");
        return 0;

      case BL_ID(_STATE,ATT_):         // retrieve attention status
        pint = bl_data(o);
        *pint = att;                   // return back attention state
        return 0;                      // OK

      case BL_ID(_STATE,PRV_):         // retrieve provision status
        pint = bl_data(o);
        *pint = prv;                   // return back provision state
        return 0;                      // OK

      case BL_ID(_GET,ATT_):           // retrieve attention status
        bl_err(-1,"bl_node: [GET:ATT] has been obsoleted");
        return att;

      case BL_ID(_GET,PRV_):           // retrieve provision status
        bl_err(-1,"bl_node: [GET:PRV] has been obsoleted");
        return prv;

      default:
        return BL_VOID;                // did not handle
    }
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
