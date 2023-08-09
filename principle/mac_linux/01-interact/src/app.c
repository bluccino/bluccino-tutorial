// app.c - implementation of module app

#include "core.h"

void interact(void)      // demonstration of module interaction
{
  int i = 1, on = 0;     // sample values
  core_LED_SET(i,on);    // app >> LED.SET(i,n) >> core
}
