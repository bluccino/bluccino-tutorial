//==============================================================================
// bl_uart.c
// UART driver
//
// Created by Hugo Pristauz on 2022-Aug-06
// Copyright © 2022 Blunetics. All rights reserved.
//==============================================================================

  #include <assert.h>
  #include <drivers/uart.h>

  #include "bluccino.h"
  #include "bl_ring.h"
  #include "bl_uart.h"

  #define STATIC_ASSERT(cond) typedef char static_assert[(cond) ? 1 : -1]

//==============================================================================
// PMI definition and logging shorthands
//==============================================================================

  #define PMI  bl_uart            // public module interface
  #define WHO  "bl_uart:"         // who is logging?

  #define LOG                     LOG_UART
  #define LOGO(lvl,col,o,val)     LOGO_UART(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_UART(lvl,col,o,val)

//==============================================================================
// locals
//==============================================================================

  static const struct device *uart = NULL;

//==============================================================================
// ring buffer
//==============================================================================

    #define RING_BUFFER_SIZE 128

    static BL_ring rxring;
    static BL_byte  rxbuf[RING_BUFFER_SIZE];

//==============================================================================
// helper: init ring buffer
//==============================================================================

  static void ring_init(void)
  {
    LOG(4,"init RX ring buffer");
    bl_ring_init(&rxring, rxbuf, RING_BUFFER_SIZE);
  }

//==============================================================================
// helper: drain UART
//==============================================================================

  static inline void uart_drain(const struct device *dev)
  {
    BL_byte c;

    while (uart_fifo_read(dev, &c, 1))
      continue;
  }

//==============================================================================
// isr: UART interrupt service request function
//==============================================================================

  static void uart_isr(const struct device *unused, void *data)
  {
    static BL_work work = BL_WORK(PMI);
    static BL_ob oo = _BL_OB(_UART,AVAIL_,0,NULL);
    int count = 0;
    BL_byte byte;
    int ret;

    BL_ring* p = &rxring; //mWriteToPublic ? &rxring : &mRingPrivate;

    LOG(8, BL_R "uart_isr()");

    while (!bl_ring_full(p) && uart_irq_update(uart)&&uart_irq_is_pending(uart))
    {
      if (!uart_irq_rx_ready(uart))
          break;   // Only the UART RX path is interrupt-enabled

      ret = uart_fifo_read(uart, &byte, sizeof(byte));
      if (!ret)
          continue;

      count++;
      bl_ring_put(p, byte);
    }

    LOG(7,"UART receive: #%d",count);
    bl_submit(&work,&oo,count);  // [#UART,AVAIL count] -> (PMI)
  }

//==============================================================================
// helper: is UART write buffer fu
//==============================================================================

    static bool is_full(void)
    {
      return false;
    }

//==============================================================================
// helper send byte via UART
//==============================================================================

  static int uart_send(BL_byte byte)
  {
    static uint32_t count = 0;

    if (uart == NULL)
      return bl_err(-1,"UART not initialized");

    count++;
    LOG(7, "UART send(0x%02X)", byte);
    uart_poll_out(uart, byte);         // waits until the transmitter empty
    return 0;
  }

//==============================================================================
// handler: [UART:AVAIL] is a received UART byte available for read?
//==============================================================================

  static int uart_avail(BL_ob *o, int val)
  {
    if (uart == NULL)
      return bl_err(-2,"UART not initialized");

    return bl_ring_avail(&rxring);
  }

//==============================================================================
// handler: [UART:READ] read received byte from UART ring buffer
// - note: always check before whether there is a byte available for read
//==============================================================================

  static int uart_read(BL_ob *o, int val)
  {
    if (uart == NULL)
      return bl_err(-3,"UART not initialized");

    if (!bl_ring_avail(&rxring))
    {
      LOG(6, "UART read: no byte available");
      return -1;
    }

    BL_byte byte = bl_ring_get(&rxring);

    LOG(6, "UART read: <0x%02X>", byte);
    return byte;
  }

//==============================================================================
// handler: [UART:FULL] is UART write buffer full?
//==============================================================================

  static int uart_full(BL_ob *o, int val)          // is write buffer full?
  {
    if (uart == NULL)
      return bl_err(-4,"UART not initialized");

    return is_full();
  }

//==============================================================================
// handler: [UART:WRITE byte] write byte
// - note: before writing always check whether write buffer is full
//==============================================================================

  static int uart_write(BL_ob *o, int val)
  {
    BL_byte byte = (BL_byte)val;

    if (uart == NULL)
      return bl_err(-5,"UART not initialized");

    LOG(6, "write <0x%02X> to UART", byte);

    return uart_send(byte);
  }

//==============================================================================
// handler: [SYS:INIT (cb)]  init UART module
//==============================================================================

  int sys_init(BL_ob *o, int val)
  {
    struct uart_config cfg =
           {
             baudrate:  57600,
             parity:    UART_CFG_PARITY_NONE,
             stop_bits: UART_CFG_STOP_BITS_1,
             data_bits: UART_CFG_DATA_BITS_8,
             flow_ctrl: UART_CFG_FLOW_CTRL_NONE,
           };

    LOG(4,BL_B "init bl_uart");
    ring_init();

    static const char *name = "UART_0";
    uart = device_get_binding(name);

    if (!uart)
    {
      LOG(1,BL_R "error -1: cannot find %s" BL_0, name);
      return -80;
    }

    int err = uart_configure(uart, &cfg);

    if (err)
    {
      uart = NULL;                      // make invalid again
      return bl_err(err,"UART: cannot configure");
    }

    uart_irq_rx_disable(uart);
    uart_irq_tx_disable(uart);

    uart_drain(uart);

    LOG(5, BL_B "UART: setup isr");
    uart_irq_callback_set(uart, uart_isr);

    uart_irq_rx_enable(uart);
    return (uart == NULL);              // return err if uart not initialized
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (D) := bl_down;  (U) := bl_up;
//
//                  +--------------------+
//                  |      bl_uart       | UART driver module
//                  +--------------------+
//                  |        SYS:        | SYS interface
// (D)->     INIT ->|        (cb)        | init module, ignore <out> arg
//                  +--------------------+
//                  |        UART:       | UART input interface
// (D)->    AVAIL ->|                    | is data byte available for read?
// (D)->     FULL ->|                    | is write buffer full?
// (D)->     READ ->|                    | read byte from receive buffer
// (D)->    WRITE ->|        byte        | write byte to send buffer
//                  |....................|
//                  |       #UART:       | UART output interface
// (U)<-    AVAIL <-|       count        | data byte is now available for read!
// (U)<-     FULL <-|         0          | write buffer is no more full
//                  +--------------------+
//
//==============================================================================

  int bl_uart(BL_ob *o, int val)
  {
    static BL_oval U = NULL;           // usually up gear or HW core

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        U = bl_cb(o,(U),WHO"(U)");     // store callback
        return sys_init(o,val);        // delegate to sys_init() handler

      case BL_ID(_UART,AVAIL_):
        return uart_avail(o,val);      // delegate to uart_avail() handler

      case BL_ID(_UART,FULL_):
        return uart_full(o,val);       // delegate to uart_full() handler

      case BL_ID(_UART,READ_):
        return uart_read(o,val);       // delegate to uart_read() handler

      case BL_ID(_UART,WRITE_):
        return uart_write(o,val);      // delegate to uart_write() handler

      case _BL_ID(_UART,AVAIL_):
      case _BL_ID(_UART,FULL_):
        return bl_out(o,val,(U));      // send event to up gear

      default:
        return -1;
    }
  }
