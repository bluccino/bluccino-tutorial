//==============================================================================
//  bl_flow.c
//  event flow tools
//
//  Created by Hugo Pristauz on 2022-10-26
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

  #include <string.h>

  #include "bluccino.h"
  #include "bl_flow.h"
  #include "bl_lisp.h"
  #include "bl_chart.h"

//==============================================================================
// logging shorthands & PMI definition
//==============================================================================

  #define WHO "bl_flow"           // who is logging

  #define LOG                     LOG_FLOW
  #define LOGO(lvl,col,o,val)     LOGO_FLOW(lvl,col WHO ":",o,val)

  #define PMI bl_flow
  BL_PMI(PMI)

//==============================================================================
// defines
//==============================================================================

   #define BAR "--------------------------------"

//==============================================================================
// event flow profile
// - translation of statements into `EF_entry` data structure as follows:
//   A = alpha                           => {EF_ALIAS,"alpha",NULL,"A"}
//   flow A >> NVM.LOAD(1,NULL,2) >> B;  => {EF_FLOW,"A",NVM.LOAD(...),"B"}
//   flow NVM.LOAD(1,NULL,2) >> B;       => {EF_FLOW,NULL,NVM.LOAD(...),"B"}
//   flow A >> NVM.LOAD(1,NULL,2);       => {EF_FLOW,"A",NVM.LOAD(...),NULL}
//   flow A >> action("end flow");       => {EF_ACTION,"A","end flow",NULL}
//   flow action("end flow") >> "B";     => {EF_ACTION,NULL,"end flow","B"}
//==============================================================================

  typedef struct EF_item               // event flow item
  {
    EF_type type;                      // type of entry
    EF_txt src;                        // source module alias or action text
    EF_txt expr;                       // event message text
    EF_txt dst;                        // dest. module alias or action text
    EF_txt from;                       // `from` module name
    EF_txt to;                         // `to` module name
    int next;                          // index to next module
    int link;                          // auxillary link index for temp lists
  } EF_item;

  typedef struct EF_items
  {
    EF_item *items;
    int count;
    int len;
  } EF_items;

    // BF_item initializer

  #define EF_ITEM() {type:EF_BAD, src:NULL,expr:NULL,dst:NULL, next:-1}

//==============================================================================
// event flow stack and event flow profile
//==============================================================================

  #define STACK_LEN 20
  #define PROFILE_LEN 1000
  #define BAG_LEN 200

  static EF_item stack_items[STACK_LEN];
  static EF_items stack = {items:stack_items, count:0, len:STACK_LEN};

  static EF_item profile_items[PROFILE_LEN];
  static EF_items profile = {items:profile_items, count:0, len:PROFILE_LEN};

  static int bag[BAG_LEN];             // bag of indices for various operations

//==============================================================================
// helper: slow down to give time for Segger RTT logging
//==============================================================================

  static inline void slow(int ms)
  {
    bl_sleep(2*7*ms);
  }

//==============================================================================
// helper item name (module name of an item with type EF_ALIAS)
//   usage: module = ef_name(idx)    // pick module name
//          symbol = ef_sym(idx)     // pick alias name
//          from = ef_from(idx)      // from module name
//          to = ef_to(idx)          // to module name
//          src = ef_src(idx)        // item source
//          dst = ef_dst(idx)        // item destination
//          expr = ef_expr(idx)      // event/action expression
//          type = ef_type(idx)      // item type
//==============================================================================

  static inline EF_txt ef_name(int idx)
  {
    return profile.items[idx].src;
  }

  static inline EF_txt ef_sym(int idx)
  {
    return profile.items[idx].dst;
  }

  static inline EF_txt ef_from(int idx)
  {
    return profile.items[idx].from;
  }

  static inline EF_txt ef_to(int idx)
  {
    return profile.items[idx].to;
  }

  static inline EF_txt ef_src(int idx)
  {
    return profile.items[idx].src;
  }

  static inline EF_txt ef_dst(int idx)
  {
    return profile.items[idx].dst;
  }

  static inline EF_txt ef_expr(int idx)
  {
    return profile.items[idx].expr;
  }

  static inline EF_type ef_type(int idx)
  {
    return profile.items[idx].type;
  }

