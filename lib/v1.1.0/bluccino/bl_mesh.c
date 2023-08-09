//==============================================================================
//  bl_mesh.c
//  Bluetooth mesh initializing / provisioning
//
//  Created by Hugo Pristauz on 19.02.2022
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================
//
// Some Notes
//
// 1) for the opcode dispatch table the following structures are defined in <acess.h>
//
//    BL_ocmap       // opcode dispatch table entry
//    {
//      const u32_t  opcode;  // OpCode encoded using BT_MESH_MODEL_OP_* macros
//      const size_t min_len; // minimum required message length
//      void (*const func)(model_t *pm, ctx_t *pc, nbs_t *buf);  // msg handler
//    };
//
// 2) mesh model instance abstraction: <access.h> defines struct bt_mesh_model
//    which is an abstraction describing a mesh model instance.
//    Here are the details:
//
//        #define BL_KEY_COUNT    CONFIG_BT_MESH_MODEL_KEY_COUNT
//        #define BL_GRP_COUNT    CONFIG_BT_MESH_MODEL_GROUP_COUNT
//
//        typedef struct bt_mesh_model_pub BL_pub;          // publisher
//        typedef BL_ocmap  BL_ocmap;        // opcode map
//
//        typedef struct BL_vnd
//                {
//                    BL_u16 company;
//                    BL_u16 id;
//                } BL_vnd;
//
//        struct bt_mesh_model       // abstraction to describe a model instance
//        {
//            union                  // mesh model ID
//            {
//                const BL_u16 id;
//                BL_vnd vnd;
//            };
//
//                // it follows internal info, mainly for persistent storage
//
//            BL_u8  elem_idx;        // belongs to Nth element
//            BL_u8  mod_idx;         // is the Nth model in the element
//            BL_u16 flags;           // information about what has changed
//
//                  // next info is for model publication
//
//            const BL_pub *const pub;       // model publishing list
//            BL_u16 keys[BL_KEY_COUNT];      // appkey list
//            BL_u16 groups[BL_GROUP_COUNT];  // subscription list (groups|vaddr)
//
//            const BL_ocmap * const op;     // opcode dispatch table
//            void *user_data;               // model-specific user data
//        };
//
// 3) Mesh Model Publication Context (see access.h)
//
//        The context should primarily be created using the
//        BT_MESH_MODEL_PUB_DEFINE macro.
//
//        struct bt_mesh_model_pub
//        {
//            BL_model *mod;      // model owning the context. init by stack.
//
//          	BL_u16 addr;         // publish address
//          	BL_u16 key;          // publish appkey index
//
//	          BL_u8  ttl;          // publish time to live
//	          BL_u8  retransmit;   // retransmit count & interval steps
//	          BL_u8  period;       // publish period
//	          BL_u8  period_div:4, // divisor for period
//	                cred:1,       // friendship credentials flag
//     	            fast_period:1,// use FastPeriodDivisor
//	                count:3;      // retransmissions left
//
//	          u32_t period_start; // start of current period
//         }
//
//==============================================================================

#if defined(__ZEPHYR__)
    #include <zephyr/settings/settings.h>
    #include <zephyr/bluetooth/l2cap.h>
#endif

  #include "bluccino.h"
  #include "bl_mesh.h"

/*
#include "zlassert.h"
#include "zlerr.h"
#include "zlclock.h"
#include "zlmesh.h"
#include "zlnotify.h"
#include "zlreset.h"
#include "zlscan.h"
#include "zltrace.h"
*/

//==============================================================================
// MESH level logging shorthands
//==============================================================================

  #define LOG                     LOG_MESH
  #define LOGO(lvl,col,o,val)     LOGO_MESH(lvl,col"mesh:",o,val)
  #define LOG0(lvl,col,o,val)     LOGO_MESH(lvl,col,o,val)

