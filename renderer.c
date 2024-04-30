#include "renderer.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "error.h"

#define PI_F 3.1415926536
#define PI_2 REAL_TO_FIXP(2 * PI_F) // 2 pi in fixed point
#define PI_OVER_2 REAL_TO_FIXP(PI_F / 2) // pi / 2 in fixed point
#define VALID_ANGLE(a) (0 <= a && a < PI_2)

// Lookup tables for sine and cosine
// 6-bit
// const fixp_t SIN[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 32, 33, 34, 35, 36, 37, 37, 38, 39, 40, 41, 41, 42, 43, 44, 44, 45, 46, 46, 47, 48, 48, 49, 50, 50, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 59, 59, 59, 60, 60, 60, 61, 61, 61, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 63, 63, 63, 63, 63, 63, 62, 62, 62, 62, 61, 61, 61, 60, 60, 60, 59, 59, 59, 58, 58, 57, 57, 56, 56, 55, 55, 54, 54, 53, 53, 52, 52, 51, 50, 50, 49, 49, 48, 47, 47, 46, 45, 44, 44, 43, 42, 41, 41, 40, 39, 38, 37, 37, 36, 35, 34, 33, 32, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -22, -23, -24, -25, -26, -27, -28, -29, -30, -31, -32, -32, -33, -34, -35, -36, -37, -37, -38, -39, -40, -41, -41, -42, -43, -44, -44, -45, -46, -46, -47, -48, -48, -49, -50, -50, -51, -52, -52, -53, -53, -54, -54, -55, -55, -56, -56, -57, -57, -58, -58, -59, -59, -59, -60, -60, -60, -61, -61, -61, -62, -62, -62, -62, -63, -63, -63, -63, -63, -63, -64, -64, -64, -64, -64, -64, -64, -64, -64, -64, -64, -64, -64, -64, -64, -64, -63, -63, -63, -63, -63, -63, -62, -62, -62, -62, -61, -61, -61, -60, -60, -60, -59, -59, -59, -58, -58, -57, -57, -56, -56, -55, -55, -54, -54, -53, -53, -52, -52, -51, -50, -50, -49, -49, -48, -47, -47, -46, -45, -44, -44, -43, -42, -41, -41, -40, -39, -38, -38, -37, -36, -35, -34, -33, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1};

