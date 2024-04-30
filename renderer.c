#include "renderer.h"

#include <stdint.h>
#include <stdlib.h>
// #include <stdbool.h>

#include "angles.h"
#include "error.h"

#define printf(...)

#define NOTHING_HEIGHT 0

void renderer_init_frame(frame_t *frame, fixp_t x, fixp_t y, fixp_t a) {
  frame->x = x;
  frame->y = y;

  // ASSERT(VALID_ANGLE(a));

  frame->sin_a = SIN(a);
  frame->cos_a = COS(a);

  for (uint16_t i = 0; i < FRAME_WIDTH; i++) {
    frame->heights[i] = NOTHING_HEIGHT;
  }
}

static void transform_point(render_point_t *t_dest, render_point_t *src,
                            frame_t *frame) {
  fixp_t sx = (src->x - frame->x);
  fixp_t sy = (src->y - frame->y);

  t_dest->x = FIXP_MULT(sx, frame->cos_a) + FIXP_MULT(sy, frame->sin_a);
  t_dest->y = FIXP_MULT(-sx, frame->sin_a) + FIXP_MULT(sy, frame->cos_a);
}

static bool is_point_behind_camera(render_point_t *t_point) {
  return t_point->x <= 0;
}

static bool is_point_out_of_view(render_point_t *t_point) {
  return t_point->x < abs(t_point->y);
}

enum point_camera_loc {
  PCL_BEHIND_CAMERA = 0,
  PCL_OUT_OF_VIEW = 1,
  PCL_IN_VIEW = 2
};

static enum point_camera_loc get_point_loc(render_point_t *point) {
  if (is_point_behind_camera(point))
    return PCL_BEHIND_CAMERA;

  if (is_point_out_of_view(point))
    return PCL_OUT_OF_VIEW;

  return PCL_IN_VIEW;
}

enum line_render_mode {
  LRM_DO_NOT_RENDER = 0,                  // 0b 0 00 00
  LRM_POINT1_BEHIND_CAM_LEFT = 0x7,       // 0b 0 01 11
  LRM_POINT1_BEHIND_CAM_RIGHT = 0xb,      // 0b 0 10 11
  LRM_POINT2_BEHIND_CAM_LEFT = 0xd,       // 0b 0 11 01
  LRM_POINT2_BEHIND_CAM_RIGHT = 0xe,      // 0b 0 11 10
  LRM_BOTH_BEHIND_CAM_FIRST_LEFT = 0x16,  // 0b 1 01 10
  LRM_BOTH_BEHIND_CAM_FIRST_RIGHT = 0x19, // 0b 1 10 01
  LRM_BOTH_IN_VIEW = 0xf,                 // 0b 0 11 11
};

#define SHOULD_RENDER(mode) (mode)
#define MODE_FIRST_POINT(mode) ((mode & 0xc) >> 2)
#define MODE_SECOND_POINT(mode) (mode & 0x3)
#define MODE_TRIM(mode) (mode < 0x3)
#define MODE_IN_VIEW(mode) (mode == 0x3)
#define MODE_TRIM_RIGHT(mode) (mode == 0x2)
#define MODE_TRIM_LEFT(mode) (mode == 0x1)

#define TRIM_FIRST_POINT(mode) (~mode & 0xc)
#define TRIM_SECOND_POINT(mode) (~mode & 0x3)
#define FIRST_POINT_IN_VIEW(mode) ((mode & 0xc) == 0xc)
#define SECOND_POINT_IN_VIEW(mode) ((mode & 0x3) == 0x3)
#define TRIM_BOTH(mode) (mode & 0x10)

// It's complicated how this works, related to the cross product.
// https://www.desmos.com/calculator/hojnglocvc for a visualization
static fixp_t origin_line_val(render_point_t *a, render_point_t *b) {
  return (FIXP_MULT(b->x - a->x, -a->y) - FIXP_MULT(b->y - a->y, -a->x));
}

