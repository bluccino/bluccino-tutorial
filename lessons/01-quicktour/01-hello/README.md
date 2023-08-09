# 01-hello demo (hello world)

## About the Sample App

* the simplest program to get running with Bluccino
* uses the Bluccino bl_prt() function
* bl_prt() is an exact equivalent for the Zephyr printk() function, supporting
  variable argument lists
* note that bl_prt() requires a newline character (\n) at the end of the line


## Lessons to Learn

* how to write a 'hello world' program within Bluccino framework
* how to print a message to console with Bluccino primitives (bl_prt)
* get familar with Zephyr project structure (src folder for the source code,
  CMakeLists.txt, project config file prj.conf, README.rst file)
* get familar with the two step Zephyr build process (CMake process, after that
  compilation and linking)
* how to run an application

## Bluccino Primitives Used

* bl_prt(): generic (formatted) print function (like printf() or printk())



# Zephyr Project structure

01-hello shows already the common Zephyr project structure, which should be
  understood (at least coarsly) from the beginning
```
  01-hello
     +- src                  source file folder (containing all sources)
     |  +- main.c            C-source file containing main() function
     +- CMakeLists.txt       CMake file (being executed if prj.conf changes)
     +- prj.conf             project config (which Zephyr modules to include?)
     +- README.rst           readme file (giving some overview information)
     +- sample.yaml          device tree binding info and test case definitions
```

## Project Configuration

* To run the app with Segger Realtime Terminal (Segger RTT) it is necessary to
  enable Segger RTT in the project configuration file (*prj.conf*).
* This is accomplished by the following configuration lines in *prj.conf*:

```
  CONFIG_RTT_CONSOLE=y
  CONFIG_USE_SEGGER_RTT=y
  CONFIG_UART_CONSOLE=n
```

## CMakeLists.txt

* important to understand is CMakeLists.txt file with the following details
* 1) setting up the project name
* 2) defining symbol LIB for the library path
* 3) defining symbol BLU for the Bluccino library path
* 4) defining symbol SRC for the folder containing the sources
* 5) defining include folders ${SRC} and ${BLU}
* 6) linking both ${SRC}/main.c file and ${BLU}/bluccino.c library source


# Exercices

## Exercise 1

* logging is essential during the debugging of event driven applications and
  using different colors for logging can be very helpful during debugging, thus
  get experienced with using colors
* replace line
```
     bl_prt("hello, world!\n");
```
  by
```
     bl_prt(BL_R "hello, world!\n");
```
* what will change?

## Exercise 2

* find the header file where symbol BL_R is defined
* what is the meaning behind BL_R?
* why is there not a comma between BL_R and "hello, world!\n" ?

## Exercise 3

* rewrite 01-hello to print "hello" in green, "world" in magenta, and any other
  characters in black!

## Exercise 4

* Bluccino supports both a bl_prt() and a bl_printf() function, in addition (on
  Zephyr RTOS environment) it would be possible to use the standard Zephyr
  printk() function for formatted console output
* why might it be preferable to use bl_prt() instead of bl_printf() or printk()
  in Bluccino applications, even it seems that bl_prt() is just an alias for
  both bl_printf() and printk()?