//==============================================================================
// items operations
// - usage: err = ef_init(&stack)
//          err = ef_push(&stack,&item)
//          err = ef_pull(&stack,&item)
//          count = ef_count(&stack)
//
// - usage: err = ef_init(&profile)
//          err = ef_push(&profile,&item)
//          count = ef_count(&profile)
//==============================================================================

  static int ef_init(EF_items *p)
  {
    return p->count = 0;
  }

  static int ef_count(EF_items *p)
  {
    return p->count;
  }

  static int ef_push(EF_items *p, EF_item *item)
  {
    if (p->count >= p->len )
      return bl_err(-1,"event flow: out of memory");

    p->items[(p->count)++] = *item;
    return 0;
  }

  static int ef_pull(EF_items *p, EF_item *item)
  {
    if (p->count <= 0)
      return bl_err(-1,"event flow: empty stack/profile");

    *item = p->items[--(p->count)];
    return 0;
  }

//==============================================================================
// helper: skip (by one-by-one iteration to next item with given from/to name
// - usage: idx = ef_skip("module",idx) // index of linked node (-1:err)
//==============================================================================

  static int ef_skip(EF_txt name, int idx)
  {
    for (int i=idx+1; i < ef_count(&profile); i++)
    {
      if (!strcmp(ef_from(i),name) || !strcmp(ef_to(i),name))
        return i;
    }
    return -1;
  }

//==============================================================================
// helper: managing module directory
// - usage: idx = ef_dir()             // seek head of module directory
//          idx = ef_last()            // seek last directory entry
//          idx = ef_next(idx)         // proceed in module dir (-1: list end)
//          idx = ef_link(idx)         // follow linked list (-1: list end)
//          idx = ef_lookup(name)
//          idx = ef_add(head_idx,idx) // add module to directory
//==============================================================================

  static int ef_dir(void)
  {
    for (int i=0; i < profile.count; i++)
    {
      EF_item *p = profile.items + i;
      if (p->type == EF_ALIAS)         // first alias is head of module list
        return i;                      // found - return index of begin of list
    }
    return -1;                         // end of list
  }

  static int ef_next(int idx)
  {
    if (idx < 0) return -1;
    return profile.items[idx].next;
  }

  static int ef_link(int idx)
  {
    if (idx < 0) return -1;
    return profile.items[idx].link;
  }

  static int ef_last()
  {
    int last = ef_dir();

    for (int idx=last; idx >= 0;)
    {
      last = idx;
      idx = ef_next(idx);
    }
    return last;                       // index of last directory item
  }

  static int ef_lookup(EF_txt name)    // lookup module name in dir
  {
    int idx = ef_dir();                // seek head of module list

    while (idx >= 0)
    {
      if (strcmp(ef_name(idx),name)==0)
        return idx;
      idx = ef_next(idx);
    };
    return -1;                         // not found
  }

  int ef_add(int first_idx, int idx)   // add module to directory
  {
    int last_idx = ef_last(first_idx);

    if (last_idx >= 0 && last_idx != idx)
    {
      profile.items[last_idx].next = idx;
      if (idx >= 0)
      {
        profile.items[idx].next = -1;
        return idx;
      }
    }
    return -1;
  }

//==============================================================================
// helper: use specific module (setup linked list)
// - usage: idx = ef_use("app")
//==============================================================================

  int ef_use(EF_txt name)
  {
    int idx = ef_lookup(name);         // lookup module name in dir

    int skip;
    for (int i=idx; i >= 0; i = skip)
    {
      skip = ef_skip(name,i);
      profile.items[i].link = skip;    // set skip index
    }

    return idx;
  }

//==============================================================================
// helper: list directory
//==============================================================================

  void ef_directory(void)
  {
slow(50);
    bl_prt(BL_0 BAR BAR "\n");
    bl_prt("Directory\n");
    bl_prt(BL_0 BAR BAR "\n");

slow(50);
    for (int idx=ef_dir(); idx >= 0; idx = ef_next(idx))
    {
slow(5);
      EF_txt name = ef_name(idx);
      bl_prt("  %03d  " BL_G "module %s:\n" BL_0,idx,name);

      if (ef_use(name) < 0)
        bl_err(-1,"event flow: internal error (ef_use)");

      for (int jdx=idx; jdx >= 0; jdx = ef_link(jdx))
      {
slow(5);
        switch (ef_type(jdx))
        {
          case EF_ALIAS:
            bl_prt(BL_Y"  %03d    alias %s = %s (from/to: %s/%s)\n"BL_0,jdx,
              ef_sym(jdx), ef_name(jdx),ef_from(jdx),ef_to(jdx));
            break;
          case EF_ACTION:
            if (!ef_src(jdx))
              bl_prt("  %03d    \"%s\" >> %s   (from/to: %s/%s)\n",jdx,
                ef_expr(jdx),ef_dst(jdx),  ef_from(jdx),ef_to(jdx));
            else if (!ef_dst(jdx))
              bl_prt("  %03d    %s >> \"%s\"   (from/to: %s/%s)\n",jdx,
                ef_src(jdx),ef_expr(jdx),  ef_from(jdx),ef_to(jdx));
            else
              bl_prt("  %03d    %s >> \"%s\" >> %s   (from/to: %s/%s)\n",jdx,
                ef_src(jdx),ef_expr(jdx),ef_dst(jdx),  ef_from(jdx),ef_to(jdx));
            break;
          default:
            bl_prt(BL_M"  %03d    %s >> %s >> %s   (from/to: %s/%s)\n"BL_0,jdx,
              ef_src(jdx),ef_expr(jdx),ef_dst(jdx),  ef_from(jdx),ef_to(jdx));
            break;
        }
      }
    }
    bl_prt(BAR BAR "\n");
  }

