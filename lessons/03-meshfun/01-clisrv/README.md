# 01-clisrv (Client/server Bluetooth mesh application)

## Sample Description

* mesh application which implements both a generic on/off server and a generic
  on/off client
* [SWITCH:STS \@ix,sts] events cause to update the on/off status of a generic
  on/off server \@1 according to the on/off state of the switch by sending
  generic on/off SET messages
* all buttons \@1 ... \@4 have the same function

## Core's Used

* the sample uses the tiny wireless (mesh) core (core/wlcore/wltiny/bl_wl.c)
  and the tiny hardware core (hwcore/hwtiny/bl_hw.c)
