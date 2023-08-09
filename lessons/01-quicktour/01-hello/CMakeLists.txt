# SPDX-License-Identifier: Apache-2.0

  cmake_minimum_required(VERSION 3.20.0)
  find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

#===============================================================================
# project personality (definitions to be pushed down to C-level)
#===============================================================================

  project(01-hello)

  set (V_SDK v1.8.0)                   # nRF Connect SDK toolchain version
  set (V_BLU v1.1.0)                  # Bluccino version def. for Cmake

  add_definitions(-DV_SDK="v1.8.0")    # definition for C-compiler
  add_definitions(-DV_BLU="v1.1.0")   # definition for C-compiler

  add_definitions(-DPROJECT="${CMAKE_PROJECT_NAME}")

#===============================================================================
# path setup
#===============================================================================

  set (LIBBLU ../../../lib/${V_BLU})
  set (BLU ${LIBBLU}/bluccino)
  set (SRC src)

  include_directories(${SRC} ${BLU})

#===============================================================================
# source files
#===============================================================================

  target_sources(app PRIVATE ${SRC}/main.c ${BLU}/bluccino.c)
