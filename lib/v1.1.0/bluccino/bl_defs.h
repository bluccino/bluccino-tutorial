//==============================================================================
//  bl_defs.h
//  Bluccino message symbol definitions
//
//  Created by Hugo Pristauz on 2022-02-22
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_DEFS_H__
#define __BL_DEFS_H__

//==============================================================================
// message class definitions
//==============================================================================

  #define BL_CL_SYMBOLS \
           "VOID","SYS",\
           "OUT","LIB","RESET","TIMER","TEST","TIC","SVC","UART", \
           "SET","GET","MESH","BLE","NGMN","LIGHT","WORK","TASK", \
           "CFGCLI","CFGSRV","HEACLI","HEASRV",\
           "GOOCLI","GOOSRV","GLVCLI","GLVSRV",\
           "BUTTON","SWITCH","LED","NODE","STATE", \
           "CTRL","SCAN","ADV","ADVT","SOS","NVM","TRANS","BT", \
           "CTS","PIN","TOF","DIGITAL","ANALOG","IO"

  #define BL_CL_ENUMS \
            _VOID = 0x7FFF,_SYS = 1, \
            _OUT,_LIB,_RESET,_TIMER,_TEST,_TIC,_SVC,_UART, \
            _SET,_GET,_MESH,_BLE,_NGMN,_LIGHT,_WORK,_TASK, \
            _CFGCLI,_CFGSRV,_HEACLI,_HEASRV,_GOOCLI,_GOOSRV,_GLVCLI,_GLVSRV, \
            _BUTTON,_SWITCH,_LED,_NODE,_STATE, \
            _CTRL,_SCAN,_ADV,_ADVT,_SOS,_NVM,_TRANS,_BT, \
            _CTS,_PIN,_TOF,_DIGITAL,_ANALOG,_IO

//==============================================================================
// message opcode symbols
//==============================================================================

  #define BL_OP_SYMBOLS \
          "VOID","INIT","LIB","OUT","WHEN","USE","TOC","READY","BUSY","CFG", \
          "SVC","INSTALL","PING","PONG","NEXT","PRV","ATT","DUE","SET","LET", \
          "GET","STS","TRIG","TICK","TOCK","CMD","VAL","DECO","LEVEL","SETUP", \
          "ONOFF","COUNT","TOGGLE","INC","DEC","PAY","ADV","NODE","SPOOL", \
          "BEACON","SEND","PRESS","RELEASE","CLICK","HOLD","MS","FOO",     \
          "CHECK","BLINK","STORE","RECALL","SAVE","LOAD","AVAIL","UPDATE", \
          "REPEAT","INTERVAL","RUN","ENABLE","MAC","SIMU","READ","WRITE",  \
          "START","STOP","CONNECT","DISCON","MTU","RECEIPE","BATTERY","ISR", \
          "SERVICE","SUPPORT","IBEACON","EDDY","DIS","HRS","BAS","CTS", \
          "FULL","ATTACH","DATA","RANGE","DIST","INPUT","OUTPUT", \
          "DEVICE","NUSSRV","NUSCLI","FOTA"

  #define BL_OP_ENUMS \
          VOID_ = 0x7FFF,INIT_ = 1,                                  \
          LIB_,OUT_,WHEN_,USE_,TOC_,READY_,BUSY_,CFG_,               \
					SVC_,INSTALL_,PING_,PONG_,NEXT_,PRV_,ATT_,DUE_,SET_,LET_,  \
          GET_,STS_,TRIG_,TICK_,TOCK_,CMD_,VAL_,DECO_,LEVEL_,SETUP_, \
          ONOFF_,COUNT_,TOGGLE_,INC_,DEC_,PAY_,ADV_,NODE_,SPOOL_,    \
          BEACON_,SEND_,PRESS_,RELEASE_,CLICK_,HOLD_,MS_,FOO_,       \
          CHECK_,BLINK_,STORE_,RECALL_,SAVE_,LOAD_,AVAIL_,UPDATE_,   \
          REPEAT_,INTERVAL_,RUN_,ENABLE_,MAC_,SIMU_,READ_,WRITE_,    \
          START_,STOP_,CONNECT_,DISCON_,MTU_,RECEIPE_,BATTERY_,ISR_, \
          SERVICE_,SUPPORT_,IBEACON_,EDDY_,DIS_,HRS_,BAS_,CTS_,      \
          FULL_,ATTACH_,DATA_,RANGE_,DIST_,INPUT_,OUTPUT_,DEVICE_,   \
          NUSSRV_,NUSCLI_,FOTA_

#endif // __BL_DEFS_H__
