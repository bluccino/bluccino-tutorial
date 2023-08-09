================================================================================
07-nvm (using WLSTD wireless core and HWSTD hardware core)
================================================================================

About the Sample App
- sample app which can count system starts and stores a button state persis-
  tently
- utilizes non-volatile memory (NVM)
- every system start a system start counter ("starts") is loaded from NVM,
  is being incremented and logged to console. After that the updated counter is
  stored back to NVM
- initially led pair @2|@4 (on dongle RED|BLUE) is alternately blinking. Each
  button click cyclically changes the blinking pair according to the id pair
  sequence @2|@4 -> 4|3 -> 3|2 -> 2|4 -> ... (on dongle: RED|BLUE -> BLUE|GREEN
  -> GREEN|RED -> RED|BLUE -> ...)
- current selected LED pair (given by LED @ix) is persistently stored in NVM
  after  each button click

Bluccino Primitives Used
- bl_hello(): setup verbose (logging) level and print a hello message
- bl_cfg(): configure button driver events
- bl_engine(): init/tick/tock engine
- bl_id(): composition of amessage ID from message class and opcode
- bl_down(): down gear (public module interface)
- bl_load(): load data from NVM
- bl_save(): save data to NVM
- BL_ob: Bluccino messaging object type
- BL_oval: Bluccino OVAL function type
- _bl_led(): LED control (using augmented messages)

Lessons to Learn
- how to load/save data from/to NVM
- how to count system starts
- to get experience with the hardware standard core (hwstd), which supports LED,
  button and NVM functionality
- how to use _bl_led() function to output LED events in app's public module
  interface (PMI) to down gear (bl_down)

#===============================================================================
# Exercises
#===============================================================================

Exercise 1
- add a general bl_log() function at the begin of app() to see all bluccino
  event messages being received by the app's public module interface (PMI)
- use an if-statement and function bl_id() to suppress logging of the frequent
  [SYS:TICK @ix,cnt] event

Exercise 2
- investigate the definition of bl_led() and _bl_led() functions!
- bl_led() is recommended for novices who are not yet familar with Bluccino
  philosophy, and for quick and dirty prototyping
- _bl_led() is the recommended form of controlling LEDs, sending augmented
  messages to the app's PMI, causing the PMI to output (unaugmented) messages
  to the down gear. Which augmented messages are emitted by _bl_led()?

Exercise 3
- _bl_led() forwards augmented messages to the PMI, from where they are forwar-
  ded to the down gear using bl_out()
- study trhe definition of bl_out(). Are the forwarded messages augmented or
  unaugmented?

Exercide 4
- usage of _bl_led() complies to an important Bluccino paradigm!
- which specific paradigm is meant, and what is the benefit of complying to this
  paradigm?
