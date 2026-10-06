#include "fractal_myflpt.h"
#include <swap.h>

//! \brief  Mandelbrot fractal point calculation function
//! \param  cx    x-coordinate
//! \param  cy    y-coordinate
//! \param  n_max maximum number of iterations
//! \return       number of performed iterations at coordinate (cx, cy)

static myflpt_t my_mul(myflpt_t a, myflpt_t b){

  if((a & 0xFFu) == 0 || (b & 0xFFu) == 0){
    return 0; // zero
  }

  uint32_t sign = (a ^ b) & 0x80000000u;
  uint64_t ma = (1ULL << 23) | ((a >> 8) & 0x7FFFFFu);
  uint64_t mb = (1ULL << 23) | ((b >> 8) & 0x7FFFFFu);
  int exponent = (int)(a & 0xFFu) + (int)(b & 0xFFu) - 500;

  uint64_t product = ma * mb; // wide enough for 24 × 24 bits
  unsigned shift = (product >= (1ULL << 47)) ? 24 : 23;
  if (shift == 24)
      ++exponent;

  if (exponent < -249)
    return 0; // underflow
  if (exponent > 5)
    return sign | 0x7FFFFFFFu; // saturate

  uint32_t fraction = (uint32_t)(product >> shift) & 0x7FFFFFu;
  return sign | (fraction << 8) | (uint32_t)(exponent + 250);

}



static int my_zero(myflpt_t x) { return (x & 0xffu) == 0; }
static unsigned my_exponent(myflpt_t x) { return x & 0xffu; }
static uint32_t my_significand(myflpt_t x) {
  return (1u << 23) | ((x >> 8) & 0x7fffffu);
}

static myflpt_t my_pack(unsigned sign, int exponent, uint32_t significand) {
  if (significand == 0 || exponent < -249) return 0;
  if (exponent > 5) return (sign ? 0x80000000u : 0) | 0x7fffffffu;
  return (sign ? 0x80000000u : 0) |
         ((significand & 0x7fffffu) << 8) | (unsigned)(exponent + 250);
}

static myflpt_t my_add(myflpt_t a, myflpt_t b) {
  if (my_zero(a)) return b;
  if (my_zero(b)) return a;

  // Put the operand with the larger magnitude first.
  if (my_exponent(a) < my_exponent(b) ||
      (my_exponent(a) == my_exponent(b) &&
       my_significand(a) < my_significand(b))) {
    myflpt_t tmp = a; a = b; b = tmp;
  }

  unsigned sign = (a & 0x80000000u) != 0;
  unsigned opposite = ((a ^ b) & 0x80000000u) != 0;
  unsigned distance = my_exponent(a) - my_exponent(b);
  uint64_t large = (uint64_t)my_significand(a) << 32;
  uint64_t small = (uint64_t)my_significand(b) << 32;
  uint64_t aligned = distance >= 64 ? 0 : small >> distance;
  uint64_t remainder = distance >= 64 ? small :
      (distance == 0 ? 0 : small & ((1ull << distance) - 1));
  uint64_t magnitude = opposite ?
      large - (aligned + (remainder != 0)) : large + aligned;
  if (magnitude == 0) return 0;

  int exponent = (int)my_exponent(a) - 250;
  while (magnitude >= ((uint64_t)(2u << 23) << 32)) {
    magnitude >>= 1;
    ++exponent;
  }
  while (magnitude < ((uint64_t)(1u << 23) << 32)) {
    magnitude <<= 1;
    --exponent;
  }
  return my_pack(sign, exponent, (uint32_t)(magnitude >> 32));
}

static myflpt_t my_sub(myflpt_t a, myflpt_t b) {
  return my_add(a, my_zero(b) ? 0 : b ^ 0x80000000u);
}

// Both arguments must be nonnegative.
static int my_ge_positive(myflpt_t a, myflpt_t b) {
  if (my_exponent(a) != my_exponent(b))
    return my_exponent(a) > my_exponent(b);
  return ((a >> 8) & 0x7fffffu) >= ((b >> 8) & 0x7fffffu);
}

