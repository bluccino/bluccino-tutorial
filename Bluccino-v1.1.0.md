--------------------------------------------------------------------------------
# Bluccino v1.1.0
--------------------------------------------------------------------------------

* migration to Nordic NCS v2.1.3
* QEMU examples
* semaphore support
* new types BL_by, BL_wd, BL_pay and BL_buf
* migration of bl_rtos.h (include path with zephyr/prefix, move #include <irq.h>
  from bl_run.c to bl_rtos.h)
* now lessons/01-quicktour/01-hello works based on Segger RTT
* also zephyr/01-basic/01-hello_world-v3.2.0 works on nRF840DK if configured
  for RTT
* run zephyr v3.2.0 samples hello_world on both QEMU and nRF840 DK. Using the
  boards directory and providing an nrf52dk_nrf52832.conf and
  nrf52840dk_nrf52840.conf file which adds Segger RTT config settings
  specifically for the particular boards. By default Segger RTT is not
  configured, as it is required for the qemu_cortex_m3 test environment :-)))
* found out that NCS v2.1.3 can be used (NCS v2.2.0 has issues with mesh ADV)
* bug fixed: nasty Segger RTT suppression (fixed introduction of log fifo)
* button debouncing (bl_hwstd)
* multi click and combined click/hold feature for buttons
* simplify bl_hwbut in tiny hardware core, supporting  the following functions
  1) using DT convenience API
  2) can deal with 1 button (like Nordic 840 dongle) or 4 buttons (Nordic 832/
     840 boards)
  3) debounced
  4) demonstration of user data access in ISR using CONTAINER_OF macro
* all BLE apps registering services in main()
* refactoring tiny BTcore, updating BLE stuff samples
* tiny BTcore still supporting one BLE service (multiple BLE services not yet
  supported)


# Important Notes

* we saw issues with NCS (Nordic nRF Connect) deployments later than
  NCS v2.1.3. Current Bluccino samples were tested with

     Zephyr commit 71ef669ea4a73495b255f27024bcd5d542bf038c

  according to NCS v2.1.3 release notes:

     https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.1.3/nrf/releases/release-notes-2.1.3.html#zephyr

  if you're facing issues with NCS/Zephyr deployments later than NCS v2.1.3
  (especially mesh samples), step back to a Zephyr deployment used for
  Bluccino v1.1.0.
