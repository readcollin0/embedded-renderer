#ifndef RENDERER_H
#define RENDERER_H

#include "display.h"
#include "renderer_fp.h"

#define FRAME_WIDTH DISPLAY_WIDTH
#define FRAME_HEIGHT DISPLAY_HEIGHT

typedef struct {
  fixp_t x;
  fixp_t y;
} render_point_t;

typedef struct {
  fixp_t x, y, sin_a, cos_a;
  fixp_t heights[FRAME_WIDTH];
} frame_t;

typedef struct {
  uint16_t pixels[FRAME_WIDTH][FRAME_HEIGHT];
} drawing_t;

void renderer_init_frame(frame_t *frame, fixp_t x, fixp_t y, fixp_t a);
void renderer_render_polygon(frame_t *frame, render_point_t points[],
                             uint16_t n);
void renderer_create_drawing(drawing_t *dest, frame_t *src);
void renderer_clear_drawing(drawing_t *drawing);

#endif