//==============================================================================
// some defines
//==============================================================================

    #define MESH_NMB_OF_MS_IN_100_MS               (100UL)
    #define MESH_NMB_OF_MS_IN_1S                   (10*MESH_NMB_OF_MS_IN_100_MS)
    #define MESH_NMB_OF_MS_IN_10S                  (10*MESH_NMB_OF_MS_IN_1S)
    #define MESH_NMB_OF_MS_IN_10MIN                (60*MESH_NMB_OF_MS_IN_10S)
    #define MESH_TT_STEP_RESOLUTION_MASK           0xC0
    #define MESH_TT_STEP_RESOLUTION_100_MS         0x00
    #define MESH_TT_STEP_RESOLUTION_1_S            0x40
    #define MESH_TT_STEP_RESOLUTION_10_S           0x80
    #define MESH_TT_STEP_RESOLUTION_10_MIN         0xC0
    #define MESH_TT_NUMBER_OF_STEPS_MASK           0x3F
    #define MESH_TT_NUMBER_OF_STEPS_UNKNOWN_VALUE  0x3F

    #define MESH_DELAY_TIME_STEP_MS          5   // smallest delay quantum
    #define MESH_REPEATS_INTERVAL_SET_MS    20   // time between SET msg repeats
    #define MESH_REPEATS_INTERVAL_GET_MS   200   // time between GET msg repeats

 // static BL_u16 primary_addr = BL_VOID_ADDR;
 // static BL_u16 primary_net_idx;
 // static const BL_comp *m_pcomp = NULL;   // temp for passing to bt_ready()

        // next one is from access.c but is not defined in access.h

    const struct bt_mesh_comp *bt_mesh_comp_get(void);

//==============================================================================
// conversion msec <-> ticks or mesh format
//==============================================================================

    BL_u8 bl_ms2mesh(BL_ms ms)
    {
        if ((ms / MESH_NMB_OF_MS_IN_100_MS) < MESH_TT_NUMBER_OF_STEPS_UNKNOWN_VALUE)
        {
            return (MESH_TT_STEP_RESOLUTION_100_MS | (ms / MESH_NMB_OF_MS_IN_100_MS));
        }
        else if ((ms / MESH_NMB_OF_MS_IN_1S) < MESH_TT_NUMBER_OF_STEPS_UNKNOWN_VALUE)
        {
            return (MESH_TT_STEP_RESOLUTION_1_S | (ms / MESH_NMB_OF_MS_IN_1S));
        }
        else if ((ms / MESH_NMB_OF_MS_IN_10S)
                 < MESH_TT_NUMBER_OF_STEPS_UNKNOWN_VALUE)
        {
            return (MESH_TT_STEP_RESOLUTION_10_S | (ms / MESH_NMB_OF_MS_IN_10S));
        }
        else if ((ms / MESH_NMB_OF_MS_IN_10MIN)
                 < MESH_TT_NUMBER_OF_STEPS_UNKNOWN_VALUE)
        {
            return (MESH_TT_STEP_RESOLUTION_10_MIN | (ms / MESH_NMB_OF_MS_IN_10MIN));
        }
        else
        {
            return (MESH_TT_STEP_RESOLUTION_10_MIN
                    | (MESH_TT_NUMBER_OF_STEPS_UNKNOWN_VALUE
                       & MESH_TT_NUMBER_OF_STEPS_MASK));
        }
    }

    int bl_mesh2ms(BL_u8 mf)    // use int and not BL_ms type
    {
        uint32_t number_of_steps = (mf & MESH_TT_NUMBER_OF_STEPS_MASK);
        uint32_t step_resolution = (mf & MESH_TT_STEP_RESOLUTION_MASK);

        if (MESH_TT_NUMBER_OF_STEPS_UNKNOWN_VALUE == number_of_steps)
        {
            return -1;      // invalid input argument
        }
        else
        {
            switch (step_resolution)
            {
                case MESH_TT_STEP_RESOLUTION_10_MIN:
                    return (uint32_t)MESH_NMB_OF_MS_IN_10MIN * number_of_steps;

                case MESH_TT_STEP_RESOLUTION_10_S:
                    return (uint32_t)MESH_NMB_OF_MS_IN_10S * number_of_steps;

                case MESH_TT_STEP_RESOLUTION_1_S:
                    return (uint32_t)MESH_NMB_OF_MS_IN_1S * number_of_steps;

                case MESH_TT_STEP_RESOLUTION_100_MS:
                    return (uint32_t)MESH_NMB_OF_MS_IN_100_MS * number_of_steps;

                default:
                    return (uint32_t)MESH_NMB_OF_MS_IN_100_MS * number_of_steps;
            }
        }
    }

    BL_u8 bl_delay_ticks(BL_u8 repeats, BL_u8 i, BL_ms delay, BL_ms interval)
    {
      uint16_t timestep_ms = MESH_DELAY_TIME_STEP_MS;   // = 5
      // BL_ms interval =  MESH_REPEATS_INTERVAL_SET_MS;
      uint16_t basedelay = (uint16_t)((repeats-i)*(int)interval / timestep_ms);
      uint16_t transitionTimeSteps = basedelay + delay / timestep_ms;

      return transitionTimeSteps;                // return delay to transition
    }

    int bl_tick2ms(BL_u8 ticks)                     // use int and not BL_ms type
    {
        return (ticks * MESH_DELAY_TIME_STEP_MS);  // * 5
    }


