//==============================================================================
// bl_spi.c
// SPI driver
//
// Created by Hugo Pristauz on 2022-Sep-11
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_spi.h"

//==============================================================================
// PMI definition and logging shorthands
//==============================================================================

  #define PMI                     bl_spi
  #define WHO                     "bl_spi:"

  #define LOG                     LOG_SPI
  #define LOGO(lvl,col,o,val)     LOGO_SPI(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_SPI(lvl,col,o,val)

//==============================================================================
// SPI structure definition
// - usage: static BL_spi_bufs txbufs[2], rxbufs[2];
//          BL_spi spi = BL_SPI(txbufs,rxbufs);
//==============================================================================
//
// here the structure we have to setup:
//
//            +-----------+                   +--------------+
//     device |     o-----|------------------>|  spi device  |
//            +-----------+                   +--------------+
//        cfg |           |
//            |           |                   +-----------+
//            |           |     +------------>|   txb[2]  |
//            |           |     |             +-----------+   +--+--+
//            +-----------+     |  txb[0].buf |     o-----|-->|  |  |
// tx.buffers |     o-----|-----+  txb[0].len |     2     |   +--+--+
//   tx.count |     2     |                   +-----------+   +--+--+--+--+
//   tx.total |     2     |        txb[1].buf |     o-----|-->|  |  |  |  |
//            +-----------+        txb[1].len |     4     |   +--+--+--+--+
// rx.buffers |     o-----|-----+             +-----------+
//   rx.count |     2     |     |
//   rx.total |     3     |     |             +-----------+
//            +-----------+     +------------>|   rxb[2]  |
//                                            +-----------+   +--+
//                                 rxb[0].buf |     o-----|-->|  |
//                                 rxb[0].len |     1     |   +--+
//                                            +-----------+   +--+--+--+--+--+
//                                 rxb[1].buf |     o-----|-->|  |  |  |  |  |
//                                 rxb[1].len |     5     |   +--+--+--+--+--+
//                                            +-----------+
//                                 rxb[2].buf |           | not in use
//                                 rxb[2].len |           |
//                                            +-----------+
//
// also note:
//   struct spi_buf {	void *buf;	size_t len; };
//   struct spi_buf_set {	const struct spi_buf *buffers;	size_t count; };
//
//==============================================================================
// tx buffer setup
// - usage: err = bl_spi_tx(&spi,ix,buf,sizeof(buf)) // setup tx buffer[ix]
//          err = bl_spi_tx(&spi,-1,NULL,0)          // no tx buffer in use
//==============================================================================

#if (!CFG_SPI_DIRECT_API)
  static
#endif

  int bl_spi_tx(BL_spi *spi, int ix, BL_u8 *buf, size_t size)
  {
    if (ix >= spi->tx.total)
      return bl_err(-1,"bl_spi_tx: index out of range");

      // for negative ix we set number of used buffers to equal zero
      // which means that there is not any tx buffer in use

    if (ix < 0)
    {
      spi->tx.count = 0;               // tx buffers are not in use
      return 0;
    }

      // for ix >= 0 we setup tx buffer[ix] ...

    spi->tx.buffers[ix].buf = buf;
    spi->tx.buffers[ix].len = size;

      // store ix+1 in tx.count!
      // the last stored value will be interpreted as the number of tx buffers

    spi->tx.count = ix+1;
    return 0;
  }

//==============================================================================
// rx buffer setup
// - usage: err = bl_spi_tx(&spi,ix,buf,sizeof(buf)) // setup tx buffer[ix]
//          err = bl_spi_tx(&spi,-1,NULL,0)          // no tx buffer in use
//==============================================================================

#if (!CFG_SPI_DIRECT_API)
  static
#endif

  int bl_spi_rx(BL_spi *spi, int ix, BL_u8 *buf, size_t size)
  {
    if (ix >= spi->rx.total)
      return bl_err(-1,"bl_spi_rx: index out of range");

      // for negative ix we set number of used buffers to equal zero
      // which means that there is not any rx buffer in use

    if (ix < 0)
    {
      spi->rx.count = 0;               // rx buffers are not in use
      return 0;
    }

      // for ix >= 0 we setup rx buffer[ix] ...

    spi->rx.buffers[ix].buf = buf;
    spi->rx.buffers[ix].len = size;

      // store ix+1 in rx.count!
      // the last stored value will be interpreted as the number of rx buffers

    spi->rx.count = ix+1;
    return 0;
  }

//==============================================================================
// spi write: [SPI:WRITE <BL_spi>]  // write to spi device
// - usage: bl_spi_tx(&spi,0,buf0,size0)    // write buffer @0
//          bl_spi_tx(&spi,1,buf1,size1)    // write buffer @1
//          bl_spi_rx(&spi,-1,NULL,0)       // no read buffer
//          err = bl_spi_write(&spi)        // write according to buf setup
//==============================================================================

#if (!CFG_SPI_DIRECT_API)
  static
#endif

  int bl_spi_write(BL_spi *spi)
  {
    static struct spi_buf_set tx;

    tx.buffers = spi->tx.buffers;
    tx.count = spi->tx.count;

    return spi_write(spi->dev,&spi->cfg,&tx);
  }

//==============================================================================
// spi read: [SPI:READ <BL_spi>]    // read from spi device
// - usage: bl_spi_tx(&spi,-1,NULL,size0)   // no write buffer
//          bl_spi_rx(&spi,0,buf0,size0)    // read buffer @0
//          bl_spi_rx(&spi,1,buf1,size1)    // read buffer @1
//          err = bl_spi_read(&spi)         // read according to buf setup
//==============================================================================

#if (!CFG_SPI_DIRECT_API)
  static
