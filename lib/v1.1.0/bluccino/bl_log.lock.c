//==============================================================================
//  bl_log.c
//  bluccino debug logging
//
//  Created by Hugo Pristauz on 2021-11-19
//  Copyright © 2021 Bluenetics GmbH. All rights reserved.
//==============================================================================

#include <stdarg.h>
#include <stdint.h>

#include "bluccino.h"

//==============================================================================
// config defaults
//==============================================================================

#ifndef CFG_LOG_PRETTY_PRINTING
  #define CFG_LOG_PRETTY_PRINTING    1      // pretty text for class tag & opcode
#endif

#ifndef CFG_LOG_MUTEX
  #define CFG_LOG_MUTEX         1      // logging mutex enabled by default
#endif

//==============================================================================
// local variables
//==============================================================================

  static BL_txt color = "";            // text color for time header
  static int debug = 4;                // debug level

//==============================================================================
// log helper
//==============================================================================
#if CFG_LOG_PRETTY_PRINTING

  BL_txt bl_cltxt(BL_cl cl)
  {
    static BL_txt text[] = BL_CL_TEXT;
    return (cl < BL_LENGTH(text)) ? text[cl] : (cl==_VOID ? "VOID" : "???");
  }

  BL_txt bl_optxt(BL_op op)
  {
    static BL_txt text[] = BL_OP_TEXT;
    return (op < BL_LENGTH(text)) ? text[op] : (op==VOID_ ? "VOID" : "???");
  }

#else

  BL_txt bl_cltxt(BL_cl cl) { return ""; }
  BL_txt bl_optxt(BL_op op) { return ""; }

#endif
//==============================================================================
// include Bluccino RTL stuff if activated
// - add forward declaration for now, since it is used in bl_rtl.c
//==============================================================================

#if (CFG_BLUCCINO_RTL)
  #include "bl_rtl.c"                 // include Bluccino RTL implementation
#endif // CFG_BLUCCINO_RTL

//==============================================================================
// RTT log driver
//==============================================================================
#ifdef __NRF_SDK__

     // For some reason, this function, while not static, is not included
     // in the RTT header files

  int SEGGER_RTT_vprintf(unsigned, const char *, va_list *);

      // in Zephyr environment we have printk(...) function
      // here is the macro for usage in compatible style

  #ifdef printk
    #undef printk
  #endif

  #define printk(...)   bl_prt(__VA_ARGS__)

  void bl_vprintf(const char * format, va_list arguments)
  {
    BL_VPRINTF(0, format, &arguments);
  }

  void bl_printf(const char * format, ...)
  {
    va_list arguments;  // lint -save -esym(530,args) sym args not initialized
    va_start(arguments, format);
    bl_vprintf(format, arguments);
    va_end(arguments);  // lint -restore
  }

#endif // __NRF_SDK__
//==============================================================================
// decorate log time stamps
// - depending on attention and provision status
//==============================================================================

  void bl_decorate(bool attention, bool provision)
  {
    color = attention ? BL_G : (provision ? BL_C : "");
  }

  int bl_verbose(int verbose)              // set verbose level
  {
    int old = debug;
    debug = verbose;
    return old;
  }

//==============================================================================
// assertion
//==============================================================================

  void bl_assert(bool assertion)
  {
    if (!assertion)
    {
      bl_log(0,BL_R"assertion violated");
      for(;;)
        bl_sleep(10);                  // sleep to support SEGGER RTT function
    }
  }

//==============================================================================
// error message: error printing only for err != 0
// - usage: err = bl_err(err,msg)
//==============================================================================

  __weak int bl_err(int err, BL_txt msg)
  {
    if (err)
    {
      if (bl_now(1))                            // errors come @ verbose level 1
        bl_prt(BL_R "error %d: %s\n" BL_0,err,msg);  // in RED text!
    }
    return err;
  }

//==============================================================================
// warning message: warning printing only for err != 0
// - usage: err = bl_wrn(err,msg)
//==============================================================================

  __weak int bl_wrn(int err, BL_txt msg)
  {
    if (err)
    {
      if (bl_now(1))                            // errors come @ verbose level 1
        bl_prt(BL_R "warning %d: %s\n" BL_0,err,msg);  // in RED text!
    }
    return err;
  }

//==============================================================================
// get clock time as minutes, seconds, milliseconds
//==============================================================================

  static void now(int *pmin, int *psec, int *pms, int *pus)  // split us time
  {
    static int min = 0;
    static int sec = 0;
    static BL_ms offset = 0;
    BL_us us = bl_us();                    // clock time now in us

    *pus = us % 1000;                      // map us to range 0 .. 999
    *pms = us/1000 - offset;

      // calculate trace time tick

    for (; *pms >= 1000; offset += 1000, sec++)
      *pms -= 1000;

    for (; sec >= 60; min++)
      sec -= 60;

    *pmin = min;  *psec = sec;
  }

