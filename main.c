#include "stdio.h"

#include "buttons.h"
#include "display.h"
#include "intervalTimer.h"

#include "angles.h"
#include "error.h"
#include "renderer.h"

#define COUNT_OF(x)                                                            \
  ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

static void draw_to_screen(drawing_t *drawing, drawing_t *last) {
  for (uint16_t x = 0; x < FRAME_WIDTH; x++) {
    for (uint16_t y = 0; y < FRAME_HEIGHT; y++) {
      if (drawing->pixels[x][y] != last->pixels[x][y]) {
        display_drawPixel(x, y, drawing->pixels[x][y]);
      }
    }
  }
}

drawing_t drawing1, drawing2;
bool last_used_drawing1 = false;

#define P(X, Y)                                                                \
  { .x = REAL_TO_FIXP(X), .y = REAL_TO_FIXP(Y) }
render_point_t cube[] = {
    // {.x = INT_TO_FIXP(1), .y = INT_TO_FIXP(1)},
    // {.x = INT_TO_FIXP(1), .y = INT_TO_FIXP(2)},
    // {.x = INT_TO_FIXP(2), .y = INT_TO_FIXP(2)},
    // {.x = INT_TO_FIXP(2), .y = INT_TO_FIXP(1)},
    // {.x = INT_TO_FIXP(1), .y = INT_TO_FIXP(1)},
    P(1, 1), P(1, 2), P(2, 2), P(2, 1), P(1, 1)};

render_point_t triangle[] = {P(-2, 1), P(-1, 1), P(-1.5, 2), P(-2, 1)};

render_point_t circle[] = {
    P(-1.0, 2.0),     P(-1.022, 2.208), P(-1.086, 2.407), P(-1.191, 2.588),
    P(-1.331, 2.743), P(-1.5, 2.866),   P(-1.691, 2.951), P(-1.895, 2.995),
    P(-2.105, 2.995), P(-2.309, 2.951), P(-2.5, 2.866),   P(-2.669, 2.743),
    P(-2.809, 2.588), P(-2.914, 2.407), P(-2.978, 2.208), P(-3.0, 2.0),
    P(-2.978, 1.792), P(-2.914, 1.593), P(-2.809, 1.412), P(-2.669, 1.257),
    P(-2.5, 1.134),   P(-2.309, 1.049), P(-2.105, 1.005), P(-1.895, 1.005),
    P(-1.691, 1.049), P(-1.5, 1.134),   P(-1.331, 1.257), P(-1.191, 1.412),
    P(-1.086, 1.593), P(-1.022, 1.792), P(-1.0, 2.0)};

render_point_t maze1[] = {P(-1, -4), P(-1, -2), P(-2, -2), P(-2, -6),
                          P(0, -6),  P(0, -3),  P(1, -3)};
render_point_t maze2[] = {P(-2, -5), P(-1, -5)};
render_point_t maze3[] = {P(0, -2), P(2, -2), P(2, -6), P(1, -6), P(1, -5)};
render_point_t maze4[] = {P(2, -4), P(1, -4)};

#define RENDER(shape) renderer_render_polygon(&frame, shape, COUNT_OF(shape));

static void draw_all(fixp_t x, fixp_t y, fixp_t a) {
  frame_t frame;
  renderer_init_frame(&frame, x, y, a);
  RENDER(cube);
  RENDER(circle);
  RENDER(maze1);
  RENDER(maze2);
  RENDER(maze3);
  RENDER(maze4);

  drawing_t *current = NULL, *last = NULL;

  if (last_used_drawing1) {
    last = &drawing1;
    current = &drawing2;
    last_used_drawing1 = false;
  } else {
    current = &drawing1;
    last = &drawing2;
    last_used_drawing1 = true;
  }

  renderer_create_drawing(current, &frame);
  draw_to_screen(current, last);
}

#define MOVE_SPEED_PER_SECOND 1
#define TURN_SPEED_PER_SECOND 1

static void move(double delta_time, fixp_t *x, fixp_t *y, fixp_t *a) {
  uint8_t buttons = buttons_read();

  double move_dist = 0;
  if (buttons & BUTTONS_BTN3_MASK)
    move_dist += delta_time * MOVE_SPEED_PER_SECOND;

  if (buttons & BUTTONS_BTN2_MASK)
    move_dist -= delta_time * MOVE_SPEED_PER_SECOND;

  if (move_dist != 0) {
    fixp_t dist = REAL_TO_FIXP(move_dist);
    *x += FIXP_MULT(dist, COS(*a));
    *y += FIXP_MULT(dist, SIN(*a));
  }

  double turn_dist = 0;
  if (buttons & BUTTONS_BTN1_MASK)
    turn_dist += delta_time * TURN_SPEED_PER_SECOND;

  if (buttons & BUTTONS_BTN0_MASK)
    turn_dist -= delta_time * TURN_SPEED_PER_SECOND;

  if (turn_dist != 0)
    *a += REAL_TO_FIXP(turn_dist);

  while (*a < 0)
    *a += PI_2;
  while (*a >= PI_2)
    *a -= PI_2;
}

static void init() {
  renderer_clear_drawing(&drawing2);
  display_init();
  display_fillScreen(DISPLAY_BLACK);
  buttons_init();
  intervalTimer_initCountUp(INTERVAL_TIMER_0);
  intervalTimer_start(INTERVAL_TIMER_0);
}

#define MIN_TIME_TICK 0.05
int main() {
  init();

  fixp_t x = REAL_TO_FIXP(0), y = REAL_TO_FIXP(0), a = REAL_TO_FIXP(0);

  while (true) {
    // printf("%f, %f, %f\n", FIXP_TO_REAL(x), FIXP_TO_REAL(y),
    // FIXP_TO_REAL(a));
    draw_all(x, y, a);
    // while (1);

    double time_change;
    do {
      time_change = intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_0);
    } while (time_change < MIN_TIME_TICK);
    intervalTimer_reload(INTERVAL_TIMER_0);

    move(time_change, &x, &y, &a);
  }

  // frame_t frame;
  // renderer_init_render(&frame, 0, 0, -1);
  // return 0;
}