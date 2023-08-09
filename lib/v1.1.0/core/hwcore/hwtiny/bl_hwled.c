//==============================================================================
// hw_tiny/bl_hwled.c
// core LED functions
//
// Created by Hugo Pristauz on 2022-Feb-18
// Copyright © 2022 Bluccino. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_hw.h"
  #include "bl_gpio.h"
  #include "bl_hwled.h"

//==============================================================================
// logging shorthands and PMI definition
//==============================================================================

  #define WHO "bl_hwled:"          // who is logging

  #define LOG                     LOG_LED
  #define LOGO(lvl,col,o,val)     LOGO_LED(lvl,col WHO,o,val)

  #define PMI bl_hwled            // public module interface

//==============================================================================
// defines
//==============================================================================

  #ifdef ONE_LED_ONE_BUTTON_BOARD      // overrules everything !!!
    #undef  CFG_NUMBER_OF_LEDS
    #define CFG_NUMBER_OF_LEDS     1
  #endif

  #ifndef CFG_NUMBER_OF_LEDS
    #define CFG_NUMBER_OF_LEDS     4
  #endif

  #define NLEDS  CFG_NUMBER_OF_LEDS

    // define devicetree node IDs (NIDs) for LEDs

  #define NID_LED0          DT_ALIAS(led0)
  #define NID_LED1          DT_ALIAS(led1)
  #define NID_LED2          DT_ALIAS(led2)
  #define NID_LED3          DT_ALIAS(led3)

  #define N                  4                   // max number of LEDs

//==============================================================================
// locals
// - LEDs are represented by their device tree specs (struct gpio_dt_spec)
// - from device tree specs we store all references in the pointer array led[]
// - additionally we want to know the on/off state of each LED
// - all this data structures are statically initialized, but keep in mind that
//   during initializing we should check the ready state of each LED device
//==============================================================================

    // A build error on this line means your board is unsupported.

  static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(NID_LED0, gpios);
  static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(NID_LED1, gpios);
  static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(NID_LED2, gpios);
  static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(NID_LED3, gpios);

    // to index an LED device (spec) we keep the references in an array

  static const struct gpio_dt_spec *led[4] = {&led0,&led1,&led2,&led3};

    // additionally we want to now the on/off state of each LED

  static bool led_onoff[4] = {0,0,0,0};

//==============================================================================
// LED set  [SET:LED @ix onoff]  // @ix = 1..4
//==============================================================================

  static int led_set(BL_ob *o, int onoff)
  {
    int ix = bl_ix(o);                 // LED index, range 1..4
    if (ix == 0 || ix > NLEDS)
      return -1;                       // bad input

    if (1 <= ix && ix <= 4)
    {
       int i = ix - 1;                 // map range 1..4 to range 0..3
       led_onoff[i] = onoff;
		   return gpio_pin_set_dt(led[i],onoff);
    }
    else if (ix < 0)                   // set all 4 LEDs to given status
    {
       int err = 0;

       for (int i = 0; i < NLEDS; i++)
       {
         led_onoff[i] = onoff;
         err = err || gpio_pin_set_dt(led[i],onoff);
       }
       return err;
    }
    return -1;                         // bad input
  }

//==============================================================================
// LED toggle
//==============================================================================

  static int led_toggle(BL_ob *o,int val)
  {
    int ix = bl_ix(o);                 // LED index, range 1..4
    if (ix == 0 || ix > NLEDS)
      return -1;                       // bad args

    if (ix > 0)
      val = (led_onoff[ix-1] == 0);    // new LED value
    else
      val = !(led_onoff[0] || led_onoff[1] || led_onoff[2] || led_onoff[3]);

    int err = led_set(o,val);          // toggle LED state

    LOGO(4,BL_Y,o,val);                // log changed LED level
    return err;
  }

//==============================================================================
// helper: LED init (index range: 1..4)
//==============================================================================

  static int led_init(int ix)
  {
    int i = ix - 1;                    // map range 1..4 to range 0..3

    if (!device_is_ready(led[i]->port))
      return bl_err(1,bl_fmt("LED @%d not ready",ix));

    int err = gpio_pin_configure_dt(led[i], GPIO_OUTPUT_INACTIVE);
    return err;
  }

//==============================================================================
// handler: [SYS.INIT (cb)] system init
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    int err = 0;

  	  // LEDs configuration & setting

    LOG(4,BL_B "init %d LED%s",NLEDS, NLEDS==1?"":"s");

    for (int ix = 1; ix <= NLEDS; ix++)
       err = err || led_init(ix);

    return err;
  }

//==============================================================================
// public module interface
// - @ix = 0: status LED (same as @ix=1)
// - @ix = 1..4: LED @ix (same as @ix=1)
// - @ix < 0: set all 4 LEDs status to given value
//==============================================================================
//
// (!) := (<parent>);
//                  +--------------------+
//                  |        LED         |
//                  +--------------------+
//                  |        SYS:        | SYS interface
// (!)->     INIT ->|       <out>        | init module, ignore <out> callback
//                  +--------------------+
//                  |        LED:        | LED interface
// (!)->      SET ->|      @ix,onoff     | set LED's onoff state
// (!)->   TOGGLE ->|        @ix         | toggle LED's onoff state
//                  +--------------------+
//
//==============================================================================

  int bl_hwled(BL_ob *o, int val)      // public module interface
  {
    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
      	return sys_init(o,val);        // delegate to sys_init() worker

      case BL_ID(_LED,SET_):
      {
        BL_ob oo ={o->cl,o->op,1,NULL};// change @ix=0 -> @ix=1
        if (bl_ix(o))
          LOGO(4,"@",o,val);

        o = bl_ix(o) ? o : &oo;        // if (bl_ix(o)==0) re-map o to &oo
	      return led_set(o,val != 0);    // delegate to led_set();
      }

      case BL_ID(_LED,TOGGLE_):
      {
        BL_ob oo = {o->cl,o->op,1,NULL};  // change @ix=0 -> @ix=1
        o = bl_ix(o) ? o : &oo;        // if (bl_ix(o)==0) re-map o to &oo
	      return led_toggle(o,val);      // delegate to led_toggle();
      }

      default:
	      return -1;                     // bad input
    }
  }

//==============================================================================
// cleanup (needed for *.c file merge of the bluccino core)
//==============================================================================

  #include "bl_clean.h"
