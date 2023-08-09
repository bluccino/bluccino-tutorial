//==============================================================================
// bl_wl.c - onoff-app based Bluetooth mesh wireless core
//==============================================================================

  #include "bl_deco.c"                 // Bluccino log decoration

  #include <zephyr/settings/settings.h>
  #include <zephyr/sys/byteorder.h>
  #include <zephyr/bluetooth/bluetooth.h>
  #include <zephyr/bluetooth/conn.h>
  #include <zephyr/bluetooth/l2cap.h>
  #include <zephyr/bluetooth/hci.h>
  #include <zephyr/bluetooth/mesh.h>
  #include <stdio.h>
  #include <stdlib.h>

  #include "bluccino.h"
  #include "bl_deco.h"                 // log time stamp decoration
  #include "bl_gpio.h"                 // GPIO shorthands
  #include "bl_gonoff.h"               // generic on/off model
  #include "bl_hw.h"                   // hardware core
  #include "bl_wl.h"                   // wireless core

  #define PMI  bl_wl                   // public module interface

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO                     "bl_wl:"

  #define LOG                     LOG_CORE
  #define LOGO(lvl,col,o,val)     LOGO_CORE(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_CORE(lvl,col,o,val)

//==============================================================================
// let's go ...
//==============================================================================

/* Model Operation Codes */
#define BT_MESH_MODEL_OP_GEN_ONOFF_GET		BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET		BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK	BT_MESH_MODEL_OP_2(0x82, 0x03)
#define BT_MESH_MODEL_OP_GEN_ONOFF_STATUS	BT_MESH_MODEL_OP_2(0x82, 0x04)

static int gen_onoff_set(struct bt_mesh_model *model,
			 struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf);

static int gen_onoff_set_unack(struct bt_mesh_model *model,
			       struct bt_mesh_msg_ctx *ctx,
			       struct net_buf_simple *buf);

static int gen_onoff_get(struct bt_mesh_model *model,
			 struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf);

static int gen_onoff_status(struct bt_mesh_model *model,
			    struct bt_mesh_msg_ctx *ctx,
			    struct net_buf_simple *buf);

/*
 * Client Configuration Declaration
 */

static struct bt_mesh_cfg_cli cfg_cli = {
};

/*
 * Health Server Declaration
 */

static struct bt_mesh_health_srv health_srv = {
};

/*
 * Publication Declarations
 *
 * The publication messages are initialized to the
 * the size of the opcode + content
 *
 * For publication, the message must be in static or global as
 * it is re-transmitted several times. This occurs
 * after the function that called bt_mesh_model_publish() has
 * exited and the stack is no longer valid.
 *
 * Note that the additional 4 bytes for the AppMIC is not needed
 * because it is added to a stack variable at the time a
 * transmission occurs.
 *
 */

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

