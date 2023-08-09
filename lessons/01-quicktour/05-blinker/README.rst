================================================================================
05-blinker - small blinker program with 4 LEDs involved
================================================================================

About the Sample App
 - this sample demonstrates circular blinking of 4 LEDs
 - the sample uses an app function with a Bluccino OVAL interface
   and uses Bluccino standard dispatching of event messages
 - the app is hierarchichally structured into public APP module interface
   (dispatcher), a worker function to which the dispatcher delegates, and a
   helper function for the worker
 - the sample is prepared for nrf52dk_nrf52832, nrf52dk_nrf52840 and
   nrf52840dongle_nrf52840 (dongle)

Lessons to Learn
- how to use the standard way of Bluccino message dispatching utilize function
  bl_id() and macro BL_ID()
- Bluccino init and output handling by app() set up by using bl_init()
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
- rewrite tock() worker to reverse the RGB cycle sequence: while currently the
  RGB cycle is R->G->B->R->... The task is to get a reversed cycle sequence
  B->G->R->B->...

Exercise 2
- dispatch a [BUTTON:CLICK] event in order to change RGB cycle direction each
  time a button is clicked
- introduce a common static flag reverse and rewrite the next() helper as to
  provide next @ix for standard direction and reverse direction, according to
  the value of reverse.
- introduce a button() worker (with OVAL interface) which toggles the reverse
  flag
- update the diagram for the public APP module interface accordingly
- rewrite tock() worker to reverse the RGB cycle sequence: while currently the
  RGB cycle is R->G->B->R->... The task is to get a reversed cycle sequence
  B->G->R->B->...