//==============================================================================
// helper: update next and link entries on arrival of alias declaration
// - usage: ef_update("alpha")
// - 1) check whether module list exists all ready
// - 2) if not exists => add list to module ditectory
// - 3) add item to module list
//==============================================================================

  static void ef_update(EF_txt name, int index)
  {
bl_log(4,BL_Y"lookup '%s' (index:%d)",name,index);
    int idx = ef_lookup(name);         // lookup module name in dir
    bool found = (idx >= 0);

    if (idx < 0)
    {
      idx = ef_last();                 // get to end of directory
bl_log(4,"go to last: %03d",idx);
    }

    if (index >= 0 && ef_type(index) == EF_ALIAS)
    {
bl_log(4,"alias: copy to/from '%s'",ef_name(index));
      profile.items[index].from = ef_name(index);
      profile.items[index].to = ef_name(index);
    }

    if (idx >= 0 && idx != index && !found)
      ef_add(idx,index);
  }

//==============================================================================
// helper: refresh from/to and linked lists
//==============================================================================

  static void ef_refresh(int idx)
  {
      // resolve from alias

    EF_txt src = ef_src(idx);
    EF_txt dst = ef_dst(idx);

    for (int i=ef_count(&profile)-1; i>=0; i--)
    {
      if (ef_type(i)==EF_ALIAS && ef_sym(i) && !strcmp(src,ef_sym(i)))
      {
        profile.items[idx].from = ef_name(i);
        bl_log(5,BL_B"refresh from := %s (%03d: alias %s)",
               ef_from(idx), i,ef_sym(i));
        break;
      }
    }

    for (int i=ef_count(&profile)-1; i>=0; i--)
    {
      if (ef_type(i)==EF_ALIAS && ef_sym(i) && !strcmp(dst,ef_sym(i)))
      {
        profile.items[idx].to = ef_name(i);
        bl_log(5,BL_B"refresh to := %s (%03d: alias %s)",
                     ef_to(idx), i,ef_sym(i));
        break;
      }
    }
  }

//==============================================================================
// helper: has bag already symbol inside (bag is given by set of indices)?
// - usage: len = bag_has(idx, &bag, int len)
// - sym: symbol text to look for
// - bag: array of indices with length len
// - return i >= 0 if there is an i with ef_sym(bag[i]) == sym (0 <= i < len)
// - otherwise return -1 (symbol not in bag)
//==============================================================================

  static int bag_has(BL_txt sym, int bag[], int len)
  {
    for (int i=0; i < len; i++)
    {
      int bdx = bag[i];
      if (!strcmp(ef_sym(bdx),sym))
        return i;
    }
    return -1;
  }

//==============================================================================
// helper: add into bag
// - usage: cur = bag_add(idx, &bag, int cur, int len)
// - idx is the number to put into bag, cur is the current length of bag
// - len is the total length of bag, bag is an array of indices with length len
// - return nupdated current length of bag
//==============================================================================

  static int bag_add(int idx, int bag[], int cur, int len)
  {
    if (cur >= len)
      return bl_err(-1,"event flow: bag is full");

    bag[cur++] = idx;                  // put index into bag
    return cur;                        // return updated length of bag
  }

