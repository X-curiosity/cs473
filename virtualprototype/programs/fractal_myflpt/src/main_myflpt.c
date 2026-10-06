#include "fractal_myflpt.h"
#include "swap.h"
#include "vga.h"
#include "cache.h"
#include "perf.h"
#include <stddef.h>
#include <stdio.h>

// Constants describing the output device
const int SCREEN_WIDTH = 512;   //!< screen width
const int SCREEN_HEIGHT = 512;  //!< screen height

// Constants describing the initial view port on the fractal function
//const float FRAC_WIDTH = 3.0; //!< default fractal width (3.0 in Q4.28)
//const float CX_0 = -2.0;      //!< default start x-coordinate (-2.0 in Q4.28)
//const float CY_0 = -1.5;      //!< default start y-coordinate (-1.5 in Q4.28)
const uint16_t N_MAX = 64;    //!< maximum number of iterations


const myflpt_t FRAC_WIDTH = 0x400000FBu; // 3.0
const myflpt_t CX_0  = 0x800000FBu; // -2.0
const myflpt_t CY_0  = 0xC00000FAu; // -1.5
const myflpt_t DELTA = 0x400000F2u; // 3.0 / 512

int main() {
   perf_init();
   volatile unsigned int *vga = (unsigned int *) 0x50000020;
   volatile unsigned int reg, hi;
   rgb565 frameBuffer[SCREEN_WIDTH*SCREEN_HEIGHT];
   //float delta = FRAC_WIDTH / SCREEN_WIDTH;
   int i;
   vga_clear();
   printf("Starting drawing a fractal\n");
#ifdef __OR1300__
   /* enable the caches */
   asm volatile ("l.nop");
   icache_write_cfg( CACHE_DIRECT_MAPPED | CACHE_SIZE_8K | CACHE_REPLACE_FIFO );
   dcache_write_cfg( CACHE_FOUR_WAY | CACHE_SIZE_8K | CACHE_REPLACE_LRU | CACHE_WRITE_BACK );
   icache_enable(1);
   dcache_enable(1);
#endif
   /* Enable the vga-controller's graphic mode */
   vga[0] = swap_u32(SCREEN_WIDTH);
   vga[1] = swap_u32(SCREEN_HEIGHT);
   vga[2] = swap_u32(1);
   vga[3] = swap_u32((unsigned int)&frameBuffer[0]);
   /* Clear screen */
   for (i = 0 ; i < SCREEN_WIDTH*SCREEN_HEIGHT ; i++) frameBuffer[i]=0;

   perf_start();
   draw_fractal(frameBuffer,SCREEN_WIDTH,SCREEN_HEIGHT,&calc_mandelbrot_point_soft, &iter_to_colour,CX_0,CY_0,DELTA,N_MAX);
   perf_stop();
   perf_print_cycles(PERF_COUNTER_RUNTIME, "Cycles for custom flpt drawing");
   perf_print_time(PERF_COUNTER_RUNTIME, "Time for custom flpt drawing");
   printf("CPU frequency for custom flpt drawing: %u kHz\n", perf_cpu_freq());
#ifdef __OR1300__
   dcache_flush();
#endif
   printf("Done\n");
}
