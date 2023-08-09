# 05-setlet (Client/server Bluetooth Mesh Application, Sendig SET or LET Messages)

## Sample Description

* mesh application which implements both a generic on/off server and a generic
  on/off client
* [SWITCH:STS \@ix,sts] events with odd \@ix (\@1,\@3) cause to update the
  on/off status of the related generic on/off server \@ix according to the
  on/off state of the switch by sending generic on/off SET messages
* [SWITCH:STS \@ix,sts] events with even \@ix (\@2,\@4) cause to update the
  on/off status of the related generic on/off server \@ix according to the
  on/off state of the switch by sending generic on/off LET messages


## Core's Used

* the sample uses the tiny wireless (mesh) core (core/wlcore/wltiny/bl_wl.c)
  and the tiny hardware core (hwcore/hwtiny/bl_hw.c)
