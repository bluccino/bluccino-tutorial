#===============================================================================
# 03-button - tiny button sample
#===============================================================================

About the Sample App
- this sample demonstrates how to subscribe to button press events, which
  control a related LED
- after setting verbose level and printing a hello message using bl_hello()
  Bluccino module is initialized with events being subscribed by the app()
  function
- app() is acting like a callback for all events being emitted by Bluccino
  module
- using bl_is() we dispatch messages, and once we receive a [BUTTON:PRESS]
  message we toggle the related ID
- identification of button and LED is accomplished by the '@ix' argument of
  the message, which is passed n therms of the bl_ix(o) data.
- the sample is prepared for nrf52dk_nrf52832 DK, nrf52dk_nrf52840 DK and
  nrf52840dongle_nrf52840 dongle

Lessons to Learn
- how to write the prototype of an app() callback ("OVAL interface") in order
  to use it for dispatching of Bluccino event messages
- awareness that "OVAL" functions must return an integer value for uniformity
  (even the return value does not give much sense in many cases)
- how to use the bl_hello() function to setup verbose level and to print a
  hello message
- how to initialize the Bluccino module if event messages need to be catched
  via an app() callback
- demonstration of event message (in our case a button event) dispatching
  and simple LED toggle control
- learn about the different button events: [BUTTON:PRESS], [BUTTON:RELEASE],
  [BUTTON:CLICK], [BUTTON:HOLD], [SWITCH:STS]

Bluccino Primitives Used
- bluccino(): public module interface of bluccino module
- bl_hello(): setup verbose (logging) level and print a hello message
- bl_init(): initialize a module
- bl_is(): used for dispatching of Bluccino event messages
- bl_led(): LED on/off or toggle control for given LED @ix

================================================================================
Exercises
================================================================================

Exercise 1
- study the log output in the console are of the form
    tag: [cl:op @ix,val]
- the tag indicates which function (module) prints the log, and the expression
  enclosed by [...] denotes a Bluccino event message
- list all different event messages you can identify in the console output
  when you (short) click or (long) hold the button
- which of the events might flow up-stream and which might flow down-stream?

Exercise 2
- rewrite the app for a nRF52840 dongle, which has one status LED (@1) and
  3 LEDs for RGB display (@2:red, @3:green, @4:blue)
- in this modified app a button click should toggle the status LED, while
  a button hold (long press) should toggle all 3 RGB LEDs at the same time
