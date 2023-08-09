# SPDX-License-Identifier: Apache-2.0

  cmake_minimum_required(VERSION 3.20.0)
  find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

#===============================================================================
# project personality (definitions to be pushed down to C-level)
#===============================================================================

  project(04-cbeacon)

  add_definitions(-DPROJECT="${CMAKE_PROJECT_NAME}")  # C code to see PROJECT
  add_definitions(-DBL_INCLUDE="main.h")              # always include "main.h"
  add_definitions(-DCFG_BLE_AUTO_INIT=0)
  add_definitions(-DCFG_LOG_BLE=1)                    # enable BLE logging

#===============================================================================
# path setup
#===============================================================================

  set (LIB ../../../lib/v1.1.0)
  set (BLU ${LIB}/bluccino)
  set (SRC src)
  set (BLE ${LIB}/core/wlcore/bttiny)  # tiny BLE core
  set (SVC ${LIB}/service)             # BLE services
  set (HWC ${LIB}/core/hwcore/hwtiny)  # tiny hardware core

  include_directories(${BLU} ${SRC} ${BLE} ${SVC} ${HWC})

#===============================================================================
# source files
#===============================================================================

  target_sources(app PRIVATE
    ${SRC}/main.c                      # main program
    ${SRC}/cbeacon.c                   # Custom beacon BLE service

    ${BLU}/bluccino.c                  # bluccino library
    ${BLE}/bl_wl.c                     # wireless BLE core
    ${BLE}/bl_ble.c                    # BLE module
    ${HWC}/bl_hw.c                     # hardware core
  )