static enum line_render_mode compute_render_mode(render_point_t *tp1,
                                                 render_point_t *tp2) {
  enum point_camera_loc p1_loc = get_point_loc(tp1);
  enum point_camera_loc p2_loc = get_point_loc(tp2);

  if (p1_loc == PCL_BEHIND_CAMERA && p2_loc == PCL_BEHIND_CAMERA)
    return LRM_DO_NOT_RENDER;

  if (p1_loc == PCL_IN_VIEW && p2_loc == PCL_IN_VIEW)
    return LRM_BOTH_IN_VIEW;

  if (p1_loc == PCL_OUT_OF_VIEW && p2_loc == PCL_OUT_OF_VIEW) {
    bool first_left = tp1->y > 0;
    bool second_left = tp2->y > 0;

    if (first_left && !second_left)
      return LRM_BOTH_BEHIND_CAM_FIRST_LEFT;
    else if (!first_left && second_left)
      return LRM_BOTH_BEHIND_CAM_FIRST_RIGHT;
    else // Either both to left or both to right of view.
      return LRM_DO_NOT_RENDER;
  }

  // At this point, at least one of the two is not behind the camera
  // and that at least one point is not in view

  // Check for one
  if (p1_loc == PCL_OUT_OF_VIEW && p2_loc == PCL_BEHIND_CAMERA ||
      p1_loc == PCL_BEHIND_CAMERA && p2_loc == PCL_OUT_OF_VIEW) {
    bool first_above = tp1->y > 0;
    bool second_above = tp2->y > 0;

    if (first_above == second_above)
      return LRM_DO_NOT_RENDER;

    fixp_t val = origin_line_val(tp1, tp2);

    // Easy quick check to see if perfectly in line with camera. If so, do not
    // render.
    if (val == 0) {
      return LRM_DO_NOT_RENDER;
    }

    if (first_above) {
      return (val > 0) ? LRM_DO_NOT_RENDER : LRM_BOTH_BEHIND_CAM_FIRST_LEFT;
    } else {
      return (val > 0) ? LRM_BOTH_BEHIND_CAM_FIRST_RIGHT : LRM_DO_NOT_RENDER;
    }
  }

  fixp_t val = origin_line_val(tp1, tp2);

  // Easy quick check to see if perfectly in line with camera. If so, do not
  // render.
  if (val == 0) {
    return LRM_DO_NOT_RENDER;
  }

  if (p1_loc == PCL_IN_VIEW) {
    return (val > 0) ? LRM_POINT2_BEHIND_CAM_LEFT : LRM_POINT2_BEHIND_CAM_RIGHT;
  } else {
    return (val > 0) ? LRM_POINT1_BEHIND_CAM_RIGHT : LRM_POINT1_BEHIND_CAM_LEFT;
  }
}

/*
static fixp_t compute_slope(render_point_t *a, render_point_t *b) {
    return FIXP_DIV(b->y - a->y, b->x - a->x);
}

// Computes the `x` value for a given `y`
static fixp_t interpolate_x(render_point_t *a, render_point_t *b, fixp_t y,
fixp_t slope_inv) { if (a->y == b->y) return (a->y + b->y) / 2;

    return FIXP_MULT(y - a->y, FIXP_DIV(b->x - a->x, b->y - a->y)) + a->x;
}*/

// Computes the `y` value for a given `x`
static fixp_t interpolate_y(render_point_t *a, render_point_t *b, fixp_t x) {
  if (a->x == b->x)
    return (a->x + b->x) / 2;

  return FIXP_MULT(x - a->x, FIXP_DIV(b->y - a->y, b->x - a->x)) + a->y;
}

static render_point_t trim_line_to_left(render_point_t *tp1,
                                        render_point_t *tp2) {
  render_point_t to_trim_s = {.x = tp1->x - tp1->y, .y = tp1->y};
  render_point_t other_s = {.x = tp2->x - tp2->y, .y = tp2->y};

  fixp_t new_y = interpolate_y(&to_trim_s, &other_s, 0);
  return (render_point_t){.x = new_y, .y = new_y};
}

static render_point_t trim_line_to_right(render_point_t *tp1,
                                         render_point_t *tp2) {
  render_point_t to_trim_s = {.x = tp1->x + tp1->y, .y = tp1->y};
  render_point_t other_s = {.x = tp2->x + tp2->y, .y = tp2->y};

  fixp_t new_y = interpolate_y(&to_trim_s, &other_s, 0);
  return (render_point_t){.x = -new_y, .y = new_y};
}

static void trim_point(render_point_t *cp, uint8_t mode,
                       render_point_t *to_trim, render_point_t *other) {
  if (!MODE_TRIM(mode)) {
    *cp = *to_trim;
    return;
  }

  if (MODE_TRIM_LEFT(mode)) {
    *cp = trim_line_to_left(to_trim, other);
  } else {
    *cp = trim_line_to_right(to_trim, other);
  }
}

static fixp_t compute_slope_inv(render_point_t *a, render_point_t *b) {
  return FIXP_DIV(b->x - a->x, b->y - a->y);
}

