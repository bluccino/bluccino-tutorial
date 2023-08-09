# SPDX-License-Identifier: Apache-2.0

	cmake_minimum_required(VERSION 3.20.0)
	set(QEMU_EXTRA_FLAGS -s)

	find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

#===============================================================================
# project personality (definitions to be pushed down to C-level)
#===============================================================================

  project(04-meshnvm)

  add_definitions(-DPROJECT="${CMAKE_PROJECT_NAME}")
  add_definitions(-DVERBOSE=4)                             # verbose level
  add_definitions(-DCFG_MESH_AUTO_INIT=1)

    # log control

  add_definitions(-DCFG_LOG_GEAR=0)    # no gear logging
  add_definitions(-DCFG_LOG_LED=0)     # no LED logging

#===============================================================================
# path setup
#===============================================================================

  set (LIB ../../../lib/v1.1.0)
  set (BLU ${LIB}/bluccino)            # Bluccino stuff
  set (HWC ${LIB}/core/hwcore/hwtiny)  # tiny hardware core
  set (WLC ${LIB}/core/wlcore/wlstd)   # standard wireless core
  set (LMO ${LIB}/module)
  set (SRC src)

  include_directories(${SRC} ${BLU} ${LMO} ${HWC} ${WLC})

#===============================================================================
# source files
#===============================================================================

target_sources(app PRIVATE
	       ${SRC}/main.c
				 ${BLU}/bluccino.c
	       ${HWC}/bl_hw.c
	       ${WLC}/bl_wl.c
	       )

target_sources_ifdef(CONFIG_MCUMGR app PRIVATE ${CORE}/smp_svr.c)
