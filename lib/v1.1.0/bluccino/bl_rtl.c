//==============================================================================
//  bl_rtl.c
//  Bluccino real time logging (supporting dongle logging)
//
//  Created by Chintan parmar on 2022-07-01
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================
// How it works
// - by using bl_log() or any LOG() macro messages are sprintf'ed into RTL FIFO,
//   and zephyr work is submitted to a FIFO work horse
// - the FIFO work horse checks for auto-initialization of the COM/USB port
// - if not initialized an auto init is performed which does a busy wait on a
//   USB port connection by a terminal (e.g. screen)
// - once a terminal has been detected execution is continued
// - optionally busy waiting on a USB port can be disabled (CFG_COM_PORT_WAIT=0)
//==============================================================================
// RTL methods: allocate an RTL segment, sprintf .., finally submit
// - usage:
//     int id = bl_rtl_alloc()         // get (allocate) an RTL segment
//     if (id)                         // if allocation was successful
//     {
//       BL_byte off = 0;              // print offset in buffer (initially 0)
//       off += sprintf(bl_rtl(id,off),"stuff (id:%d)",id);
//       off += sprintf(bl_rtl(id,off),"\n");
//       bl_rtl_submit(id,off);          // submit RTL segment to print FIFO
//     }
// - note: bl_rtl_submit implicitely frees up the allocated RTL segment
//==============================================================================

  #include <assert.h>
  #include <usb/usb_device.h>
  #include <drivers/uart.h>

  static void now(int *pmin, int *psec, int *pms, int *pus);  // split us time

//==============================================================================
// COM port support
// - we have to set symbol CFG_COM_PORT_INIT according to the necessity of
//   initializing a COM port
//==============================================================================

#ifndef CFG_COM_PORT_INIT
  #ifndef CONFIG_USB_DEVICE_DRIVER
    #define CONFIG_USB_DEVICE_DRIVER 0 // default 0 if not provided
  #endif

  #if (CONFIG_USB_DEVICE_DRIVER)
    #define CFG_COM_PORT_INIT   1      // yes, init COM port
  #else
    #define CFG_COM_PORT_INIT   0      // no need to init COM port
  #endif
#endif

#ifndef CFG_COM_PORT_WAIT
  #define CFG_COM_PORT_WAIT     1      // COM port poll is activated by default
#endif

#ifndef CFG_RTL_PRINT_DELAY
  #define CFG_RTL_PRINT_DELAY   0      // no RTL print delay by default
#endif

#ifndef CFG_RTL_DEBUG
  #define CFG_RTL_DEBUG         0      // no RTL debug by default
#endif

#ifndef CFG_RTL_VERBOSE
  #define CFG_RTL_VERBOSE       0      // RTL verbose level by default
#endif

//==============================================================================
// logging shorthands
//==============================================================================

  enum RTL_LOG {RTL_INIT,RTL_WORKER,RTL_ALLOC,RTL_SUBMIT,RTL_LOCATE,
                RTL_REDUCE,RTL_NOW,RTL_FETCH,RTL_FREE};

  #if (CFG_RTL_DEBUG)
    static void rtl_log(int lvl, int mode, BL_txt who, int a1, int a2, int a3);
    static int rtl_debug = CFG_RTL_VERBOSE;// RTL debug level

    #define LOG(lvl,mode,who,a1,a2,a3)    rtl_log(lvl,mode,who,a1,a2,a3)
  #else
    #define LOG(lvl,mode,who,a1,a2,a3)     // empty
  #endif

//==============================================================================
// forward declaration of COM port init
//==============================================================================

  static void rtl_init(void);

//==============================================================================
// COM port init: initializes COM port for real time logging.
//==============================================================================
#if CFG_COM_PORT_INIT

  static void com_port_init(void)
  {
    if (usb_enable(NULL))
      return;

      // poll if the DTR flag was set

    #if (CFG_COM_PORT_WAIT)
      const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
      uint32_t dtr = 0;

      while (!dtr)
      {
        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        bl_sleep(100);                 // give CPU resource to low prio threads
      }
    #endif

    //bl_log(4,BL_B "USB/COM port init");
  }

