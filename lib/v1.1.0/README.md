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


## ToDo
* question: use [BLE.START] instead of [ADV.START]?
* do we need an asyncronous mechanism for [BLE.ENABLE] and [BLE.READY]?
* migrate GPIO samples to Zephyr v3.2.0

## Roadmap
* test of lessons 01/02/03
* test of BLE samples
* complete migration

## Done
* checked: can run onoff sample <v2.7.0> built with <v2.7.0> ZDK  832-board n5
* bug fixed: 01-quicktour/03-button does not toggle intended LED
* onoff samples <n2.2.0> and <v3.2.0> are different
* got mesh samples working <n2.0.0> zephyr version
* issue fixed: cannot run onoff sample mesh sampley :-((( => use NCS v2.1.3
* all /lessons/01-quicktour samples are running
* get all mesh samples running
* migration to n2.1.3 and test framework for lessons complete
* configure QEMU board in CMakeLists.txt, test continuous printk
  output on QEMU
* get printk running on RTT without loss of messages
* 10-ui is now also working with dongle

--------------------------------------------------------------------------------
# Bluccino V1.0.10
--------------------------------------------------------------------------------
* tiny BLE core
* lessons/04-blestuff samples
* Bluccino timer support and work package submission (bl_timer, bl_work)
* Unit test harness

--------------------------------------------------------------------------------
# Bluccino V1.0.9
--------------------------------------------------------------------------------
* bl_dbg() only tests for proper debug level (no more prints time stamp)
* bl_now() takes over role of bl_dgb()
* supporting Bluccino RTL (real time logging) - see 'samples' folder
* [LED:SET -1,onoff] to set all LEDs simultaneously to onoff value
* [LED:TOGGLE -1] to toggle all LEDs simultaneously
* add __struct macro to bl_rtos.h
* tiny BLE core with Eddystone service implemented
* rename BL_packet into BL_packet, rename adv to ap (advertising parameters)
* new wl_core in core/wlcore/wltiny
* bl_ble code implementation for gatt callback registration and authentication
* add service registry mechanism BLE for wireless core
* add library registry mechanism for top gear
* bug fix in tiny wireless core
* extend BL_goo structure with 'acked' flag to distinguish SET/LET initiators
* add lesson/02-messaging samples
* add bl_ix() syntactic sugar
* Bluccino Release V1.0.9

## ToDo
* tiny wireless core with bugs
* investigate notification

--------------------------------------------------------------------------------
# Bluccino V1.0.8
--------------------------------------------------------------------------------
* macro definitions for Bluccino standard symbol support (bl_symb.h)
* supporting Bluccino RTL (real time logging) - see 'samples' folder

## Changelog History

* Bluccino RTL support implemented
* macro definitions for Bluccino standard symbol support (bl_symb.h)
* logflood samples added
* pimp README files and add nonblocking COM/USB support during init
* Bluccino README.md file updated (V0.1.8 summary)
* migrate all 01-quicktour samples to Bluccino V1.0.8 and rename
  CFG_COM_PORT_POLL to CFG_COM_PORT_WAIT
* tested all project in samples/03-logging folder
* tested all project in lessons/01-quicktour folder
* update bl_vers.h to support V1.0.8 version logging
* tested all project in lessons/03-meshfun folder
* ready for Bluccino V0.1.8 alpha deployment

--------------------------------------------------------------------------------
# Bluccino V1.0.7
--------------------------------------------------------------------------------

* more precise timing for bl_run()/bl_engine()
* introduce bl_after() function
* new hwstd hardware core with NVM support
* mesh publisher (bl_spool) for repeated publishing of mesh messages
* support of transitions (bl_trans)
* logging.h and config.h in many cases replaced by CMakeLists.txt definitions
* bl_node module for mesh node house keeping (replaces bl_house)
* new (simplified) top gear
* add bl_duty(o,duty,period) inline function
* add bl_run monitoring
* add bl_util.h to support bl_rand() (random function)
* enable/disable interrupts (bl_irq())

## Roadmap:

- extend wlstd mesh core with 3 additional gonoff client/server models
- extend wlstd mesh core with 3 additional glevel client/server models

## Changelog History

* more precise timing for bl_run()/bl_engine()
* new hwstd hardware core with NVM support
* mesh publisher (bl_spool) for repeated publishing of mesh messages
* support of transitions (bl_trans)
- test all 01-quicktour/* samples
- [SYS:TICK] and [SYS:TOCK] -> SYS_TICK_ix_BL_pace_cnt, SYS_TOCK_ix_BL_pace_cnt
* logging.h and config.h in many cases replaced by CMakeLists.txt definitions
* bl_node module for mesh node house keeping (replaces bl_house)
* new (simplified) top gear
* add bl_duty(o,duty,period) inline function
* add bl_after() inline function (bl_time.h)
* add bl_reset module (to implement reset logic in wlcore/wlstd/bl_wl)
* add [GET:PRV] and [GET:ADD] input interfaces to bl_wl of wlstd core
* add bl_run monitoring
- rename Bluccino V1.0.6 to V1.0.7
* Bluccino V1.0.7 release
- event spin out of bl_run monitoring (see 01-bluccino app)
* Bluccino V1.0.7 patch 1
* add bl_util.h to support bl_rand() (random function)
* Bluccino V1.0.7 patch 2
- enhanced bl_rand() function to support unsigned modulo n random numbers
* Bluccino V1.0.7 patch 3
* enable/disable interrupts
* Bluccino V1.0.7 patch 4

--------------------------------------------------------------------------------
# Bluccino V1.0.6
--------------------------------------------------------------------------------

* use GEAR logging instead of API logging
* introduce _bl_out() for augmented output
* add augmented _bl_led() function
* bl_cb() must not warn for defaults which are NULL
* introduce syntactic sugar bl_fwd() as simplified bl_out() function
* introduce bl_post() function and SYS: message definition symbols
* define message IDs for SYS:, LED:, BUTTON: and SWITCH: classes

## Roadmap:

## Changelog History
* use GEAR logging instead of API logging
* introduce _bl_out() for augmented output
* add augmented _bl_led() function
* bl_cb() must not warn for defaults which are NULL
- slight modification of standard log macro (intro of WHO define)
* introduce syntactic sugar bl_fwd() as simpliefied bl_out() function
* introduce bl_post() function and SYS: message definition symbols
* define message IDs for SYS:, LED:, BUTTON: and SWITCH: classes
- replace BL_ID() macro calls in bl_hwled.c case statements by message ID defs
- replace BL_ID() macro calls in bl_hwbut.c case statements by message ID defs
- Bluccino V1.0.6a Beta
* more precise timing for bl_run()/bl_engine()
* new hwstd hardware core with NVM support
* mesh publisher (bl_spool) for repeated publishing of mesh messages
* support of transitions (bl_trans)
- test all 01-quicktour/* samples
- [SYS:TICK] and [SYS:TOCK] -> SYS_TICK_ix_BL_pace_cnt, SYS_TOCK_ix_BL_pace_cnt
* logging.h and config.h in many cases replaced by CMakeLists.txt definitions
* bl_node module for mesh node house keeping (replaces bl_house)
* new (simplified) top gear
* add bl_duty(o,duty,period) inline function
* add bl_after() inline function (bl_time.h)
* add bl_reset module (to implement reset logic in wlcore/wlstd/bl_wl)
* add [GET:PRV] and [GET:ADD] input interfaces to bl_wl of wlstd core

--------------------------------------------------------------------------------
# Bluccino V1.0.5
--------------------------------------------------------------------------------

## Goals:
+ new organisation (bl_run.c/.h, bl_gear.c/.h, ...)

## Changelog History
+ new organisation (bl_run.c/.h, bl_gear.c/.h, ...)
- Bluccino v1.0.5

--------------------------------------------------------------------------------
# Bluccino V1.0.4
--------------------------------------------------------------------------------

## Goals:

## Changelog History
- Bluccino v1.0.4

--------------------------------------------------------------------------------
# Bluccino V1.0.3
--------------------------------------------------------------------------------

## Goals:
- bug: housekeep starts flashing the blue LED in the startup sequence, but it
       should start the status LED
- bug: wltiny has an issue wit GOOLET/GOOSET (seems they fire at the same time)
+ test all training lecture sample programs
- bug: crash in 07-attention

## Changelog History
- some bug fixes in interface docs
- test all training lecture sample programs
- Bluccino v1.0.3

--------------------------------------------------------------------------------
# Bluccino V1.0.2
--------------------------------------------------------------------------------

## Goals:
+ bug fix: already defined LOG/LOGO macros in case of #include "symbol.h"
- test all training lecture sample programs
- bug: crash in 07-attention

## Changelog History
+ bug fix: already defined LOG/LOGO macros in case of #include "symbol.h"
- test 01-quicktour and 03-meshfun samples
- Bluccino v1.0.2

--------------------------------------------------------------------------------
# Bluccino V1.0.1
--------------------------------------------------------------------------------

## Goals:
+ provide a bl_core.c/.h module as a default template for HW core and WL core
  integration
+ separation of bluccino() and bl_in() function => bluccino.c
+ reorganization of file structure for cores
+ seperation of bl_run() and bl_engine() into bl_run.c/.h module
+ separation of bl_up() and bl_down() into bl_gear.c/.h module
+ renaming bl_api.c to bl_time.c, split bl_api.h into bl_time.h and bl_msg.h
+ test all 03-meshfun training lecture sample programs

## Changelog History
+ provide a bl_core.c/.h module as a default template for HW core and WL core
  integration
+ separation of bluccino() and bl_in() function => bluccino.c
+ reorganization of file structure for cores
+ seperation of bl_run() and bl_engine() into bl_run.c/.h module
+ separation of bl_up() and bl_down() into bl_gear.c/.h module
+ renaming bl_api.c to bl_time.c, split bl_api.h into bl_time.h and bl_msg.h
- test 03-meshfun/03-ledsrv in combi with 03-meshfun/02-swcli - works well :-)))
- test 03-meshfun/04-clisrv - works well :-)))
- modify and test 03-meshfun/05-nvm with different LEDs blinking after restart
+ test all 03-meshfun training lecture sample programs
- Bluccino v1.0.1
