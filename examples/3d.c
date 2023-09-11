#define OLIVEC_IMPLEMENTATION
#include "olive.c"

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);

#define PI 3.14159265359

#define WIDTH 800
#define HEIGHT 600
#define BACKGROUND_COLOR 0xFF181818
#define GRID_COUNT 10
#define GRID_PAD 0.5 / GRID_COUNT
#define GRID_SIZE ((GRID_COUNT - 1) * GRID_PAD)
#define CIRCLE_RADIUS 5
#define Z_START 0.5
#define ABOBA_PADDING 50

static uint32_t pixels[WIDTH * HEIGHT];
static float angle = 0;

uint32_t *render(float dt) {
  angle += 0.25 * PI * dt;

  Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);

  olivec_fill(oc, BACKGROUND_COLOR);
  for (int ix = 0; ix < GRID_COUNT; ++ix) {
    for (int iy = 0; iy < GRID_COUNT; ++iy) {
      for (int iz = 0; iz < GRID_COUNT; ++iz) {
        float x = ix * GRID_PAD - GRID_SIZE / 2;
        float y = iy * GRID_PAD - GRID_SIZE / 2;
        float z = Z_START + iz * GRID_PAD;

        float cx = 0.0;
        float cz = Z_START + GRID_SIZE / 2;

        float dx = x - cx;
        float dz = z - cz;

        float a = atan2f(dz, dx);
        float m = sqrtf(dx * dx + dz * dz);

        dx = cosf(a + angle) * m;
        dz = sinf(a + angle) * m;

        x = dx + cx;
        z = dz + cz;

        x /= z;
        y /= z;

        uint32_t r = ix * 255 / GRID_COUNT;
        uint32_t g = iy * 255 / GRID_COUNT;
        uint32_t b = iz * 255 / GRID_COUNT;
        uint32_t color =
            0xFF000000 | (r << (0 * 8)) | (g << (1 * 8)) | (b << (2 * 8));
        olivec_circle(oc, (x + 1) / 2 * WIDTH, (y + 1) / 2 * HEIGHT,
                      CIRCLE_RADIUS, color);
      }
    }
  }

  size_t size = 8;
  olivec_text(oc, "aboba", ABOBA_PADDING,
              HEIGHT - ABOBA_PADDING - default_font.height * size, default_font,
              size, 0xFFFFFFFF);

  return pixels;
}

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

#define SCALE_DOWN_FACTOR 8
static_assert(WIDTH % SCALE_DOWN_FACTOR == 0,
              "WIDTH must be divisible by the SCALE_DOWN_FACTOR");
#define SCALED_DOWN_WIDTH (WIDTH / SCALE_DOWN_FACTOR)
static_assert(HEIGHT % SCALE_DOWN_FACTOR == 0,
              "HEIGHT must be divisible by the SCALE_DOWN_FACTOR");
#define SCALED_DOWN_HEIGHT (HEIGHT / SCALE_DOWN_FACTOR)

char char_canvas[SCALED_DOWN_WIDTH * SCALED_DOWN_HEIGHT];

char color_to_char(uint32_t pixel) {
  size_t r = OLIVEC_RED(pixel);
  size_t g = OLIVEC_GREEN(pixel);
  size_t b = OLIVEC_BLUE(pixel);
  
  size_t bright = r;
  if (bright < g)
    bright = g;
  if (bright < b)
    bright = b;

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

void compress_pixels(uint32_t *pixels) {
  Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);
  for (size_t y = 0; y < SCALED_DOWN_HEIGHT; ++y) {
    for (size_t x = 0; x < SCALED_DOWN_WIDTH; ++x) {
      Olivec_Canvas soc =
          olivec_subcanvas(oc, x * SCALE_DOWN_FACTOR, y * SCALE_DOWN_FACTOR,
                           SCALE_DOWN_FACTOR, SCALE_DOWN_FACTOR);
      char_canvas[y * SCALED_DOWN_WIDTH + x] =
          color_to_char(compress_pixels_chunk(soc));
    }
  }
}

int main(void) {
  for (;;) {
    compress_pixels(render(1.f / 60.f));
    for (size_t y = 0; y < SCALED_DOWN_HEIGHT; ++y) {
      for (size_t x = 0; x < SCALED_DOWN_WIDTH; ++x) {
        putc(char_canvas[y * SCALED_DOWN_WIDTH + x], stdout);
        putc(char_canvas[y * SCALED_DOWN_WIDTH + x], stdout);
      }
      putc('\n', stdout);
    }

    usleep(1000 * 1000 / 60);
    printf("\033[%dA", SCALED_DOWN_HEIGHT);
    printf("\033[%dD", SCALED_DOWN_WIDTH);
  }
  return 0;
}

#elif PLATFORM == WASM_PLATFORM
// Do nothing
#else
#error "Unknown platform"
#endif
