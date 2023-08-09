//==============================================================================
//  bl_flow.h
//  event flow tools
//
//  Created by Hugo Pristauz on 2022-10-26
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_FLOW_H__
#define __BL_FLOW_H__

  typedef const char *EF_txt;          // short hand for const char *

//==============================================================================
// log macros
//==============================================================================

#ifndef CFG_LOG_FLOW
  #define CFG_LOG_FLOW    1           // FLOW logging is by default off
#endif

#if (CFG_LOG_FLOW)
  #define LOG_FLOW(l,f,...)    BL_LOG(CFG_LOG_FLOW-1+l,f,##__VA_ARGS__)
  #define LOGO_FLOW(l,f,o,v)   bl_logo(CFG_LOG_FLOW-1+l,f,o,v)
#else
  #define LOG_FLOW(l,f,...)    {}     // empty
  #define LOGO_FLOW(l,f,o,v)   {}     // empty
#endif

//==============================================================================
// event flow typedef
//==============================================================================

  typedef enum EF_type                 // event flow type
  {
    EF_BAD,                            // bad type
    EF_ALIAS,                          // indicating an `alias` declaration
    EF_FLOW,                           // indicating an event flow statement
    EF_ACTION,                         // indicating an action
    EF_SCENARIO,                       // indicating a scenario
  } EF_type;

//==============================================================================
// declaration macros
// - usage: typedef struct
//          {
//            struct{ In(BUTTON); In(LED); } in;
//            struct{ Out(BUTTON); Out(LED); } out;
//          } Gpio;
//   expands to:
//          typedef struct
//          {
//            struct { _BUTTON_ BUTTON; _LED_ LED; } in;
//            struct { _BUTTON_ *BUTTON; _LED_ *LED; } out;
//          }
//==============================================================================

  #define In(CL)   _##CL##_ CL
  #define Out(CL)  _##CL##_ *CL

//==============================================================================
// event definition (for event flow)
// - usage: Interface(BUTTON) = {With(BUTTON,PRESS), With(BUTTON,CLICK))
//   expands to: _BUTTON_ BUTTON = {PRESS:BUTTON_PRESS, CLICK:BUTTON_CLICK}
//==============================================================================

//  #define EventDefine(cl,op)  _##cl##_ cl = {op:cl##_##op}

  #define Interface(cl)  _##cl##_ cl
  #define With(cl,op) op:cl##_##op

//==============================================================================
// event flow handler
//==============================================================================

  #define Message(cl,op,txt,...) \
    static int cl##_##op(__VA_ARGS__) { return __event_symbol__(txt); } \

//==============================================================================
// event symbol
//==============================================================================

  int __event_symbol__(EF_txt msg);

//==============================================================================
// flow statement
//==============================================================================

  int *__flow_decl__(void);

  #define Flow   *__flow_decl__() =

//==============================================================================
// alias symbol definition statement
//==============================================================================

  int *__alias_symbol__(EF_txt name);

  #define Alias(sym)  *__alias_symbol__(#sym)

//==============================================================================
// module declaration statement
//==============================================================================

  int __module__(const char *name);

  #define Module(name)  static int name() { return __module__(#name); }

//==============================================================================
// action statement
//==============================================================================

  int *__action__(EF_txt txt);

  #define Action(txt)  *__action__(txt)

//==============================================================================
// event flow scenario
//==============================================================================

  #define Sequence(fct)   __sequence__(#fct); fct();

  void __sequence__(EF_txt name);

//==============================================================================
// alias symbol definitions
//==============================================================================

  #define A Alias(A)
  #define B Alias(B)
  #define C Alias(C)
  #define D Alias(D)
  #define E Alias(E)
  #define F Alias(F)
  #define G Alias(G)
  #define H Alias(H)
  #define I Alias(I)
  #define J Alias(J)
  #define K Alias(K)
  #define L Alias(L)
  #define M Alias(M)
  #define N Alias(N)
  #define O Alias(O)
  #define P Alias(P)
  #define Q Alias(Q)
  #define R Alias(R)
  #define S Alias(S)
  #define T Alias(T)
  #define U Alias(U)
  #define V Alias(V)
  #define W Alias(W)
  #define X Alias(X)
  #define Y Alias(Y)
  #define Z Alias(Z)

//==============================================================================
// list event flow profile
//==============================================================================

  void ef_list(void);
  void ef_directory(void);
  void ef_summary(void);

  #define Summary ef_summary

#endif // __BL_FLOW_H__
