#ifndef ANGLES_H
#define ANGLES_H

#include "renderer_fp.h"

#define PI_F 3.1415926536
#define PI_2 REAL_TO_FIXP(2 * PI_F)      // 2 pi in fixed point
#define PI_OVER_2 REAL_TO_FIXP(PI_F / 2) // pi / 2 in fixed point
#define VALID_ANGLE(a) (0 <= a && a < PI_2)

// Lookup tables for sine and cosine
extern const fixp_t SIN[];

#define SIN(a) SIN[a]
#define COS(a)                                                                 \
  (a <= (PI_2 - PI_OVER_2) ? SIN[a + PI_OVER_2] : SIN[a + PI_OVER_2 - PI_2])

#endif