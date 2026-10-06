#ifndef FRACTAL_FLPT_H
#define FRACTAL_FLPT_H

#include <stdint.h>

//! Colour type (5-bit red, 6-bit green, 5-bit blue)
typedef uint16_t rgb565;



//! \brief Pointer to fractal point calculation function
typedef uint16_t (*calc_frac_point_p)(float cx, float cy, uint16_t n_max);

uint16_t calc_mandelbrot_point_soft(float cx, float cy, uint16_t n_max);

//! Pointer to function mapping iteration to colour value
typedef rgb565 (*iter_to_colour_p)(uint16_t iter, uint16_t n_max);

rgb565 iter_to_bw(uint16_t iter, uint16_t n_max);
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max);
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max);

void draw_fractal(rgb565 *fbuf, int width, int height,
                  calc_frac_point_p cfp_p, iter_to_colour_p i2c_p,
                  float cx_0, float cy_0, float delta, uint16_t n_max);

#endif // FRACTAL_FLPT_H
