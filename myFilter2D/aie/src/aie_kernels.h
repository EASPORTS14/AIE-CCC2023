#ifndef _KERNELS_16B_H_
#define _KERNELS_16B_H_

#include <adf/window/types.h>
#include <adf/stream/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// #define PARALLEL_FACTOR_32b 8 // Parallelization factor for 32b operations (8x mults)
// #define SRS_SHIFT 10          // SRS shift used can be increased if input data likewise adjusted)
// #define IMAGE_SIZE 4096       // 256x16
// #define MAX_KERNEL_SIZE 128


const int kernel_width = 3;
const int kernel_height = 3;

#ifdef INLINE
#define INLINE_DECL inline
#else
#define INLINE_DECL
#endif

#define PARALLEL_FACTOR_16b 16 // Parallelization factor for 16b operations (16x mults)
#define SRS_SHIFT 10           // SRS shift used can be increased if input data likewise adjusted)

void filter2D(input_window_int16* input, const int16_t (&coeff)[16], output_window_int16* output);

#endif