//==============================================================================
// log time stamp of current ms/us time
// - standard Bluccino bl_now() function for non-activated Bluccino RTL
// - usage: if (bl_now(lvl)) ...  // only log if proper log level
//==============================================================================
#if (!CFG_BLUCCINO_RTL)

  BL_short bl_now(int lvl)
  {
    if (lvl > debug)
      return 0;

    int min, sec, ms, us;
    now(&min,&sec,&ms,&us);

      // print header in green if in attention mode,
      // yellow if node is provisioned, otherwise white by default

    bl_prt("%s#%d[%03d:%02d:%03d.%03d] " BL_0, color,lvl, min,sec,ms,us);

    for (int i=0; i < lvl; i++)
      bl_prt("  ");                   // indentation

    return 1;  // keep in mind: mutex is locked and needs to be unlocked
  }

#endif
//==============================================================================
// check if debug tracing for given log leve, and occasionally print time stamp
// - usage: if (bl_dbg(lvl)) ...   // only execute for proper debug level
//==============================================================================

  bool bl_dbg(int lvl)
  {
    return (lvl <= debug);
  }

//==============================================================================
// begin log / end log
// - usage: ok = bl_begin(lev)   // begin log
//          err = bl_end()       // always (!!!) returns 0
//==============================================================================

  #if (CFG_LOG_MUTEX)
    K_MUTEX_DEFINE(log_mutex);
  #endif

  BL_short bl_begin(int lvl)
  {
    if (lvl > debug)
      return 0;                        // suppress logging (mutex not locked)

    #if (CFG_LOG_MUTEX)
      if ( k_mutex_lock(&log_mutex, K_MSEC(50)) != 0)
      {
        bl_printf(BL_R "*** log mutex timeout\n");
        return 0;
      }
bl_printf(BL_R"lock\n"BL_0);
    #endif

    bl_now(lvl);
    return true;                       // mutex locked (true)!
  }

  int bl_end(bool locked)
  {
    #if (CFG_LOG_MUTEX)
      if (locked)
      {
bl_printf(BL_R"unlock\n"BL_0);
        k_mutex_unlock(&log_mutex);
      }
    #endif
    return 0;                          // always (!!!) must be false
  }

//==============================================================================
// log messages
// - standard bl_logo() function, used if RTL is not activated
//==============================================================================
#if (!CFG_BLUCCINO_RTL)

  void bl_logo(int lev, BL_txt msg, BL_ob *o, int value) // log event message
  {
    bool locked = bl_begin(lev);
    if ( !locked )
     return;

      // keep in mind: bl_now locked a mutex !!!

    int ix = bl_ix(o);
    BL_txt aug = BL_ISAUG(o->cl) ? "#" : "";
    BL_cl cl = BL_UNAUG(o->cl);

    BL_txt col = (msg[0] != '@') ? "" : (value ? BL_G : BL_M);
    msg = (msg[0] == '@') ? msg+1 : msg;

    #if CFG_LOG_PRETTY_PRINTING             // pretty text for class tag & opcode
      if (ix > 0 && BL_HW(ix))
        bl_prt("%s%s [%s%s:%s @<%s|%s>,%d]\n"BL_0, col,msg,
               aug,bl_cltxt(cl), bl_optxt(o->op), BL_IDTXT(ix),value);
      else
        bl_prt("%s%s [%s%s:%s @%d,%d]\n"BL_0, col,msg,
               aug,bl_cltxt(cl), bl_optxt(o->op), bl_ix(o),value);
    #else
      bl_prt("%s%s [%s%d:%d @%d,%d]\n"BL_0,col,msg,
             aug,cl, o->op, bl_ix(o),value);
    #endif

      // keep in mind: mutex locked by bl_now() needs to be unlocked!!!

    bl_end(locked);
  }

#endif // !CFG_BLUCCINO_RTL
//==============================================================================
// bl_hello (syntactic sugar to set verbose level and print a hello message)
// - usage: bl_hello(verbose,msg)
//==============================================================================

  void bl_hello(int verbose, BL_txt msg)
  {
    bl_verbose(verbose);               // set verbose level
		bl_prt("*** Bluccino v%s\n",BL_VERSION);
    bl_log(0,BL_R "%s" BL_0,msg);      // print hello message in red
  }
