# SPDX-License-Identifier: Apache-2.0

  cmake_minimum_required(VERSION 3.20.0)
  find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

#===============================================================================
# project personality (definitions to be pushed down to C-level)
#===============================================================================

  project(05-roulette)

  add_definitions(-DPROJECT="${CMAKE_PROJECT_NAME}")
  add_definitions(-DBL_INCLUDE="main.h")    # auto-include main.h
  add_definitions(-DCFG_LOG_GEAR=0)         # no gear logging
  add_definitions(-DCFG_LOG_LED=0)          # no LED logging
  add_definitions(-DCFG_LOG_BUTTON=0)       # no button logging

#===============================================================================
# path setup
#===============================================================================

  set (LIB ../../../lib/v1.1.0)             # library path
  set (BLU ${LIB}/bluccino)                 # Bluccino library modules
  set (HWC ${LIB}/core/hwcore/hwtiny)       # tiny hardware core
  set (SRC src)

  include_directories(${SRC} ${BLU} ${HWC})

#===============================================================================
# source files
#===============================================================================

  target_sources(app PRIVATE
    ${SRC}/main.c
    ${BLU}/bluccino.c
    ${HWC}/bl_hw.c
  )
