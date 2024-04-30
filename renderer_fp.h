#ifndef RENDERER_FP_H
#define RENDERER_FP_H

#include <stdint.h>


#define FIXP_RIGHT_BITS 7
#define FIXP_LEFT_BITS 9

typedef int16_t fixp_t;
typedef int32_t fixpd_t;

#define INT_TO_FIXP(int) ((fixp_t) (int << FIXP_RIGHT_BITS))
#define FIXP_TO_INT(fixp) (fixp >> FIXP_RIGHT_BITS)
#define REAL_TO_FIXP(real) ((fixp_t) (real * (1 << FIXP_RIGHT_BITS) + 0.5))
#define FIXP_TO_REAL(fixp) (((float) fixp) / (1 << FIXP_RIGHT_BITS))
#define FIXP_MULT(a, b) ((fixp_t) ((((fixpd_t) a) * ((fixpd_t) b)) >> FIXP_RIGHT_BITS))
#define FIXP_DIV(a, b) ((fixp_t) ((((fixpd_t) a) << FIXP_RIGHT_BITS) / ((fixpd_t) b)))


typedef uint16_t ufixp_t;
typedef uint32_t ufixpd_t;

#define INT_TO_UFIXP(int) ((ufixp_t) (int << FIXP_RIGHT_BITS))
#define FIXP_TO_INT(fixp) (fixp >> FIXP_RIGHT_BITS)
#define REAL_TO_UFIXP(real) ((ufixp_t) (real * (1 << FIXP_RIGHT_BITS) + 0.5))
#define UFIXP_TO_REAL(fixp) (((float) fixp) / (1 << FIXP_RIGHT_BITS))
#define UFIXP_MULT(a, b) ((ufixp_t) ((((ufixpd_t) a) * ((ufixpd_t) b)) >> FIXP_RIGHT_BITS))
#define FIXP_DIV(a, b) ((ufixp_t) ((((ufixpd_t) a) << FIXP_RIGHT_BITS) / ((ufixpd_t) b)))


#endif