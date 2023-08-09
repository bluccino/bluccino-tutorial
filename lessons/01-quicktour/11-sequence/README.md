# 11-sequence


## About the Sample App

* sample to demonstrate use of Bluccino timers and work package submission
* the sample runs best (with RGB colors) on nrf52840dongle_nrf52840 dongle
* but also nrf52dk_nrf52832 and nrf52dk_nrf52840 DKs can be used for demo


## Implemented Scenario

```
        mai           test        sequence                    work/timer
        (M)           (T)           (S)                         (W)(T)
         |             |             |                           |  |
         | SYS:INIT    |             |                           |  |
         o------------>|             |  [TASK:INIT]->sequence    |  |
         |             o---------------------------------------->|  |
         |             |             |     "let's go ...",88     |  |
         |             |             |                           |  |
         |             |             |        TASK:INIT          |  |
         |             |             |<--------------------------o  |
         |             |             |     "let's go ...",88     |  |
         |             |             |                           |  |
         |             |     +-------v--------+                  |  |
         |             |     |    state 0     |                  |  |
         |             |     +-------v--------+                  |  |
         |             |             |   [TASK:NEXT] @ +2000ms   |  |
         |             |             o----------------------------->|
         |             |             |             1             |  |
         |             |             |                           |  - +2000ms
         |             |             |          TASK:NEXT        |  |
         |             |             |<-----------------------------o
         |             |             |              1            |  |
         |             |     +-------v--------+                  |  |
         |             |     |    state 1     |                  |  |
         |             |     +-------v--------+                  |  |
         |             |             |   [TASK:NEXT] @ +5000ms   |  |
         |             |             o----------------------------->|
         |             |             |             2             |  |
         |             |             |                           |  - +5000ms
         |             |             |          TASK:NEXT        |  |
         |             |             |<-----------------------------o
         |             |             |              2            |  |
         |             |     +-------v--------+                  |  |
         |             |     |    state 2     |                  |  |
         |             |     |     stop       |                  |  |
         |             |     +-------v--------+                  |  |
         |             |             |                           |  |
```

## Bluccino Primitives Used

* bluccino(): public module interface of bluccino module
* bl_hello(): setup verbose (logging) level and print a hello message
* bl_init(): initialize a module
* bl_is(): used for dispatching of Bluccino event messages
* bl_led(): LED on/off or toggle control for given LED @ix
* bl_logo(): log (message) object


## Lessons to Learn

* how to use Bluccino work package submission
* how to use Bluccino timers
* how to implement a processing sequence with delays between steps


## Bluccino work package submission

 * target: simple method to submit work to a Bluccino OVAL-interface based
   worker function which offloads execution and datato a seperate thread
 * Zephyr work package data structures are auto initialized during the first
   work package submission, an explicite call to an initializing function on
   Bluccino level is not required
 * two data structures are used:
   1) static BL_work structure which comprises a (Zephyr) strukt k_work
   2) fuction pointer of a worker module which processes the work package
      in a separate thread
   3) BL_ob and int data elements to hold copy values of an event message
      which triggers the worker module to start processing the work package
 * an initializer macro BL_WORK(worker) is provided to
   1) init the worker module function reference
   2) and to init BL_ob data with the (invalid) message ID [VOID:VOID]
 * a work package submission function bl_submit(&work,o,val) is provided to
   submit a work package which
   1) auto-initializes struct k_work if BL_ob data has an in initial
      [VOID:VOID] value
   2) stores the message data given by o and val into the BL_work structure
      (overwriting the [VOID:VOID] initial value)
   3) submits the Zephyr work package to the Zephyr work queue
 * there is an internal worker (executed in a separate task) which processes
   the submitted work and posts the Bluccino message to the worker module


# Exercises


## Exercise 1

* ToDo
