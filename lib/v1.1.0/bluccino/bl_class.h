//==============================================================================
// bl_class.h
// Bluccino C++ base class definitions
//
// Created by Hugo Pristauz on 2022-09-28
// Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================
// Class Hierarchies
//
// BlMod (Bluccino module - base class for all modules)
//  +- BlDown      (down gear)      instance: blDown
//  +- BlTop       (top gear)       instance: blUp
//  +- BlUp        (up gear)        instance: blTop
//  +- BlCore      (core module)    instance: blCore
//  +- BlHw        (harware core)   instance: blHw
//  +- BlWl        (wireless core)  instance: BlWl
//  +- BlGpio      (GPIO module)    instance: blGpio
//  :
//  +- MyApp       (example)        instance: myApp
//  +- MyLed       (example)        instance: myLed
//  +- MyBut       (example)        instance: myBut
//
// BlIfc (bluccino interface - base class for all interface classes)
//  +- BlSys       (SYS interface)
//  +- BlLed       (LED interface)
//  +- BlBut       (BUTTON interface)
//  +- BlSwitch    (SWITCH interface)
//  +- BlNvm       (NVM interface)
//  +- BlPin       (PIN interface)
//  +- BlBle       (BLE interface)
//  +- BlMesh      (MESH interface)
//
// Actual modules classes derive both from BlMod (Bluccino module base class)
// and one or more (derived) interface classes
//
// Examples:
//
//    class MyLed : public BlMod BlSys BlLed
//    class MyBut : public BlMod BlSys BlBut
//    class MyApp : public BlMod BlSys BlLed BlBut
//
//==============================================================================

#ifndef __BL_CLASS_H__
#define __BL_CLASS_H__

  #define In  virtual int
  #define Out virtual int

  class BlMod;                         // forward declaration

//==============================================================================
// BlMsg class (the base class of all Bluccino messages)
//==============================================================================

  class BlMsg
  {
    public:
      BL_cl cl;                        // (interface) class tag
      BL_op op;                        // operation code
      int ix;                          // instance index
      BL_data data;                    // data reference
      int val;                         // simple data
      BL_txt who;                      // who (which module) emitted the message

    public:
      BlMsg() {}

      BlMsg(BL_cl cl_, BL_op op_)
      {
        cl = cl_; op = op_; ix = 0; data = 0; val = 0;  who = "";
      }

      BlMsg(BL_cl cl_, BL_op op_, int ix_)
      {
        cl = cl_; op = op_; ix = ix_; data = 0; val = 0; who = "";
      }

      BlMsg(BL_cl cl_, BL_op op_, int ix_, int val_)
      {
        cl = cl_; op = op_; ix = ix_; data = 0; val = val_; who = "";
      }

      BlMsg(BL_cl cl_, BL_op op_, int ix_, BL_data data_, int val_)
      {
        cl = cl_; op = op_; ix = ix_; data = data_; val = val_; who = "";
      }

      BlMsg(BL_ob *o, int val_, BL_txt who_="")
      {
        cl = o->cl;  op = o->op;  ix = o->ix;
        data = o->data;  val = val_;  who = who_;
      }

      int logo(int lvl,BL_txt txt)
      {
        BL_ob oo = {cl,op,ix,data};
        if (who && *who)
        {
          char buf[40];
          snprintf(buf,sizeof(buf),"%s %s >>",txt,who);
          bl_logo(lvl,buf,&oo,val);
        }
        else
          bl_logo(lvl,txt,&oo,val);
        return 0;
      }

      int log() { return logo(1,""); }

      bool is(BlMsg m)               // does message ID match a given one
      {
        return (cl == m.cl && op == m.op);
      }

      bool is(BL_cl cl_)
      {
        return (cl == cl_);
      }

      bool is(BL_cl cl_, BL_op op_)
      {
        return (cl == cl_ && op == op_);
      }

      inline bool equals(BlMsg m)    // do all msg attributes match given one
      {
        return (cl==m.cl && op==m.op && ix==m.ix && data==m.data && val==m.val);
      }

      friend bool operator==(BlMsg &m1, BlMsg &m2);
  };

