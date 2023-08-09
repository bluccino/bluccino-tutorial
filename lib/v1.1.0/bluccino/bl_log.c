//==============================================================================
//  bl_log.c
//  bluccino debug logging
//
//  Created by Hugo Pristauz on 2021-11-19
//  Copyright © 2021 Bluenetics GmbH. All rights reserved.
//==============================================================================

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "bluccino.h"

//==============================================================================
// locals
//==============================================================================

  static BL_txt color = "";            // text color for time header
  static int debug = 4;                // debug level

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
// log helper
//==============================================================================
#if CFG_LOG_PRETTY_PRINTING

  BL_txt bl_cltxt(BL_cl cl)
  {
    static BL_txt text[] = BL_CL_TEXT;
    bool aug = BL_ISAUG(cl);
    cl = BL_UNAUG(cl);

    if (!aug)
      return (cl < BL_LEN(text)) ? text[cl] : (cl==_VOID ? "VOID" : "???");
    else
    {
      static char buf[25];
      sprintf(buf,"#%s",bl_cltxt(cl));
      return buf;
    }
  }

  BL_txt bl_optxt(BL_op op)
  {
    static BL_txt text[] = BL_OP_TEXT;
    return (op < BL_LEN(text)) ? text[op] : (op==VOID_ ? "VOID" : "???");
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
// log fifo data structures
//==============================================================================
#if (CFG_LOG_SPOOLER)

  #define LOGFIFO_LEN CFG_LOG_FIFO_LEN

  typedef char BL_logbuf[CFG_LOG_BUF_LEN];

  typedef struct BL_logfifo
  {
    BL_logbuf buffers[LOGFIFO_LEN];    // array of log buffers in fifo
    int gdx;                           // get-index
    int pdx;                           // put-idx
    volatile int avail;                // available log buffers in log fifo
    volatile int dropped;              // number of logs being dropped
  } BL_logfifo;

  static BL_logfifo fifo = {.gdx=0, .pdx=0, .avail=0, .dropped=0};

#endif
//==============================================================================
// log fifo access functions
//==============================================================================
#if (CFG_LOG_SPOOLER)

  static K_MUTEX_DEFINE(fifo_mutex);

  static int fifo_get(BL_logbuf *p)
  {
    if (k_mutex_lock(&fifo_mutex,K_MSEC(500)))
    {
      bl_prt(BL_R "*** log fifo get: mutex issue\n" BL_0);
      return -2;
    }

    if (fifo.avail == 0)
    {
      k_mutex_unlock(&fifo_mutex);
      return -1;
    }

    BL_logbuf *q = fifo.buffers + fifo.gdx;

    #if (CFG_LOG_DEBUG >= 2)
      bl_prt("get: gdx=%02d, pdx=%02d, avail=%d: '%s'\n",
             fifo.gdx, fifo.pdx, fifo.avail, *q);
    #endif

    memcpy(p,q,sizeof(BL_logbuf));
    fifo.avail--;
    fifo.gdx = (fifo.gdx + 1) % LOGFIFO_LEN;

    k_mutex_unlock(&fifo_mutex);
    return 0;
  }

  static int fifo_put(BL_logbuf *p)
  {
    if (k_mutex_lock(&fifo_mutex,K_MSEC(500)))
    {
      bl_prt(BL_R "*** log fifo put: mutex issue\n" BL_0);
      return -2;
    }

    if (fifo.avail >= LOGFIFO_LEN)
    {
      fifo.dropped++;
      k_mutex_unlock(&fifo_mutex);
      return -1;
    }

    BL_logbuf *q = fifo.buffers + fifo.pdx;

    #if (CFG_LOG_DEBUG >= 2) // this log can screw-up message order
      bl_prt("put: gdx=%02d, pdx=%02d, avail=%d: '%s'\n",
              fifo.gdx, fifo.pdx, fifo.avail, *p);
    #endif

    memcpy(q,p,sizeof(BL_logbuf));
    fifo.avail++;
    fifo.pdx = (fifo.pdx + 1) % LOGFIFO_LEN;

    k_mutex_unlock(&fifo_mutex);
    return 0;
  }

  static int fifo_avail(void)
  {
    return fifo.avail;
  }
/*
  static int fifo_free(void)
  {
    return LOGFIFO_LEN - fifo.avail;
  }
*/
  static int fifo_dropped(void)
  {
    if (!fifo.dropped)
      return 0;

    int dropped;

    k_mutex_lock(&fifo_mutex,K_FOREVER);
    dropped = fifo.dropped;
    fifo.dropped = 0;
    k_mutex_unlock(&fifo_mutex);

    return dropped;
  }

#endif // CFG_LOG_SPOOLIMG
//==============================================================================
// log spooler
//==============================================================================
#if (CFG_LOG_SPOOLER)

  static volatile bool spooler_active = false;

  static void spooler_worker(struct k_work *work)
  {
    spooler_active = true;

    if (CFG_LOG_DELAY > 0)
      BL_SLEEP(CFG_LOG_DELAY);

    #if (CFG_LOG_DEBUG >= 1)
      bl_prt(BL_Y"log spooler start (%d messages)\n" BL_0, fifo.avail);
    #endif

    while ( fifo_avail() )
    {
      int dropped = fifo_dropped();  // any log messages dropped?

      if (dropped)
        bl_prt(BL_R "*** %d message%s dropped\n" BL_0, dropped, dropped?"s":"");

      static BL_logbuf buf;
      fifo_get(&buf);
      bl_prt("%s\n" BL_0,buf);

      if (CFG_LOG_DELAY > 0)
        BL_SLEEP(CFG_LOG_DELAY);

        // do a number of initial delays according to configuration
        // this has proven to be necessary, since Segger RTT screws up
        // log output during initial phase

      static int count = CFG_LOG_INITIAL_DELAYS;
      if (count > 0)
      {
        count--;
        bl_sleep(4*CFG_LOG_DELAY);
        #if (CFG_LOG_DEBUG >= 2)
	        bl_prt("bl_log init delay: %d ms\n" BL_0,4*CFG_LOG_DELAY);
	      #endif
      }
    }

    #if (CFG_LOG_DEBUG >= 1)
      bl_prt(BL_Y"log spooler stop\n" BL_0);
    #endif
    spooler_active = false;
  }

  K_WORK_DEFINE(spooler_work, spooler_worker);

  static void run_spooler(void)
  {
    if ( !spooler_active )
      k_work_submit(&spooler_work);
  }

#endif // CFG_LOG_SPOOLIMG
//==============================================================================
// general log function with spoooler
//==============================================================================
#if (CFG_LOG_SPOOLER)

  static K_MUTEX_DEFINE(log_mutex);

  static int prepare_header(int lvl, char *buf, int len)
  {
    int min, sec, ms, us;
    now(&min,&sec,&ms,&us);

    int n = snprintf(buf,len,"%s#%d[%03d:%02d:%03d.%03d] " BL_0,
                     color,lvl, min,sec,ms,us);

      // n can be negative (err) or >= len !

    n = BL_MIN(n,len-1);

      // indent by 2*lvl spaces

    if (n >= 0 && n + 2*lvl < len+1)
    {
      for (int i=0; i < lvl; i++)
      {
        sprintf(buf+n,"  ");                   // indentation
        n += 2;
      }
    }

    return n;
  }

  int bl_log(int lvl, const char *fmt,...)
  {
    if (lvl > debug)
      return 0;

      // since buf is static and thus not re-entrant, buf is a shared resource
      // and we must secure the following code as a mutual exclusive region

    if (k_mutex_lock(&log_mutex,K_MSEC(500)))
    {
      bl_prt(BL_R "*** bl_log: mutex issue\n" BL_0);
      return -2;
    }
    else
    {
      static BL_logbuf buf;

        // prepare header in buffer andreturn the number of characters which
        // have been used for the header

      int len = sizeof(buf)/sizeof(char);        // length of buffer
      int used = prepare_header(lvl,buf,len);    // returns nmb. of used chars

        // now print varargs into log buffer, which is prepared with header

      va_list ap;
      va_start(ap,fmt);
      vsnprintf(buf+used, len-used, fmt, ap);
      va_end(ap);

        // log buffer is complete now, so put it into the fifo

      fifo_put(&buf);
    }
    k_mutex_unlock(&log_mutex);

      // activate spooler (if not active)

    run_spooler();

    static int count = CFG_LOG_INITIAL_DELAYS;
    if (count > 0)
    {
      count--;
      bl_sleep(5*CFG_LOG_DELAY);
      #if (CFG_LOG_DEBUG >= 2)
        bl_prt("bl_log init delay: %d ms\n" BL_0,5*CFG_LOG_DELAY);
      #endif
    }

    return 0;
  }

#endif // CFG_LOG_SPOOLIMG
//==============================================================================
// simple general log function (without spoooler)
//==============================================================================
#if (!CFG_LOG_SPOOLER)

  int bl_log(int lvl, const char *fmt,...)
  {
    if ( bl_now(lvl) )
	  {
      va_list ap;
	    //bl_prt(fmt BL_0, ##__VA_ARGS__);
      va_start(ap,fmt);
      vprintk(fmt,ap);
      va_end(ap);

	    if (*fmt)
      {
        bl_prt("\n"BL_0);
        if (CFG_LOG_DELAY > 0)
          BL_SLEEP(CFG_LOG_DELAY);
      }
	  }
    return 0;
  }

#endif // !CFG_LOG_SPOOLIMG
//==============================================================================
// set time stamp color
//==============================================================================

  BL_txt bl_color(BL_txt col)
  {
    BL_txt old = color;
    color = col;
    return old;
  }

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
//          cnt = bl_err(2,NULL); // set alternate error log level (black color)
//          cnt = bl_err(1,NULL); // set original error log level (red color)
// - errors are counted, and counter value is returned for NULL msg args
//==============================================================================

  __weak int bl_err(int err, BL_txt msg)
  {
    static int count = 0;
    static int lvl = 1;

    if (msg == NULL)
    {
      lvl = err;
      return count;
    }
    else if (err)
    {
      count++;                         // count errors
      if (bl_now(lvl))                 // errors come @ verbose level 1
        bl_prt("%serror %d: %s\n" BL_0, lvl<=1?BL_R:BL_0, err,msg);
    }
    return err;
  }

//==============================================================================
// warning message: warning printing only for err != 0
// - usage: err = bl_wrn(err,msg)
//          bl_wrn(2,NULL);     // set alternate warning log level (black color)
//          bl_wrn(1,NULL);     // set original warning log level (red color)
// - warnings are counted, and counter value is returned for NULL msg args
//==============================================================================

  __weak int bl_wrn(int err, BL_txt msg)
  {
    static int count = 0;
    static int lvl = 1;

    if (msg == NULL)
    {
      lvl = err;
      return count;
    }
    else if (err)
    {
      count++;                         // count errors
      if (bl_now(lvl))                 // errors come @ verbose level 1
        bl_prt("%swarning %d: %s\n" BL_0, lvl<=1?BL_R:BL_0, err,msg);
    }
    return err;
  }

//==============================================================================
// Generic formatting function
// - usage: txt = bl_fmt("a:%d, b:%s",5,"bingo");  // overwrite buffer
//          txt = bl_fmt("%&additional text");     // add to buffer contents
//          txt = bl_fmt(NULL);                    // just return buffer pointer
//==============================================================================

  BL_txt bl_fmt(const char *fmt, ...)
  {
    static char buf[CFG_LOG_FMT_BUF_SIZE] = {'\0'};

      // first thing is to return pointer to buffer if `fmt` arg equals NULL

    if (fmt == NULL)
      return buf;

      // next is to check for "%&..." type of format string (append to existing)

    char *p = buf;                     // init buffer pointer to begin of buffer
    int len = 0;                       // zero length of current contents

    if (fmt[0] == '%' && fmt[1] == '&')
    {
      fmt += 2;                        // skip head "%&" of format string
      len = strlen(buf);               // determine current buffer contents len
      p += len;                        // set buffer pointer to append position
    }

      // now sprintf into buffer

    va_list ap;
    va_start(ap,fmt);
    vsnprintf(p,BL_LEN(buf)-len,fmt,ap);
    va_end(ap);

    return buf;
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

    return 1;
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
// log messages
// - standard bl_logo() function, used if RTL is not activated
//==============================================================================
#if (!CFG_BLUCCINO_RTL)

  int bl_logo(int lvl, BL_txt msg, BL_ob *o, int value) // log event message
  {
    if (lvl > debug)
      return 0;

    int ix = bl_ix(o);
    BL_txt aug = BL_ISAUG(o->cl) ? "#" : "";
    BL_cl cl = BL_UNAUG(o->cl);

    BL_txt col = (msg[0] != '@') ? "" : (value ? BL_G : BL_M);
    msg = (msg[0] == '@') ? msg+1 : msg;

    #if CFG_LOG_PRETTY_PRINTING             // pretty text for class tag & opcode
      if (ix > 0 && BL_HW(ix))
        bl_log(lvl,"%s%s [%s%s:%s @<%s|%s>,%d]" BL_0, col,msg,
               aug,bl_cltxt(cl), bl_optxt(o->op), BL_IDTXT(ix),value);
      else
        bl_log(lvl,"%s%s [%s%s:%s @%d,%d]" BL_0, col,msg,
               aug,bl_cltxt(cl), bl_optxt(o->op), bl_ix(o),value);
    #else
      bl_log(lvl,"%s%s [%s%d:%d @%d,%d]" BL_0,col,msg,
             aug,cl, o->op, bl_ix(o),value);
    #endif
    return 0;
  }

#endif // !CFG_BLUCCINO_RTL
//==============================================================================
// bl_hello (syntactic sugar to set verbose level and print a hello message)
// - usage: bl_hello(verbose,msg)
//==============================================================================

  void bl_hello(int verbose, BL_txt msg)
  {
    bl_verbose(verbose);               // set verbose level
		bl_prt("*** Bluccino v%s, BOARD: %s\n",BL_VERSION,CONFIG_BOARD);
    bl_log(0,BL_R "%s" BL_0,msg);      // print hello message in red
  }
