//==============================================================================
// bl_mesh.h - Bluetooth mesh typedefs
// central header for Zephyr Bluetooth/BLE/Mesh stuff
//
// Created by Hugo Pristauz on 19.02.2022
// Copyright © 2019 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_MESH_H__
#define __BL_MESH_H__

  #include <zephyr/bluetooth/mesh.h>
  #include "bl_blue.h"

//==============================================================================
// some defines
//==============================================================================

  #define BL_VOID_ADDR            BT_MESH_ADDR_UNASSIGNED
  #define BL_VOID_KEY             BT_MESH_KEY_UNUSED

  #define BL_UNUSED(x) (void)(x)

//==============================================================================
// time type
// - we define BL_ms to represent milisec's since system start or clock restart
//   in 64-bit signed integer representation.
// - this allows
//     a) negative time stamps to indicate invalid time stamps
//     b) sufficient long time () before overrun
//        (note 1 year .= 2^35 ms, 2^28 years .= 268 million years = 2^63 ms)
// - BL_ms is an expensive data structure (8 bytes), so we define an inexpen-
//   sive BL_tick type for measuring and scheduling delays
//==============================================================================

  typedef int16_t BL_tick;             // mili seconds

//==============================================================================
// basic types for mesh communication
//==============================================================================

  typedef uint16_t BL_level;           // 0 .. 10000
  typedef int16_t BL_signed;           // 0x8000 .. 0 .. 0x7ffff
//typedef uint16_t BL_val;             // 0 .. 0xffff => use BL_word
  typedef uint16_t FL_addr;            // 0 .. 0xffff (mesh element address)
//typedef const uint8_t *BL_pay;       // const byte *  (e.g. payloads)

  typedef uint16_t BL_iid;             // mesh model instance ID
  typedef uint16_t BL_mmid;            // mesh model ID

//==============================================================================
// data structure to control transitions
//==============================================================================

  typedef struct BL_trans              // transition
          {
            int target;                // target level of transition
            int basis;                 // basis (starting) level of transition
            BL_ms begin;               // beginning time of transition
            int tt;                    // transition time in ms (duration)
          } BL_trans;

//==============================================================================
// legacy
//==============================================================================

    #define BL_VAL_MAX     UINT16_MAX  // max value of BfValT type
    #define BL_SIGNED_MIN  INT16_MIN   // min value of BfSignedT type
    #define BL_SIGNED_MAX  INT16_MAX   // max value of BfSignedT type
    #define BL_LEVEL_MAX   10000       // max value of BfLevelT type

    #define BL_RANGE_SIGNED  ((long)BL_SIGNED_MAX-(long)BL_SIGNED_MIN)

    #define BL_LONGLEVEL2SIGNED(level)   (((long)level * BL_RANGE_SIGNED) / BL_LEVEL_MAX + BL_SIGNED_MIN)
    #define BL_LONGSIGNED2LEVEL(signd)   ((((long)signd-BL_SIGNED_MIN) * BL_LEVEL_MAX) / BL_RANGE_SIGNED)

    #define BL_LEVEL2SIGNED(level)       (  (BL_signed) BL_LONGLEVEL2SIGNED(level)                 )
    #define BL_SIGNED2LEVEL(signd)       (  (BL_level)(((BL_LONGSIGNED2LEVEL(signd)+1)/10)*10)     )

    #define BL_S2P(sgn)                  ((uint16_t)(BL_SIGNED2LEVEL(sgn)/100))

//==============================================================================
// transition type
//==============================================================================

  typedef struct BL_transition
  {
  	bool just_started;
  	uint8_t type;
  	uint8_t tt;
  	uint8_t rt;
  	uint8_t delay;
  	uint32_t quo_tt;
  	uint32_t counter;
  	uint32_t total_duration;
  	int64_t start_timestamp;

  	struct k_timer timer;
  } BL_transition;

//==============================================================================
// event types
//==============================================================================

    typedef enum BL_event    // bit masks
            {
                BL_EVT_ON_EDGE    = 0x01,
                BL_EVT_OFF_EDGE   = 0x02,
                BL_EVT_ON_HOLD    = 0x04,
                BL_EVT_OFF_HOLD   = 0x08,
                BL_EVT_ON_PULSE   = 0x10,
                BL_EVT_OFF_PULSE  = 0x20,
                BL_EVT_2CLICKS    = 0x40,
            } BL_event;

//==============================================================================
// persisting handler entry
//==============================================================================

    typedef struct BL_persist
    {
      	BL_txt pkey;
      	BL_txt ppath;
        void (*store)(BL_txt ppath);
      	int (*recall)(BL_txt ppath, void *pctx);
    } BL_persist;

//==============================================================================
// short hand for Zephyr structure types
//==============================================================================

