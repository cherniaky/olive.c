#define OLIVEC_IMPLEMENTATION
#include "olive.c"

#define WIDTH 800
#define HEIGHT 600
#define BACKGROUND_COLOR 0xFF181818
#define CIRCLE_RADIUS 100
#define CIRCLE_COLOR 0x99AA2020

static uint32_t pixels[WIDTH * HEIGHT];
static float triangle_angle = 0;
static float circle_x = WIDTH / 2;
static float circle_y = HEIGHT / 2;
static float circle_dx = 100;
static float circle_dy = 100;

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);

#define PI 3.14159265359

static inline void rotate_point(float *x, float *y) {
  float dx = *x - WIDTH / 2;
  float dy = *y - HEIGHT / 2;
  float mag = sqrtf(dx * dx + dy * dy);
  float dir = atan2f(dy, dx) + triangle_angle;
  *x = cosf(dir) * mag + WIDTH / 2;
  *y = sinf(dir) * mag + HEIGHT / 2;
}

uint32_t *render(float dt) {
  Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);

  olivec_fill(oc, BACKGROUND_COLOR);

  {
    triangle_angle += 0.5f * PI * dt;

    float x1 = WIDTH / 2, y1 = HEIGHT / 8;
    float x2 = WIDTH / 8, y2 = HEIGHT / 2;
    float x3 = WIDTH * 7 / 8, y3 = HEIGHT * 7 / 8;
    rotate_point(&x1, &y1);
    rotate_point(&x2, &y2);
    rotate_point(&x3, &y3);
    olivec_triangle(oc, x1, y1, x2, y2, x3, y3, 0xFF2020AA);
  }

  {
    float x = circle_x + circle_dx * dt;
    if (x - CIRCLE_RADIUS < 0 || x + CIRCLE_RADIUS >= WIDTH) {
      circle_dx *= -1;
    } else {
      circle_x = x;
    }

    float y = circle_y + circle_dy * dt;
    if (y - CIRCLE_RADIUS < 0 || y + CIRCLE_RADIUS >= HEIGHT) {
      circle_dy *= -1;
    } else {
      circle_y = y;
    }

    olivec_circle(oc, circle_x, circle_y, CIRCLE_RADIUS, CIRCLE_COLOR);
  }

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

#define SCALE_DOWN_FACTOR 20
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
