//==============================================================================
// bl_spi.h
// SPI driver
//
// Created by Hugo Pristauz on 2022-Sep-11
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

#ifndef __BL_SPI_H__
#define __BL_SPI_H__

  #include <drivers/spi.h>
  #include "bl_gpio.h"            // use GPIO short hands

//==============================================================================
// config defaults
//==============================================================================

#ifndef CFG_SPI_DIRECT_API
  #define CFG_SPI_DIRECT_API    0      // no direct API per default
#endif

//==============================================================================
// SPI Logging
//==============================================================================

#ifndef CFG_LOG_SPI
    #define CFG_LOG_SPI    1           // SPI logging is by default on
#endif

#if (CFG_LOG_SPI)
    #define LOG_SPI(l,f,...)    BL_LOG(CFG_LOG_SPI-1+l,f,##__VA_ARGS__)
    #define LOGO_SPI(l,f,o,v)   bl_logo(CFG_LOG_SPI-1+l,f,o,v)
#else
    #define LOG_SPI(l,f,...)    {}     // empty
    #define LOGO_SPI(l,f,o,v)   {}     // empty
#endif

//==============================================================================
// type definitions in context with SPI interface
//==============================================================================

  typedef struct spi_buf  BL_spi_buf;

    // type BL_spi_buf_set is similar to type spi_buf_set except
    // the pointer to buffer array is not qualified as constant.
    // this allows us to change actual data arrays per command
    // using

  typedef struct BL_spi_buf_set
          {
	          BL_spi_buf *buffers;
	          size_t count;
	          size_t total;
          } BL_spi_buf_set;

  typedef struct BL_spi
          {
            const struct device *dev;
            struct spi_config cfg;      // init with {0}
            BL_spi_buf_set tx;
            BL_spi_buf_set rx;
          } BL_spi;

  #define BL_SPI(txbufs,rxbufs)                                         \
                {                                                       \
                  dev: NULL,                                            \
                  cfg:{0},                                              \
                  tx:{buffers:txbufs, count:0, total:BL_LEN(txbufs)},   \
                  rx:{buffers:rxbufs, count:0, total:BL_LEN(rxbufs)},   \
                }

//==============================================================================
// public module interface
//==============================================================================

  int bl_spi(BL_ob *o, int val);

//==============================================================================
// direct API
//==============================================================================

#if (CFG_SPI_DIRECT_API)

  int bl_spi_tx(BL_spi *spi, int ix, BL_u8 *buf, size_t size);
  int bl_spi_rx(BL_spi *spi, int ix, BL_u8 *buf, size_t size);
  int bl_spi_write(BL_spi *spi);
  int bl_spi_read(BL_spi *spi);
  int bl_spi_trans(BL_spi *spi);

#endif

//==============================================================================
// syntactic sugar: init SPI device
// - usage: err = _SPI_INIT(&spi, (to))
//==============================================================================

  static inline int _SPI_INIT(BL_spi *spi, BL_oval to)
  {
    return _bl_msg((to), _SPI,INIT_, 0,spi,0);
  }

//==============================================================================
// syntactic sugar: setup TX buffer @ix
// - usage: err = _SPI_TX(&spi,ix, buf,size, (to))
//==============================================================================

  static inline
  int _SPI_TX(BL_spi *spi,int ix, BL_buf buf,size_t size, BL_oval to)
  {
    BL_tray tray = {data:buf, size:size, any:spi};
    return _bl_msg((to), _SPI,TX_, ix,&tray,0);
  }

//==============================================================================
// syntactic sugar: setup RX buffer @ix
// - usage: err = _SPI_RX(&spi,ix, buf,size, (to))
//==============================================================================

  static inline
  int _SPI_RX(BL_spi *spi,int ix, BL_buf buf,size_t size, BL_oval to)
  {
    BL_tray tray = {data:buf, size:size, any:spi};
    return _bl_msg((to), _SPI,RX_, ix,&tray,0);
  }

//==============================================================================
// syntactic sugar: write to SPI device
// - usage: _SPI_TX(&spi,0, txbuf0,size0, (to))  // setup TX buffer @0
//          _SPI_TX(&spi,1, txbuf1,size1, (to))  // setup TX buffer @1
//          err = _SPI_WRITE(&spi, (to))
//==============================================================================

  static inline int _SPI_WRITE(BL_spi *spi, BL_oval to)
  {
    return _bl_msg((to), _SPI,WRITE_, 0,spi,0);
  }

//==============================================================================
// syntactic sugar: read from SPI device
// - usage: _SPI_RX(&spi,0, rxbuf0,size0, (to))  // setup RX buffer @0
//          _SPI_RX(&spi,1, rxbuf1,size1, (to))  // setup RX buffer @1
//          err = _SPI_READ(&spi, (to))
//==============================================================================

  static inline int _SPI_READ(BL_spi *spi, BL_oval to)
  {
    return _bl_msg((to), _SPI,READ_, 0,spi,0);
  }

//==============================================================================
// syntactic sugar: transmit to/from SPI device
// - usage: _SPI_TX(&spi,0, txbuf0,size0, (to))  // setup TX buffer @0
//          _SPI_TX(&spi,1, txbuf1,size1, (to))  // setup TX buffer @1
//          _SPI_RX(&spi,0, rxbuf0,size0, (to))  // setup RX buffer @0
//          _SPI_RX(&spi,1, rxbuf1,size1, (to))  // setup RX buffer @1
//          err = _SPI_WRITE(&spi, (to))
//==============================================================================

  static inline int _SPI_TRANS(BL_spi *spi, BL_oval to)
  {
    return _bl_msg((to), _SPI,TRANS_, 0,spi,0);
  }

#endif // __BL_SPI_H__
