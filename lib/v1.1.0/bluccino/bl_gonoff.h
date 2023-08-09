//==============================================================================
//  bl_gonoff.h
//  generic onoff model
//
//  Created by Hugo Pristauz on 20.02.2022
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_GONOFF_H__
#define __BL_GONOFF_H__

  #include "bl_mesh.h"
  #include "bl_type.h"

//==============================================================================
// mesh model ID and mesh model operation codes
//==============================================================================

  #define BL_GOOCLI        BT_MESH_MODEL_ID_GEN_ONOFF_CLI
  #define BL_GOOSRV        BT_MESH_MODEL_ID_GEN_ONOFF_SRV

  #define BL_GOOGET        BT_MESH_MODEL_OP_2(0x82, 0x01)
  #define BL_GOOSET        BT_MESH_MODEL_OP_2(0x82, 0x02)
  #define BL_GOOLET        BT_MESH_MODEL_OP_2(0x82, 0x03)
  #define BL_GOOSTS        BT_MESH_MODEL_OP_2(0x82, 0x04)

//==============================================================================
// typedefs for generic on/off messages
//==============================================================================

  typedef struct BL_gooset
          {
            uint8_t target;       // the target value of the generic OnOff state
            uint8_t tid;          // transaction identifier
            uint8_t tt;           // format as defined in section 3.1.3 (opt.)
            uint8_t delay;        // msg execution delay in 5 ms steps (C.1)
          } BL_gooset;

  typedef struct BL_goosts
          {
            uint8_t target;       // target value of generic OnOff state (opt.)
            uint8_t current;      // present value of generic OnOff state.
            uint8_t remain;       // format as defined in section 3.1.3 (C.1)
          } BL_goosts;

//==============================================================================
// universal type for communication between app and generic on/off models
//==============================================================================

  typedef struct BL_goo           // generic on/off data structure
          {
            BL_trans trans;
            int delay;            // delay [ms]
            int tt;               // delay [ms]
            BL_byte tid;          // transaction identifier
            int remain;           // remaining time [ms] to transition
            bool acked;           // received a SET or LET?
          } BL_goo;

//==============================================================================
// generic on/off state (used at generic on/off server model layer)
//==============================================================================

    typedef struct BL_gonoff_state
    {
        uint8_t onoff;
        uint8_t target_onoff;

        BL_goo goo;               // generic on/off data structure

        uint8_t last_tid;
        uint16_t last_src_addr;
        uint16_t last_dst_addr;
        int64_t last_msg_timestamp;

        int32_t tt_delta;

        BL_transition *transition;
    } BL_gonoff_state;

//==============================================================================
// [GOOSRV:] message definition
// - [GOOSRV:STS id,<BL_goo>,sts] generic on/off model status message
//==============================================================================

  #define GOOSRV_STS_ix_BL_goo_sts BL_ID(_GOOSRV,STS_)

    // augmented messages

  #define _GOOSRV_STS_ix_BL_goo_sts _BL_ID(_GOOSRV,STS_)

//==============================================================================
// [GOOCLI:] message definition
// - [GOOCLI:SET id,<BL_goo>,onoff] generic on/off model SET message
// - [GOOCLI:LET id,<BL_goo>,onoff] generic on/off model SET message
// - [GOOCLI:GET id,<BL_goo>,onoff] generic on/off model SET message
//==============================================================================

  #define GOOCLI_SET_ix_BL_goo_onoff BL_ID(_GOOCLI,SET_)
  #define GOOCLI_LET_ix_BL_goo_onoff BL_ID(_GOOCLI,LET_)
  #define GOOCLI_GET_ix_0_0          BL_ID(_GOOCLI,GET_)

    // augmented messages

  #define _GOOCLI_SET_ix_BL_goo_onoff _BL_ID(_GOOCLI,SET_)
  #define _GOOCLI_LET_ix_BL_goo_onoff _BL_ID(_GOOCLI,LET_)
  #define _GOOCLI_GET_ix_0_0          _BL_ID(_GOOCLI,GET_)

//==============================================================================
// syntactic sugar: send generic on/off SET message via mesh (using GOOCLI @ix)
// - usage: val = bl_gooset(id,NULL,onoff)
//          val = bl_gooset(id,goo,onoff)
//==============================================================================

  static inline int bl_gooset(int id, BL_goo *goo, int onoff)
  {
    return bl_post((bl_down),GOOCLI_SET_ix_BL_goo_onoff, id,goo,onoff);
  }

  static inline int _bl_gooset(int id, BL_goo *goo, int onoff, BL_oval to)
  {
    return bl_post((to), _GOOCLI_SET_ix_BL_goo_onoff, id,goo,onoff);
  }

//==============================================================================
// syntactic sugar: send generic on/off LET message via mesh (using GOOCLI @ix)
// - usage: val = bl_goolet(id,onoff)  // (BL_DOWN)<-[GOOCLI:LET @ix,onoff]
//==============================================================================

  static inline int bl_goolet(int id, BL_goo *goo, int onoff)
  {
    return bl_post((bl_down),GOOCLI_LET_ix_BL_goo_onoff, id,goo,onoff);
  }

  static inline int _bl_goolet(int id, BL_goo *goo, int onoff, BL_oval to)
  {
    return bl_post((to), _GOOCLI_LET_ix_BL_goo_onoff, id,goo,onoff);
  }

//==============================================================================
// syntactic sugar: send generic on/off GET message via mesh (using GOOCLI @ix)
// - usage: val = bl_googet(id)       // (BL_DOWN)<-[GOOCLI:GET @ix]
//==============================================================================

  static inline int bl_googet(int id)
  {
    return bl_post((bl_down), GOOCLI_GET_ix_0_0, id,NULL,0);
  }

  static inline int _bl_googet(int id, BL_oval to)
  {
    return bl_post(to, GOOCLI_GET_ix_0_0, id,NULL,0);
  }

//==============================================================================
// miscellaneous
//==============================================================================
/*
    #define STATE_OFF        0x00
    #define STATE_ON         0x01
    #define STATE_DEFAULT    0x01
    #define STATE_RESTORE    0x02

    #define BL_OFF   ((uint8_t)0)   // symbolic value for OFF
    #define BL_ON    ((uint8_t)1)   // symbolic value for ON
*/
//==============================================================================
// dynamic onoff model generation
//==============================================================================
/*
    extern uint8_t            zl_tid_onoff;          // TID for gonoff messages
    extern BL_gonoff_state zl_gonoff_state0;  // onoff state for GONOFF_CLI0

    BL_model *zl_gonoff_srv_add(BL_element *pel, BL_pub *ppub, BL_gonoff_state *pud);
    BL_model *zl_gonoff_cli_add(BL_element *pel, BL_pub *ppub, BL_gonoff_state *pud);
*/
//==============================================================================
// opcode map table
//==============================================================================
/*
    extern const BL_ocmap zl_gonoff_srv_op[];
    extern const BL_ocmap zl_gonoff_cli_op[];
*/
//==============================================================================
// publish generic onoff state
//==============================================================================
/*
    void zl_tx_gonoff(BL_model *model);
*/
//==============================================================================
// send generic onoff message
//==============================================================================
/*
    int zl_tx_gonoff_set(BL_model *pmod, uint8_t on, uint8_t tid, uint8_t tt, uint8_t d);
    int zl_tx_gonoff_let(BL_model *pmod, uint8_t on, uint8_t tid, uint8_t tt, uint8_t d);
    int zl_tx_gonoff_get(BL_model *pmod);
*/

#endif // __BL_GONOFF_H__
