#ifndef FRACTAL_MYFLPT_H
#define FRACTAL_MYFLPT_H



#include <stdint.h>

#define MY_TWO  ((myflpt_t)0x000000FBu) // 2.0
#define MY_FOUR ((myflpt_t)0x000000FCu) // 4.0

//! Colour type (5-bit red, 6-bit green, 5-bit blue)
typedef uint16_t rgb565;

typedef uint32_t myflpt_t;


//! \brief Pointer to fractal point calculation function
typedef uint16_t (*calc_frac_point_p)(myflpt_t cx, myflpt_t cy, uint16_t n_max);

uint16_t calc_mandelbrot_point_soft(myflpt_t cx, myflpt_t cy, uint16_t n_max);

//! Pointer to function mapping iteration to colour value
typedef rgb565 (*iter_to_colour_p)(uint16_t iter, uint16_t n_max);

rgb565 iter_to_bw(uint16_t iter, uint16_t n_max);
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max);
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max);

void draw_fractal(rgb565 *fbuf, int width, int height,
                  calc_frac_point_p cfp_p, iter_to_colour_p i2c_p,
                  myflpt_t cx_0, myflpt_t cy_0, myflpt_t delta, uint16_t n_max);

#endif // FRACTAL_MYFLPT_H