#endif

  int bl_spi_read(BL_spi *spi)
  {
    static struct spi_buf_set rx;

    rx.buffers = spi->rx.buffers;
    rx.count = spi->rx.count;

    return spi_read(spi->dev,&spi->cfg,&rx);
  }

//==============================================================================
// helper: spi transceive: [SPI:TRANS <BL_spi>]  // transceive to spi device
// - usage: bl_spi_tx(&spi,0,buf0,size0)    // write buffer @0
//          bl_spi_tx(&spi,1,buf1,size1)    // write buffer @1
//          bl_spi_rx(&spi,-1,NULL,0)       // no read buffer
//          bl_spi_rx(&spi,0,buf0,size0)    // read buffer @0
//          bl_spi_rx(&spi,1,buf1,size1)    // read buffer @1
//          err = bl_spi_trans(&spi)        // write/read according to buf setup
//==============================================================================

#if (!CFG_SPI_DIRECT_API)
  static
#endif

  int bl_spi_trans(BL_spi *spi)
  {
    static struct spi_buf_set tx;
    static struct spi_buf_set rx;

    tx.buffers = spi->tx.buffers;
    tx.count = spi->tx.count;

    rx.buffers = spi->rx.buffers;
    rx.count = spi->rx.count;

    return spi_transceive(spi->dev, &spi->cfg, &tx, &rx);
  }

//==============================================================================
// handler: [SPI:TRANS <BL_spi>] transmit to/from SPI device
//==============================================================================

  static int spi_trans_(BL_ob *o, int val)
  {
    BL_spi *spi = bl_data(o);          // pointer to SPI control structure

    return bl_spi_trans(spi);
  }

//==============================================================================
// handler: [SPI:READ <BL_spi>] read from SPI device
//==============================================================================

  static int spi_read_(BL_ob *o, int val)
  {
    BL_spi *spi = bl_data(o);          // pointer to SPI control structure

    return bl_spi_read(spi);
  }

//==============================================================================
// handler: [SPI:WRITE <BL_spi>] write to SPI device
//==============================================================================

  static int spi_write_(BL_ob *o, int val)
  {
    BL_spi *spi = bl_data(o);          // pointer to SPI control structure

    return bl_spi_write(spi);
  }

//==============================================================================
// handler: [SPI:RX @ix,<BL_xmit>] setup RX buffer @ix
//==============================================================================

  static int spi_rx_(BL_ob *o, int val)
  {
    int ix = bl_ix(o);                 // index of data buffer
    BL_tray *tray = bl_data(o);        // transmission data
    BL_spi *spi = tray->any;           // pointer to SPI control structure

    return bl_spi_rx(spi,ix, tray->data,tray->size);
  }

//==============================================================================
// handler: [SPI:INIT <BL_spi>]        // init SPI device
//==============================================================================

  static int spi_init_(BL_ob *o, int val)
  {
    BL_spi *spi = bl_data(o);

    int err = !device_is_ready(spi->dev);

    if (err)
    {
      BL_txt name = spi->dev->name;
      bl_log(1,BL_R "error %d: fc_nor: SPI device %s is not ready", err,name);
    }

    return err;
  }

//==============================================================================
// handler: [SPI:TX @ix,<BL_xmit>] setup TX buffer @ix
//==============================================================================

  static int spi_tx_(BL_ob *o, int val)
  {
    int ix = bl_ix(o);                 // index of data buffer
    BL_tray *tray = bl_data(o);        // transmission data
    BL_spi *spi = tray->any;           // pointer to SPI control structure

    return bl_spi_tx(spi,ix, tray->data,tray->size);
  }

//==============================================================================
// public module interface
//==============================================================================
//
// (D) := bl_spi;
//                  +--------------------+
//                  |       bl_spi       | SPI driver
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (D)->     INIT ->|        (cb)        | system init
//                  +--------------------+
//                  |        SPI:        | SPI input interface
// (D)->     INIT ->|      <BL_spi>      | init SPI device
// (D)->    WRITE ->|      <BL_spi>      | write to SPI device
// (D)->     READ ->|      <BL_spi>      | read from SPI device
// (D)->    TRANS ->|      <BL_spi>      | transmit to/from SPI device
// (D)->       TX ->|    @ix,<BL_xmit>   | setup TX buffer @ix
// (D)->       RX ->|    @ix,<BL_xmit>   | setup RX buffer @ix
//                  +--------------------+
//
//==============================================================================

  int bl_spi(BL_ob *o, int val)
  {
    switch (bl_id(o))                  // dispatch message ID
    {
      case BL_ID(_SYS,INIT_):          // [SYS:INIT <cb>] init module
        return 0;                      // nothing to do

      case BL_ID(_SPI,INIT_):          // [SPI:WRITE <BL_spi>]
        return spi_init_(o,val);       // delegate to spi_init_() handler

      case BL_ID(_SPI,WRITE_):         // [SPI:WRITE <BL_spi>]
        return spi_write_(o,val);      // delegate to spi_write_() handler

      case BL_ID(_SPI,READ_):          // [SPI:READ <BL_spi>]
        return spi_read_(o,val);       // delegate to spi_read_() handler

      case BL_ID(_SPI,TRANS_):         // [SPI:TRANS <BL_spi>]
        return spi_trans_(o,val);      // delegate to spi_trans_() handler

      case BL_ID(_SPI,TX_):            // [SPI:TX @ix,<BL_xmit>]
        return spi_tx_(o,val);         // delegate to spi_tx_() handler

      case BL_ID(_SPI,RX_):            // [SPI:RX @ix,<BL_xmit>]
        return spi_rx_(o,val);         // delegate to spi_rx_() handler

      default:
        LOGO(1,BL_R "undispatched::",o,val);
        return -1;                     // bad arg
    }
  }
