//
// Copyright 2021 Xilinx, Inc.
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
//

#include "kernel_overlay.h"
#include "common.h"
#include <cstring>

using namespace std;

// Photoshop style overlay function
//   f(a,b) = 2ab              if a < 0.5
//          = 1-2(1-a)(1-b)    otherwise
//   a is the base layer, b is the top layer
uint8_t ps_overlay(uint8_t a, uint8_t b) 
{
	uint8_t result;
	int32_t temp;
	if (a < 128)
	{
		temp = 2 * a * b;
	}
	else
	{
		temp = 65536 - 2 * (256 - a) * (256 - b);
	}
	
	result = temp >> 8;
	return result;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Input window w_s0: control packet. 64 bytes
// Input window w_s1: , 64 * 3 bytes, one byte per pixel and per component
// Output window w_s2: RGBA data window, 64 * 4 bytes, four bytes per pixel
void kernel_overlay(input_window_uint32* restrict w_s0, input_window_uint8* restrict w_s1, output_window_uint32* restrict w_s2)
{

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//Acquire S0 control packet
	window_acquire(w_s0);

	overlay_S0_control& s0_ctrl(*reinterpret_cast<overlay_S0_control*>(w_s0->ptr));

	uint8_t ovl_R, ovl_G, ovl_B;
	ovl_R = (uint8_t)s0_ctrl.OVL_R;
	ovl_G = (uint8_t)s0_ctrl.OVL_G;
	ovl_B = (uint8_t)s0_ctrl.OVL_B;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//Process data
	for (int i = 0; i < s0_ctrl.MCU_N; i++)	// processing data of all MCUs
	{
		window_acquire(w_s1);
		window_acquire(w_s2);

		//Get working pointers into the acquired S1/S2 windows
		uint8_t* restrict in_RGB   =reinterpret_cast<uint8_t* restrict>(w_s1->ptr);
		uint32_t* restrict out_RGB  =reinterpret_cast<uint32_t* restrict>(w_s2->ptr);

		for (int j = 0; j < 64; j++)
		{
			uint8_t in_R, in_G, in_B;
			uint8_t mixed_R, mixed_G, mixed_B;

			in_R = *(in_RGB + j * 3);
            in_G = *(in_RGB + j * 3 + 1);
            in_B = *(in_RGB + j * 3 + 2);

            mixed_R = ps_overlay(in_R, ovl_R);
			mixed_G = ps_overlay(in_G, ovl_G);
			mixed_B = ps_overlay(in_B, ovl_B);

			*(out_RGB + j) = mixed_B + (mixed_G << 8) + (mixed_R << 16);
		}

		window_release(w_s1);
		window_release(w_s2);

		printf("[KERNEL_OVERLAY] finish processing of MCU %d\n", i);
	}

	//All done, release S0 window
	window_release(w_s0);

    printf("[KERNEL_OVERLAY] finish one iteration\n");

}
