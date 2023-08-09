//==================================================================================================
// ifc/nvm.h
// NVM interface class
//
// Created by Hugo Pristauz on 2022-Oct-15
// Copyright © 2022 Bluenetics. All rights reserved.
//==================================================================================================

#ifndef __IFC_NVM_H__
#define __IFC_NVM_H__

//==============================================================================
// config defaults
//==============================================================================

  #ifndef CFG_LOG_BL_NVM
    #define CFG_LOG_BL_NVM   0
  #endif

//==================================================================================================
// NVM message class definition
//==================================================================================================
//                  +--------------------+
//                  |        NVM:        | NVM input interface
//           LOAD --|      <BL_tray>     | load NVM data
//           SAVE --|      <BL_tray>     | save NVM data
//        SUPPORT --|                    | is NVM functionality supported?
//          READY --|       ready        | notification that NVM is now ready
//                  +--------------------+
//==================================================================================================

  class Nvm_Load : public BlMsg
  {
    public:
      Nvm_Load(BL_tray &tray) : BlMsg(_NVM,LOAD_, 0,&tray,0) {}
  };

  class Nvm_Save : public BlMsg
  {
    public:
      Nvm_Save(BL_tray &tray) : BlMsg(_NVM,SAVE_, 0,&tray,0) {}
  };

  class Nvm_Support : public BlMsg
  {
    public:
      Nvm_Support() : BlMsg(_NVM,SUPPORT_, 0,NULL,0) {}
  };

  class Nvm_Ready : public BlMsg
  {
    public:
      Nvm_Ready(bool ready) : BlMsg(_NVM,READY_, 0,NULL,ready) {}
  };

//==================================================================================================
// BlNvm class (button interface)
//==================================================================================================

  class BlNvm : public BlIfc
  {
    public:
      virtual int NVM_LOAD(BL_tray &tray) { return BL_VOID2; }
      virtual int NVM_SAVE(BL_tray &tray) { return BL_VOID2; }
      virtual int NVM_SUPPORT() { return BL_VOID2; }
      virtual int NVM_READY(bool ready) { return BL_VOID2; }

    public:
      BlNvm() : BlIfc(_NVM) {}
      int dispatch(BlMsg &msg)
      {
        switch (msg.op)
        {
          case LOAD_:    return NVM_LOAD( *((BL_tray*)msg.data) );
          case SAVE_:    return NVM_SAVE( *((BL_tray*)msg.data) );
          case SUPPORT_: return NVM_SUPPORT();
          case READY_:   return NVM_READY((bool)msg.val);
          default: return BL_VOID1;
        }
      }
  };

#endif // __IFC_NVM_H__
