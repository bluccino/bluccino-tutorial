//==============================================================================
// main.h for 05-roulette (posting custom messages)
//==============================================================================
//
// roulette
//       +-------------+                  +-------------+
//       |             |  RANDOM:RESULT   |             |   LED:SET
//       |    main     |----------------->|     app     |------------>(bl_down)
//       |             | @cix,"color",val |             |   @ix,onf
//       +-------------+                  +-------------+
//
//        main                        app                        bl_down
//        (M)                         (A)                         (D)
//         |       RANDOM:RESULT       |                           |
//         o-------------------------->|          LED:SET          |
//         |     @cix,"color",val      o-------------------------->|
//         |                           |           @-1,0           |
//         |                           |                           |
//         |                           |          LED:SET          |
//         |                           o-------------------------->|
//         |                           |         @(cix+2),1        |
//
//==============================================================================
// roulette.b (event message flow)
//
//  (M) := main;  (A) := app;
//
//  (M) -> [RANDOM:RESULT @cix,"color",val] -> (A);
//
//==============================================================================

#ifndef __MAIN_H__
#define __MAIN_H__

  #define BL_CL_TEXT  {BL_CL_SYMBOLS,"RANDOM"}        // class tag text
  #define BL_OP_TEXT  {BL_OP_SYMBOLS,"RESULT"}        // opcode text

  typedef enum BL_cl  {BL_CL_ENUMS,_RANDOM} BL_cl;    // class tag enum values
  typedef enum BL_op  {BL_OP_ENUMS,RESULT_} BL_op;     // opcode enum values

#endif // __MAIN_H__