BT_MESH_MODEL_PUB_DEFINE(gen_onoff_pub_srv, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(gen_onoff_pub_cli, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(gen_onoff_pub_srv_s_0, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(gen_onoff_pub_cli_s_0, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(gen_onoff_pub_srv_s_1, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(gen_onoff_pub_cli_s_1, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(gen_onoff_pub_srv_s_2, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(gen_onoff_pub_cli_s_2, NULL, 2 + 2);

/*
 * Models in an element must have unique op codes.
 *
 * The mesh stack dispatches a message to the first model in an element
 * that is also bound to an app key and supports the op code in the
 * received message.
 *
 */

/*
 * OnOff Model Server Op Dispatch Table
 *
 */

static const struct bt_mesh_model_op gen_onoff_srv_op[] = {
	{ BT_MESH_MODEL_OP_GEN_ONOFF_GET,       BT_MESH_LEN_EXACT(0), gen_onoff_get },
	{ BT_MESH_MODEL_OP_GEN_ONOFF_SET,       BT_MESH_LEN_EXACT(2), gen_onoff_set },
	{ BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK, BT_MESH_LEN_EXACT(2), gen_onoff_set_unack },
	BT_MESH_MODEL_OP_END,
};

/*
 * OnOff Model Client Op Dispatch Table
 */

static const struct bt_mesh_model_op gen_onoff_cli_op[] = {
	{ BT_MESH_MODEL_OP_GEN_ONOFF_STATUS, BT_MESH_LEN_EXACT(1), gen_onoff_status },
	BT_MESH_MODEL_OP_END,
};

struct onoff_state {
  int id;
	uint8_t current;
	uint8_t previous;
	uint8_t led_gpio_pin;
	const struct device *led_device;
};

/*
 * Declare and Initialize Element Contexts
 * Change to select different GPIO output pins
 */

static struct onoff_state onoff_state[] = {
	{ .id=1, .led_gpio_pin = DT_GPIO_PIN(DT_ALIAS(led0), gpios) },
	{ .id=2, .led_gpio_pin = DT_GPIO_PIN(DT_ALIAS(led1), gpios) },
	{ .id=3, .led_gpio_pin = DT_GPIO_PIN(DT_ALIAS(led2), gpios) },
	{ .id=4, .led_gpio_pin = DT_GPIO_PIN(DT_ALIAS(led3), gpios) },
};

/*
 *
 * Element Model Declarations
 *
 * Element 0 Root Models
 */

static struct bt_mesh_model root_models[] = {
	BT_MESH_MODEL_CFG_SRV,
	BT_MESH_MODEL_CFG_CLI(&cfg_cli),
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_srv_op,
		      &gen_onoff_pub_srv, &onoff_state[0]),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, gen_onoff_cli_op,
		      &gen_onoff_pub_cli, &onoff_state[0]),
};

/*
 * Element 1 Models
 */

static struct bt_mesh_model secondary_0_models[] = {
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_srv_op,
		      &gen_onoff_pub_srv_s_0, &onoff_state[1]),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, gen_onoff_cli_op,
		      &gen_onoff_pub_cli_s_0, &onoff_state[1]),
};

/*
 * Element 2 Models
 */

static struct bt_mesh_model secondary_1_models[] = {
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_srv_op,
		      &gen_onoff_pub_srv_s_1, &onoff_state[2]),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, gen_onoff_cli_op,
		      &gen_onoff_pub_cli_s_1, &onoff_state[2]),
};

/*
 * Element 3 Models
 */

static struct bt_mesh_model secondary_2_models[] = {
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_srv_op,
		      &gen_onoff_pub_srv_s_2, &onoff_state[3]),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, gen_onoff_cli_op,
		      &gen_onoff_pub_cli_s_2, &onoff_state[3]),
};

/*
 * Button to Client Model Assignments
 */

struct bt_mesh_model *mod_cli_sw[] = {
		&root_models[4],
		&secondary_0_models[1],
		&secondary_1_models[1],
		&secondary_2_models[1],
};

/*
 * LED to Server Model Assigmnents
 */

struct bt_mesh_model *mod_srv_sw[] = {
		&root_models[3],
		&secondary_0_models[0],
		&secondary_1_models[0],
		&secondary_2_models[0],
};

//==============================================================================
// Root and Secondary Element Declarations
//==============================================================================

	static struct bt_mesh_elem elements[] = {
		BT_MESH_ELEM(0, root_models, BT_MESH_MODEL_NONE),
		BT_MESH_ELEM(0, secondary_0_models, BT_MESH_MODEL_NONE),
		BT_MESH_ELEM(0, secondary_1_models, BT_MESH_MODEL_NONE),
		BT_MESH_ELEM(0, secondary_2_models, BT_MESH_MODEL_NONE),
	};

	static const struct bt_mesh_comp comp = {
		.cid = BT_COMP_ID_LF,
		.elem = elements,
		.elem_count = ARRAY_SIZE(elements),
	};

//==============================================================================
// Bluetooth Mesh global variables
//==============================================================================

	static uint8_t trans_id;
	static uint16_t primary_addr;
	static uint16_t primary_net_idx;

//==============================================================================
// Generic OnOff Model Server Message Handlers
// Mesh Model Specification 3.1.1
//==============================================================================

	static int gen_onoff_get(struct bt_mesh_model *model,
				 struct bt_mesh_msg_ctx *ctx,
				 struct net_buf_simple *buf)
	{
		NET_BUF_SIMPLE_DEFINE(msg, 2 + 1 + 4);
		struct onoff_state *onoff_state = model->user_data;

	  LOG(5,BL_Y "addr 0x%04x onoff 0x%02x",
		      bt_mesh_model_elem(model)->addr, onoff_state->current);

		bt_mesh_model_msg_init(&msg, BT_MESH_MODEL_OP_GEN_ONOFF_STATUS);
		net_buf_simple_add_u8(&msg, onoff_state->current);

		if (bt_mesh_model_send(model, ctx, &msg, NULL, NULL)) {
			printk("Unable to send On Off Status response\n");
		}

		return 0;
	}