#endif // CFG_COM_PORT_INIT
//==============================================================================
// log fifo
// - CFG_BLUCCINO_RTL holds the value of the RTL print buffer size
//
//       head              size            head           size
//    |------------>|<--------------->|------------>|<-------------->|
//    +----------------------------------------------------------------...--+
//    |size|off|lock|********\0_______|size|off|lock|*********\0_____| ...  |
//    +----------------------------------------------------------------...--+
//    ^                               ^   SEGHEAD   |   off     ^    ^
//    |                               |------------>|---------->|    |
//    |                               |                              |
//    |              id               |             skip             |
//    |------------------------------>|----------------------------->|
//   buf                            buf+id
//==============================================================================

  #define SEGSIZE   200                // maximum RTL segment net length

  #if (SEGSIZE > 255)
    #error SEGSIZE                     // must not exceed 255
  #endif

    // RTL buffer size should be greater than or equal a minimum buffer size

  #define MINLEN (5*SEGSIZE)            // minimum length for RTL buffer

  #if (CFG_BLUCCINO_RTL < MINLEN)
    #define BUFLEN MINLEN+1            // actual RTL print buffer size
  #else
    #define BUFLEN CFG_BLUCCINO_RTL+1  // actual RTL print buffer size
  #endif

    // dimensioning defined and checked now - let's proceed with typedefs

  typedef struct BL_seg                // RTL segment
  {
    BL_byte size;                      // size of segment char buffer
    BL_byte off;                       // print offset
    bool lock;                         // segment is locked (or not)
  } BL_seg;

  typedef struct BL_rtl
  {
    char buf[BUFLEN];                  // RTL buffer
    volatile BL_short put;             // segment put index
    volatile BL_short get;             // segment get index
    volatile BL_short count;           // number of segments in RTL fifo
    volatile BL_short drops;           // number of message drops
    volatile bool spool;               // printer spooler active?
  } BL_rtl;

  static BL_rtl rtl = {put:1, get:1, count:0, drops:0};

    // define SEGSIZE as size of segment header + size of buffer
    // remember:
    //      head = SEGHEAD      size = SEGLEN
    //    |<----------------->|<------------------->|
    //    +-----------------------------------------+
    //    | size | off | lock |************\0_______|
    //    +-----------------------------------------+
    //    |       SEGSIZE = SEGHEAD + SEGLEN        |
    //    |---------------------------------------->|

  #define SEGHEAD sizeof(BL_seg)
  #define SEGLEN (SEGSIZE-SEGHEAD)

//==============================================================================
// segment IDs - comprise a segment index and a segment offset
// - usage: id = BL_RTL_ID(idx,off)
//          idx = BL_RTL_IDX(id)
//          off = BL_RTL_OFF(id)
//==============================================================================

  #define BL_RTL_ID(idx,off)   BL_HWLW(idx,off)
  #define BL_RTL_IDX(id)       BL_HW(id)
  #define BL_RTL_OFF(id)       BL_LW(id)

//==============================================================================
// helper: wrap around if segment does not fit
// - usage: rtl.get = wrap(rtl.get)
//          rtl.put = wrap(rtl.put)
//==============================================================================

  static BL_short wrap(BL_short idx)
  {
    return (idx + SEGSIZE > BUFLEN) ? 1 : idx;
  }

//==============================================================================
// helper: move get or put index forward by seg size (occasionally wrap around)
// - usage: rtl.get = move(rtl.get, &seg)
//          rtl.put = move(rtl.put, &seg)
//==============================================================================

  static BL_short move(BL_short idx, BL_seg *s)
  {
    idx += (SEGHEAD + s->size);        // move index forward by segment size
    return wrap(idx);                   // occasionally wrap around index
  }

