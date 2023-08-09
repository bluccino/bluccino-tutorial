# 01-interact

## Description

This sample demonstrates the C-implementation of a simple *module interaction*

```
   app >> LED.SET(i,on) >> core
```

between module `app` (interaction source module) and module `core` (interaction
destination module) as displayed in figure 1.

```
             +---(app)----+                   +---(core)---+
             |            |   LED.SET(i,on)   |            |
             |         LED|------------------>|LED         |
             |            |                   |            |
             +------------+                   +------------+
                 source                         destination
                 module                           module

                     Figure 1: simple interaction
```

//==============================================================================
// event.h (event definition)
//==============================================================================
#ifndef __MESSAGE_H__
#define __MESSAGE_H__

  #include "profile.h"

//==============================================================================
// SYS events
//==============================================================================

  Interface SYS
  {
    Message(SYS,INIT)();
    Message(SYS,REG)(void(*cb)(int i));
  } SYS;

//==============================================================================
// LED events
//==============================================================================

  Interface LED
  {
    Message(LED,SET)(int i);
    Message(LED,TOGGLE)(int i);
  } LED;

//==============================================================================
// BUTTON events
//==============================================================================

  Interface BUTTON
  {
    Message(BUTTON,PRESS)(int i);
    Message(BUTTON,CLICK)(int i);
  } BUTTON;

#endif // __MESSAGE_H__
```

## Event Flow Profile

```
//==============================================================================
// flow.h: event flow profile
//==============================================================================

  #include "message.h"

//==============================================================================
// event flow: initializing
//==============================================================================

  static void(*click)(int i);  // dummy variable

  Flow initializing()
  {
    Module M = "main", A = "app", D = "driver";

    "main entry point" >> M;
    M >> SYS.INIT() >> A;
    A >> SYS.REG(click) >> D;
  }

//==============================================================================
// event flow: button_click_toggles_led
//==============================================================================

  static int i;                        // dummy variable

  Flow button_click_toggles_led()
  {
    Module M = "main", A = "app", D = "driver", R = "RTOS"

    "button pressed" >> R;
    R >> BUTTON.PRESS(i) >> D;
    D >> BUTTON.CLICK(i) >> A;
    A >> LED.TOGGLE(i) >> D;
    D >> LED.TOGGLE(i) >> R;
    R >> "toggle LED @i";
  }
```