//==============================================================================
// helper: setup alias indices
// - usage: len = ef_alias(idx, &table, int len)
//==============================================================================

  static int ef_alias(int idx, int bag[], int len)
  {
    int n = 0;

    for (int i=idx; i >= 0; i = ef_link(i))  // go through whole linked list
    {
      if (ef_type(i) == EF_ALIAS)
      {
        EF_txt sym = ef_sym(i);        // fetch alias symbol
bl_log(4,"consider: %03d alias %s",i,sym);

        if (bag_has(sym,bag,n))
          continue;
bl_log(4,"add %d to bag (alias %s)",i,ef_sym(i));
        n = bag_add(i,bag,n,len);    // add symbol index to bag
        if (n < 0) return n;           // report error to caller
      }
    }
    return n;                          // return number of item indices in bag
  }

//==============================================================================
// helper: get bag of related modules
// - usage: len = ef_alias(idx, &table, int len)
//==============================================================================
/*
  static int related(int idx, int bag[], int len)
  {
    int n = 0;

    for (int i=idx; i >= 0; i = ef_link(i))  // go through whole linked list
    {
      if (ef_type(i) == EF_ALIAS)
      {
        EF_txt sym = ef_sym(i);        // fetch alias symbol
bl_log(4,"consider: %03d alias %s",i,sym);

        if (bag_has(sym,bag,n))
          continue;
bl_log(4,"add %d to bag (alias %s)",i,ef_sym(i));
        n = bag_add(i,bag,n,len);    // add symbol index to bag
        if (n < 0) return n;           // report error to caller
      }
    }
    return n;                          // return number of item indices in bag
  }
*/
//==============================================================================
// list: interface
//==============================================================================

  void ef_interface(void)
  {
    EF_txt name = "app";
    int idx = ef_use(name);
    int n = ef_alias(idx,bag,BL_LEN(bag));

    bl_prt(BL_R"aliases of %s\n"BL_0,name);
    for (int i=0; i < n; i++)
    {
      int bdx = bag[i];
      bl_prt("  %s = %s\n",ef_sym(bdx), ef_name(bdx));
    }
    bl_prt("\n");
  }

//==============================================================================
// helper safe string
//==============================================================================

  static EF_txt safe(EF_txt txt)
  {
    return (txt == NULL) ? "NULL" : txt;
  }

//==============================================================================
// list event flow profile
//==============================================================================

  void ef_list(void)
  {
 slow(50);
    bl_prt("\n");
    bl_prt(BL_0 BAR BAR "\n");
    bl_prt("Event Flow Profile\n");
    bl_prt(BAR BAR "\n");

slow(50);
    for (int i=0; i<profile.count;i++)
    {
slow(5);
      EF_item *p = profile.items + i;
      switch (p->type)
      {
        case EF_ALIAS:
          bl_prt("  alias %s = %s   (%s|%s)\n",
                 safe(p->dst),safe(p->src), ef_from(i),ef_to(i));
          break;
        case EF_FLOW:
          bl_prt("  flow %s >> %s >> %s   (%s>>%s)\n",
                 safe(p->src),safe(p->expr),safe(p->dst), ef_from(i),ef_to(i));
          break;
        case EF_ACTION:
          if (p->src == 0)
            bl_prt("  flow action(\"%s\") >> %s   (%s>>%s)\n",
                   safe(p->expr),safe(p->dst), ef_from(i),ef_to(i));
          else if (p->dst == 0)
            bl_prt("  flow %s >> action(\"%s\")   (%s>>%s)\n",
                   safe(p->src),safe(p->expr), ef_from(i),ef_to(i));
          else
            bl_prt("  flow %s >> action(\"%s\") >> %s   (%s>>%s)\n",
                  safe(p->src),safe(p->expr),safe(p->dst), ef_from(i),ef_to(i));
          break;
        case EF_BAD:
          bl_prt("  bad profile item\n");
          break;

        case EF_SCENARIO:
          bl_prt("  scenario %s",ef_expr(i));
      }
    }
  }

//==============================================================================
// module declaration statement
//==============================================================================

  int __module__(const char *name)
  {
    LOG(2,"module %s",name);

    EF_item item = EF_ITEM();
    int err = ef_pull(&stack,&item);

    if (err || item.type != EF_ALIAS)
    {
      bl_err(-1,"event flow error: no alias symbol in alias declaration");
      ef_init(&stack);
    }
    else
    {
      item.src = name;
      LOG(5,BL_G "push declaration %s = %s to stack",item.dst,item.src);
		  err = ef_push(&profile,&item);
      ef_add_alias(item.dst,item.src);

      if (!err)
      {
        int idx = ef_count(&profile) - 1;
        ef_update(name,idx);   // update profile (refresh next and link entries)
      }
    }
    return 0;
  }

