//==============================================================================
//  bl_chart.c
//  event flow chart and graphical interface generation
//
//  Created by Hugo Pristauz on 2022-10-27
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_flow.h"
  #include "bl_chart.h"

//==============================================================================
// shorthands ("bl_" prefix is to cumbersome when working in LISP style)
//==============================================================================

  #define car     bl_car
  #define cdr     bl_cdr
  #define cons    bl_cons
  #define symb    bl_sym
  #define islist  bl_islist
  #define issym   bl_issym
  #define isnil   bl_isnil
  #define append  bl_append
  #define assoc   bl_assoc
  #define logl    bl_logl
  #define prt     bl_prt
  #define NIL     BL_NIL

  #define namecar(ref)  bl_name(bl_car(ref))
  #define namecdr(ref)  bl_name(bl_cdr(ref))

//==============================================================================
// create symbol from non empty string (otherwise return NIL)
//==============================================================================

  BL_lisp symbol(BL_txt txt)
  {
    return (!txt || !*txt) ? NIL : symb(txt);
  }

//==============================================================================
// list structures to track some items in some specific order
// - modules = ((app (...)) (ldrv (...)) (bdrv (...)))
// - scenarios = ((init-flow (...)) (toggle-flow (...)) (off-flow (...)))
//==============================================================================

  typedef struct EF_lists
          {
            BL_lisp modules;
            BL_lisp scenarios;
            BL_lisp context;           // context of current scenario
          } EF_lists;

  static EF_lists lists = {modules:0, scenarios:0, context:0};

//==============================================================================
// list banner
// - usage: banner("scenarios:"); // print "scenarios:" between banner lines
//          banner(NULL); // print banner line only
//==============================================================================

  #define BAR "--------------------------------"

  void banner(BL_txt txt)
  {
    if (!txt)
      bl_prt(BAR BAR "\n");
    else
    {
       banner(NULL);                   // print banner line
       bl_prt("%s\n",txt);
       banner(NULL);                   // print banner line
    }
  }

//==============================================================================
// provide flow: lookup flow-name in lists.flow structure and create if !exists
// - usage: pair = ef_current_scenario(symb("my-flow")) // pair=(my-flow ...)
//==============================================================================

  BL_lisp ef_current_scenario(BL_lisp sym)
  {
    BL_lisp pair = assoc(sym, lists.scenarios);

    if (!pair)
    {
      BL_lisp aliases = bl_cons(symb("*alias*"),BL_NIL);
      BL_lisp flows = bl_cons(symb("*flow*"),BL_NIL);

      BL_lisp assoc_table = bl_cons(aliases,BL_NIL);
      assoc_table = bl_append(assoc_table,flows);
      pair = bl_cons(sym,assoc_table);

      lists.scenarios = bl_append(lists.scenarios,pair);
//logl(3,BL_G "scenarios: ",lists.scenarios);
    }

    return lists.context = pair;
  }

//==============================================================================
// helper: convert type enum value to type symbol
// - usage: sym = type_sym(ef_type);
//==============================================================================

  static BL_lisp type_sym(EF_type type)
  {
    BL_txt typetext;
    switch (type)
    {
      case EF_FLOW: typetext = "<flow>"; break;
      case EF_ACTION: typetext = "<action>"; break;
      default: typetext = "<?>"; break;
    }
    return symbol(typetext);
  }

//==============================================================================
// add alias or flow to current scenario
// - usage: aliases = ef_add_alias("G","gpio") // G = gpio();
// - usage: flows = ef_add_flow(type,"app","[LED:OFF]","ldrv");
// -        // A >> LED.OFF() >> L;
//==============================================================================

  BL_lisp ef_add_alias(BL_txt alias, BL_txt module)
  {
    BL_lisp context = cdr(lists.context);
    BL_lisp aliases = assoc(symb("*alias*"),context);

    if (bl_islist(aliases))
    {
      BL_lisp pair = cons(symb(alias),symb(module));
      aliases = append(aliases,pair);
      return aliases;
    }
    return BL_BAD;
  }

  BL_lisp ef_add_flow(EF_type type, BL_txt from, BL_txt expr, BL_txt to)
  {
    BL_lisp context = cdr(lists.context);
    BL_lisp flows = assoc(symb("*flow*"),context);
//bl_log(4,"(%p) %s >> %s >> %s (%p)",from, from,expr,to, to);

    if (bl_islist(flows))
    {
      BL_lisp head = cons(type_sym(type), symbol(from));
      BL_lisp tail = cons(symbol(expr), symbol(to));
      BL_lisp flow = cons(head,tail);

//logl(3,BL_C"add flow: ",flow);
//bl_sleep(50);
      flows = append(flows,flow);
//logl(3,BL_C"flows: ",flows);
//bl_sleep(100);
      return flows;
    }
    return BL_BAD;
  }

//==============================================================================
// list scenarios
//==============================================================================

  void ef_scenarios(void)
  {
    BL_lisp scenarios = lists.scenarios;

    banner("scenarios");

    for (;islist(scenarios);scenarios = cdr(scenarios))
    {
      BL_lisp scenario = car(scenarios);
      prt("  scenario " BL_G "%s:\n"BL_0,namecar(scenario));

        // print aliases

      bl_prt("    alias:\n");
      BL_lisp aliases = assoc(symb("*alias*"),scenario);

      for (aliases = cdr(aliases); islist(aliases); aliases = cdr(aliases))
      {
        BL_lisp pair = car(aliases);
        bl_prt("      %s = %s();\n",namecar(pair),namecdr(pair));
      }

        // print flows

      bl_prt("    flow:\n");
      BL_lisp flows = assoc(symb("*flow*"),scenario);

      for (flows = cdr(flows); islist(flows); flows = cdr(flows))
      {
bl_sleep(5);
        BL_lisp flow = car(flows);
        BL_lisp head = car(flow);
        BL_lisp tail = cdr(flow);
        BL_lisp type = car(head);

        if (type == symb("<flow>"))
        {
          bl_prt("      %s >> %s >> %s;\n",
               namecdr(head), namecar(tail), namecdr(tail));
        }
        else if (type == symb("<action>"))
        {
          BL_lisp from = cdr(head), to = cdr(tail);
          if ( isnil(from) )
            bl_prt("      Action(\"%s\") >> %s;\n",namecar(tail),namecdr(tail));
          else if (isnil(to) )
            bl_prt("      %s >> Action(\"%s\");\n",namecdr(head),namecar(tail));
          else
            bl_prt("      " BL_Y "%s:" BL_0 " %s >> %s >> %s;\n",
               namecar(head), namecdr(head), namecar(tail), namecdr(tail));
        }
        else
          bl_err(-1,"ef_scenarios: bad data");
      }
    }

    banner(NULL);
  }
