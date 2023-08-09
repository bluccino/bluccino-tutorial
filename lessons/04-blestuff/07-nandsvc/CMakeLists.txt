# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

#===============================================================================
# project personality (definitions to be pushed down to C-level)
#===============================================================================

  project(07-nandsvc)

  add_definitions(-DPROJECT="${CMAKE_PROJECT_NAME}") # C-code knows project name
  add_definitions(-DBL_INCLUDE="main.h")             # std. include of main.h
  add_definitions(-DCFG_BLE_AUTO_INIT=0)
  add_definitions(-DCFG_LOG_BLE=1)                   # enable BLE logging

#===============================================================================
# path setup
#===============================================================================

  set (LIB ../../../lib/v1.1.0)
  set (BLU ${LIB}/bluccino)
  set (SRC src)
  set (WLC ${LIB}/core/wlcore/bttiny)  # tiny BLE core
  set (SVC ${LIB}/service)             # BLE services

  include_directories(${SRC} ${BLU} ${WLC} ${SVC})

#===============================================================================
# source files
#===============================================================================

  target_sources(app PRIVATE
    ${SRC}/main.c                      # main program
    ${SRC}/app.c                       # application module

    ${BLU}/bluccino.c                  # bluccino library
    ${BLU}/bl_deco.c                   # decorate log header

    ${WLC}/bl_wl.c                     # wireless BLE core
    ${WLC}/bl_ble.c                    # wireless BLE core
    ${SRC}/nandsvc.c                   # nand service
  )
