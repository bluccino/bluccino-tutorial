#===============================================================================
# 09-clisrv (client/server application)
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
        ||      +----------------------------------------------+      ||
        ||      |                                              |      ||
  ) ) ) ) ) ) ) )             hwtiny/wlstd core                ) ) ) ) ) ) ) )
        ||      |                                              |      ||
        ||      +----------------------------------------------+      ||
        ||============================================================||
        ||                                                            ||


#===============================================================================
# Exercises
#===============================================================================