//  typedef struct k_timer           BL_timer;        // timer structure
//  typedef struct k_work            BL_work;         // worker structure
//  typedef struct device            BL_device;       // device controller
//  typedef struct butctl            BL_button;       // button controller

    typedef struct bt_mesh_model_pub BL_pub;          // publisher
    typedef struct bt_mesh_model     BL_model;        // mesh model
    typedef struct bt_mesh_msg_ctx   BL_ctx;          // context
    typedef struct net_buf_simple    FL_nbs;          // simple net buffer
    typedef struct bt_mesh_model_op  BL_ocmap;        // opcode map
    typedef struct bt_mesh_elem      BL_element;      // mesh element
    typedef struct bt_mesh_comp      BL_comp;         // device composition

//==============================================================================
// data structure for de-ja-vu
//==============================================================================

    typedef struct BL_dejavu
    {
        uint8_t tid;               // transaction ID of de-ja-vu
        uint16_t src;              // source address of de-ja-vu
        uint16_t dst;              // destination address of de-ja-vu
        BL_ms time;             // time stamp of de-ja-vu
    } BL_dejavu;

//==============================================================================
// short hand for Zephyr structure types
//==============================================================================

#define BL_DNP_DATA_LENGTH 8

    typedef struct BL_msg  // message data structure, containing data buffer, net-buffer & publisher
    {
        uint8_t data[BL_DNP_DATA_LENGTH];  // payload data buffer
        FL_nbs nbs;                        // simple netbuf
        BL_pub pub;                        // publishing data structure
    } BL_msg;

    static inline void bl_msg_init(BL_msg *pmsg)
    {
        pmsg->nbs.data = pmsg->data;
        pmsg->nbs.len  = 0;
        pmsg->nbs.size = BL_DNP_DATA_LENGTH;
        pmsg->nbs.__buf = pmsg->data;

        pmsg->pub.update = NULL;
        pmsg->pub.msg = &pmsg->nbs;
    }

//==============================================================================
// opcode & company ID
//==============================================================================

    #define BL_SIG_CID     0xFFFF  // used for Bluetooth SIG (non vendor) models
    #define BL_BNX_CID     0x07C6  // used for Bluenetics (vendor) models

    #define BL_SIG_OC(oc)  {.oc=oc, .cid = BL_SIG_CID}
    #define BL_BNX_OC(oc)  {.oc=oc, .cid = BL_BNX_CID}

//==============================================================================
// conversion msec <-> ticks or mesh format
//==============================================================================

    uint8_t bl_ms2mesh(BL_ms ms);
    int bl_mesh2ms(uint8_t mf);                // use int instead of BL_ms type
    BL_u8 bl_delay_ticks(BL_u8 repeats, BL_u8 i, BL_ms delay, BL_ms interval);
    int bl_tick2ms(uint8_t ticks);             // use int instead of BL_ms type

    static inline int bl_delay2mesh(BL_ms ms)
    {
      return bl_ms2mesh(ms);
    }

    static inline int bl_delay2ms(BL_byte ticks)
    {
      return bl_tick2ms(ticks);
    }

    static inline int bl_tt2ms(BL_byte mf)
    {
      return bl_mesh2ms(mf);
    }

    #define BL_MS2MESH(ms)          bl_ms2mesh(ms)
    #define BL_MESH2MS(mf)          bl_mesh2ms(mf)
    #define BL_TICK2MS(ticks)       bl_tick2ms(ticks)

//==============================================================================
// provisioning
//==============================================================================

    bool bl_is_provisioned(void);         // wrapper for bt_mesh_is_provisioned()

    uint16_t bl_primary_addr(void);
//  bool bl_provisioned(void);
    void bl_node_reset(void);             // reset provisioning status


    void bl_mesh_start(const BL_comp *pcomp);

//==============================================================================
// set/get device composition pointer
//==============================================================================

    void bl_set_comp(const BL_comp *pcomp);
    //const BL_comp *bl_get_comp(void);
    const BL_comp *bl_comp_pointer(void);  // get device composition pointer

//==============================================================================
// get instance ID from model pointer
//==============================================================================

    uint8_t bl_element_count(void);
    uint8_t bl_model_count(uint8_t el, bool vnd);

    BL_model *bl_model(BL_iid zid);        // get model pointer from instance ID
    BL_iid bl_iid(BL_model *pmod);

//==============================================================================
// set appkey, publishing address and subscription address
//==============================================================================

    BL_element *bl_element_pointer(uint8_t ele_idx);
    BL_model *bl_model_pointer(uint8_t ele_idx, uint8_t mod_idx, bool vnd);

    int bl_set_element_addr(uint8_t ele_idx, uint16_t addr);

//==============================================================================
// reset mesh model
//==============================================================================

    void bl_reset_model(uint8_t ele_idx, uint8_t mod_idx, bool vnd);
    void bl_reset_tree(void);

//==============================================================================
// get element address
//==============================================================================

    uint16_t bl_get_element_addr(uint8_t ele_idx);

//==============================================================================
// source and destination address of mesh message
//==============================================================================

    FL_addr bl_src(BL_ctx *ctx);        // source address of mesh message
    FL_addr bl_dst(BL_ctx *ctx);        // destination address of mesh message
    FL_addr bl_me(BL_model *pmod);      // model's element address

//==============================================================================
// mesh init
//==============================================================================

    int bl_mesh_init(const BL_comp *pcomp);

#endif // __BL_MESH_H__
