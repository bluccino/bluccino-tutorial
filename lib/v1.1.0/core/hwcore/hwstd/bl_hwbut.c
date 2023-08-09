//==============================================================================
// bl_hwbut.c
// Bluccino HW core supporting basic functions for button & LED
//
// Created by Hugo Pristauz on 2022-Feb-18
// Copyright © 2022 Bluccino. All rights reserved.
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
// config defaults
//==============================================================================

  #ifndef CFG_DEBUG_DEBOUNCING
    #define CFG_DEBUG_DEBOUNCING 0     // no debouncing debug by default
  #endif

//==============================================================================
// Get button configuration from the devicetree sw0 alias. This is mandatory.
//==============================================================================

  static BL_word mask = 0xFFFF;        // all button events enabled

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

  #define MS_COOLDOWN 50               // 50 ms debounce cool time

  typedef struct BL_button             // data structure for button processing
          {
            int ix;                    // button index (1..4)
            bool state;                // button state
            bool toggle;               // toggle switch state
            BL_ms time;                // button press time for click detect.
            bool hold;                 // button is in a HOLD phase
            BL_byte clicks;            // number of button clicks
            BL_byte pulses;            // number of second-pulses
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
            .hold = 0,                 \
            .clicks = 0,               \
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

  static int T_hold = 400;             // click grace time (ms) for hold
  static int T_multi = 400;            // click grace time for multi click

//==============================================================================
// some helpers
//==============================================================================

  static void reset(BL_button *p)
  {
    p->time = p->pulses = p->clicks = p->hold = 0;
  }

//==============================================================================
// click modul (click/hold detection)
// - usage: val = click(o,val)   // val = 0:click, 1:hold
// - supports 4 buttons (@ix:1..4) and the default button (@ix:0)
//==============================================================================
//
// (B) := (bl_hwbut);
//
//                  +--------------------+
//                  |        click       | module click
//                  +--------------------+
//                  |       BUTTON:      | BUTTON interface
// (B)<-    CLICK <-|     @ix,clicks     | button click (click time < ms)
// (B)<-     HOLD <-|       @ix,ms       | button hold (hold time >= ms)
//                  |....................|
//                  |      #BUTTON:      | internal BUTTON interface
// (#)->    PRESS ->|       @ix,0        | receive button press event
// (#)->  RELEASE ->|       @ix,ms       | receive button release event
//                  +--------------------+
//
//==============================================================================

  static int click(BL_ob *o, int val)
  {
    //static BL_oval B = bl_hwbut;

    int ix = bl_ix(o);                 // short hand for @ix
    BL_ms now = bl_ms();

    if (ix < 0 || ix > NBUT)
      return -1;                       // bad args

    BL_button *p = button + (ix-1);

    switch (bl_id(o))
    {
      case BL_ID(_BUTTON,PRESS_):
      {
        if (p->time)
        {
          LOG(5,BL_M "time: %d ms", (int)(now - p->time));
        }

        p->time = now;                 // store button press time stamp

        if ( p->clicks == 0 && mask & BL_CLICK)
        {
          LOG(5,BL_B "button click (begin)");
          _bl_pmi(_BUTTON,CLICK_, ix, NULL,p->clicks);
        }

        p->clicks++;                   // count button clicks
        LOG(5,BL_G "clicks:%d", p->clicks);
        return 0;
      }

      case BL_ID(_BUTTON,RELEASE_):
        if (p->time)                   // has time been set before?
        {
          if (now >= p->time + T_hold)     // grace time exceeded => hold ?
          {
            BL_ms dt = now-p->time;  // hold time
            if (p->clicks <= 1)
            {
              if (mask & BL_HOLD)
              {
                LOG(4,BL_B "button hold event");
                _bl_pmi(_BUTTON,HOLD_, ix,NULL,(int)dt);
                _bl_pmi(_BUTTON,TOCK_, ix,NULL,0);
              }
            }
          }
        }
        return 0;

      default:
        return -1;                     // bad arg
    }
  }

//==============================================================================
// button worker - posts [BUTTON:PRESS @ix 1] or [BUTTON:RELEASE @ix 0]
// - ISR routine sets `id` (button ID) and `debounced` (debounced button state)
//   as global arguments and submits a button_work
// - this invokes the button_worker thread which picks the two args in order to
//   process [BUTTON:PRESS @ix 1] or [BUTTON:RELEASE @ix,0]
//==============================================================================

  static void state_change(BL_button *p, int val)
  {
    static BL_oval C = click;          // process CLICK/HOLD events

    LOG(5,BL_Y "button @%d: %d -> %d", p->ix, p->state,val);
    p->state = val;

      // post button state to module interface for output

    if (val)                           // [BUTTON:PRESS 0] event
		{
      if (mask & BL_PRESS)
        _bl_pmi(_BUTTON,PRESS_, p->ix,NULL,0);

      p->toggle = !p->toggle;
      if (mask & BL_SWITCH)
        _bl_pmi(_SWITCH,STS_, p->ix,NULL,p->toggle);

      bl_msg((C), _BUTTON,PRESS_, p->ix,NULL,0);
    }
    else                               // [BUTTON:RELEASE ms] event
    {
      int dt = (int)(bl_ms() - p->time);
      if (mask & BL_RELEASE)
        _bl_pmi(_BUTTON,RELEASE_, p->ix,NULL,dt);

      bl_msg((C), _BUTTON,RELEASE_, p->ix,NULL,dt);
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
    k_work_reschedule(&cooldown_work, K_MSEC(MS_COOLDOWN));
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
// worker: ticking
//==============================================================================

  static int sys_tick(BL_ob *o, int val)
  {
    BL_ms now = bl_ms();

    for (int i=0; i < NBUT; i++)
    {
      BL_button *p = button + i;
      int held = now - p->time;     // hold time

      if ( p->time )
      {
        if ( held >= T_hold && !p->hold && p->state)
        {
          p->hold = true;             // button @(i+1) entered HOLD state
	   		  if ((mask & BL_HOLD) && p->clicks <= 1)
				  {
            LOG(5,BL_Y "button hold (begin)");
            _bl_pmi(_BUTTON,HOLD_,p->ix,NULL,0);
            p->pulses++;
				  }
          else if ((mask & BL_CLICK) && p->clicks > 1)
          {
            LOG(4,BL_B "button click-hold event");
            _bl_pmi(_BUTTON,CLICK_, bl_ix(o),NULL,-(p->clicks));
            reset(p);
          }
        }
        else if ( held >= T_multi && !p->hold && !p->state && p->clicks)
        {
	  		  if (mask & BL_CLICK && p->clicks > 0)
				  {
            LOG(5,BL_B "button clicked %d times (end)",p->clicks);
            _bl_pmi(_BUTTON,CLICK_,p->ix,NULL,p->clicks);
			    }
          p->clicks = 0;             // clear button click counter
          // LOG(5,BL_G "clicks:%d", p->clicks);
        }

        if ( held > T_hold && p->state == 0)
          reset(p);  // clear press time & hold state
      }

      if (mask & BL_TOCK && p->hold && p->clicks <= 1)
      {
        int tocktime = 1000 * p->pulses;
        if ( now >= p->time + tocktime )
        {
          LOG(5,BL_B "button pulse %d ms",p->clicks);
          _bl_pmi(_BUTTON,TOCK_,p->ix,NULL,p->pulses);
          p->pulses++;
        }
      }
    }

    return 0;                      // OK
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
// (!)->     TICK ->|       @ix,cnt      | tick module
//                  +--------------------+
//                  |       BUTTON:      | BUTTON output interface
// (U)<-    PRESS <-|        @ix,0       | button press at time 0
// (U)<-  RELEASE <-|        @ix,ms      | button release after elapsed ms-time
// (U)<-    CLICK <-|        @ix,n       | number of button clicks
// (U)<-     HOLD <-|       @ix,ms       | button hold event at ms-time
// (U)<-     TOCK <-|       @ix,ms       | button tock event, every 1000 ms
// (!)->      CFG ->|        mask        | config button event mask
// (!)->       MS ->|         ms         | set click/hold discrimination time
//                  +--------------------+
//                  |       SWITCH:      | SWITCH interface
// (U)<-      STS <-|       @ix,sts      | emit status of toggle switch event
//                  +--------------------+
//                  |        SET:        | SET interface
// (D)->       MS ->|         ms         | set grace time for click/hold events
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

      case BL_ID(_SYS,TICK_):
      	return sys_tick(o,val);           // delegate to sys_tick() worker

      case BL_ID(_BUTTON,CFG_):
			  mask = (BL_word)val;              // store event mask
      	return 0;                         // OK

      case BL_ID(_BUTTON,MS_):            // config click/hold discrim. time
			  T_hold = T_multi = val;           // store grace time
      	return 0;                         // OK

      case _BL_ID(_BUTTON,PRESS_):
      case _BL_ID(_BUTTON,RELEASE_):
      case _BL_ID(_BUTTON,CLICK_):
      case _BL_ID(_BUTTON,HOLD_):
      case _BL_ID(_BUTTON,TOCK_):
        return bl_out(o,val,(U));         // post to output subscriber

      case _BL_ID(_SWITCH,STS_):
        return bl_out(o,val,(U));         // post to output subscriber

      default:
	      return -1;                        // bad input
    }
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
