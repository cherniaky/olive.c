#define WASM_PLATFORM 0
#define TERM_PLATFORM 1

#if PLATFORM == TERM_PLATFORM

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static size_t actual_width = 0;
static size_t actual_height = 0;
static size_t scaled_down_width = 0;
static size_t scaled_down_height = 0;
static char *char_canvas = 0;

char color_to_char(uint32_t pixel) {
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

uint32_t compress_pixels_chunk(Olivec_Canvas oc) {
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

void resize_char_canvas(size_t new_width, size_t new_height) {
  assert(new_width % SCALE_DOWN_FACTOR == 0 &&
         "Width must be divisible by SCALE_DOWN_FACTOR");
  assert(new_height % SCALE_DOWN_FACTOR == 0 &&
         "Height must be divisible by SCALE_DOWN_FACTOR");
  actual_width = new_width;
  actual_height = new_height;
  scaled_down_width = actual_width / SCALE_DOWN_FACTOR;
  scaled_down_height = actual_height / SCALE_DOWN_FACTOR;
  free(char_canvas);
  char_canvas =
      malloc(sizeof(*char_canvas) * scaled_down_width * scaled_down_height);
}

void compress_pixels(Olivec_Canvas oc) {
  if (actual_width != oc.width || actual_height != oc.height) {
    resize_char_canvas(oc.width, oc.height);
  }

  for (size_t y = 0; y < scaled_down_height; ++y) {
    for (size_t x = 0; x < scaled_down_width; ++x) {
      Olivec_Canvas soc =
          olivec_subcanvas(oc, x * SCALE_DOWN_FACTOR, y * SCALE_DOWN_FACTOR,
                           SCALE_DOWN_FACTOR, SCALE_DOWN_FACTOR);
      char_canvas[y * scaled_down_width + x] =
          color_to_char(compress_pixels_chunk(soc));
    }
  }
}

int main(void) {
  for (;;) {
    compress_pixels(render(1.f / 60.f));
    for (size_t y = 0; y < scaled_down_height; ++y) {
      for (size_t x = 0; x < scaled_down_width; ++x) {
        putc(char_canvas[y * scaled_down_width + x], stdout);
        putc(char_canvas[y * scaled_down_width + x], stdout);
      }
      putc('\n', stdout);
    }

    usleep(1000 * 1000 / 60);
    printf("\033[%zuA", scaled_down_height);
    printf("\033[%zuD", scaled_down_width);
  }
  return 0;
}

#elif PLATFORM == WASM_PLATFORM
// Do nothing
#else
#error "Unknown platform"
#endif