//===================================================================================================
// is node provisioned?
//===================================================================================================
/*
    bool bl_is_provisioned(void)         // wrapper for bt_mesh_is_provisioned()
    {
        return bt_mesh_is_provisioned();
    }
*/
//===================================================================================================
// set/get device composition pointer
//===================================================================================================

    static const BL_comp *the_comp = NULL;       // 'the' pointer to device composition

    void bl_set_comp(const BL_comp *pcomp)
    {
        the_comp = pcomp;
    }

    // const BL_comp *bl_get_comp(void)
    // {
    //     return the_comp;
    // }

//===================================================================================================
// get pointer to device composition (from access level)
//===================================================================================================

    const BL_comp *bl_comp_pointer(void)        // get device composition from mesh stack
    {
        return bt_mesh_comp_get();              // fetch pointer from mesh stack access level
    }

//===================================================================================================
// get pointer to element by element index
//===================================================================================================

    BL_element *bl_element_pointer(BL_u8 ele_idx)
    {
        const BL_comp *pcomp = bl_comp_pointer();

        if ( 0 <= ele_idx && ele_idx < pcomp->elem_count)
            return pcomp->elem + ele_idx;

        bl_err(BL_ERR_BADARG, "bl_element_pointer(): bad element index");
        return NULL;
    }

//===================================================================================================
// get pointer to SIG or vendor model by element and model index
//===================================================================================================

    static BL_model *bl_sig_model_pointer(BL_u8 ele_idx, BL_u8 mod_idx)
    {
        BL_element *pel = bl_element_pointer(ele_idx);

        if (!pel)
            return NULL;       // error has been already reported!

        if ( 0 <= mod_idx && mod_idx < pel->model_count)
            return pel->models + mod_idx;

        bl_err(BL_ERR_BADARG, "bl_sig_model_pointer(): bad model index");
        return NULL;
    }

    static BL_model *bl_vnd_model_pointer(BL_u8 ele_idx, BL_u8 mod_idx)
    {
        BL_element *pel = bl_element_pointer(ele_idx);

        if (!pel)
            return NULL;       // error has been already reported!

        if ( 0 <= mod_idx && mod_idx < pel->vnd_model_count)
            return pel->vnd_models + mod_idx;

        bl_err(BL_ERR_BADARG, "bl_vnd_model_pointer(): bad model index");
        return NULL;
    }

    BL_model *bl_model_pointer(BL_u8 ele_idx, BL_u8 mod_idx, bool vnd)
    {
        if (vnd)                  // vendor or SIG model?
            return bl_vnd_model_pointer(ele_idx, mod_idx);
        else
            return bl_sig_model_pointer(ele_idx, mod_idx);
    }

