//==============================================================================
// bl_hook.c - hooking up in Zephyr Bluetooth mesh stack
//            copyright(c): Bluenetics 2021, Hugo Pristauz (2021-Nov-16)
//==============================================================================

#include "bluccino.h"
#include "bl_hook.h"


   typedef struct net_buf_simple FL_nbs;

//==============================================================================
// pull & trace payload
//==============================================================================

  static uint32_t payload(uint8_t len, uint8_t *pay) // trace payload contents
  {
    KN_byte n = pay[0];
    KN_byte type = pay[1];
    uint16_t mid = pay[2] + (pay[3]<<8);           // manufacturer ID 0x03da
    uint32_t seq = pay[4]|(pay[5]<<8)|(pay[6]<<16)|(pay[7]<<24); // seq counter

    KN_byte sts = pay[8];                          // switch status

    KN_byte sig[4];
    sig[0] = pay[9];
    sig[1] = pay[10];
    sig[2] = pay[11];
    sig[3] = pay[12];

    if ( kn_verbose(3) )
      printk("--> buf[%d], n = %d, type: %d, mid: %04x, seq: %08x,"
             " sts: <%02x>, sig: %02x-%02x-%02x-%02x\n",
             len, n, type, mid, seq, sts, sig[0], sig[1], sig[2], sig[3]);

    return seq;             // return sequence counter
  }

  static uint32_t pull_payload(uint8_t *pay, FL_nbs *buf, int len)
  {
    pay[0] = len-1;

    for (int i=1; i < len; i++) // start from i=1, as we did pay[0] manually
    {
        if (buf->len > 0)
            pay[i] = (buf->len > 0) ? net_buf_simple_pull_u8(buf):0;
        else
            pay[i] = 0;
    }

    return payload(len, pay);         // trace payload
  }

//==============================================================================
// scan callback which debug traces iBeacon packets
//==============================================================================

    // kn_tweak() is directly called from bt_mesh_scan_cb() in
    // adv.c module. Here we tweaked the mesh stack by coming up
    // to the application and checking whether packet is an iBeacon
    // packet

  #define SEQ  0xee181502  // sequence count of iBeacon which we are seeking for

  static void scan_ibeacon_cb(uint8_t typ, const bt_addr_le_t *addr,
                              int8_t rssi, FL_nbs *buf)
  {
//  const BL_u8 *paddr = addr->a.val;  // pointer to Bluetooth address
//printk(" => hook\n");

  	if (typ == 0xff)
    {
      KN_byte pay[13];

      uint32_t seq = pull_payload(pay, buf, sizeof(pay));
      if (seq == SEQ )
      {
        if (kn_verbose(4))
          printk("iBeacon packet received ...\n");

          // notify application

        KN_event event = {KN_SCAN,NULL};
        kn_event(&event,0,rssi);
      }
    }
  }

//==============================================================================
// hook-in scan callback
//==============================================================================

  void bl_hook(BL_hook kind)
  {
    switch (kind)
    {
      case BL_HOOK_SCAN_IBEACON:
        kn_trace(1,BL_C"scanner hook -> scan_ibeacon_cb()\n"BL_0,0,0);
        bl_scan_hook(scan_ibeacon_cb);     // set up hook in scanner
        break;
      default:
        kn_trace(0,BL_R"bad arg: bl_hook\n"BL_0,0,0);
    }
  }
