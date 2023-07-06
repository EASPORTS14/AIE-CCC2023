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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MCU_NUM 16
#define IMG_W   MCU_NUM*8
#define IMG_H   8
#define OVL_R 100
#define OVL_G 0
#define OVL_B 0


// pixel value saturation, int32 -> uint8
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

// pixel value saturation, float -> uint8
uint8_t pixel_sat_float_8(float in)
{
	uint8_t result;
	if (in < 0) {
		result = 0;
	} else if (in > 255) {
		result = 255;
	} else {
		result = (uint8_t)in;
	}
	return result;
}

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

int main()
{
    FILE *fp_s0 = fopen("./s0.txt", "w");
    FILE *fp_s1 = fopen("./s1.txt", "w");
    FILE *fp_s2 = fopen("./s2_exp.txt", "w");
    FILE *fp_rgb_raw = fopen("./rgb_raw.txt", "w");
	FILE *fp_rgb_rcv = fopen("./rgb_recover.txt", "w");
	FILE *fp_rgb_mix = fopen("./rgb_mixed.txt", "w");
	
    uint8_t rgb_raw_array[MCU_NUM * 64][3];
    uint8_t yuv_array[MCU_NUM * 64][3];
	uint8_t rgb_rcv_array[MCU_NUM * 64][3];
    uint8_t rgb_out_array[MCU_NUM * 64][3];


    for (int i = 0; i < MCU_NUM * 64; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            rgb_raw_array[i][j] = rand() % 256;
        }

        float temp = 0.257 * rgb_raw_array[i][0] + 0.504 * rgb_raw_array[i][1] + 0.098 * rgb_raw_array[i][2] + 16;
        yuv_array[i][0] = pixel_sat_float_8(temp);
        
        temp = -0.148 * rgb_raw_array[i][0] - 0.291 * rgb_raw_array[i][1] + 0.439 * rgb_raw_array[i][2] + 128;
        yuv_array[i][1] = pixel_sat_float_8(temp);

        temp = 0.439 * rgb_raw_array[i][0] - 0.368 * rgb_raw_array[i][1] -0.071 * rgb_raw_array[i][2] + 128;
        yuv_array[i][2] = pixel_sat_float_8(temp);

        int16_t y = (uint16_t)yuv_array[i][0] - 16;
        int16_t u = (uint16_t)yuv_array[i][1] - 128;
        int16_t v = (uint16_t)yuv_array[i][2] - 128;

        int32_t r = (76284 * y + 104595 * v) >> 16;
        int32_t g = (76284 * y -  53281 * v - 25625 * u) >> 16;
        int32_t b = (76284 * y + 132252 * u) >> 16;

        rgb_rcv_array[i][0] = pixel_sat_32_8(r);
		rgb_rcv_array[i][1] = pixel_sat_32_8(g);
		rgb_rcv_array[i][2] = pixel_sat_32_8(b);
		
		rgb_out_array[i][0] = ps_overlay(rgb_rcv_array[i][0], OVL_R);
        rgb_out_array[i][1] = ps_overlay(rgb_rcv_array[i][1], OVL_G);
        rgb_out_array[i][2] = ps_overlay(rgb_rcv_array[i][2], OVL_B);

    }

    fprintf(fp_s0, "%5d %5d %5d %5d\n", IMG_W, IMG_H,   OVL_R, OVL_G);
    fprintf(fp_s0, "%5d %5d %5d %5d\n", OVL_G, MCU_NUM, 0,     0);
    fprintf(fp_s0, "%5d %5d %5d %5d\n", 0, 0, 0, 0);
    fprintf(fp_s0, "%5d %5d %5d %5d\n", 0, 0, 0, 0);

    int pixel_cnt = 0;

    for (int i = 0; i < MCU_NUM; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (int l = 0; l < 4; l++)
            {
                for (int k = 0; k < 16; k++)
                {
                    fprintf(fp_s1, "%5d ", yuv_array[pixel_cnt+l*16+k][j]);
                }
                fprintf(fp_s1, "\n");
            }
        }
        pixel_cnt += 64;
    }

    for (int i = 0; i < MCU_NUM * 64; i++)
    {
        uint32_t rgb_data = (rgb_out_array[i][0] << 16) + (rgb_out_array[i][1] << 8) + rgb_out_array[i][2];		
        fprintf(fp_s2, "%10d\n", rgb_data);
    }

    for (int i = 0; i < MCU_NUM * 64; i++)
    {
        fprintf(fp_rgb_raw, "%4d %4d %4d\n", rgb_raw_array[i][0], rgb_raw_array[i][1], rgb_raw_array[i][2]);
    }

    for (int i = 0; i < MCU_NUM * 64; i++)
    {
        fprintf(fp_rgb_rcv, "%4d %4d %4d\n", rgb_rcv_array[i][0], rgb_rcv_array[i][1], rgb_rcv_array[i][2]);
    }

    for (int i = 0; i < MCU_NUM * 64; i++)
    {
        fprintf(fp_rgb_mix, "%4d %4d %4d\n", rgb_out_array[i][0], rgb_out_array[i][1], rgb_out_array[i][2]);
    }

    fclose(fp_s0);
    fclose(fp_s1);
    fclose(fp_s2);
    fclose(fp_rgb_raw);
	fclose(fp_rgb_rcv);
	fclose(fp_rgb_mix);



}
