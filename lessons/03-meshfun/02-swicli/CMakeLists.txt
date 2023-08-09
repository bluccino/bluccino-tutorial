# SPDX-License-Identifier: Apache-2.0

  cmake_minimum_required(VERSION 3.20.0)
  set(QEMU_EXTRA_FLAGS -s)

  find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

#===============================================================================
# project personality (definitions to be pushed down to C-level)
#===============================================================================

  project(02-swicli)

  add_definitions(-DPROJECT="${CMAKE_PROJECT_NAME}")
  add_definitions(-DCFG_MESH_AUTO_INIT=1)

#===============================================================================
# path setup
#===============================================================================

  set (LIB ../../../lib/v1.1.0)
  set (BLU ${LIB}/bluccino)            # Bluccino stuff
  set (HWC ${LIB}/core/hwcore/hwtiny)  # tiny hardware core
  set (WLC ${LIB}/core/wlcore/wltiny)  # tiny wireless core
  set (SRC src)

  include_directories(${SRC} ${BLU} ${HWC} ${WLC})

#===============================================================================
# source files
#===============================================================================

  target_sources(app PRIVATE
               ${SRC}/main.c           # main program
               ${WLC}/bl_wl.c          # wireless core
               ${HWC}/bl_hw.c          # hardware core
               ${BLU}/bluccino.c       # work bench
              )
