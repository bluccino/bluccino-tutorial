# 04-light - fundamental basics about message sending, receiving and dispatching

## About the Sample App

* this sample demonstrates fundamental basics how to send,   
  receive and dispatch Bluccino messages
* the sample is prepared for nrf52dk_nrf52832 DK, nrf52dk_nrf52840 DK and
 nrf52840dongle_nrf52840 (dongle)

## Lessons to learn

* How to use `BL_ob` type to define a Bluccino message object
* how to send a message to a module (`app`) on base of a Bluccino message object
* how to write an *oval* interface for an `app` module which is capable to
  receive Bluccino messages
* how to dispatch a Bluccino message
* how to access Bluccino message arguments

## Bluccino Primitives Used

* bl_hello(): set verbose level and print hello message
* bluccino(): public module interface of bluccino module
* bl_init(): initialize a module
* bl_is(): check for a given Bluccino message ID
* bl_ix(): syntactic sugar to access instance index
* bl_data(): syntactic sugar to access data reference
* bl_log(): standard Bluccino log function
* BL_txt: type short hand for `const char *`
* BL_ob: declare Bluccino message object

# Exercises

## Exercise 1

* extend the `app` function by calling `bl_led()` to actually turn LED
  \@ix off or on, whenever a message `[LIGHT:LEVEL \@ix,"room",val]`
  is being received and the level (`val`) is either zero or non-zero.

## Exercise 2

* define Bluccino message objects `red`, `green` and `blue` for the three RGB LEDs of an nRF52840 dongle
* initialize the objects for messages [LED:SET \@1] (red), [LED:SET \@2] (green) and [LED:SET \@3] (blue)
* 5 seconds after program start switch on the red LED
* after another 5s turn off the red and turn on the green LED
* after another 5s turn off the green and turn on the blue LED
* after another 5s turn off the blue LED
* to send a message to the LED driver use `bl_down` as the destination module
* add ${HWC}/bl_hw.c in the target_sources section of CMakeLists.txt