//==============================================================================
// event symbol
//==============================================================================

  int __event_symbol__(EF_txt msg)
  {
slow(5);
    LOG(2,"event %s",msg);

    if (ef_count(&stack) == 0)
    {
      ef_init(&stack);
      return bl_err(-1,"no source symbol for flow clause");
    }

    EF_item item = EF_ITEM();
    int err = ef_pull(&stack,&item);

    if (item.type != EF_ALIAS)
    {
      ef_init(&stack);
      return bl_err(-1,"bad source for flow clause");
    }

    item.type = EF_FLOW;               // change to a flow clause
    item.src = item.dst;
    item.expr = msg;
    item.dst = NULL;
slow(5);
    LOG(5,BL_C "push %s >> %s onto stack",item.src,item.expr);
    err = ef_push(&stack,&item);
    return err;
  }

//==============================================================================
// action expression
//==============================================================================

  int *__action__(EF_txt txt)
  {
    static int err = 0;
slow(5);
    LOG(6,"action \"%s\"",txt);

    if (ef_count(&stack) == 0)   // syntax: action("begin") >> A
    {
      EF_item item = EF_ITEM();
      item.type = EF_ACTION;
      item.expr = txt;

      LOG(5,BL_C "push action(\"%s\") to stack",txt);
      err = ef_push(&stack,&item);
      return &err;
    }

      // otherwise expect: B >> action("end");

    EF_item item = EF_ITEM();
    err = ef_pull(&stack,&item);
    if (err)
    {
      ef_init(&stack);
      bl_err(err,"event flow error: no alias symbol in alias declaration");
      return &err;
    }

    if (item.type != EF_ALIAS)
    {
      ef_init(&stack);
      err = bl_err(-1,"alias expected");
      return &err;
    }

      // convert {EF_ALIAS,NULL,NULL,dst} to {EF_ACTION,src,expr,NULL} ...

    item.type = EF_ACTION;
    item.src = item.dst;
    item.expr = txt;
    item.dst = NULL;
slow(5);
    LOG(5,BL_G "push %s >> action(\"%s\") to stack",item.src,item.expr);

    err = ef_push(&stack,&item);
    return &err;
  }

//==============================================================================
// alias statement
//==============================================================================

  int *__alias_symbol__(EF_txt name)
  {
	  static int err = 0;
slow(5);
	  LOG(2,"alias %s",name);

      // syntax could be: flow action("begin") >> A

    if (ef_count(&stack) > 0)
    {
      EF_item item;
      err = ef_pull(&stack,&item);

      if (item.type != EF_ACTION)
      {
        item.dst = name;
        err = ef_push(&stack,&item);
        LOG(5,BL_M "push action(\"%s\") >> %s to stack",item.expr,item.dst);
        err = ef_push(&stack,&item);
        return &err;
      }
      else
      {
        item.dst = name;
        LOG(5,BL_Y "push alias %s onto stack",item.dst);
        err = ef_push(&stack,&item);    // undo pull operation
        return &err;
      }
    }

    EF_item item = EF_ITEM();
    item.type = EF_ALIAS;
    item.dst = name;

    LOG(5,BL_Y "push alias %s onto stack",item.dst);
    err = ef_push(&stack,&item);
	  return &err;
  }

//==============================================================================
// flow statement
//==============================================================================

  int *__flow_decl__(void)
  {
    static int err = 0;
slow(5);
    LOG(2,"flow declaration");

    EF_item item = EF_ITEM();
    err = ef_pull(&stack,&item);
    if (!err)
    {
      err = ef_push(&profile,&item);
//    ef_add_flow(item.src,item.expr,item.dst);

 		  int idx = ef_count(&profile) - 1;
			ef_refresh(idx);        // update profile (refresh next and link entries)

      ef_pull(&profile,&item);  // refresh
      ef_push(&profile,&item);  // refresh

      ef_add_flow(item.type,item.from,item.expr,item.to);
    }

    ef_init(&stack);
    return &err;
  }

  void ef_summary(void)
  {
    ef_list();
    ef_directory();
    ef_interface();
    ef_scenarios();                    // list scenarios
  }

//==============================================================================
// stream statement
//==============================================================================

  void __sequence__(EF_txt name)
  {
    LOG(2,"stream declaration: %s",name);
    ef_current_scenario(bl_sym(name));
  }