//==============================================================================
// helper: get pointer RTL segment, given segment ID
// - usage: pseg = segment(id)
//==============================================================================

  static BL_seg *segment(int id)
  {
    BL_short idx = BL_RTL_IDX(id);
	  char *p = rtl.buf + idx;           // points to begin of segment
	  return (BL_seg*)p;                 // cast to segment pointer
  }

//==============================================================================
// helper: locate char pointer to RTL segment (id) with respect to offset (off)
// - usage: BL_txt p = locate(id,off)
//==============================================================================

  static char *locate(int id, BL_byte off)
  {
    BL_short idx = BL_RTL_IDX(id);
    BL_byte skip = SEGHEAD + BL_RTL_OFF(id);
    BL_short total = idx + skip + off;

    LOG(3,RTL_LOCATE,"locate", id,off,total);
	  return (rtl.buf + total);           // points to char buf @ (id,off)
  }

//==============================================================================
// RTL method: alloc RTL segment (allocate a segment)
// - usage: id = rtl_alloc() // id = 0 if no free segment
//==============================================================================

  static int rtl_alloc(void)
  {
    int id = 0;                   // invalid ID by default

      // first action is to wrap around put index if put > get and
      // segment at current put location would exceed buffer size

    bl_irq(0);                         // enter critical region - interrupts off
    {
      if (rtl.put >= rtl.get)          // safety operation (redundant)
        rtl.put = wrap(rtl.put);       // wrap around put index

        // next case is to allocate segment:
        // 1) for put >= get the segment fit must be within buffer end
        // 2) for put < get the segment fit must be within get index

      if (rtl.put >= rtl.get)          // put >= get: fit segment within buf end
      {
        if (rtl.put+SEGSIZE <= BUFLEN) // fit within buffer end
        {
          id = BL_RTL_ID(rtl.put,0);   // allocation successful - assign id
          rtl.put += SEGSIZE;          // move to next put position
        }
      }
      else                             // put < get: fit segment within get idx
      {
        if (rtl.put + SEGSIZE <= rtl.get)
        {
          id = BL_RTL_ID(rtl.put,0);   // allocation successful - assign id
          rtl.put += SEGSIZE;          // move to next put position
        }
      }

        // alloc has been done and was either successful (id >= 0) or failed,
        // but there is still house keeping required

	    if (id)                          // in case of a successful allocation
	    {
	      BL_seg *s = segment(id);       // cast to segment pointer
	      s->size = SEGLEN;              // init print buffer size of segment
        s->off = 0;                    // init print offset of segment
	      s->lock = true;
        rtl.count++;                   // one more segment allocated
	    }
    }
    bl_irq(1);                         // exit critical region - interrupts on

    LOG(2,RTL_ALLOC, "alloc",id,0,0);
    return id;
  }

