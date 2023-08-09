//==============================================================================
// bl_gpio.h
// shorthannd typedefs and helpers for GPIO setup
//
// Created by Hugo Pristauz on 2022-Jan-09
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================
// quick usage:
//   int ix = bl_digin(13,0);       // standard P0.13 config (button @1)
//   int val = bl_pin_get(ix);      // get digital input value
//
//   int ox = bl_digout(17,0);      // standard P0.17 config (button @1)
//   int val = bl_pin_set(ox,1);    // set digital output value to 1
//==============================================================================

#ifndef __BL_GPIO_H__
#define __BL_GPIO_H__

#ifdef __cplusplus
  extern "C" {
#endif

  #include <zephyr/device.h>
  #include <zephyr/drivers/gpio.h>

//==============================================================================
// some shorthands
//==============================================================================

  typedef uint32_t             GP_pins;          // PINs qualifier in irs
  typedef gpio_flags_t         GP_flags;         // GPIO flags (32 bit unsigned)
  typedef gpio_pin_t           GP_pin;           // GPIO pin

  typedef const struct device  GP_dev;           // const GPIO device pointer
  typedef struct gpio_callback GP_ctx;           // GPIO callback context

  typedef void (*GP_isr)(GP_dev *dev, GP_ctx *ctx, GP_pins pins);

//==============================================================================
// GPIO device tree specification
// - typedef
//   struct gpio_dt_spec {GP_dev *port;	GP_pin pin;	GP_flags dt_flags;} GP_ds;
//==============================================================================

  typedef struct gpio_dt_spec  GP_ds;            // GPIO device tree spec

//==============================================================================
// aditional definitions (Bluccino v1.1.0)
// - usage: GP_ds ds = GP_IO(PIN5,gpios,0)
//==============================================================================

  #define GP_IO(alias,prop,val)   GPIO_DT_SPEC_GET_OR(alias,gpios,{val})

//==============================================================================
// syntactic sugar: check whether port is ready, otherwise report error
// - usage: GP_ds button = GP_IO(SW0_NODE, gpios,0);
// - usage: err = gp_ready(button.port)
//==============================================================================

  static int inline gp_ready(GP_dev *port)
  {
    if (device_is_ready(port))
      return 1;

    bl_err(-ENODEV,bl_fmt("port device %s not ready", port ? port->name:"???"));
    return 0;        // device not ready
  }

//==============================================================================
// GPIO pin structure
// - usage: #define BUTTON  DT_ALIAS(sw0)
// - usage: BL_pin button = BL_PIN(BUTTON,gpios,0)
//==============================================================================

  typedef struct BL_pin
  {
    GP_ds ds;                          // input I/O pin (devicetree spec)
    GP_ctx ctx;                        // context
  } BL_pin;

  #define BL_PIN(nid,prop,val)      {.ds = GP_IO(nid, prop,{val}) }

//==============================================================================
// setup pin (similar to: BL_pin pin = BL_PIN(DT_ALIAS(sw0),gpios,0));
// - usage: err = bl_pin("GPIO_0",13,GP_ACTLOW|GP_PULLUP); // setup button@1 pin
//==============================================================================

  static inline int bl_pin(BL_pin *pin, BL_txt port,BL_byte nmb,BL_byte flags)
  {
    pin->ds.port = device_get_binding("GPIO_0");
    pin->ds.pin = nmb;                 // set pin number
    pin->ds.dt_flags = flags;          // set dt_flags (8 bit)
    return (pin->ds.port == NULL);     // error if port equals NULL
  }

//==============================================================================
// GPIO configuration (device tree flags - fit into a byte)
// - button input: for nRF52 DK/dongle use: BL_PULLUP|BL_ACTLOW
// - LED output:   for nRF52 DK/dongle use: BL_ACTLOW
//==============================================================================

  #define BL_DISCON     GPIO_GPIO_DISCONNECTED   // 0x00

  #define BL_ACTHIGH    GPIO_ACTIVE_HIGH         // 0x00
  #define BL_ACTLOW     GPIO_ACTIVE_LOW          // 0x01

  #define BL_PUSHPULL   GPIO_PUSH_PULL           // 0x00
  #define BL_SINGLE     GPIO_SINGLE_ENDED        // 0x02

  #define BL_OPENSRC    GPIO_OPEN_SOURCE         // 0x02
  #define BL_OPENDRAIN  GPIO_OPEN_DRAIN          // 0x06

  #define BL_PULLUP     GPIO_PULL_UP             // 0x10
  #define BL_PULLDOWN   GPIO_PULL_DOWN           // 0x20

//==============================================================================
// pin mode definitions (flags needing 16 bit representation)
// - see: zephyr/include/dt-bindings/gpio/gpio.h
// - usage: bl_pin_cfg(&button, BL_DIGIN)
//          bl_pin_cfg(&button, BL_DIGIN|BL_PULLUP)
//          bl_pin_cfg(&button, BL_DIGIN|BL_PULLUP|BL_ACTLOW)
//          bl_pin_cfg(&led, BL_DIGOUT)
//          bl_pin_cfg(&led, BL_DIGOUT|BL_OPEN_DRAIN)
//          bl_pin_cfg(&led, BL_DIGOUT|BL_OPEN_DRAIN|BL_ACTLOW)
//==============================================================================

    // signal direction for digital pins

  #define BL_DIGIN      GPIO_INPUT             // 0x0100 cfg pin as dig input
  #define BL_DIGOUT     GPIO_OUTPUT            // 0x0200 cfg pin as dig output

    // digital output level initializing qualifiers

  #define BL_INIT_LOW   GPIO_OUTPUT_INIT_LOW   // 0x0400 low output lvl @ init
  #define BL_INIT_HIGH  GPIO_OUTPUT_INIT_HIGH  // 0x0800 high output lvl @ init

//==============================================================================
// pin interrupt mask definitions (flags needing 32 bit representation)
// - see: zephyr/include/dt-bindings/gpio/gpio.h
// - usage: bl_pin_cfg(&button, BL_DIGIN|BL_INT_RISE)
//          bl_pin_cfg(&button, BL_DIGIN|BL_PULLUP)
//          bl_pin_cfg(&button, BL_DIGIN|BL_PULLUP|BL_ACTLOW)
//          bl_pin_cfg(&led, BL_DIGOUT)
//          bl_pin_cfg(&led, BL_DIGOUT|BL_OPEN_DRAIN)
//          bl_pin_cfg(&led, BL_DIGOUT|BL_OPEN_DRAIN|BL_ACTLOW)
//==============================================================================

    // basic interrupt flags

  #define BL_DISABLE   GPIO_INT_DISABLE        // 0x02000 interrupt disable
  #define BL_ENABLE    GPIO_INT_ENABLE         // 0x04000 interrupt enable
  #define BL_INTLOGIC  GPIO_INT_LEVELS_LOGICAL // 0x08000 interrupt enable
  #define BL_INTEDGE   GPIO_INT_EDGE           // 0x10000 interrupt on edge
  #define BL_INTLOW    GPIO_INT_LOW_0          // 0x20000 interrupt on low level
  #define BL_INTHIGH   GPIO_INT_HIGH_1         // 0x40000 interrupt on high lvl
  #define BL_DEBOUNCE  GPIO_INT_DEBOUNCE       // 0x80000 edge debouncing

    // composite interrupt flags

  #define BL_MASKALL   GPIO_INT_MASK           // mask (disable) all interrupts
  #define BL_EDGEFALL  GPIO_INT_EDGE_FALLING   // 0x30000 int. on rising edge
  #define BL_EDGERISE  GPIO_INT_EDGE_RISING    // 0x50000 int. on rising edge
  #define BL_EDGEBOTH  GPIO_INT_EDGE_BOTH      // 0x70000 int. on both edges

//==============================================================================
// GPIO pin configuration (helper)
// - see: zephyr/include/drivers/gpio.h
// - usage: gp_pin_cfg(ds,flags)
// -        gp_pin_cfg(ds,BL_DIGIN)
// -        gp_pin_cfg(ds,BL_DIGIN | GPIO_INT_DEBOUNCE)
// -        gp_pin_cfg(ds,BL_DIGOUT)
//==============================================================================

  static inline int gp_pin_cfg(const GP_ds *p, GP_flags flags)
  {
    int err = gpio_pin_configure_dt(p,flags);
    return bl_err(err,bl_fmt("failed to configure %s @ pin %d",
                             p->port->name,p->pin));
  }

//==============================================================================
// GPIO pin interrupt configuration (helper)
// - usage: gp_int_cfg(ds,flags)
// -        gp_int_cfg(ds,GPIO_INT_EDGE_TO_ACTIVE)
// -        gp_int_cfg(ds,GPIO_INT_EDGE_BOTH)
//==============================================================================

  static inline int gp_int_cfg(const GP_ds *p, GP_flags flags)
  {
    int err = gpio_pin_interrupt_configure_dt(p,flags);
    return bl_err(err,bl_fmt("failed to config %s pin %d interrupt\n",
  			                     p->port->name, p->pin));
  }

//==============================================================================
// GPIO pin add interrupt callback (helper)
// - usage: err = gp_add_cb(ds,ctx,irs)
// -        err = gp_add_cb(&but,&but_ctx,but_irs)
//==============================================================================

  static inline int gp_add_cb(const GP_ds *p, GP_ctx *ctx, GP_isr isr)
  {
    gpio_init_callback(ctx, isr, BIT(p->pin));
    return gpio_add_callback(p->port, ctx);
  }

//==============================================================================
// get I/O input value
// - usage: val = bl_pin_get(&ds)
//==============================================================================

  static inline int gp_pin_get(const GP_ds *p)
  {
    return gpio_pin_get_dt(p);
  }

//==============================================================================
// set I/O output value
// - usage: gp_pin_set(&ds,val)
//==============================================================================

  static inline int gp_pin_set(const GP_ds *p, int value)
  {
    return gpio_pin_set_dt(p,value);
  }

//==============================================================================
// resolve device binding
// - usage: GP_dev i2c = bl_dev("I2C_0")
//==============================================================================

  static inline GP_dev *bl_dev(BL_txt label)
  {
    GP_dev *dev = device_get_binding(label);
    if (dev == NULL)
      bl_log(1,"error -1: cannot resolve device binding: %s", label);

    return dev;
  }

//==============================================================================
// config I/O pin
// - usage: err = bl_pin_cfg(pin,flags)
// -        err = bl_pin_cfg(&button, BL_INPUT)
//==============================================================================

  static inline int bl_pin_cfg(BL_pin *pin, GP_flags flags)
  {
    if (!device_is_ready(pin->ds.port))
    {
      LOG_GPIO(1,BL_R"error: pin %s not ready", pin->ds.port->name);
      return -1;
    }

    return gp_pin_cfg(&pin->ds, flags);
  }

//==============================================================================
// attach interrupt handler to I/O pin
// - usage: err = bl_pin_attach(pin,flags,cb)
// -        err = bl_pin_attach(&button, GPIO_INT_EDGE_TO_ACTIVE, but_cb)
// -        err = bl_pin_attach(&button, GPIO_INT_EDGE_BOTH, but_cb)
//==============================================================================

  static inline int bl_pin_attach(BL_pin *pin, GP_flags flags, GP_isr isr)
  {
    if (!device_is_ready(pin->ds.port))
    {
      LOG_GPIO(1,BL_R"error: pin %s not ready", pin->ds.port->name);
      return -1;
    }

    if (flags)
      gp_int_cfg(&pin->ds, flags);
    else
      return -1;

    if (isr)
      return gp_add_cb(&pin->ds, &pin->ctx, isr);
    else
      return -2;
  }

//==============================================================================
// get input pin value
// - usage: val = bl_pin_get(&pin)
//==============================================================================

  static inline int bl_pin_get(BL_pin *pin)
  {
    return gpio_pin_get_dt(&pin->ds);
  }

//==============================================================================
// set output pin value
// - usage: bl_pin_set(&pin,val)
//==============================================================================

  static inline int bl_pin_set(BL_pin *pin, int value)
  {
    return gpio_pin_set_dt(&pin->ds,value);
  }

//==============================================================================
// get pin name
// - usage: BL_pin pin = BL_PIN(alias,prop,val)
//          BL_txt name = bl_pin_name(&pin)
//==============================================================================

  static inline BL_txt bl_pin_name(BL_pin *pin)
  {
    return pin->ds.port->name;
  }

//==============================================================================
// get pin index
// - usage: BL_pin pin = BL_PIN(alias,prop,val)
//          int ix = bl_pin_ix(&pin)
//==============================================================================

  static inline int bl_pin_ix(BL_pin *pin)
  {
    return pin->ds.pin;
  }

//==============================================================================
// footer
//==============================================================================

#ifdef __cplusplus
  }
#endif

#endif // __BL_GPIO_H__
