//==============================================================================
// bl_hwbut.c
// Bluccino HW core supporting basic functions for button & LED
//
// Created by Hugo Pristauz on 2022-Feb-18
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================
// - tiny bl_hwbut bittpon river uses devicetree convenience API
// - can deal both with 1-button-boards (like Nordic 840 dongle) or 4-button-
//   boards (like Nordic 832/840 DKs)
// - debouncing
// - demonstration of user data access in ISR using CONTAINER_OF macro
//==============================================================================

  #include <assert.h>

  #include "bluccino.h"
  #include "bl_hw.h"
  #include "bl_hwbut.h"
  #include "bl_gpio.h"

//==============================================================================
// logging shorthands
//==============================================================================

  #define WHO "bl_hwbut"          // who is logging

  #define LOG                     LOG_BUTTON
  #define LOGO(lvl,col,o,val)     LOGO_BUTTON(lvl,col WHO ":",o,val)

  #define PMI  bl_hwbut           // public module interface
  BL_PMI(PMI)                     // define static inline function _bl_pmi()

//==============================================================================
// Define shorthands for button node IDs, and check if they are supported
//==============================================================================

  #define BUTTON1    DT_ALIAS(sw0)     // DT node ID for button @1
  #define BUTTON2    DT_ALIAS(sw1)     // DT node ID for button @2
  #define BUTTON3    DT_ALIAS(sw2)     // DT node ID for button @3
  #define BUTTON4    DT_ALIAS(sw3)     // DT node ID for button @4

  #if !DT_NODE_HAS_STATUS(BUTTON1, okay)
    #define BUTTON1_OK  0
  #else
    #define BUTTON1_OK  1
  #endif

  #if !DT_NODE_HAS_STATUS(BUTTON2, okay)
    #define BUTTON2_OK  0
  #else
    #define BUTTON2_OK  1
  #endif

  #if !DT_NODE_HAS_STATUS(BUTTON3, okay)
    #define BUTTON3_OK  0
  #else
    #define BUTTON3_OK  1
  #endif

  #if !DT_NODE_HAS_STATUS(BUTTON4, okay)
    #define BUTTON4_OK  0
  #else
    #define BUTTON4_OK  1
  #endif

  #define NBUT (BUTTON1_OK + BUTTON2_OK + BUTTON3_OK + BUTTON4_OK)

//==============================================================================
// Define a BL_button data structure per each button. We need:
//==============================================================================

  typedef struct BL_button             // data structure for button processing
          {
            int ix;                    // button index (1..4)
            bool state;                // button state
            bool toggle;               // toggle switch state
            BL_ms time;                // timestamp of press debouncing end
            const GP_ds ds;            // button pin device spec
            GP_ctx context;            // button context (to assign IRS handler)
          } BL_button;

    // initializer for BL_button variables

  #define BL_BUTTON(_ix,nid)           \
          {                            \
            .ix = _ix,                 \
            .state = 0,                \
            .toggle = 0,               \
            .time = 0,                 \
            .ds=GP_IO(nid,gpios,{0}),  \
          }

  BL_button button[NBUT] =
            {
              #if (BUTTON1_OK)
                BL_BUTTON(1,BUTTON1),   // button @1
              #endif
              #if (BUTTON2_OK)
                BL_BUTTON(2,BUTTON2),   // button @2
              #endif
              #if (BUTTON3_OK)
                BL_BUTTON(3,BUTTON3),   // button @3
              #endif
              #if (BUTTON4_OK)
                BL_BUTTON(4,BUTTON4),   // button @4
              #endif
            };

