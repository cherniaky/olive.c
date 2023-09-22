#include "vc.c"

#include "./assets/Sadge.c"

#define WIDTH 960
#define HEIGHT 720

float sinf(float);

static uint32_t dst[WIDTH * HEIGHT];
static float global_time = 0;

#define SRC_SCALE 2

Olivec_Canvas vc_render(float dt) {
  global_time += dt;

  float t = sinf(10 * global_time);

  Olivec_Canvas dst_canvas = olivec_canvas(dst, WIDTH, HEIGHT, WIDTH);

  olivec_fill(dst_canvas, 0xFF181818);

  int factor = 100;
  int w = Sadge_width * SRC_SCALE - t * factor;
  int h = Sadge_height * SRC_SCALE + t * factor;

  olivec_sprite_blend(
      dst_canvas, WIDTH / 2 - w / 2, HEIGHT - h, w, h,
      olivec_canvas(Sadge_pixels, Sadge_width, Sadge_height, Sadge_width));

  return dst_canvas;
}
