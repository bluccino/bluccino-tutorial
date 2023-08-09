================================================================================
02-swicli (using WLTINY wireless core and HWTINY hardware core)
================================================================================

- switch-client mesh application: switch status updates are posted down to the
  generic on/off client with corresponding instance @ix
- This is shown in the following event flow chart

			    BL_UP                APP                BL_DOWN
           (U)                 (A)                 (D)
					  |                   |                   |
					  |   [SWITCH:STS]    |                   |
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
                 |      SWITCH:       |  SWITCH interface
     (U)-> STS ->|      @ix,sts       |  receive switch @ix status
                 +--------------------+
                 |      GOOCLI:       |  GOOCLI ifc. (generic on/off client)
     (D)<- SET <-| @ix,<BL_goo>,onoff |  publish generic on/off SET message
                 +--------------------+

- in the wireless core dephts button press events are toggling an internal
  switch state, while status updates (sts) of the toggle switch are posted to
	the app as [SWITCH:STS @ix,sts] events
- the app dispatches the SWITCH:STS messages and posts in to the down gear in
  terms of [LED:SET @ix,onoff] event messages, where the onoff value corresponds
	to the status value (sts) of the switch
- Note the path definitions to HWTINY and WLTINY cores:

	  set (LIB ../../../lib/V1.0)
	  set (HWC ${LIB}/core/hwcore/hwtiny)  # tiny hardware core
	  set (WLC ${LIB}/core/wlcore/wltiny)  # tiny wireless core

- Also note that the core (BL_CORE, HWTINY, WLTINY) needs to be initialized
  and to be ticked/tocked. This is done by the bl_engine() (Bluccino engine)
	call in main()