//==============================================================================
// RTL method: submit RTL segment for printing
// - usage: rtl_submit(id,off) // no action/error if id == 0
//==============================================================================

  static void rtl_submit(int id, BL_byte off) // submit segment to RTL FIFO
  {
    if (!id)                           // ignore call in case of invalid ID
      return;

    BL_short idx, head, free, tail;
    BL_seg *s;
/*
int ID = BL_RTL_ID(BL_RTL_IDX(id),0);
int offset = BL_RTL_OFF(id);
s = segment(ID);         // cast to segment pointer
int next = BL_RTL_IDX(id) + SEGHEAD + off + offset + 1;
for(int i=0;i<3;i++)
  bl_printf("\n<%03d|%03d> @%d(%03d): "BL_Y"%s\n"BL_0, BL_HW(id),BL_LW(id),s->off,next, bl_rtl(ID,0));
*/
    bl_irq(0);                         // enter critical region - interrupts off
    {
      s = segment(id);         // cast to segment pointer
      s->off = BL_RTL_OFF(id) + off;

      idx = BL_RTL_IDX(id);
      head = idx + SEGHEAD;
      free = head + s->off + 1;        // free location for next segment
      tail = head + s->size;           // tail of actual segment
    }
    bl_irq(1);                         // exit critical region - interrupts on


    LOG(2,RTL_SUBMIT,"submit", id,off, 0);

    bool reduce = false;
    bl_irq(0);                         // enter critical region - interrupts off
    {
      if (rtl.put == tail)             // if no other task has booked a segment
      {
        rtl.put = free;
        s->size = s->off + 1;          // reduce segment size (+1 for '\0' term)
        reduce = true;
      }

//    s->lock = false;                 // segment is now ready for printing
    }
    bl_irq(1);                         // exit critical region - interrupts on

//int next = BL_RTL_IDX(id)+SEGHEAD+s->size;
//for(int i=0;i<2;i++)
//  bl_printf("\n<%03d|%03d> @%d(%03d): "BL_Y"%s\n"BL_0, BL_HW(id),BL_LW(id),s->off,next, bl_rtl(ID,0));

    bl_irq(0);                         // enter critical region - interrupts off
	  {
      s->lock = false;                 // segment is now ready for printing
    }
    bl_irq(1);                         // exit critical region - interrupts on

    #if (CFG_RTL_DEBUG)
      if (!reduce)
        bl_printf("*** warning: RTL segment size not reduced\n");
      else
        LOG(2,RTL_REDUCE,"reduce", id,off, 0);
    #endif
  }

//==============================================================================
// work horse tasks, to fetch RTL segment from FIFO and print
// if (rtl_avail())
// {
//   int id = rtl_fetch();             // fetch RTL buffer and set lock flag
//   BL_txt p = locate(id,0);          // locate character buffer to be printed
//   bl_printf("%s",p);                // print RTL segment contents to RTOS
//   rtl_free(id);                     // free RTL segment
// }
//==============================================================================

//==============================================================================
// RTL method: are there RTL buffers rtl_available for printing?
// - usage: ok = rtl_avail()               // return boolean value of availability
//==============================================================================

  static bool rtl_avail(void)
  {
    if (rtl.count == 0)
      return 0;

      // we can assert here: rtl.count > 0
      // next step to check is whether next segment to be fetched is locked

    bool locked;
    bl_irq(0);                         // enter critical region - interrupts off
    {
	    BL_seg *s = segment(rtl.get);    // retrieve pointer to next segment
	    locked = s->lock;                // is next segment locked?
	  }
    bl_irq(1);                         // exit critical region - interrupts on

    return !locked;                    // next segment available if not locked
  }

//==============================================================================
// RTL method: fetch next RTL segment, if available
// - usage: id = fetch()               // return segment ID (or error: id=0)
//==============================================================================

  static int rtl_fetch(void)
  {
    if (!rtl_avail())
      return 0;                        // no more RTL segments in FIFO

    int id = 0;
    bool locked;
    bl_irq(0);                         // enter critical region - interrupts off
    {
      id = BL_RTL_ID(rtl.get,0);       // ID of next segment in RTL FIFO
	    BL_seg *s = segment(id);         // retrieve segment pointer
      locked = s->lock;                // lock status before segment locking
      s->lock = true;                  // lock segment
    }
    bl_irq(1);                         // exit critical region - interrupts on


    LOG(1,RTL_FETCH,"fetch",id,0,0);
    return locked ? 0 : id;            // return ID depending on lock status
  }

//==============================================================================
// RTL method: free RTL segment (reverse operation to rtl_alloc())
// - usage: rtl_free(id) // return segment ID (or error: -1)
//==============================================================================

  static int rtl_free(int id)
  {
    if (!id)
      return -1;                       // ignore if invalid segment ID

    bl_irq(0);                         // enter critical region - interrupts off
    {
	    BL_seg *s = segment(id);         // get segment pointer by ID
      rtl.get = move(rtl.get,s);       // move get index forward or wrap around

        // for debugging we clear lock,size,off fields (redundant operations)

      s->lock = false;                 // unlock segment (redundant operation)
      s->size = 0;                     // clear size (redundant operation)
      s->off = 0;                      // clear offset (redundant operation)

      rtl.count--;                     // one less allocated segment
    }
    bl_irq(1);                         // exit critical region - interrupts on

    LOG(1,RTL_FREE,"free", id,0,0);
    return 0;                          // ok
  }

