// © Copyright 2021 Xilinx, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef KERNEL_H
#define KERNEL_H

#define FIR_TAPS          			8
#define NUM_SAMPLES            		256
#define AIE_MUL_LANES				8
#define NUM_MUL_8_LANES				(NUM_SAMPLES/AIE_MUL_LANES)
#define NUM_MUL_LOOP				2
#define LOOP_CYCLES					(NUM_MUL_8_LANES/NUM_MUL_LOOP)
#define FIR_T_COEFF					int16
#define FIR_T_DATA					int16

// Kernel prototype
void fir_asym_8t_16int_vectorized(input_window_int16 * in, output_window_int16 * out);

#endif // KERNEL_H