uint16_t calc_mandelbrot_point_soft(myflpt_t cx, myflpt_t cy, uint16_t n_max) {
  myflpt_t x = cx;
  myflpt_t y = cy;
  uint16_t n = 0;
  if (n_max == 0) return 0;

  do {
    myflpt_t xx = my_mul(x, x);
    myflpt_t yy = my_mul(y, y);
    myflpt_t two_xy = my_mul(MY_TWO, my_mul(x, y));

    x = my_add(my_sub(xx, yy), cx);
    y = my_add(two_xy, cy);
    ++n;
    if (my_ge_positive(my_add(xx, yy), MY_FOUR)) break;
  } while (n < n_max);
  return n;
}


//! \brief  Map number of performed iterations to black and white
//! \param  iter  performed number of iterations
//! \param  n_max maximum number of iterations
//! \return       colour
rgb565 iter_to_bw(uint16_t iter, uint16_t n_max) {
  if (iter == n_max) {
    return 0x0000;
  }
  return 0xffff;
}


//! \brief  Map number of performed iterations to grayscale
//! \param  iter  performed number of iterations
//! \param  n_max maximum number of iterations
//! \return       colour
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max) {
  if (iter == n_max) {
    return 0x0000;
  }
  uint16_t brightness = iter & 0xf;
  return swap_u16(((brightness << 12) | ((brightness << 7) | brightness<<1)));
}


//! \brief Calculate binary logarithm for unsigned integer argument x
//! \note  For x equal 0, the function returns -1.
int ilog2(unsigned x) {
  if (x == 0) return -1;
  int n = 1;
  if ((x >> 16) == 0) { n += 16; x <<= 16; }
  if ((x >> 24) == 0) { n += 8; x <<= 8; }
  if ((x >> 28) == 0) { n += 4; x <<= 4; }
  if ((x >> 30) == 0) { n += 2; x <<= 2; }
  n -= x >> 31;
  return 31 - n;
}


//! \brief  Map number of performed iterations to a colour
//! \param  iter  performed number of iterations
//! \param  n_max maximum number of iterations
//! \return colour in rgb565 format little Endian (big Endian for openrisc)
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max) {
  if (iter == n_max) {
    return 0x0000;
  }
  uint16_t brightness = (iter&1)<<4|0xF;
  uint16_t r = (iter & (1 << 3)) ? brightness : 0x0;
  uint16_t g = (iter & (1 << 2)) ? brightness : 0x0;
  uint16_t b = (iter & (1 << 1)) ? brightness : 0x0;
  return swap_u16(((r & 0x1f) << 11) | ((g & 0x1f) << 6) | ((b & 0x1f)));
}

rgb565 iter_to_colour1(uint16_t iter, uint16_t n_max) {
  if (iter == n_max) {
    return 0x0000;
  }
  uint16_t brightness = ((iter&0x78)>>2)^0x1F;
  uint16_t r = (iter & (1 << 2)) ? brightness : 0x0;
  uint16_t g = (iter & (1 << 1)) ? brightness : 0x0;
  uint16_t b = (iter & (1 << 0)) ? brightness : 0x0;
  return swap_u16(((r & 0xf) << 12) | ((g & 0xf) << 7) | ((b & 0xf)<<1));
}

//! \brief  Draw fractal into frame buffer
//! \param  width  width of frame buffer
//! \param  height height of frame buffer
//! \param  cfp_p  pointer to fractal function
//! \param  i2c_p  pointer to function mapping number of iterations to colour
//! \param  cx_0   start x-coordinate
//! \param  cy_0   start y-coordinate
//! \param  delta  increment for x- and y-coordinate
//! \param  n_max  maximum number of iterations
void draw_fractal(rgb565 *fbuf, int width, int height,
                  calc_frac_point_p cfp_p, iter_to_colour_p i2c_p,
                  myflpt_t cx_0, myflpt_t cy_0, myflpt_t delta, uint16_t n_max) {
  rgb565 *pixel = fbuf;
  myflpt_t cy = cy_0;
  for (int k = 0; k < height; ++k) {
    myflpt_t cx = cx_0;
    for(int i = 0; i < width; ++i) {
      uint16_t n_iter = (*cfp_p)(cx, cy, n_max);
      rgb565 colour = (*i2c_p)(n_iter, n_max);
      *(pixel++) = colour;
      cx = my_add(cx, delta);
    }
    cy = my_add(cy, delta);
  }
}
