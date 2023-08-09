//==============================================================================
// bl_uart.h
// UART driver
//
// Created by Hugo Pristauz on 2022-Aug-06
// Copyright © 2022 Blunetics. All rights reserved.
//==============================================================================

#ifndef __BL_UART_H__
#define __BL_UART_H__

//  #include <stdbool.h>
//  #include <stdint.h>

//==============================================================================
// UART Logging
//==============================================================================

#ifndef CFG_LOG_UART
    #define CFG_LOG_UART    1           // UART logging is by default on
#endif

#if (CFG_LOG_UART)
    #define LOG_UART(l,f,...)    BL_LOG(CFG_LOG_UART-1+l,f,##__VA_ARGS__)
    #define LOGO_UART(l,f,o,v)   bl_logo(CFG_LOG_UART-1+l,f,o,v)
#else
    #define LOG_UART(l,f,...)    {}     // empty
    #define LOGO_UART(l,f,o,v)   {}     // empty
#endif

//==============================================================================
// UART function vector typedefs
//==============================================================================
/*
    typedef bool    (*KernelUartAvailT)(void);
    typedef uint8_t (*KernelUartReadT)(void);
    typedef void    (*KernelUartReceiveT)(uint8_t byte);

    typedef bool    (*KernelUartFullT)(void);
    typedef void    (*KernelUartWriteT)(uint8_t byte);
    typedef uint8_t (*KernelUartTransmitT)(void);
*/
//==============================================================================
// Send related
//==============================================================================
/*
    extern KernelUartFullT KernelUartFull;          // vector for checking before write
    extern KernelUartWriteT KernelUartWrite;        // vector for writing byte (in order to transmit)
*/
//==============================================================================
// Receive related
//==============================================================================
/*
    extern KernelUartAvailT KernelUartAvail;        // vector for checking before read
    extern KernelUartReadT  KernelUartRead;         // vector for reading byte (receive)
*/
//==============================================================================
// Synchronization related
//==============================================================================

//  void KernelUartDisableToWriteToVisibleBuffer(void);   // to prevent ISR to write into the visible buffer
//  void KernelUartRefreshBuffer(void);                   // pump received bytes from the private to the public buffer
//  void KernelUartEnableToWriteToVisibleBuffer(void);    // to allow ISR to write into the visible buffer

//==============================================================================
// kernel cross UART functions
//==============================================================================
/*
#if (IS_TOPIC_UART_EMU)

    bool KernelUartCrossAvail(void);
    uint8_t KernelUartCrossRead(void);
    bool KernelUartCrossFull(void);
    void KernelUartCrossWrite(uint8_t byte);

#endif // (IS_TOPIC_UART_EMU)
*/
//==============================================================================
// kernel UART init
//==============================================================================
/*
    void KernelUartInit(void);
*/
//==============================================================================
// Debug helpers
//==============================================================================
/*
    void KernelUartPrintState(void);
*/

//==============================================================================
// public module interface
//==============================================================================

  int bl_uart(BL_ob *o, int val);

#endif // __BL_UART_H__
