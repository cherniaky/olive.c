#define OLIVEC_IMPLEMENTATION
#include "./olive.c"

#include "./assets/nikita.c"

#define WIDTH 800
#define HEIGHT 600
#define SCALE_DOWN_FACTOR 10

float sinf(float);

uint32_t dst[WIDTH*HEIGHT];
float global_time = 0;

#define SRC_SCALE 3

uint32_t *render(float dt)
{
    global_time += dt;

    float t = sinf(10*global_time);

    olivec_fill(olivec_canvas(dst, WIDTH, HEIGHT, WIDTH), 0xFF181818);

    Olivec_Canvas dst_canvas = olivec_canvas(dst, WIDTH, HEIGHT, WIDTH);

    int factor = 100;
    int w = 400 - t*factor;
    int h = 400 + t*factor;

    olivec_copy(
        olivec_canvas(png, png_width, png_height, png_width),
        olivec_subcanvas(dst_canvas, WIDTH/2 - w/2, HEIGHT - h, w, h));

    return dst;
}

#include "vc.c"
