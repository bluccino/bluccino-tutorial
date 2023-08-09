//==============================================================================
// bl_work.h
// Bluccino (swiss knife) work package submission & semaphore support
//
// Created by Hugo Pristauz on 2022-Aug-27
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

#ifndef __BL_WORK_H__
#define __BL_WORK_H__

//==============================================================================
// Bluccino work data structure definition and initializer
// - usage:
//   BL_work work = BL_WORK(worker);
//   bl_submit(&work,o,val);           // submit work to execute immediately
//   bl_schedule(&work,o,val,time);    // schedule work to execute @ time
//==============================================================================

  typedef struct BL_work
          {
            struct k_work work;        // Zephyr work strukture
            BL_oval oval;              // OVAL interface based worker
            BL_ob oo;                  // Bluccino message object
            int val;                   // value to be transmitted to work horse
          } BL_work;

    // init agregate for BL_work (marks work package as uninitialized)

  #define BL_WORK(module)  {oval:module, oo:{_VOID,VOID_,0,NULL}, val:0}

//==============================================================================
// Bluccino work data structure and work processing in a thread
// -usage: static BL_work work = BL_WORK(module);
//         bl_submit(&work,o,val);     // submit work to execute immediately
//==============================================================================

  int bl_submit(BL_work *p, BL_ob *o, int val);

//==============================================================================
// definition of Bluccino semaphore structure
//==============================================================================

  typedef struct BL_sem
          {
            bool init;                 // is semaphore initialized
            struct k_sem sem;          // Zephyr semaphore
          } BL_sem;

    // init aggregate for semaphores

  #define BL_SEM(cnt,lim)   {init:false, sem:{count:cnt,limit:lim}}

//==============================================================================
// semaphore give operation (return 0 iff sem counter can be increased)
// - usage: BL_sem sem = BL_SEM(0,1); // binary semaphore @ counter:0, limit:1
//          bl_give(&sem); // increment semaphore counter if counter < limit
//==============================================================================

  static inline int bl_give(BL_sem *s)
  {
    if (!s->init)
    {
      k_sem_init(&s->sem,s->sem.count,s->sem.limit);
      s->init = true;
    }
    int ceiling = (s->sem.count >= s->sem.limit);
    k_sem_give(&s->sem);
    return ceiling;
  }

//==============================================================================
// semaphore take operation (return non-zero error wait times out
// - usage: BL_sem sem = BL_SEM(0,1); // binary semaphore @ counter:0, limit:1
//          bl_take(&sem,ms);  // wait for sem. counter > 0 (timeout after ms)
//==============================================================================

  static inline int bl_take(BL_sem *s, int ms)
  {
    if (!s->init)
    {
      k_sem_init(&s->sem,s->sem.count,s->sem.limit);
      s->init = true;
    }
    return k_sem_take(&s->sem,K_MSEC(ms));
  }

#endif // __BL_WORK_H__