//==============================================================================
// RTL method: increase the log drop counter
// - usage: drops = drop(0)            // read number of drops and clear drops
//          drop(1)                    // increment drops counter
//==============================================================================

  static BL_short rtl_drop(BL_byte mode)
  {
    int drop_cnt;

    bl_irq(0);
    {
      if ( !mode )                     // read number of drops and clear drops
      {
        drop_cnt = rtl.drops;
        rtl.drops = 0;
      }
      else                             // increment drops counter
      {
        rtl.drops++;
        drop_cnt = rtl.drops;
      }
    }
    bl_irq(1);

    return drop_cnt;
  }

//==============================================================================
// print work horse - send fifo logs data bl_prt
// - K_WORK_DEFINE(rtl_work,rtl_worker); // assign print work with work horse
// - mind that we need to do occasionally auto initializing of comport
//==============================================================================

  static void rtl_worker(struct k_work *work)
  {
    static bool initialized = false;

    bool spool = false;
    bl_irq(0);
    {
      if (!rtl.spool)
        spool = rtl.spool = true;
    }
    bl_irq(1);

    if (!spool)                 // no spooler job for me, since
      return;                   // a colleague task is already spooling

bl_printf(BL_R"spooling on (#%d)\n"BL_0, rtl.count);

      // in case of USB output: auto initialize

    if (!initialized)
    {
      initialized = true;
      #if (CFG_COM_PORT_INIT)
        com_port_init();               // initializing in case of COM ports
      #endif                           // e.g. logging from dongles to COM port
    }

    LOG(1,RTL_WORKER,"worker", 0,0,0);

    int drops = rtl_drop(0);           // read number of drops and clear drops

    if (drops)
      bl_printf(BL_R"*** RTL: %d messages dropped\n"BL_0,drops);

    while (rtl_avail())                // number of available log msg's in fifo
    {
      #if (CFG_RTL_PRINT_DELAY)
        bl_sleep(CFG_RTL_PRINT_DELAY);
      #endif

      int id = rtl_fetch();
      if (id)
      {
        BL_txt p = bl_rtl(id,0);
        #if (CFG_RTL_DEBUG)
          bl_irq(0);
        #endif
        bl_printf("%s",p);             // print directly to RTOS
        #if (CFG_RTL_DEBUG)
          bl_irq(1);
        #endif
        rtl_free(id);
      }
    }

bl_printf(BL_R"spooling off\n"BL_0);
    rtl.spool = false;                // done with spooling
  }

  static K_WORK_DEFINE(rtl_work, rtl_worker);  // assign work  with work horse

  static void rtl_work_init(void)
  {
    k_work_init(&rtl_work, rtl_worker);
  }

//==============================================================================
//submit RTL segment to print fifo
// - usage: bl_rtl_submit(id,off) // no action/error if id == 0
//==============================================================================

  void bl_rtl_submit(int id, BL_byte off)
  {
    static BL_ms blackout = 0;
    BL_ms now = bl_ms();

    rtl_submit(id,off);

    if (!blackout)
    {
      k_work_submit(&rtl_work);        // continue at button_rtl_worker()
      blackout = now + 100;            // activate 100 ms blackout phase
    }

    if (blackout && now >= blackout)
      blackout = 0;                    // clear blackout phase
  }