static fixp_t height_from_depth(fixp_t depth) {
  // printf("Depth: %f\n", FIXP_TO_REAL(depth));

  // printf("Capped: %f\n", FIXP_TO_REAL(depth));
  // printf("Frame Height: %f\n", FIXP_TO_REAL(INT_TO_FIXP(FRAME_HEIGHT)));
  // printf("Capped: %f\n", FIXP_TO_REAL(FIXP_DIV(INT_TO_FIXP(FRAME_HEIGHT),
  // depth)));

  return FIXP_DIV(INT_TO_FIXP(FRAME_HEIGHT), depth);
}

#define HEIGHT_CAP INT_TO_FIXP(FRAME_HEIGHT)
static void render_line(frame_t *frame, render_point_t *tp1,
                        render_point_t *tp2) {
  printf("tp1: (%f, %f)\n", FIXP_TO_REAL(tp1->x), FIXP_TO_REAL(tp1->y));
  printf("tp2: (%f, %f)\n", FIXP_TO_REAL(tp2->x), FIXP_TO_REAL(tp2->y));

  enum line_render_mode lrm = compute_render_mode(tp1, tp2);
  if (!SHOULD_RENDER(lrm))
    return;

  printf("Continuing with render mode %x...\n", lrm);

  render_point_t cp1, cp2;
  trim_point(&cp1, MODE_FIRST_POINT(lrm), tp1, tp2);
  trim_point(&cp2, MODE_SECOND_POINT(lrm), tp2, tp1);

  printf("cp1: (%f, %f)\n", FIXP_TO_REAL(cp1.x), FIXP_TO_REAL(cp1.y));
  printf("cp2: (%f, %f)\n", FIXP_TO_REAL(cp2.x), FIXP_TO_REAL(cp2.y));

  // Adjust left/right based on distance.
  // Change distance to height
  cp1.y = FIXP_DIV((FRAME_WIDTH / 2) * cp1.y, cp1.x);
  cp2.y = FIXP_DIV((FRAME_WIDTH / 2) * cp2.y, cp2.x);

  printf("Transformed y:\n");
  printf("cp1: (%f, %f)\n", FIXP_TO_REAL(cp1.x), FIXP_TO_REAL(cp1.y));
  printf("cp2: (%f, %f)\n", FIXP_TO_REAL(cp2.x), FIXP_TO_REAL(cp2.y));

  cp1.x = height_from_depth(cp1.x);
  cp2.x = height_from_depth(cp2.x);

  printf("X is now height:\n");
  printf("cp1: (%f, %f)\n", FIXP_TO_REAL(cp1.x), FIXP_TO_REAL(cp1.y));
  printf("cp2: (%f, %f)\n", FIXP_TO_REAL(cp2.x), FIXP_TO_REAL(cp2.y));

  if (cp1.y == cp2.y)
    return;

  int16_t start, end;
  fixp_t slope = -compute_slope_inv(&cp1, &cp2);
  fixp_t height;

  if (cp1.y <= cp2.y) {
    start = FIXP_TO_INT(-cp2.y) + (FRAME_WIDTH / 2);
    end = FIXP_TO_INT(-cp1.y) + (FRAME_WIDTH / 2);
    height = cp2.x;
  } else {
    start = FIXP_TO_INT(-cp1.y) + (FRAME_WIDTH / 2);
    end = FIXP_TO_INT(-cp2.y) + (FRAME_WIDTH / 2);
    height = cp1.x;
  }

  if (start < 0)
    start = 0;

  if (end > FRAME_WIDTH - 1)
    end = FRAME_WIDTH - 1;

  printf("Range: (%d, %d)\n", start, end);
  printf("Slope: %f\n", FIXP_TO_REAL(slope));
  printf("Init. Depth: %f\n", FIXP_TO_REAL(height));

  for (uint16_t i = start; i <= end; i++) {
    fixp_t *frame_height = &(frame->heights[i]);
    if ((*frame_height == NOTHING_HEIGHT) || (height > *frame_height))
      *frame_height = (height > HEIGHT_CAP) ? HEIGHT_CAP : height;
    height += slope;
  }
}

void renderer_render_polygon(frame_t *frame, render_point_t points[],
                             uint16_t n) {
  render_point_t last_tpoint, next_tpoint;
  transform_point(&last_tpoint, &points[0], frame);

  for (uint16_t i = 1; i < n; i++) {
    printf(" p1: (%f, %f)\n", FIXP_TO_REAL(points[i - 1].x),
           FIXP_TO_REAL(points[i - 1].y));
    printf(" p2: (%f, %f)\n", FIXP_TO_REAL(points[i].x),
           FIXP_TO_REAL(points[i].y));
    transform_point(&next_tpoint, &points[i], frame);
    render_line(frame, &last_tpoint, &next_tpoint);
    last_tpoint = next_tpoint;
  }
}

