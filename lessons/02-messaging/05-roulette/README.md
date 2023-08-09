# 05-roulette - send messages with random stuff

## About the Sample App

* this sample demonstrates fundamental basics how to send,   
  receive and dispatch Bluccino messages
* the sample is prepared for nrf52dk_nrf52832 DK, nrf52dk_nrf52840 DK and
 nrf52840dongle_nrf52840 (dongle)

## Lessons to learn

* how to define custom message class tags
* how to define custom message opcodes
* how to add debug information for class tags and opcodes in order utilize `bl_logo` debugging
* How to use `bl_msg` function to send a message

## Bluccino Primitives Used

* bl_hello(): set verbose level and print hello message
* bluccino(): public module interface of bluccino module
* bl_init(): initialize a module
* bl_rand(): random number function
* bl_msg(): send a Bluccino message to a module
* bl_sleep(): low power sleeping for some given milliseconds
* bl_ix(): syntactic sugar to access instance index
* bl_data(): syntactic sugar to access data reference
* bl_log(): standard Bluccino log function
* bl_logo(): special log function for message objects
* BL_txt: type short hand for `const char *`