// 7-bit
const fixp_t SIN[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 60, 61, 62, 63, 64, 65, 66, 67, 67, 68, 69, 70, 71, 72, 72, 73, 74, 75, 76, 77, 77, 78, 79, 80, 80, 81, 82, 83, 84, 84, 85, 86, 87, 87, 88, 89, 89, 90, 91, 92, 92, 93, 94, 94, 95, 96, 96, 97, 98, 98, 99, 100, 100, 101, 101, 102, 103, 103, 104, 104, 105, 105, 106, 107, 107, 108, 108, 109, 109, 110, 110, 111, 111, 112, 112, 113, 113, 114, 114, 115, 115, 115, 116, 116, 117, 117, 118, 118, 118, 119, 119, 119, 120, 120, 120, 121, 121, 121, 122, 122, 122, 123, 123, 123, 123, 124, 124, 124, 124, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 127, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 126, 126, 126, 125, 125, 125, 125, 125, 124, 124, 124, 124, 123, 123, 123, 122, 122, 122, 122, 121, 121, 121, 120, 120, 119, 119, 119, 118, 118, 118, 117, 117, 116, 116, 116, 115, 115, 114, 114, 113, 113, 112, 112, 111, 111, 110, 110, 109, 109, 108, 108, 107, 107, 106, 106, 105, 104, 104, 103, 103, 102, 101, 101, 100, 100, 99, 98, 98, 97, 96, 96, 95, 94, 94, 93, 92, 92, 91, 90, 90, 89, 88, 87, 87, 86, 85, 84, 84, 83, 82, 81, 81, 80, 79, 78, 77, 77, 76, 75, 74, 73, 73, 72, 71, 70, 69, 68, 68, 67, 66, 65, 64, 63, 62, 61, 61, 60, 59, 58, 57, 56, 55, 54, 53, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26, -27, -28, -29, -30, -31, -32, -33, -33, -34, -35, -36, -37, -38, -39, -40, -41, -42, -43, -44, -45, -46, -47, -48, -49, -50, -50, -51, -52, -53, -54, -55, -56, -57, -58, -59, -59, -60, -61, -62, -63, -64, -65, -66, -66, -67, -68, -69, -70, -71, -72, -72, -73, -74, -75, -76, -76, -77, -78, -79, -80, -80, -81, -82, -83, -83, -84, -85, -86, -86, -87, -88, -89, -89, -90, -91, -91, -92, -93, -94, -94, -95, -96, -96, -97, -98, -98, -99, -99, -100, -101, -101, -102, -103, -103, -104, -104, -105, -105, -106, -107, -107, -108, -108, -109, -109, -110, -110, -111, -111, -112, -112, -113, -113, -114, -114, -115, -115, -115, -116, -116, -117, -117, -118, -118, -118, -119, -119, -119, -120, -120, -120, -121, -121, -121, -122, -122, -122, -123, -123, -123, -123, -124, -124, -124, -124, -125, -125, -125, -125, -126, -126, -126, -126, -126, -126, -127, -127, -127, -127, -127, -127, -127, -127, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -127, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -126, -126, -126, -125, -125, -125, -125, -125, -124, -124, -124, -124, -123, -123, -123, -122, -122, -122, -122, -121, -121, -121, -120, -120, -120, -119, -119, -118, -118, -118, -117, -117, -116, -116, -116, -115, -115, -114, -114, -113, -113, -112, -112, -111, -111, -110, -110, -109, -109, -108, -108, -107, -107, -106, -106, -105, -104, -104, -103, -103, -102, -102, -101, -100, -100, -99, -98, -98, -97, -96, -96, -95, -94, -94, -93, -92, -92, -91, -90, -90, -89, -88, -87, -87, -86, -85, -84, -84, -83, -82, -81, -81, -80, -79, -78, -78, -77, -76, -75, -74, -73, -73, -72, -71, -70, -69, -68, -68, -67, -66, -65, -64, -63, -62, -62, -61, -60, -59, -58, -57, -56, -55, -54, -54, -53, -52, -51, -50, -49, -48, -47, -46, -45, -44, -43, -42, -41, -41, -40, -39, -38, -37, -36, -35, -34, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0};

#define SIN(a) SIN[a]
#define COS(a) (a <= (PI_2 - PI_OVER_2) ? SIN[a + PI_OVER_2] : SIN[a + PI_OVER_2 - PI_2])


void renderer_init_frame(frame_t *frame, fixp_t x, fixp_t y, fixp_t a) {
    frame->x = x;
    frame->y = y;
    
    ASSERT(VALID_ANGLE(a));
    
    frame->sin_a = SIN(a);
    frame->cos_a = COS(a);
    
    for (uint16_t i = 0; i < DISPLAY_WIDTH; i++) {
        frame->depths[i] = 0;
    }
}



static void transform_point(render_point_t *t_dest, render_point_t *src, frame_t *frame) {
    fixp_t sx = (src->x - frame->x);
    fixp_t sy = (src->y - frame->y);

    t_dest->x =  FIXP_MULT(sx, frame->cos_a) + FIXP_MULT(sy, frame->sin_a);
    t_dest->y = FIXP_MULT(-sx, frame->sin_a) + FIXP_MULT(sy, frame->cos_a);
}



static bool is_point_behind_camera(render_point_t *t_point) {
    return t_point->x <= 0;
}

static bool is_point_out_of_view(render_point_t *t_point) {
    return t_point->x < abs(t_point->y);
}