//===================================================================================================
// get element count and model count
//===================================================================================================

    BL_u8 bl_element_count(void)
    {
        const BL_comp *pcomp = bl_comp_pointer();
        return pcomp->elem_count;
    }

    BL_u8 bl_model_count(BL_u8 el, bool vnd)
    {
        BL_element *pel = bl_element_pointer(el);

        if (!pel)
           return 0;    // error: bad input args (already reported)

        if (vnd)
            return pel->vnd_model_count;
        else
            return pel->model_count;
    }

//==============================================================================
// get model pointer from instance ID
//==============================================================================

    BL_model *bl_model(BL_iid zid)        // get model pointer from instance ID
    {
        const BL_comp *pcomp = bl_comp_pointer();

        for (BL_u8 el = 0; el < pcomp->elem_count; el++)        // all elements
        {
            BL_element *pel = bl_element_pointer(el);
            bl_assert(pel);

                // now walk through all SIG models of this element

            for (BL_u8 mi = 0; mi < pel->model_count; mi++)     // all sig models
            {
                if (zid == 0)
                   return (pel->models + mi);
                zid--;           // otherwise decrement and continue seeking
            }

                // now walk through all vendor models of this element

            for (BL_u8 mi = 0; mi < pel->vnd_model_count; mi++) // all vnd models
            {
                if (zid == 0)
                   return (pel->vnd_models + mi);
                zid--;           // otherwise decrement and continue seeking
            }
        }

        bl_err(BL_ERR_BADARG, "bl_model(): bad model index");
        return NULL;
    }

//==============================================================================
// get instance ID from model pointer
//==============================================================================

    BL_iid bl_iid(BL_model *pmod)
    {
        const BL_comp *pcomp = bl_comp_pointer();
        BL_iid zid = 0;

        for (BL_u8 el = 0; el < pcomp->elem_count; el++)        // all elements
        {
            BL_element *pel = bl_element_pointer(el);
            bl_assert(pel);

                // now walk through all SIG models of this element

            for (BL_u8 mi = 0; mi < pel->model_count; mi++)     // all sig models
            {
                if (pmod == pel->models + mi)
                   return zid;
                zid++;           // otherwise increment and continue seeking
            }

                // now walk through all vendor models of this element

            for (BL_u8 mi = 0; mi < pel->vnd_model_count; mi++) // all vnd models
            {
                if (pmod == pel->vnd_models + mi)
                   return zid;
                zid++;           // otherwise increment and continue seeking
            }
        }

        bl_err(BL_ERR_BADARG, "bl_iid(): bad model pointer");
        return 0xFFFF;
    }

//==============================================================================
// set element address
//==============================================================================

    int bl_set_element_addr(BL_u8 ele_idx, BL_u16 addr)
    {
        BL_element *pel = bl_element_pointer(ele_idx);

        if (! pel )
            return 1;             // error has been already reported!

        LOG(4,"bl_set_element_addr(%d): [%04x]",ele_idx, addr);
        pel->addr = addr;
        return 0;
    }

//===================================================================================================
// reset device composition tree
//===================================================================================================

    void bl_reset_tree(void)
    {
        LOG(3,"bl_reset_tree()");

        BL_u8 ne = bl_element_count();
        for (int el = 0; el < ne; el++)
        {
            LOG(3,"element #%d (of %d)", el, ne);

            bl_set_element_addr(el, 0x0000);     // reset element #el address

            BL_u8 nm = bl_model_count(el, 0);     // sig model count
            for (int mi = 0; mi < nm; mi++)
            {
                LOG(4,"BT SIG model(%d,%d,0) of #%d", el, mi, nm);

                BL_model *pmod = bl_model_pointer(el, mi, 0);
                bl_reset_model(el, pmod->mod_idx, 0);
            }

            nm = bl_model_count(el, 1);     // sig model count
            for (int mi = 0; mi < nm; mi++)
            {
                LOG(4,"vendor model(%d,%d,1) of #%d\n", el, mi, nm);

                BL_model *pmod = bl_model_pointer(el, mi, 1);
                bl_reset_model(el, pmod->mod_idx, 0);
            }
        }
    }

