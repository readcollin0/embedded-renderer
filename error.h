#ifndef ERROR_H
#define ERROR_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define __FILENAME__                                                           \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG() printf("Got to %s : %s : %d\n", __FILENAME__, __func__, __LINE__);
// #define LOG() printf("Got here!");

#define RUN_ON_ZYBO
#ifdef RUN_ON_ZYBO
#define ERROR(...)
#define ASSERT(...)
#else
// #define ERROR(fmsg, args...) {printf(fmsg, args); exit(1);}
// #define ASSERT(cond) {if (!(cond)) ERROR("(%s:%d) Assertion failed!\n",
// __FILENAME__, __LINE__);}
#endif

#endif