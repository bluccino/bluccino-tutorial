================================================================================
04-rgbcycle - tiny sample where any of 4 buttons cycles RGB LEDs
================================================================================

About the Sample App
 - this sample demonstrates how to use the Bluccino HW core for simple RGB LED
   control of a nRF52840 dongle via its button
 - the sample runs best (with RGB colors) on nrf52840dongle_nrf52840 dongle
 - but also nrf52dk_nrf52832 and nrf52dk_nrf52840 DKs can be used for demo

Bluccino Primitives Used
- bluccino(): public module interface of bluccino module
- bl_hello(): setup verbose (logging) level and print a hello message
- bl_init(): initialize a module
- bl_is(): used for dispatching of Bluccino event messages
- bl_led(): LED on/off or toggle control for given LED @ix
- bl_logo(): log (message) object

Lessons to Learn
- how to use the standard way of Bluccino message dispatching utilize function
  bl_id() and macro BL_ID()
- how to organize Bluccino applications in
    1) dispatcher (public module interface)
    2) worker functions (working on the dispatched message)
    3) helpers (helping the workers with detail stuff)
- how to use the bl_logo() function for quick and dirty message object logging
- how to do a reasonable documentation

#===============================================================================
# Exercises
#===============================================================================

Exercise 1
- study the log output in the console. List all the different messages you can
  identify when clicking (short) the button and holding (long) the button
- which different instances (tags) can you identify?
- note: log messages have the following form:  tag: [cl:op @ix,val]

Exercise 2
- study the definition of symbol BUTTON_PRESS_ix_0_0
- rewrite the application by using macro BL_ID() instead of BUTTON_PRESS_ix_0_0
- what might be the reason for the Bluccino recommendation to use symbol
  BUTTON_PRESS_ix_0_0 instead of using BL_ID(_BUTTON,CLICK_), even both approa-
  ches are working?

Exercise 3
- rewrite the sample app by dispatching [BUTTON:CLICK] events instead of
  [BUTTON:PRESS] events
- also update the diagram for the public APP module interface

Exercise 4
- rewrite the sample app by dispatching [BUTTON:CLICK] events instead of
  [SWITCH:STS] events
- also update the diagram for the public APP module interface

Exercise 5
- rewrite the sample app in such a way that only a button press of button @ix
  can select the next RGB LED, if LED @ix is currently selected.

Exercise 6
- replace the statement bl_cfg((bl_down),_BUTTON,BL_PRESS) in main() by
  bl_cfg((bl_down),_BUTTON,BL_EDGE)
- which kind of changes do you recognize?
- what could be the meaning of adding this statement?

Exercise 7
- similarily to exercise 5 use (instead) the statement
   bl_cfg((bl_down),_BUTTON,BL_SWITCH) in main()
- which kind of changes do you recognize?
- what could be the meaning of adding this statement?

Exercise 8
- use the initial code of the sample as a basis
- for LED control replace statement bl_led(1,-1) by bl_led(1,val)
- does the app still show the same behavior?
- explain why the behavior is the same, or is different?

Exercise 9
- start with the modified code of exercise 7 and make changes in order to
	dispatch on message [SWITCH:STS] instead of [BUTTON:PRESS]
- does the app now show the (correct) initial behavior?
- explain why the behavior is now correct, or is still incorrect?