enum point_camera_loc {
    PCL_BEHIND_CAMERA,
    PCL_OUT_OF_VIEW,
    PCL_IN_VIEW
};

static enum point_camera_loc get_point_flags(render_point_t *point) {
    if (is_point_behind_camera(point)) {
        if (is_point_out_of_view(point))
            return PCL_OUT_OF_VIEW;
        else
            return PCL_IN_VIEW;
    }
    return PCL_BEHIND_CAMERA;
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

#define FIRST_POINT_MODE(mode) ((mode & 0xc) >> 2)
#define SECOND_POINT_MODE(mode) (mode & 0x3)
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

static enum line_render_mode compute_render_mode(render_point_t *point1, render_point_t *point2) {
    enum point_camera_loc first_point_loc = get_point_flags(point1);
    enum point_camera_loc second_point_loc = get_point_flags(point2);

    if (first_point_loc == PCL_BEHIND_CAMERA && second_point_loc == PCL_BEHIND_CAMERA)
        return LRM_DO_NOT_RENDER;

    if (first_point_loc == PCL_IN_VIEW && second_point_loc == PCL_IN_VIEW)
        return LRM_BOTH_IN_VIEW;
    
    if (first_point_loc == PCL_OUT_OF_VIEW && second_point_loc == PCL_OUT_OF_VIEW) {
        bool first_left = point1->y > 0;
        bool second_left = point2->y > 0;

        if (first_left && !second_left)
            return LRM_BOTH_BEHIND_CAM_FIRST_LEFT;
        else if (!first_left && second_left)
            return LRM_BOTH_BEHIND_CAM_FIRST_RIGHT;
        else // Either both to left or both to right of view.
            return LRM_DO_NOT_RENDER;
    }


    fixp_t val = origin_line_val(point1, point2);

    if (val == 0)   {
        return LRM_DO_NOT_RENDER;
    }
    else if (first_point_loc == PCL_IN_VIEW) {
        return (val > 0) ? LRM_POINT2_BEHIND_CAM_LEFT 
                         : LRM_POINT2_BEHIND_CAM_RIGHT;
    }
    else {
        return (val > 0) ? LRM_POINT1_BEHIND_CAM_RIGHT 
                         : LRM_POINT1_BEHIND_CAM_LEFT;
    }
}

static fixp_t compute_slope(render_point_t *a, render_point_t *b) {
    return FIXP_DIV(b->y - a->y, b->x - a->x);
}

static fixp_t compute_slope_inv(render_point_t *a, render_point_t *b) {
    return FIXP_DIV(b->x - a->x, b->y - a->y);
}

// Computes the `y` value for a given `x`
static fixp_t interpolate_y(render_point_t *point1, render_point_t *point2, fixp_t x, fixp_t slope) {
    if (point1->x == point2->x)
        return (point1->x + point2->x) / 2;
    
    return FIXP_MULT(x - point1->x, slope) + point1->y;
}

// Computes the `x` value for a given `y`
static fixp_t interpolate_x(render_point_t *point1, render_point_t *point2, fixp_t y, fixp_t slope_inv) {
    if (point1->y == point2->y)
        return (point1->y + point2->y) / 2;
    
    return FIXP_MULT(y - point1->y, slope_inv) + point1->x;
}

void renderer_render_polygon(frame_t *frame, render_point_t points[], uint16_t n) {
    
}


void renderer_create_frame(drawing_t *dest, frame_t *src) {

}

void debug_routine() {

    frame_t frame;
    renderer_init_frame(&frame, REAL_TO_FIXP(-.5), REAL_TO_FIXP(1.5), REAL_TO_FIXP(5.497787));
    
    render_point_t test_point = {.x = 128, .y = 128};
    render_point_t trans_point;
    transform_point(&trans_point, &test_point, &frame);

    printf("(%d, %d)\n", trans_point.x, trans_point.y);
    printf("(%f, %f)\n", FIXP_TO_REAL(trans_point.x), FIXP_TO_REAL(trans_point.y));
}

