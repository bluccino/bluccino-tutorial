//==============================================================================
//  bl_unit.c
//  Bluccino unit test harness
//
//  Created by Hugo Pristauz on 2022-08-28
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_unit.h"

  #ifndef CFG_MAX_UNITTESTS
    #define CFG_MAX_UNITTESTS   200    // max 200 unit tests per test suite
  #endif

  typedef struct UT_test               // unit test result
  {
    int major;                         // major test number
    int minor;                         // minor test number
    BL_txt name;                       // name of test
    int errors;                        // number of errors
  } UT_test;

  typedef struct UT_control
  {
    BL_txt suite;
    BL_txt claim;
    UT_test tests[CFG_MAX_UNITTESTS];
    int group;                         // current test group
    int number;                        // current test number
  } UT_control;

//==============================================================================
// Unit test control structure
//==============================================================================

  static UT_control control =          // unit test control structure
         {
           number:0, group:0,          // current test number = group = 0
         };

//==============================================================================
// slow down for SEgger RTT
//==============================================================================

  static void slow(int ms)
  {
    bl_sleep(7*ms);
  }

//==============================================================================
// public API: halt system
//==============================================================================

  void ut_halt(void)
  {
    for (;;)
      bl_sleep(1000);
  }

//==============================================================================
// public API: begin test suite
// - store name
// - store verbose level
//==============================================================================

  void ut_suite(BL_txt suite)
  {
    int verbose = bl_verbose(4);       // temporary change of verbose level

    bl_log(0,BL_M "Unit test suite: %s", suite);
    control.suite = suite;
    UT_test *p = control.tests + control.number;
    p->major = p->minor = 0;
    p->name = suite;
    control.group = 0;
    control.number++;

    bl_verbose(verbose);               // restore verbose level
  }

//==============================================================================
// public API: begin new test
// - store name
// - store verbose level
//==============================================================================

  void ut_begin(int major, BL_txt name)
  {
    int verbose = bl_verbose(4);       // temporary change of verbose level

    bl_log(1,BL_M "Test %d (%s) ...",major,name);

    if (!control.number && control.number >= BL_LEN(control.tests))
      bl_err(-1,"ut_check: out of table entries");
    else
    {
      UT_test *p = control.tests + control.number;
      p->major = (p-1)->major+1;
      p->minor = 0;
      p->name  = name;
      p->errors = 0;

      control.group = control.number;
      control.number++;
    }

    bl_verbose(verbose);               // restore verbose level
  }

//==============================================================================
// public API: begin new test
// - store name
// - store verbose level
//==============================================================================

  static int ut_check(int err, BL_txt txt)
  {
    int verbose = bl_verbose(4);       // temporary change of verbose level

    slow(5);

    if (!control.number || control.number >= BL_LEN(control.tests))
      bl_err(-1,"ut_check: out of table entries");
    else
    {
      UT_test *p = control.tests + control.number;
	    p->major = (p-1)->major;
	    p->minor = (p-1)->minor + 1;
      p->name = txt;
      control.number++;

      if (err == 0)
        bl_log(2,BL_G "%2d.%02d: OK    %s",p->major,p->minor,txt);
      else
        bl_log(2,BL_R "%2d.%02d: FAIL  %s (error: %d)",
               p->major,p->minor, txt,err);

      slow(5);                         // slow down for Segger RTT

      p->errors += (err ? 1 : 0);
      control.tests[control.group].errors += (err != 0);
    }

    bl_verbose(verbose);               // restore verbose level
    return err;
  }

//==============================================================================
// public API: begin new test
// - store name
// - store verbose level
//==============================================================================

  int ut_end(void)
  {
    int verbose = bl_verbose(4);       // temporary change of verbose level

    UT_test *p = control.tests + control.number;
    bl_verbose(verbose);               // restore verbose level
    return p->errors;
  }

//==============================================================================
// public API: print test results
// - usage: ut_result(1)   // print results and call ut_halt() function at end
//          ut_result(0)   // print results without calling ut_halt() function
//==============================================================================

  void ut_result(bool halt)
  {
    int total = 0;
    #define HALF_LINE "----------------------------------------"
    #define LINE      HALF_LINE HALF_LINE
    #define TAB ""

    int verbose = bl_verbose(4);       // temporary change of verbose level
    slow(5);   // slow down for Segger RTT

    bl_prt(TAB "\n");
    bl_prt(TAB LINE"\n");
    bl_prt(TAB "  Results for test suite: %s\n",control.suite);
    bl_prt(TAB LINE"\n");

    for (int i=1; i < control.number; i++)
    {
      UT_test *p = control.tests + i;
      if (p->minor == 0)
        bl_prt(TAB "%s  Test %d %s %s\n" BL_0, p->errors ? BL_R : "",
             p->major, p->errors ? "FAIL:":"OK:", p->name);
      else
        bl_prt(TAB "%s    Case %2d.%02d %-4s %s\n" BL_0, p->errors ? BL_R : "",
             p->major,p->minor, p->errors ? "FAIL:":"OK:", p->name);

      if (p->minor > 0)
        total += p->errors;

      slow(5);                         // slow down for Segger RTT
    }

    bl_prt(TAB LINE"\n");
    bl_prt(TAB "%s  Total: %d error%s\n" BL_0, total?BL_R:"",
           total, total==1?"":"s");
    bl_prt(TAB LINE"\n");

    if (control.number >= BL_LEN(control.tests))
      bl_prt(BL_R TAB "table overflow: not all tests could be recorded\n");

    bl_verbose(verbose);               // restore verbose level

    if (halt)
      ut_halt();
  }

//==============================================================================
// public API: claim a condition
//==============================================================================

  void ut_claim(BL_txt txt)
  {
    UT_test *p = control.tests + control.number;
    int major = (p-1)->major;
    int minor = (p-1)->minor + 1;
    //control.number++;

    slow(5);
    bl_log(2,BL_M "%2d.%02d: %s",major,minor,txt);

    control.claim = txt;
  }

//==============================================================================
// public API: assert condition regarding a claim
//==============================================================================

  void ut_assert(int condition)
  {
    ut_check(!condition,control.claim);
  }
