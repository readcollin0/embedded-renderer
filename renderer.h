#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_fp.h"
#include "display_config.h"

typedef struct {
    fixp_t x;
    fixp_t y;
} render_point_t;

typedef struct {
    fixp_t x, y, sin_a, cos_a;
    fixp_t depths[DISPLAY_WIDTH];
} frame_t;

typedef struct {
    uint16_t heights[DISPLAY_WIDTH];
} drawing_t;


void renderer_init_frame(frame_t *frame, fixp_t x, fixp_t y, fixp_t a);
void renderer_render_polygon(frame_t *frame, render_point_t points[], uint16_t n);
void renderer_create_frame(drawing_t *dest, frame_t *src);


#endif