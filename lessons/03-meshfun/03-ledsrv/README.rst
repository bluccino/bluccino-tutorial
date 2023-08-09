================================================================================
03-ledsrv (using WLTINY wireless core and HWTINY hardware core)
================================================================================

- LED-server mesh application: generic on/off server status updates are posted
  up to the app, which forwards the status updates down to the LED driver with
  corresponding instance @ix and on/off status
- the app is prepared for nrf52dk_nrf52832, nrf52dk_nrf52840 and nrf52840 dongle

================================================================================
more details
================================================================================

- This is shown in the following event flow chart

			    BL_UP                APP                BL_DOWN
           (U)                 (A)                 (D)
					  |                   |                   |
					  |   [GONOFF:STS]    |                   |
					  o------------------>|                   |
					  |      @ix,sts      |                   |
					  |                   |                   |
					  |            +-------------+            |
					  |            | onoff = sts |            |
					  |            +-------------+            |
					  |                   |                   |
					  |                   |     [LED:SET]     |
					  |                   o------------------>|
					  |                   |     @ix,onoff     |
					  |                   |                   |

- from the event flow chart we can easily derive the app's module interface

                 +--------------------+
                 |        app         |
                 +--------------------+
                 |      GONOFF:       |  GONOFF input interface
     (U)-> STS ->|      @ix,sts       |  receive generic on/off @ix status
                 +--------------------+
                 |        LED:        |  LED output interface
     (D)<- SET <-|      @ix,onoff     |  post LED SET message to down gear
                 +--------------------+

- in the wireless core dephts mesh messages are received by a generic on/off
  server model, which posts a status update up to the app level
- the app dispatches the [GONOFF:STS @ix,<BL_goo>,sts] messages and posts LED
  messages to the down gear using the syntactic sugar function bl_led(), which
  results in posting an [LED:SET @ix,onoff] event messages to the down gear
- Note the path definitions to HWTINY and WLTINY cores:

	  set (LIB ../../../lib/V1.0)
	  set (HWC ${LIB}/core/hwcore/hwtiny)  # tiny hardware core
	  set (WLC ${LIB}/core/wlcore/wltiny)  # tiny wireless core

- Also note that the core (BL_CORE, HWTINY, WLTINY) needs to be initialized
  and to be ticked/tocked. This is done by the bl_engine() (Bluccino engine)
	call in main()
