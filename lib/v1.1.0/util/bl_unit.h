//==============================================================================
//  bl_unit.h
//  Bluccino unit test harness
//
//  Created by Hugo Pristauz on 2022-08-28
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================
// how to use:
//
// int test_module(int nmb)
// {
//   ut_begin(nmb,"Module Test");
//   ut_claim("can init module");
//   ut_assert(module_init()==0);
//   return ut_end();
// }
//
// int test_driver(int nmb)
// {
//   ut_begin(nmb,"Driver Test");
//   ut_claim("can init driver");
//   ut_assert(driver_init()==0);
//   return ut_end();
// }
//
// void main(void)
// {
//   bl_hello(4,"unit test sample");
//   ut_suite("Unit test demo");       // start unit test suite
//   test_module(1);
//   test_driver(2);
//     :  :
//   ut_result(true);                  // enter halt mode after printing results
// }
//==============================================================================

#ifndef __BL_UNIT_H__
#define __BL_UNIT_H__

#ifdef __cplusplus
  extern "C" {
#endif

  #include "bl_type.h"

//==============================================================================
// public API: print test results
//==============================================================================

  void ut_suite(BL_txt suite);
  void ut_result(bool all);

  void ut_begin(int major, BL_txt name);
  int ut_end(void);

  void ut_claim(BL_txt txt);
  void ut_assert(int condition);

  void ut_halt(void);

//==============================================================================
// C declarations done
//==============================================================================

#ifdef __cplusplus
  }
#endif

//==============================================================================
// C++ class definition for unit test harness
// - usage: UnitTest ut;
//          ut.suite("test suite"); ut.begin(1,"1st test"); ...
//==============================================================================

#ifdef __cplusplus

  class UnitTest
  {
    public:
      void suite(BL_txt txt) { ut_suite(txt); }
      void result(bool all) { ut_result(all); }

      void begin(int major, BL_txt name) { ut_begin(major,name); }
      int end(void) { return ut_end(); }

      void claim(BL_txt txt) { ut_claim(txt); }
      void assert(int condition) { ut_assert(condition); }

      void halt(void) { ut_halt(); }
  };

#endif

//==============================================================================
// footer
//==============================================================================

#endif // __BL_UNIT_H__
