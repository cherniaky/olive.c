#define OLIVEC_IMPLEMENTATION
#include <olive.c>

Olivec_Canvas vc_render(float dt);

#ifndef VC_PLATFORM
#error "Please define VC_PLATFORM macro"
#endif

#define VC_WASM_PLATFORM 0
#define VC_TERM_PLATFORM 1

#if VC_PLATFORM == VC_TERM_PLATFORM

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static size_t vc_term_actual_width = 0;
static size_t vc_term_actual_height = 0;
static size_t vc_term_scaled_down_width = 0;
static size_t vc_term_scaled_down_height = 0;
static char *vc_term_char_canvas = 0;

static char vc_term_color_to_char(uint32_t pixel) {
  size_t r = OLIVEC_RED(pixel);
  size_t g = OLIVEC_GREEN(pixel);
  size_t b = OLIVEC_BLUE(pixel);
  size_t a = OLIVEC_ALPHA(pixel);

  size_t bright = r;
  if (bright < g)
    bright = g;
  if (bright < b)
    bright = b;
  bright = bright * a / 255;

  char table[] = " .:a@#";
  size_t n = sizeof(table) - 1;
  return table[bright * n / 256];
}

static uint32_t vc_term_compress_pixels_chunk(Olivec_Canvas oc) {
  size_t r = 0;
  size_t g = 0;
  size_t b = 0;
  size_t a = 0;

  for (size_t y = 0; y < oc.height; ++y) {
    for (size_t x = 0; x < oc.width; ++x) {
      r += OLIVEC_RED(OLIVEC_PIXEL(oc, x, y));
      g += OLIVEC_GREEN(OLIVEC_PIXEL(oc, x, y));
      b += OLIVEC_BLUE(OLIVEC_PIXEL(oc, x, y));
      a += OLIVEC_ALPHA(OLIVEC_PIXEL(oc, x, y));
    }
  }

  r /= oc.width * oc.height;
  g /= oc.width * oc.height;
  b /= oc.width * oc.height;
  a /= oc.width * oc.height;

  return OLIVEC_RGBA(r, g, b, a);
}

#ifndef VC_TERM_SCALE_DOWN_FACTOR
#define VC_TERM_SCALE_DOWN_FACTOR 20
#endif // VC_TERM_SCALE_DOWN_FACTOR

static void vc_term_resize_char_canvas(size_t new_width, size_t new_height) {
  assert(new_width % VC_TERM_SCALE_DOWN_FACTOR == 0 &&
         "Width must be divisible by VC_TERM_SCALE_DOWN_FACTOR");
  assert(new_height % VC_TERM_SCALE_DOWN_FACTOR == 0 &&
         "Height must be divisible by VC_TERM_SCALE_DOWN_FACTOR");
  vc_term_actual_width = new_width;
  vc_term_actual_height = new_height;
  vc_term_scaled_down_width = vc_term_actual_width / VC_TERM_SCALE_DOWN_FACTOR;
  vc_term_scaled_down_height =
      vc_term_actual_height / VC_TERM_SCALE_DOWN_FACTOR;
  free(vc_term_char_canvas);
  vc_term_char_canvas =
      malloc(sizeof(*vc_term_char_canvas) * vc_term_scaled_down_width *
             vc_term_scaled_down_height);
}

static void vc_term_compress_pixels(Olivec_Canvas oc) {
  if (vc_term_actual_width != oc.width || vc_term_actual_height != oc.height) {
    vc_term_resize_char_canvas(oc.width, oc.height);
  }

  for (size_t y = 0; y < vc_term_scaled_down_height; ++y) {
    for (size_t x = 0; x < vc_term_scaled_down_width; ++x) {
      Olivec_Canvas soc = olivec_subcanvas(
          oc, x * VC_TERM_SCALE_DOWN_FACTOR, y * VC_TERM_SCALE_DOWN_FACTOR,
          VC_TERM_SCALE_DOWN_FACTOR, VC_TERM_SCALE_DOWN_FACTOR);
      vc_term_char_canvas[y * vc_term_scaled_down_width + x] =
          vc_term_color_to_char(vc_term_compress_pixels_chunk(soc));
    }
  }
}

int main(void) {
  for (;;) {
    vc_term_compress_pixels(vc_render(1.f / 60.f));
    for (size_t y = 0; y < vc_term_scaled_down_height; ++y) {
      for (size_t x = 0; x < vc_term_scaled_down_width; ++x) {
        putc(vc_term_char_canvas[y * vc_term_scaled_down_width + x], stdout);
        putc(vc_term_char_canvas[y * vc_term_scaled_down_width + x], stdout);
      }
      putc('\n', stdout);
    }

    usleep(1000 * 1000 / 60);
    printf("\033[%zuA", vc_term_scaled_down_height);
    printf("\033[%zuD", vc_term_scaled_down_width);
  }
  return 0;
}

#elif VC_PLATFORM == VC_WASM_PLATFORM
// Do nothing
#else
#error "Unknown VC platform"
#endif