//===================================================================================================
// get element address of this node
//===================================================================================================

    BL_u16 bl_get_element_addr(BL_u8 ele_idx)
    {
        BL_element *pel = bl_element_pointer(ele_idx);

        if (! pel )
            return 1;             // error has been already reported!

//      bl_trc('i',2,"bl_get_element_addr(%d): [%04x]\n",ele_idx, pel->addr);

        return pel->addr;
    }

//==============================================================================
// get primary address (address of primary element)
//==============================================================================
/*
    BL_u16 bl_primary_addr(void)
    {
            return primary_addr;
    }
*/
//==============================================================================
// source and destination address of mesh message
//==============================================================================

    FL_addr bl_src(BL_ctx *ctx)         // source address of mesh message
    {
        return ctx->addr;               // source address
    }

    FL_addr bl_dst(BL_ctx *ctx)         // destination address of mesh message
    {
        return ctx->recv_dst;           // destination address
    }

//==============================================================================
// element address of model
//==============================================================================

    FL_addr bl_me(BL_model *pmod)
    {
        BL_element *pel = bl_element_pointer(pmod->elem_idx);
        return pel->addr;
    }

//==============================================================================
// provisioning link has been opened
//==============================================================================
/*
    static void link_open(bt_mesh_prov_bearer_t bearer)
    {
	LOG(2, ">>> provisioning link has been opened\n");
        bl_special_notify(BL_NOTIFY_ATTENTION_ON);
    }
*/
//==============================================================================
// provisioning link has been closed
//==============================================================================
/*
    static void link_close(bt_mesh_prov_bearer_t bearer)
    {
	    	bl_trc('i', 2, ">>> provisioning link has been closed\n");
        bl_special_notify(BL_NOTIFY_ATTENTION_OFF);
    }
*/
//==============================================================================
// provisioning complete
//==============================================================================
/*
    static void prov_complete_worker(struct k_work *work)
    {
        bl_trc('i',3,"prov_complete_worker(): notify kernel\n");
        bl_special_notify(BL_NOTIFY_PROVISIONED);
    }

    K_WORK_DEFINE(prov_complete_work, prov_complete_worker);

    static void prov_complete(BL_u16 net_idx, BL_u16 addr)
    {
			  bl_trc('i',2, ">>> provisioning complete (net_idx 0x%04x @ 0x%04x)\n",
                   net_idx, addr);
	    	primary_addr = addr;
	    	primary_net_idx = net_idx;
        k_work_submit(&prov_complete_work);
    }
*/
//==============================================================================
// node reset
//==============================================================================
/*
// ??????????????????????????????????????????????????????????????????
#include "kernsend.h"
// ??????????????????????????????????????????????????????????????????

    static void node_reset_worker(struct k_work *work)
    {
        if ( bl_trc('i',3,"") )
            printk("node_reset_worker(): notify kernel\n");

        primary_addr = BL_VOID_ADDR;
        bl_special_notify(BL_NOTIFY_NODERESET);

        if ( bl_trc('i',3,"") )
            printk("node_reset_worker(): enable provisioning\n");

        bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    }

    K_WORK_DEFINE(node_reset_work, node_reset_worker);	// define worker

    static void prov_reset(void)
    {
        bl_trc('i', 3, ">>> node resetted (unprovisioned)\n");
        k_work_submit(&node_reset_work);
    }

    void bl_node_reset(void)   // public function
    {
			  bl_trc('i', 2, "bl_node_reset() ...");
        bt_mesh_reset();      // actually reset mesh
        prov_reset();         // notify higher levels & advertise again
    }
*/
//==============================================================================
// OOB provisioning (if supported)
//==============================================================================
/*
#define IS_OOB_PROVISIONING 0		// do not support OOB provisioning
#if (IS_OOB_PROVISIONING)

		static int output_number(bt_mesh_output_action_t action, u32_t number)
		{
			printk("OOB Number %06u\n", number);
			return 0;
		}

		static int output_string(const char *str)
		{
			printk("OOB String %s\n", str);
			return 0;
		}

#endif // (IS_OOB_PROVISIONING)
*/
//==============================================================================
// Disable OOB security for SiLabs Android app
//==============================================================================
/*
	  static BL_u8 dev_uuid[16] = { 0xdd, 0xdd };

	  static const struct bt_mesh_prov prov =
    {
        .uuid = dev_uuid,

		    #ifdef OOB_AUTH_ENABLE
            .output_size = 6,
      			.output_actions = BT_MESH_DISPLAY_NUMBER | BT_MESH_DISPLAY_STRING,
      			.output_number = output_number,
      			.output_string = output_string,
        #endif

        .link_open = link_open,
        .link_close = link_close,
        .complete = prov_complete,
        .reset = prov_reset,
    };
*/
//==============================================================================
// Bluetooth ready callback: to init mesh after Bluetooth ready
//==============================================================================
/*
  	static void bl_bt_ready(int err)                          // Bluetooth ready
  	{
        struct bt_le_oob oob;

		    if (err)
        {
            bl_err(err, "Bluetooth initializing failed!");
			      return;
        }

            // otherwise Bluetooth is successfuly initialized

        bl_trc('i', 3, ANSI_B "[===== Bluetooth initialized! =====]\n" ANSI_0);

            // next step is to init mesh

		    err = bt_mesh_init(&prov, m_pcomp);
        if (err)
        {
            bl_err(err, "mesh initializing failed!");
            return;
        }

            // load settings from persistent memory

		    if (IS_ENABLED(CONFIG_SETTINGS))
        {
		        bl_trc('i', 2, "bl_bt_ready(): going to load settings\n");
            settings_load();
		        bl_trc('i', 2, "bl_bt_ready(): settings loaded\n");
        }

            // use identity address as device UUID

        if (bt_le_oob_get_local(BT_ID_DEFAULT, &oob))
            bl_err(BL_ERR_FAILED, "OOB: identity address unavailable!");
        else
            memcpy(dev_uuid, oob.addr.a.val, 6);

            // activate mesh provision advertising. note that Zephyr's
            // bt_mesh_prov_enable() (in adv.c) has been overlayed !!!

//      bl_mesh_prov_enable(BT_MESH_PROV_GATT | BT_MESH_PROV_ADV);
        bt_mesh_prov_enable(BT_MESH_PROV_GATT | BT_MESH_PROV_ADV);

            // now mesh is successfully initialized

        bl_trc('i', 3, ANSI_B "[===== Mesh initialized! =====]\n" ANSI_0);
    }

*/
//==============================================================================
// mesh init
//==============================================================================
/*
    int bl_mesh_init(const BL_comp *pcomp)
    {
        bl_trc('i', 3, "bl_mesh_init(0x%lx)\n", (long)pcomp);
        m_pcomp = pcomp;								// store temporarily

            // we set now a restore tic! with every setting restored the tic
            // is updated, after BT init we want to be sure that after resume
            // every setting is completely restored!

        bl_trc('i', 2, "=-> set a tic before enabling Bluetooth\n");
        bl_tic();     // set tick time

        int err = bt_enable(bl_bt_ready);
        if (err)
            bl_err(err, "Bluetooth/mesh init failed!");

            // now make sure that all setings are restored by waiting 200ms
            // longer than the last setting restore

        bl_trc('i', 4, "--> wait at least 200ms after last setting restored ...\n");

        while ( bl_toc() < 200 )  // bl_toc(): elapsed time since last bl_tic()
            bl_sleep(50);

        bl_trc('i', 2, "=-> all settings are now restored!\n");


            // next two lines seem a little weird. they are needed since after
            // power off/on mesh sets up its own scan callback, which we need
            // to stop and to setup again OUR scan callback. these lines are a
            // bit dangerous, since when they were in bl_enocean_register() they
            // caused a re-boot hang-up. at this point they seem to work proper-
            // ly, probably because mesh traffic is not yet up and running here!

//printk("==> bl_mesh_scan_disable() ...\n");
//      bl_mesh_scan_disable();
//      bl_mesh_scan_enable();
//printk("==> bl_mesh_scan_enable() ... done!\n");

        if (bl_is_provisioned())
            bl_off_on_timeout(true);  // activate off/on timeout

        return err;
    }
*/
//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