#define BG_COLOR DISPLAY_BLACK
#define GRAD_1_COLOR DISPLAY_WHITE
#define GRAD_1_CAP (REAL_TO_FIXP(1.0 * FRAME_HEIGHT * 2 / 3))
#define GRAD_2_COLOR DISPLAY_LIGHT_GRAY
#define GRAD_2_CAP (REAL_TO_FIXP(1.0 * FRAME_HEIGHT * 1 / 2))
#define GRAD_3_COLOR DISPLAY_GRAY
#define GRAD_3_CAP (REAL_TO_FIXP(1.0 * FRAME_HEIGHT * 1 / 3))
#define GRAD_FINAL_COLOR DISPLAY_DARK_GRAY

#define COLOR_CAP(ratio, color)                                                \
  {                                                                            \
    if (height > (FRAME_HEIGHT * (1.0 * ratio)))                               \
      return color;                                                            \
  }

static uint16_t color_from_height(uint16_t height) {
  if (height == 0)
    return BG_COLOR;

  COLOR_CAP(1 / 2, DISPLAY_WHITE);
  COLOR_CAP(1 / 5, DISPLAY_LIGHT_GRAY);
  COLOR_CAP(1 / 6, DISPLAY_GRAY);

  return DISPLAY_DARK_GRAY;
}

void renderer_create_drawing(drawing_t *dest, frame_t *src) {
  for (uint16_t x = 0; x < FRAME_WIDTH; x++) {
    uint16_t height =
        FIXP_TO_INT(src->heights[x]); // (depth == NOTHING_HEIGHT) ? 0 :
                                      // height_from_depth(depth);
    uint16_t margin = (FRAME_HEIGHT - height) / 2;
    uint16_t color = color_from_height(height);

    for (uint16_t y = 0; y < FRAME_HEIGHT; y++) {
      uint16_t *cur_pixel = &(dest->pixels[x][y]);
      if (margin < y && y < FRAME_HEIGHT - margin)
        *cur_pixel = color;
      else
        *cur_pixel = BG_COLOR;
    }
  }
}

void renderer_clear_drawing(drawing_t *drawing) {
  for (uint16_t x = 0; x < FRAME_WIDTH; x++) {
    for (uint16_t y = 0; y < FRAME_HEIGHT; y++) {
      drawing->pixels[x][y] = BG_COLOR;
    }
  }
}

/*
void debug_routine() {

    // frame_t frame;
    // renderer_init_frame(&frame, REAL_TO_FIXP(-.5), REAL_TO_FIXP(1.5),
REAL_TO_FIXP(5.497787));

    // render_point_t test_point = {.x = 128, .y = 128};
    // render_point_t trans_point;
    // transform_point(&trans_point, &test_point, &frame);

    // printf("(%d, %d)\n", trans_point.x, trans_point.y);
    // printf("(%f, %f)\n", FIXP_TO_REAL(trans_point.x),
FIXP_TO_REAL(trans_point.y));



    // render_point_t point1 = {.x = REAL_TO_FIXP(10.7), .y = REAL_TO_FIXP(-3)},
    //                point2 = {.x = REAL_TO_FIXP(0), .y = REAL_TO_FIXP(10)};
    // printf("(%f, %f)\n", FIXP_TO_REAL(point1.x), FIXP_TO_REAL(point1.y));
    // printf("(%f, %f)\n", FIXP_TO_REAL(point2.x), FIXP_TO_REAL(point2.y));

    // test_trim(&point1, &point2);



    render_point_t polygon[] = {
        {.x = INT_TO_FIXP(0), .y = INT_TO_FIXP(0)},
        {.x = INT_TO_FIXP(1), .y = INT_TO_FIXP(1)},
        {.x = INT_TO_FIXP(2), .y = INT_TO_FIXP(1)},
        {.x = INT_TO_FIXP(3), .y = INT_TO_FIXP(0)}
    };

    frame_t frame;
    renderer_init_frame(&frame, REAL_TO_FIXP(1.5), REAL_TO_FIXP(-2),
PI_OVER_2-30); renderer_render_polygon(&frame, polygon, COUNT_OF(polygon)); for
(uint16_t i = 0; i < FRAME_WIDTH; i++) { printf("%.2d: %f\n", i,
FIXP_TO_REAL(frame.heights[i]));
    }


    drawing_t drawing;
    renderer_create_drawing(&drawing, &frame);

    for (uint16_t y = 0; y < FRAME_HEIGHT; y++) {
        for (uint16_t x = 0; x < FRAME_WIDTH; x++) {
            uint16_t color = drawing.pixels[y][x];
            if (color == BG_COLOR)
                printf("  ");
            else
                printf("██");
        }
        printf("\n");
    }

}

*/