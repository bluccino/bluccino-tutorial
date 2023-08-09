//==============================================================================
// bl_type.h - bluccino typedefs
//==============================================================================

#ifndef __BL_TYPE_H__
#define __BL_TYPE_H__

  #include <stddef.h>
  #include <stdint.h>
  #include <stdbool.h>
  #include <stdlib.h>

  typedef uint32_t BL_id;              // bluccino message ID

  #define BL_LEN(a)     ((int)(sizeof(a)/sizeof(a[0])))        // array length

    // macros for mesh message identification

  #define BL_OP(mid)    ((BL_op)  ((mid) & 0xffff))
  #define BL_CL(mid)    ((BL_cl)  (((mid) >> 16) & 0xffff))

  #define BL_ID_(cl,op) (BL_id)(((BL_id)(cl)<<16)|BL_HASH(op)) // aug'ed mID

    // similarily we use the concept of "augmented class tags", denoting a kind
    // of internal interface which have the second most significant bit ("aug
    // bit") of the lower 2 byte nibble set. Functions like bl_out will always
    // clear the aug bit before posting

  #define BL_AUGBIT     0x00008000      // or mask to set hash bit
  #define BL_AUGCLR     0x00007FFF      // and mask to clear hash bit

     // macro BL_AUG() sets opcode's hashbit, macro BL_CLEAR() clears opcode's
     // hashbit, BL_AUGED() checks if opcode's hash bit is set

  #define BL_AUG(cl)    ((BL_cl)((uint32_t)(cl)|BL_AUGBIT))
  #define BL_ISAUG(cl)  (((cl) & BL_AUGBIT) != 0)
  #define BL_UNAUG(cl)  ((BL_cl)((uint32_t)(cl)&BL_AUGCLR)) // clear AUG bit

    // macros for mesh message identification

  #define BL_ID(cl,op)  (BL_id)(((BL_id)(cl)<<16)|(op))         // message ID
  #define _BL_ID(cl,op) (BL_id)(((BL_id)BL_AUG(cl)<<16)|(op))   // aug'ed mID

    // macro for composition of service ID

  #define BL_SID(op)    BL_ID(_SVC,op)

    // useful macros for min(), max(), abs() and saturation

  #define BL_MAX(x,y)   (((x) > (y)) ? (x) : (y))
  #define BL_MIN(x,y)   (((x) < (y)) ? (x) : (y))
  #define BL_ABS(x)     ((x) < 0    ? -(x) : (x))

  #define BL_SAT(x,min,max) BL_MAX(min,BL_MIN(x,max))

    // get i-th byte of a 64-bit address

  #define BL_BYTE(addr,nmb)  ((uint8_t)((uint64_t)(addr) >> (8*nmb)) & 0xff)

//==============================================================================
// typedefs
//==============================================================================

  typedef uint8_t  BL_byte;             // unsigned 8-bit byte
  typedef uint8_t  BL_by;               // we love short type identifiers :-)
  typedef uint16_t BL_word;             // unsigned 16-bit word
  typedef uint16_t BL_wd;               // we love short type identifiers :-)
  typedef int16_t  BL_short;            // signed 16-bit word

  typedef int8_t   BL_s8;               // we love short type identifiers :-)
  typedef int16_t  BL_s16;              // we love short type identifiers :-)
  typedef int32_t  BL_s32;              // we love short type identifiers :-)
  typedef int64_t  BL_s64;              // we love short type identifiers :-)

  typedef uint8_t  BL_u8;               // we love short type identifiers :-)
  typedef uint16_t BL_u16;              // we love short type identifiers :-)
  typedef uint32_t BL_u32;              // we love short type identifiers :-)
  typedef uint64_t BL_u64;              // we love short type identifiers :-)

  typedef const char *BL_txt;           // short hand for const char pointer
	typedef const void *BL_data;          // Bluccino messaging data reference
  typedef BL_data *BL_args;             // array of BL_data

  typedef const BL_byte *BL_pay;        // pay load (const byte *)
  typedef BL_byte *BL_buf;              // data buffer (byte *)
/*
  typedef struct BL_xmit                // data transmission buffer (TX/RX)
  {
    BL_byte *buf;                       // data buffer pointer
    size_t size;                        // data buffer size
    void *any;                          // auxillary pointer
  } BL_xmit;
*/
    // we define BL_us to represent microseconds since system start or clock
    // restart in 64-bit signed integer representation. This allows
    //
    //     a) negative time stamps to indicate invalid time stamps or elapsed
    //        time arithmetic with positiv/negative results
    //     b) sufficient long time () before overrun
    //        (1 year .= 2^38 us, 2^25 years .= 268000 years = 2^63 us)

  typedef int64_t BL_us;               // micro seconds
  typedef int64_t BL_ms;               // mili seconds
  typedef int64_t BL_sec;              // seconds

  typedef int *BL_pint;                // pointer to int (used in messages)

  typedef enum
  {
      BL_ERR_ANY     = 1100,           // any error
      BL_ERR_BADARG  = 1200,           // bad input arg
      BL_ERR_FAILED  = 1300,           // operation failed
      BL_ERR_MEMORY  = 1400,           // out of memory
  } BL_err;

  typedef struct BL_pace               // tick/tock pace control
          {
            BL_ms period;              // tick/tock period
            BL_ms time;                // tick/tock time
          } BL_pace;                   // tick/tock pace control

  #define BL_PACE(period,time) {period,time}   // initializing aggregate

  typedef struct BL_tray               // data tray to deposit/retrieve data
          {
            void *data;                // data pointer
            size_t size;               // data size
            union
            {
              BL_txt key;              // data key
              void *any;               // auxillary pointer
            };
          } BL_tray;                   // data tray to deposit/retrieve data

  typedef BL_tray BL_dac;              // alias (legacy: data access structure)


  #define BL_LO(x)           ((BL_byte)  ((x) & 0xff))
  #define BL_HI(x)           ((BL_byte)  (((x) >> 8) & 0xff))
  #define BL_HILO(hi,lo)     ((uint16_t) ((((uint16_t)(hi)) << 8) | (lo)))

  #define BL_LB(x)           ((BL_byte)  ((x) & 0xff))
  #define BL_HB(x)           ((BL_byte)  (((x) >> 8) & 0xff))
  #define BL_HBLB(hi,lo)     ((uint16_t) ((((uint16_t)(hi)) << 8) | (lo)))

  #define BL_LW(x)           ((BL_word)  ((x) & 0xffff))
  #define BL_HW(x)           ((BL_word)  (((x) >> 16) & 0xffff))
  #define BL_HWLW(hi,lo)     ((uint32_t) ((((uint32_t)(hi)) << 16) | (lo)))

    // return values and error codes

  #define BL_VOID            (0xF00F00)     // interface is not supported (general)
  #define BL_VOID1           (0xF00F01)     // interface is not supported (interface dispatch)
  #define BL_VOID2           (0xF00F02)     // interface is not supported (interface virtual)
  #define BL_OK                     (0)     // everything OK

  #define BL_ERR                   (-1)     // general error
  #define BL_ERRMSG                (-2)     // unsupported message error
  #define BL_ERRLIB                (-9)     // no library found to support

#endif // __BL_TYPE_H__
