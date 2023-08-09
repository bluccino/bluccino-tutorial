//==============================================================================
//  bl_chart.h
//  event flow chart and graphical interface generation
//
//  Created by Hugo Pristauz on 2022-10-27
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_CHART_H__
#define __BL_CHART_H__

  #include "bl_type.h"
  #include "bl_lisp.h"

//==============================================================================
// provide flow: lookup flow-name in lists.flow structure and create if !exists
// - usage: pair = ef_current_scenario(bl_sym("my-flow")) // pair=(my-flow ...)
//==============================================================================

  BL_lisp ef_current_scenario(BL_lisp name);

//==============================================================================
// list scenarios
//==============================================================================

  void ef_scenarios(void);

//==============================================================================
// add alias or flow to current scenario
// - usage: aliases = ef_add_alias("G","gpio") // G = gpio();
// - usage: flows = ef_add_flow("app","[LED:OFF]","ldrv"); // A>>LED.OFF()>>L;
//==============================================================================

  BL_lisp ef_add_alias(BL_txt alias, BL_txt module);
  BL_lisp ef_add_flow(EF_type type, BL_txt from, BL_txt expr, BL_txt to);

#endif // __BL_CHART_H__