//==============================================================================
// alloc RTL segment (auto init module and allocate a segment)
// - usage: id = bl_rtl_alloc()        // id == 0 if no free segment
//==============================================================================

  int bl_rtl_alloc(void)
  {
    static bool init = false;

	  if (!init)                         // in case of USB output: auto initialize
	  {
		  rtl_init();                      // initializing in case of COM ports
		  init = true;
	  }

    int id = rtl_alloc();              // actual allocation of RTL segment

    if (!id)                           // cannot allocate RTL segment?
      rtl_drop(1);                     // increment drops counter

    return id;
  }

//==============================================================================
// locate char pointer to RTL segment (id) with respect to offset (off)
// - usage: BL_txt p = bl_rtl(id,off) // locate p @ offset off
//==============================================================================

  char *bl_rtl(int id,BL_byte off)     // actual buffer pointer for sprintf()
  {
    return locate(id,off);             // locate character pointer @ (id,off)
  }

//==============================================================================
// log time stamp for current time (now - in ms:us)
// - Bluccino bl_now() function to be used if Bluccino RTL is activated
// - usage: if (bl_now(lvl)) ...
//          id = bl_now(lvl+BL_RTL_SUSPEND);
// - if lvl arg is increased by constant BL_RTL_SUSPEND it means that the RTL
//   segment will be suspended and not immediately being printed
//==============================================================================

  int bl_now(int lvl)
  {
    static char space[] = "                    ";
    bl_assert(sizeof(space)==10*2+1);

    bool suspend = (lvl >= BL_RTL_SUSPEND/2);
    lvl = suspend ? lvl-BL_RTL_SUSPEND : lvl;

    if (lvl > debug)
      return 0;

    int min, sec, ms, us;
    now(&min,&sec,&ms,&us);

      // prepare indentation: truncate lvl to interval [0,10] and set
      // pointer to indentation

    lvl = BL_SAT(lvl,0,10);            // saturate lvl to interval [0,10]
    BL_txt indent = space + (10-lvl)*2;

      // print header in green if in attention mode,
      // yellow if node is provisioned, otherwise white by default

    int id = bl_rtl_alloc();
    BL_byte off = 0;
    off += sprintf(bl_rtl(id,off), "%s#%d[%03d:%02d:%03d.%03d] %s" BL_0,
                          color,lvl, min,sec,ms,us, indent);

      // we have to update id by incorporating offset

    BL_short idx = BL_RTL_IDX(id);
    id = BL_RTL_ID(idx,off);

    LOG(3,RTL_NOW,"now", id,0,0);

    if (!suspend)
      bl_rtl_submit(id,off);

    return id;
  }

//==============================================================================
// log messages
// - RTL bl_logo() function, used if RTL is not activated
//==============================================================================

  int bl_logo(int lev, BL_txt msg, BL_ob *o, int value) // log event message
  {
    int id = bl_now(lev);              // print time stamp if proper log level

    if ( !id )
     return;                           // return if unproper log level

    BL_txt aug = BL_ISAUG(o->cl) ? "#" : "";
    BL_cl cl = BL_UNAUG(o->cl);

    BL_txt col = (msg[0] != '@') ? "" : (value ? BL_G : BL_M);
    msg = (msg[0] == '@') ? msg+1 : msg;

    int ix = bl_ix(o);
    BL_byte off = 0;

    #if CFG_LOG_PRETTY_PRINTING             // pretty text for class tag & opcode
      if (ix > 0 && BL_HW(ix))
        off += sprintf(bl_rtl(id,off),"%s%s [%s%s:%s @<%s|%s>,%d]\n"BL_0, col,
                 msg, aug,cltext(cl), optext(o->op), BL_IDTXT(ix),value);
      else
        off += sprintf(bl_rtl(id,off),"%s%s [%s%s:%s @%d,%d]\n"BL_0, col,msg,
                 aug,cltext(cl), optext(o->op), bl_ix(o),value);
    #else
      off += sprintf(bl_rtl(id,off),"%s%s [%s%d:%d @%d,%d]\n"BL_0,col,msg,
              aug,cl, o->op, bl_ix(o),value);
    #endif
    bl_rtl_submit(id,off);
    return 0;
  }