//==============================================================================
// button worker - posts [BUTTON:PRESS @ix 1] or [BUTTON:RELEASE @ix 0]
// - ISR routine sets `id` (button ID) and `debounced` (debounced button state)
//   as global arguments and submits a button_work
// - this invokes the button_worker thread which picks the two args in order to
//   process [BUTTON:PRESS @ix 1] or [BUTTON:RELEASE @ix,0]
//==============================================================================

  static void state_change(BL_button *p, int val)
  {
    LOG(5,BL_Y "button @%d: %d -> %d", p->ix, p->state,val);
    p->state = val;

      // post button state to module interface for output

    if (val)                           // [BUTTON:PRESS 0] event
		{
      p->time = bl_ms();
      _bl_pmi(_BUTTON,PRESS_, p->ix,NULL,0);

      p->toggle = !p->toggle;
      _bl_pmi(_SWITCH,STS_, p->ix,NULL,p->toggle);
    }
    else                               // [BUTTON:RELEASE ms] event
    {
      int dt = (int)(bl_ms() - p->time);
      _bl_pmi(_BUTTON,RELEASE_, p->ix,NULL,dt);
      p->time = 0;
    }
  }

//==============================================================================
// define delayablework package
//==============================================================================

  static void cooldown_expired(struct k_work *work)
  {
    for (int i=0; i < NBUT; i++)
    {
      BL_button *p = button + i;
      int val = gpio_pin_get_dt(&button[i].ds);

      if (val != p->state)
        state_change(p, val);
    }
  }

  K_WORK_DELAYABLE_DEFINE(cooldown_work, cooldown_expired);

//==============================================================================
// provide button ISR callback (button pressed/released)
//==============================================================================

  static void isr(GP_dev *dev, GP_ctx *ctx, GP_pins pins)
  {
    BL_button *p = CONTAINER_OF(ctx,BL_button,context);

    LOG(5,"button ISR, pin @%d (mask:0x%04x)",p->ix,pins);
    k_work_reschedule(&cooldown_work, K_MSEC(CFG_DEBOUNCE_TIME));
  }

//==============================================================================
// configure button
// - check whether device is ready
// - configure GPIO
//==============================================================================

  static int config(int ix)
  {
    assert(ix >= 1 && ix <= NBUT);
    BL_button *p = button + (ix-1);       // pointer to proper button structure

    if (!gp_ready(p->ds.port))
      return -ENODEV;

    LOG(5,"set up button @%d (%s, pin %d)", p->ix, p->ds.port->name, p->ds.pin);

    gp_pin_cfg(&p->ds, GPIO_INPUT);
    gp_int_cfg(&p->ds, GPIO_INT_EDGE_BOTH);
    gp_add_cb(&p->ds, &p->context, isr);

    return 0;
  }

//==============================================================================
// worker: init module
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    LOG(4,BL_B "init bl_hwbut ...");

    for (int i=1; i <= NBUT; i++)
      config(i);

    return 0;
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (!) := (<parent>);  (#) := (bl_hwbut);  (U) := (bl_up);  (D) := (bl_down);
//
//                  +--------------------+
//                  |      bl_hwbut      | module bl_hwbut
//                  +--------------------+
//                  |        SYS:        | SYS interface
// (!)->     INIT ->|       <out>        | init module, ignore <out> callback
//                  +--------------------+
//                  |       BUTTON:      | BUTTON output interface
// (U)<-    PRESS <-|        @ix,0       | button press at time 0
// (U)<-  RELEASE <-|        @ix,ms      | button release after elapsed ms-time
//                  +--------------------+
//                  |       SWITCH:      | SWITCH interface
// (U)<-      STS <-|       @ix,sts      | emit status of toggle switch event
//                  +--------------------+
//
//==============================================================================

  int bl_hwbut(BL_ob *o, int val)         // BUTTON core module interface
  {
    static BL_oval U = bl_up;             // to store output callback

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        U = bl_cb(o,(U),WHO"(U)");        // store output callback
      	return sys_init(o,val);           // delegate to sys_init() worker

      case _BL_ID(_BUTTON,PRESS_):
      case _BL_ID(_BUTTON,RELEASE_):
        return bl_out(o,val,(U));         // post to output subscriber

      case _BL_ID(_SWITCH,STS_):
        return bl_out(o,val,(U));         // post to output subscriber

      default:
	      return BL_VOID;                   // not dispatched
    }
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
