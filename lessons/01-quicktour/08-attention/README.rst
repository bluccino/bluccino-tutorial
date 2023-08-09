#===============================================================================
# 08-attention sample
#===============================================================================

Simple Bluetooth mesh demo supporting provisioning and attention messages to be
forwarded from the core to Bluccino top gear layer where color of log time
stamps are controlled according to provision and attention state.

High Level App Behavior:

  * periodic loggings with log text
       - "I'm a (unprovisoned) mesh device" (in unprovisioned case)
       - "I'm a (provisoned) mesh node" (in provisioned case)
  * All log messages show color switch of the log header (time stamps)
    when attention is in process or node state changes from device state
    (unprovisioned) to node state (provisioned)
  * App is also logging an addtional log message "attention" during active
    attention mode

# Lessons to Learn

  * how to use the bl_log() function
  * Bluccino top gear functionality to update log time stamp coloring according
    provision and attention state changes

# Project Configuration

  * Stack size needs to be configured as (minimum) 1024 bytes
    (see prj.conf: CONFIG_MAIN_STACK_SIZE=1024)

# Application Architecture

                    +----------------+
                    |      MAIN      |
                    +----------------+
    =====================|=====|============================================
    ||                   |     |  Bluccino  top gear                      ||
    =====================|=====|====================^=====^=================
        ||               |     |                    |     |           ||
        ||       [:INIT] v     v [:TICK]      [:PRV]|     |[:ATT]     ||
        ||      +----------------------------------------------|      ||
        ||      |                                              |      ||
  ) ) ) ) ) ) ) )            hwtiny/wltiny core                ) ) ) ) ) ) ) )
        ||      |                                              |      ||
        ||      +----------------------------------------------+      ||
        ||============================================================||
        ||                                                            ||


#===============================================================================
# Exercises
#===============================================================================

Exercise 1:
- in main() there is a log statement
    bl_log(1,"%s",provision?msg1:msg2);
- try to replace this statement by the simpler one
    bl_log(1,provision?msg1:msg2)
  and try to interprete the compiler's moaning!
- what is the reason why such simplification does not work, and what might
  be the motivation for the specific implementation of bl_log()?
