================================================================================
06-ledtoggle - tiny LED sample, where buttons toogle related LEDs
================================================================================

App Description
- all LEDs are off at the beginning
- any button press activites blinking of first LED for 5s
- any further button press activates blinking of next LED for 10s

Lessons to Learn
- learn how to setup a standard Bluccino framework, reacting on standard
  timing events (tick/tock), button events and controlling the board's LEDs
- demonstration of role of main() - Bluccino main()'s have usually 2-3 lines
- show standard implementation of an APP module with dispatcher and workers
- demonstrate how module interfaces (of app) should be documented
- demonstrate how worker functions should be documented

================================================================================
Exercises
================================================================================

Exercise 1
- function sys_tock() uses 'val' to decide an LED toggle on every 5th tock.
- the toggle period clearly depends also on the tock period, which has been
  defined to be 500 ms in main()
- if the tock period is increased to 1000ms then the LED toggle period is also
  increased, which might not be desirable
- use bl_period() function to rewrite the code in order to make the LED toggle
  period independent of the tock period