//==============================================================================
// notify app level about receive of GOOSET or GOO LET
//==============================================================================

  static int goo_notify(struct onoff_state *p, bool acked)
  {
    static BL_goo goo[4];              // 4 generic on/off servers

    BL_ms now = bl_ms();

      // do we have a static BL_goo structure?

    int idx = p->id - 1;
    if (idx < 0 || idx >= BL_LEN(goo))
      return bl_err(-1,"goo_notify: model @ix out of range");

        // fill BL_goo structure

    BL_goo *g = goo + idx;
    g->trans.basis = p->previous;
    g->delay = 0;
    g->trans.tt = 0;
    g->trans.target = p->current;
    g->trans.begin = now;
    g->remain = 0;
    g->tid = 0;                        // don't care about TID
    g->acked = acked;                  // was it an acknowledged message

      // submit message

    return _bl_post((PMI), _GOOSRV_STS_ix_BL_goo_sts, p->id,goo+idx,p->current);
  }

//==============================================================================
// GOOLET or GOOSET server message handler
//==============================================================================

static int gen_onoff_let_or_set(struct bt_mesh_model *model,
			       struct bt_mesh_msg_ctx *ctx,
			       struct net_buf_simple *buf, bool acked)
{
	struct net_buf_simple *msg = model->pub->msg;
	struct onoff_state *p = model->user_data;                           //@@@4.8
	int err;

	p->current = net_buf_simple_pull_u8(buf);                           //@@@4.8
/*
	if (bl_dbg(4))
  	printk(BL_G"addr 0x%02x state 0x%02x\n"BL_0,
	       bt_mesh_model_elem(model)->addr, p->current);                //@@@4.8
*/

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//   - dont directly set LED                                          //@@@4.6
//	gpio_pin_set(p->led_device, p->led_gpio_pin,
//		     p->current);
// int id = p->led_gpio_pin - 16;
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	/*
	 * If a server has a publish address, it is required to
	 * publish status on a state change
	 *
	 * See Mesh Profile Specification 3.7.6.1.2
	 *
	 * Only publish if there is an assigned address
	 */

  bool provision = (model->pub->addr != BT_MESH_ADDR_UNASSIGNED);

	if (p->previous != p->current && provision)                         //@@@4.8
	{
		BL_LOG(5,"publish last 0x%02x cur 0x%02x\n", p->previous, p->current);                                  //@@@4.8

      // init publisher and add current onoff status

		bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_GEN_ONOFF_STATUS);
		net_buf_simple_add_u8(msg, p->current);                           //@@@4.8

		  // publish mesh message and report potential error

		err = bt_mesh_model_publish(model);
		bl_err(err,"bt_mesh_model_publish");
	}

    // log the receive of GOOSET or GOOLET message and notify app level

  if (acked)
	  LOG(4,BL_G "rcv [GOOSRV:SET @%d,%d]",p->id,p->current);
  else
	  LOG(4,BL_G "rcv [GOOSRV:LET @%d,%d]",p->id,p->current);

  goo_notify(p, acked);                  // notify application

    // finally update previous

	p->previous = p->current;
	return 0;
}

//==============================================================================
// GOOSET_UNACK server message handler
//==============================================================================

static int gen_onoff_set_unack(struct bt_mesh_model *model,
			       struct bt_mesh_msg_ctx *ctx,
			       struct net_buf_simple *buf)
{
  return gen_onoff_let_or_set(model,ctx,buf,false);  // false means: unack'ed
}

//==============================================================================
// GOOSET server message handler
//==============================================================================

static int gen_onoff_set(struct bt_mesh_model *model,
			 struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	gen_onoff_let_or_set(model, ctx, buf, true);   // true means: ack'ed
  LOG(5,"rcv [GOOSRV:SET @ix,onoff]");
	gen_onoff_get(model, ctx, buf);

	return 0;
}

//==============================================================================
// GOO server message handler
//==============================================================================

static int gen_onoff_status(struct bt_mesh_model *model,
			    struct bt_mesh_msg_ctx *ctx,
			    struct net_buf_simple *buf)
{
	uint8_t	state;

	state = net_buf_simple_pull_u8(buf);

	LOG(5,BL_Y "node 0x%04x OnOff status from 0x%04x with state 0x%02x",
	      bt_mesh_model_elem(model)->addr, ctx->addr, state);

	return 0;
}

static int output_number(bt_mesh_output_action_t action, uint32_t number)
{
	LOG(1,BL_R "OOB Number %06u", number);
	return 0;
}

static int output_string(const char *str)
{
	LOG(1,BL_R"OOB String %s", str);
	return 0;
}

static void prov_complete(uint16_t net_idx, uint16_t addr)
{
	LOG(4,BL_Y "provisioning complete (net:0x%04x, addr:0x%04x)", net_idx, addr);
	primary_addr = addr;
	primary_net_idx = net_idx;

	_bl_post((PMI), _MESH_PRV_0_0_sts, 0,NULL,1);  // (BL_WL)<-[#MESH:PRV 1]
}