//==============================================================================
// BlMod class (the base class of all Bluccino modules)
//==============================================================================

  class BlMod
  {
    private:
      BL_txt who;                      // who am I (module name)?

    public:
      BlMod() { who = ""; }
      BlMod(BL_txt name) { who = name; }

      int logx(BlMsg &msg, BL_cl cl)   // log with exclusion of `cl` classs tags
		  {
			  !msg.is(cl) && msg.logo(1,who);	 return 0;
		  }

      virtual int input(BlMsg &msg)    // pass message by reference
      {
        if (!msg.is(_SYS,TICK_))
          msg.logo(4,"BlMod::input:");
        return 0;
      }

      virtual int output(BlMsg &msg)
      {
        msg.logo(4,"BlMod::output:");
        return 0;
      }

        // next method allows to pass a message by value,
        // i.e. myApp.in(Led_Toggle(-1))

//    int in(BlMsg msg) { return input(msg); }   // pass message by value

      friend int operator>>(BlMod *from, BlMsg msg);
  };

//==============================================================================
// class Interface (bluccino message interface)
//==============================================================================

  class BlIfc
  {
    public: //private:
      BL_cl cl;

    public:
      BlIfc(BL_cl cl_) { cl = cl_; }
      bool is(BL_cl cl_) { return cl_ == cl; }

      virtual int dispatch(BlMsg &msg)
      {
        msg.logo(1,BL_R"BlIfc::dispatch:");
        return 0;
      }
  };

//==============================================================================
// BlMod class: comparision operator
//==============================================================================

  inline bool operator==(BlMsg &m1, BlMsg &m2)
  {
    return (m1.cl==m2.cl && m1.op==m2.op);
  }

//==============================================================================
// BlMod class: operator to send to module
// - usage: err = Led_Set(2,1) >> myApp
// - note: also err = myApp << Led_Set(2,1) would work simultaneously
//==============================================================================
/*
  inline int operator<<(BlMod &to, BlMsg msg)
  {
    return to.input(msg);    // [...] >> (to);
  }
*/
  inline int operator>>(BlMsg msg, BlMod &to)
  {
    return to.input(msg);    // [...] >> (to);
  }

//==============================================================================
// BlMod class: efficient operator to send to module (given by module address)
// - usage: auto msg = Led_Set(2,1); err = msg >> pApp
//==============================================================================

  inline int operator>>(BlMsg &msg, BlMod *to)
  {
    return to->input(msg);
  }

//==============================================================================
// BlMod class: auxillary operator to send message from interface
// - usage: LED >> msg >> blDown
//==============================================================================

  inline BlMsg *operator>>(BlIfc &ifc, BlMsg &msg)
  {
//  msg.logo(1,BL_G "Left: BlMsg *operator>>(BlIfc&,BlMsg&)");
    return ifc.is(msg.cl) ? &msg : NULL;
  }

  inline int operator>>(BlMsg *msg, BlMod &module)
  {
//  msg->logo(1,BL_G "Right: int operator>>(BlMsg*,BlMod&)");
    return msg ? module.input(*msg) : -1;
  }

//==============================================================================
// BlMod class: output operator
// - usage: err = this >> Led_Set(2,1)
//==============================================================================

  inline int operator>>(BlMod *from, BlMsg msg)
  {
    msg.who = from->who;
//  bl_log(4,BL_C"%s >> [%s:%s @%d,%d]", msg.who,
//         BL_IDTXT(BL_ID(msg.cl,msg.op)), msg.ix,msg.val);
    return from->output(msg);    // [...] >> (to);
  }

//==============================================================================
// input operator to send message into message interface (highly efficient)
// - usage: err = msg >> app.LED
//==============================================================================

  inline int operator>>(BlMsg &msg, BlIfc &ifc)
  {
    return ifc.dispatch(msg);
  }

//==============================================================================
// down gear class (translates from BlMsg type  to Oval messages)
//==============================================================================

  class BlDown : public BlMod
  {
    public:
      BlDown(BL_txt txt) : BlMod(txt) {}
      virtual int input(BlMsg &msg);
  };

  extern BlDown blDown;                // class instance

//==============================================================================
// Bluccino class (swiss knife)
// - usage: Bluccino bl;
//==============================================================================

  class Bluccino
  {
    public:
      void hello(int verbose, BL_txt msg);
      void engine(BlMod &app, int tick_ms, int tock_ms);

//    int init(BlMod &module) { return module << Sys_Init(); }
//    int init(BlMod &module, BlMod &cb) { return module << Sys_Init(cb); }
      int init();
      int init(BL_oval module);
      int init(BlMod &cb);
      int init(BL_oval module,BL_oval cb);

        // work bench

      void bench(int verbose, BL_txt txt)
      {
        hello(verbose,txt);  init();
      }

        // gear

      BlMod &down = blDown;

        // timing swiss knife

      void sleep(BL_ms ms);
  };

  extern Bluccino bl;                  // Bluccino swiss knife

//==============================================================================
// footer
//==============================================================================

#endif // __BL_CLASS_H__
