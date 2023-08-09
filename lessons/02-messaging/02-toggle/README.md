# 02-toggle - toggle all LEDs at the same time

# About the Sample App

* this sample demonstrates the standard Bluccino style to dispatch received
  messages in a public module interface utilizing a switch statement
* any button being pressed causes to toggle all LEDs at once
* the sample is prepared for nrf52dk_nrf52832 DK, nrf52dk_nrf52840 DK and
 nrf52840dongle_nrf52840 (dongle)

## Bluccino Primitives Used

* bl_hello(): set verbose level and print hello message
* bl_engine(): Bluccino init/tick/tock engine
* bl_id(): check for a given Bluccino message ID
* BL_ID(): macro to compose message ID from class tag and opcode
* bl_down(): down gear
* bl_oval: oval interface type (standard Bluccino function handler type)
* bl_sg(): send a message to a module