static void prov_reset(void)
{
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	_bl_post((PMI), _MESH_PRV_0_0_sts, 0,NULL,0);  // (BL_WL)<-[#MESH:PRV 0]
}

static uint8_t dev_uuid[16] = { 0xdd, 0xdd };

//==============================================================================
// callback: provisioning link open/close                             //@@@3.3
//==============================================================================

  static void link_open(bt_mesh_prov_bearer_t bearer)
  {
  	_bl_post((PMI), _MESH_ATT_0_0_sts, 0,NULL,1);  // (BL_WL)<-[#MESH:ATT 1]
  }

  static void link_close(bt_mesh_prov_bearer_t bearer)
  {
  	_bl_post((PMI), _MESH_ATT_0_0_sts, 0,NULL,0);  // (BL_WL)<-[#MESH:ATT 0]
  }

//==============================================================================
// table: provisioning table
//==============================================================================

/* Disable OOB security for SILabs Android app */

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
#if 1
	.output_size = 6,
	.output_actions = (BT_MESH_DISPLAY_NUMBER | BT_MESH_DISPLAY_STRING),
	.output_number = output_number,
	.output_string = output_string,
#else
	.output_size = 0,
	.output_actions = 0,
	.output_number = 0,
#endif
  .link_open = link_open,                                             //@@@3.1
  .link_close = link_close,                                           //@@@3.2
	.complete = prov_complete,
	.reset = prov_reset,
};

/*
 * Bluetooth Ready Callback
 */

static void bt_ready(int err)
{
	struct bt_le_oob oob;

	if (err)
	{
		bl_err(err,"init Bluetoothfailed");
		return;
	}

	LOG(3,"init Bluetooth complete");

	err = bt_mesh_init(&prov, &comp);
	if (err)
	{
	  bl_err(err,"init mesh failed");
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS))
		settings_load();

	  // Use identity address as device UUID

	if (bt_le_oob_get_local(BT_ID_DEFAULT, &oob))
		bl_err(-1,"identity address unavailable");
	else
		memcpy(dev_uuid, oob.addr.a.val, 6);

	bt_mesh_prov_enable(BT_MESH_PROV_GATT | BT_MESH_PROV_ADV);

	LOG(3,BL_C "init mesh complete");
}

//==============================================================================
// publishing worker task
//==============================================================================

  static int pub_id;                   // publish ID
  static uint8_t pub_val;              // publish value
  static uint16_t pub_op;              // mesh model opcode

  static void pub_worker(struct k_work *work)
  {
    struct bt_mesh_model *mod_cli, *mod_srv;
		struct bt_mesh_model_pub *pub_cli, *pub_srv;
		uint8_t idx = pub_id-1;

    mod_cli = mod_cli_sw[idx];
    pub_cli = mod_cli->pub;

    mod_srv = mod_srv_sw[idx];
    pub_srv = mod_srv->pub;

		/* If unprovisioned, just call the set function.
		 * The intent is to have switch-like behavior
		 * prior to provisioning. Once provisioned,
		 * the button and its corresponding led are no longer
		 * associated and act independently. So, if a button is to
		 * control its associated led after provisioning, the button
		 * must be configured to either publish to the led's unicast
		 * address or a group to which the led is subscribed.
		 */

    if (primary_addr == BT_MESH_ADDR_UNASSIGNED)
    {
            NET_BUF_SIMPLE_DEFINE(msg, 1);
            struct bt_mesh_msg_ctx ctx = { .addr = idx + primary_addr};

              // This is a dummy message sufficient for the led server

            net_buf_simple_add_u8(&msg, pub_val);
            (void)gen_onoff_set_unack(mod_srv, &ctx, &msg);
            return;
    }

    if (pub_cli->addr == BT_MESH_ADDR_UNASSIGNED)
            return;

    BL_txt op = (pub_op == BT_MESH_MODEL_OP_GEN_ONOFF_SET) ? "SET" : "LET";
    LOG(4,BL_G "pub [GOOCLI:%s @%d,%d]", op, pub_id,pub_val);

    LOG(5,"publish to 0x%04x onoff 0x%04x idx 0x%04x",
               pub_cli->addr, pub_val, idx);
    bt_mesh_model_msg_init(pub_cli->msg,pub_op);
    net_buf_simple_add_u8(pub_cli->msg, pub_val);
    net_buf_simple_add_u8(pub_cli->msg, trans_id++);

    int err = bt_mesh_model_publish(mod_cli);
    bl_err(err,"bt_mesh_model_publish");
  }

  K_WORK_DEFINE(pub_work, pub_worker);      // assign work with workhorse

