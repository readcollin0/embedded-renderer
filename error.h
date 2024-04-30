#ifndef ERROR_H
#define ERROR_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define ERROR(fmsg, args...) {printf(fmsg, args); exit(1);}
#define ASSERT(cond) {if (!(cond)) ERROR("(%s:%d) Assertion failed!\n", __FILENAME__, __LINE__);}


#endif