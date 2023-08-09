# 10-ui - a user interface based on single button and 4 LEDs

## Description

* this sample demonstrates how to build a simple user interface based on a
  single button (or optional more), a status LED (LED @0) and either one RGB LED
  (red: LED @2, green: LED @3, blue: LED @4) or 3 separate LEDs @2,@3,@4.
* if a board provides only 1 button, it is very helpful if the application
  receives pre-processed button event types, which are either press/release
  events, multi-click events, click & hold or multi click and hold events. Also
  toggle switch events are of value. In addition to perform system reboot or
  a factory reset operation (to factory-initialize NVM settings) the button tock
  event type simplifies code on the application level


## Application (app)

* after setting verbose level and printing a hello message using bl_hello()
  Bluccino module is initialized with events being subscribed by the app()
  function
* app() is acting like a callback for all events being emitted by Bluccino
  module
* the app controls the blink sequences via an auxillary module `blink`, which is
  installed as a library module in the top gear.
* the sample is well configured for an nRF52840 dongle, as well as Nordic 832
  and Nordic 840 DKs (nrf52dk_nrf52832, nrf52840dk_nrf52840)


## Library Module `blink`

* to run either solid LED patterns or blink sequences a library module `blink`
  has been provided
* for the setting of various LED patterns Bluccino message [LED:PATTERN ...] is
  being consulted
* the blink module can run a single pattern (with termination of the blinker
  after completing the pattern), a repeat pattern (which is repeated until a
  new pattern is set), or a combination of initial pattern and repeat pattern
* The blink module is installed by main() function as a library in the top gear
* Here are some examples of blink patterns
```
   [LED:BLINK @0,"s",0]              // status LED @0 on
   [LED:BLINK @0,"!s",0]             // status LED off
   [LED:BLINK @0,"s-",500]           // blinking status LED
   [LED:BLINK @0,"r",0]              // RGB LED red on
   [LED:BLINK @0,"!r",0]             // RGB LED red off
   [LED:BLINK @0,"-",0]              // all LEDs off
   [LED:BLINK @0,"rrr---:r-",500]    // init pattern "rrr---",then repeat "r-"
   [LED:BLINK @0,"",0]               // blinker off
```


## How to Test the Sample

On a nRF52840 dongle we have a status LED (yellow, @1) and an RGB LED (@2,@3,@4)
On the RGB LED instance ID @2 controls RED, @3 controls GREEN, and @4 controls
BLUE. On a nRF52dk_nrf52832 or nRF52840dk_nRF52840 board channels @1-@4 control
4 separate LEDs. The following description is for a dongle, and the reader must
translate the description using aboce assignment scheme when using a board with
4 LEDs.

1) Startup

* after clearing flash memory and flashing the board the app shows 3 flashes
  with period 1s of both status and RGB lED in white color (all 4 LEDs).
*  the logging shows `system start: #1` and `loaded LED @ix = 4 from NVM`
```
   #0[000:00:000.000] 10-ui (click any of the buttons)
   #3[000:00:000.214]       init core ...
   #3[000:00:000.366]       init HW core ...
   #2[000:00:008.057]     set blink pattern "w-w-w-:",500
   #1[000:00:008.240]   system start: #1
   #1[000:00:009.278]   loaded LED @ix = 4 from NVM
   #2[000:03:500.244]     deactivate blinker
```

2) Clicking Button @1

* a single click on button @1 activates a low duty flashing with 1 flash of the
  blue LED (LED @4, according to NVM settings)
* double clicking button @1 changes the blink sequence to a low duty pattern
  with 2 blue flashes
* triple clicking button @1 changes the blink sequence to a low duty pattern
  with 3 blue flashes


* press and hold of button @1 for more than 11 seconds shows first slow blinking
  of blue LED (with high duty), then fast blinking (with high duty) of blue LED.
* after 6 seconds the blink pattern changes into first slow and then fast white
  blinking. Button release during white blinking phase causes system reset,
  which increments the system-start-counter (watch logs after system restart)  
* after 11 seconds the blink pattern changes into fast red blinking. Button
  release during red blinking phase causes factory reset (next system start
  is labeled as number 1)
* single clicking of button @1 activates a blue blinking pattern with 1 blue
  flash per period
* double clicking of button @1 activates a blue blinking pattern with 2 blue
  flashes per period
* triple clicking of button @1 activates a blue blinking pattern with 2 blue
  flashes per period
* button press & hold (press time less than 6s) clears all LEDs


## Lessons to Learn
- learn about the different button events: [BUTTON:PRESS], [BUTTON:RELEASE],
  [BUTTON:CLICK], [BUTTON:HOLD], [SWITCH:STS], [BUTTON:TOCK]

## Bluccino Primitives Used
- bluccino(): public module interface of bluccino module
- bl_hello(): setup verbose (logging) level and print a hello message
- bl_init(): initialize a module
- bl_is(): used for dispatching of Bluccino event messages
- bl_led(): LED on/off or toggle control for given LED @ix