//==============================================================================
// worker: publish GOO message
//==============================================================================

  static int pub(BL_ob *o,int val)   // here's how we hook in ...
  {
    // id = 1..4, since it adresses elements 1..4
    // button indices are 0..3, thus swnum = bl_ix(o) - 1

    if (bl_ix(o) < 1 || bl_ix(o) > 4)        // ignore IDs not in 1..4
      return -2;                       // ID OUT OF RANGE

    pub_id = bl_ix(o);                    // store client ID (switch number)
    pub_val = (uint8_t)val;            // store publish value

    switch (o->op)
    {
      case SET_:
        pub_op = BT_MESH_MODEL_OP_GEN_ONOFF_SET;
        break;
      case LET_:
        pub_op = BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK;
        break;
      default:
        return -1;                     // bad message opcode
    }

      // finally we submit a button press work, which refreshes the
      // onoff state with the new value and publishes the state

    LOGO(3,"publish:",o,val);
    k_work_submit(&pub_work);          // submit button work (that's all)
    return 0;                          // OK
  }

//==============================================================================
// handler: [SYS:INIT (cb)]
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
  	k_work_init(&pub_work, pub_worker);

      // init Bluetooth subsystem

    int err = bt_enable(bt_ready);
    bl_err(err,"init Bluetooth failed");

    return err;
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (D) := bl_down;  (U) := bl_up;
//                  +--------------------+
//                  |       bl_wl        | wireless core module
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (D)->     INIT ->|        (cb)        | init module, store (cb) callback
//                  +--------------------+
//                  |       #MESH:       | MESH output interface
// (U)<-      PRV <-|       onoff        | provision on/off
// (U)<-      ATT <-|       onoff        | attention on/off
//                  +--------------------+
//                  |      GOOCLI:       | GOOCLI input interface
// (D)->      SET ->| @ix,<BL_goo>,onoff | publish ack'ed generic on/off SET
// (D)->      LET ->| @ix,<BL_goo>,onoff | publish unack'ed generic on/off SET
// (D)->      GET ->|        @ix         | request GOO server status
//                  |....................|
//                  |      #GOOSRV:      | GOOSRV output interface
// (U)<-      STS <-|  @ix,<BL_goo>,sts  | notify GOO server status
//                  +--------------------+
//                  |        GET:        | GET input interface
// (!)->      ATT ->|                    | gett node's attention status
// (!)->      PRV ->|                    | gett node's provision status
//                  +--------------------+
//
//==============================================================================

  int bl_wl(BL_ob *o, int val)         // public interface
  {
    static bool att = false;           // attention
    static bool prv = false;           // provision

    static BL_oval D = bl_deco;        // decorate (coloring log time stamps)
    static BL_oval U = bl_up;          // out goes to bl_up by default

    switch (bl_id(o))                  // dispatch message ID
    {
      case SYS_INIT_0_cb_0:            // [SYS:INIT <out>]
        LOG(3,BL_C "init tiny wireless core ...");
        U = bl_cb(o,(U),WHO"(U)");     // store output callback
        bl_install((D));               // install bl_deco in top gear
        return sys_init(o,val);        // delegate to sys_init() handler

      case _MESH_PRV_0_0_sts:          // [#SET:PRV val]  (provision)
        prv = val;
        return bl_out(o,val,(U));      // output to subscriber

      case _MESH_ATT_0_0_sts:          // [#SET:ATT val]  (attention)
        att = val;
        return bl_out(o,val,(U));      // output to subscriber

      case GOOCLI_LET_ix_BL_goo_onoff: // pub [GOOCLI:LET @ix,<BL_goo>,onoff]
      case GOOCLI_SET_ix_BL_goo_onoff: // pub [GOOCLI:SET @ix,<BL_goo>,onoff]
      case GOOCLI_GET_ix_0_0:          // pub [GOOCLI:GET @ix]
        return pub(o,val);             // pub [GOOCLI:LET/SET/GET] message

      case _GOOSRV_STS_ix_BL_goo_sts:  // [#GOOSRV:STS @ix,<BL_goo>,sts]
        return bl_out(o,val,(U));      // output [GOOSRV:STS ...] to up gear

      case NVM_SUPPORT_0_0_0:            // [NVM:AVAIL]
			  LOG(5,BL_B "NVM not supported by bl_wl");
			  return -1;                     // no, NVM functionality not available

      case GET_ATT_0_0_0:
        return att;                    // return attention state

      case GET_PRV_0_0_0:
        return prv;                    // return provision state

      default:
        return -1;                     // bad input
    }
  }