//==============================================================================
// debugging print of current RTL fifo contents
//==============================================================================
#if (CFG_RTL_DEBUG)

  static void rtl_log(int lvl, int mode, BL_txt who, int a1,int a2,int a3)
  {
    if (lvl > rtl_debug)
      return;

    int id = a1;                     // short hand
    int idx = BL_RTL_IDX(id);          // short hand
    int off = BL_RTL_OFF(id);          // short hand
    BL_seg *s;
    char c = '|';

    switch (mode)
    {
      case RTL_INIT:
        bl_irq(0);
        {
          bl_printf("  rtl_%s: RTL FIFO: size: %d(%d), g/p: %d/%d, c/d: %d/%d"
            " (seg H/L/S: %d/%d/%d)\n",  who, sizeof(rtl.buf)-1,sizeof(rtl),
            rtl.get, rtl.put, rtl.count,rtl.drops, SEGHEAD, SEGLEN, SEGSIZE);
        }
        bl_irq(1);
        break;

      case RTL_ALLOC:  // a1 = id
      case RTL_FETCH:
      case RTL_FREE:
        s = segment(id);  c = s->lock ? '|' : ':';
        bl_irq(0);
        {
          bl_printf("    rtl_%s: <%d%c%d> @ %d,  g/p: %d/%d, s/o: %d/%d, c/d: %d/%d\n",
            who, idx,c,off,s->off, rtl.get,rtl.put,
            s->size,s->off, rtl.count,rtl.drops);
        }
        bl_irq(1);
        break;

      case RTL_NOW:  // a1 = id, a2 = off
        s = segment(id);  c = s->lock ? '|' : ':';
        bl_irq(0);
        {
  			  bl_printf("    rtl_%s: <%d%c%d> @ %d\n", who, idx,c,off,a2);
        }
        bl_irq(1);
 			  break;

      case RTL_LOCATE:  // a1 = id, a2 = off, a3 = total
        s = segment(id);  c = s->lock ? '|' : ':';
        bl_irq(0);
        {
  			  bl_printf("      rtl_%s:%d - <%d%c%d> @ %d, idx:%d, head+off+off: %d+%d+%d\n",
	  		    who,a3, idx,c,off,a2, idx, SEGHEAD,off,a2);
        }
        bl_irq(1);
			  break;

      case RTL_SUBMIT:  // a1 = id, a2 = off
        s = segment(id);  c = s->lock ? '|' : ':';
        bl_irq(0);
        {
          bl_printf("  rtl_%s: <%d%c%d> @ %d, g/p: %d/%d, s/o: %d/%d, c/d: %d/%d\n",
            who, idx,c,off,a2, rtl.get,rtl.put,
            s->size,s->off, rtl.count,rtl.drops);
        }
        bl_irq(1);
        break;

      case RTL_REDUCE:  // a1 = id, a2 = off
        s = segment(id);  c = s->lock ? '|' : ':';
        bl_irq(0);
        {
          bl_printf("    rtl_%s: <%d%c%d> @ %d, g/p: %d/%d, s/o: %d/%d, c/d: %d/%d\n",
            who, idx,c,off,a2, rtl.get,rtl.put,
          s->size,s->off, rtl.count,rtl.drops);
        }
        bl_irq(1);
        break;

      case RTL_WORKER:
        bl_irq(0);
        {
          bl_printf("  rtl_%s: g/p: %d/%d, c/d: %d/%d\n",
            who, rtl.get,rtl.put, rtl.count,rtl.drops);
        }
        bl_irq(1);
        break;
    }
  }

#endif // (CFG_RTL_DEBUG)
//==============================================================================
// initializes real time logging
//==============================================================================

  static void rtl_init(void)
  {
    rtl_work_init();
    LOG(1,RTL_INIT,"init",0,0,0);
    // bl_prt("rtl_init\n");
  }
