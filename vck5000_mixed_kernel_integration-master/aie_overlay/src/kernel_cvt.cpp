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

#include "kernel_cvt.h"
#include "common.h"

using namespace std;

// pixel value saturation, int16 -> uint8
uint8_t pixel_sat_32_8(int32_t in)
{
	uint8_t result;
	if (in < 0) {
		result = 0;
	} else if (in > 255) {
		result = 255;
	} else {
		result = in;
	}
	return result;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Color space conversion kernel: input is uint8 YUV data and output is float RGB data
// Input window w_s0: control packet. 64 bytes
// Input window w_s1: YUV data window, 64 * 3 bytes, one byte per pixel and per component
// Output window w_s2: RGB data window, 64 * 3 bytes, one byte per pixel and per component
void kernel_cvt(input_window_uint32* restrict w_s0, input_window_uint8* restrict w_s1, output_window_uint8* restrict w_s2)
{

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//Acquire S0 control packet
	window_acquire(w_s0);
	overlay_S0_control& s0_ctrl(*reinterpret_cast<overlay_S0_control*>(w_s0->ptr));

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//Process data
	for (int i = 0; i < s0_ctrl.MCU_N; i++)	// processing data of all MCUs
	{

		// The input window size will be 64 x 3, because each MCU will include three 8x8 bytes blocks (Y,U,V) of data from decoded JPEG files
		// The output window size will be 64 x 3, RGB format will be used for output data

		window_acquire(w_s1);
		window_acquire(w_s2);

		//Get working pointers into the acquired S1/S2 windows
		uint8_t* restrict in_YUV   =reinterpret_cast<uint8_t* restrict>(w_s1->ptr);
		uint8_t* restrict out_RGB  =reinterpret_cast<uint8_t* restrict>(w_s2->ptr);

		for (int j = 0; j < 64; j++)
		{
			int16_t in_Y, in_U, in_V;
			int32_t out_R, out_G, out_B;
			
			// R = 1.164 * (Y - 16) + 1.596 * (V - 128)
			// G = 1.164 * (Y - 16) - 0.813 * (V - 128) - 0.391 * (U - 128)
			// B = 1.164 * (Y - 16) + 2.018 * (U - 128)

			in_Y = (int16_t)(*(in_YUV + j)) - 16;
			in_U = (int16_t)(*(in_YUV + 64 + j)) - 128;
			in_V = (int16_t)(*(in_YUV + 128 + j)) - 128;

			out_R = 76284 * in_Y + 104595 * in_V;
            out_G = 76284 * in_Y -  53281 * in_V - 25625 * in_U;
            out_B = 76284 * in_Y + 132252 * in_U;

			out_R = pixel_sat_32_8(out_R >> 16);
			out_G = pixel_sat_32_8(out_G >> 16);
			out_B = pixel_sat_32_8(out_B >> 16);

			*(out_RGB + j * 3)     = out_R;
			*(out_RGB + j * 3 + 1) = out_G;
			*(out_RGB + j * 3 + 2) = out_B;
		}

		window_release(w_s1);
		window_release(w_s2);

	}

	//All done, release S0 window
	window_release(w_s0);
}
