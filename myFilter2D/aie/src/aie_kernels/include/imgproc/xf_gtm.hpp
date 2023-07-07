/*
 * Copyright (C) 2019-2022, Xilinx, Inc.
 * Copyright (C) 2022-2023, Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _XF_GTM_HPP_
#define _XF_GTM_HPP_

#include "ap_int.h"
#include "hls_stream.h"
#include "common/xf_common.hpp"
#include "hls_math.h"
#include "hls_stream.h"
#include "xf_channel_combine.hpp"
#include "xf_channel_extract.hpp"
#include "xf_cvt_color.hpp"
#include "xf_duplicateimage.hpp"
#include <iostream>
#include <assert.h>

#define __ABS(X) ((X) < 0 ? (-(X)) : (X))

namespace xf {
namespace cv {

static int highest_bit(unsigned int a) {
    int count;
    std::frexp(a, &count);
    return count;
}
template <int WIDTH>
void logarithm(ap_uint<WIDTH> in, ap_ufixed<16, 4>& out) {
    ap_uint<WIDTH> input = (unsigned int)(in); // input rawbits converted to uint

    int N = highest_bit((unsigned int)input); // To calculate the position of MSB bit

    ap_ufixed<32, 16> in_fixed = in; // input rawbits converted to float, stored in ap_fixed type for further operations

    ap_fixed<WIDTH, 4> op1 = (in_fixed >> N); // x/2^N

    ap_fixed<WIDTH, 1> op2 = (op1 - 1) / (op1 + 1);

    ap_fixed<WIDTH, 4> op3 = 1 + (op2 * op2) / 3;

    ap_fixed<WIDTH, 4> op4 = 2 * op2 * op3; // (1 + 2x + 2x^3/3  )

    ap_ufixed<WIDTH, 1> eval = 0.69314f; // ln2

    ap_ufixed<WIDTH, 1> eval1 = 0.43429f; // loge

    ap_ufixed<WIDTH, 4> finalVal = op4 + N * eval;

    out = finalVal * eval1;
}

template <int WIDTH = 16>
void exponential(ap_fixed<WIDTH, 4> in, ap_ufixed<16, 8>& out) {
    ap_ufixed<WIDTH, 4> eval = 1.44269f;

    ap_ufixed<WIDTH, 4> eval1 = 0.69314f;

    ap_fixed<WIDTH, 4> in_base2 = in * eval;

    int int_part = in_base2.to_int();

    ap_ufixed<WIDTH, 8> int_out, int_out1;

    int_out = 1 << __ABS(int_part);

    if (int_part < 0) int_out = (ap_ufixed<WIDTH, 8>)1 / int_out;

    ap_fixed<WIDTH, 1> frac_part = (in_base2 - int_part) * eval1;

    ap_fixed<WIDTH, 1> sq = (frac_part * frac_part);
    ap_fixed<WIDTH, 1> sq_comp = sq / 2;
    ap_fixed<WIDTH, 1> cube = (sq * frac_part);
    ap_fixed<WIDTH, 1> cube_comp = cube / 6;
    ap_ufixed<WIDTH, 4> frac_out = 1 + frac_part + sq_comp + cube_comp;

    out = int_out * frac_out;
}
template <int SIN_CHANNEL_IN_TYPE,
          int SIN_CHANNEL_OUT_TYPE,
          int ROWS,
          int COLS,
          int NPC,
          int XFCVDEPTH_YIMAGE = _XFCVDEPTH_DEFAULT,
          int TC>
void xFcompute_mean(xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_YIMAGE>& yimage,
                    ap_ufixed<16, 4>& mean_fixed,
                    ap_ufixed<16, 4>& L_max,
                    ap_ufixed<16, 4>& L_min,
                    int rows,
                    int cols) {
// clang-format off
		#pragma HLS INLINE OFF
	// clang-format on		
		
		const int PXL_WIDTH = XF_PIXELWIDTH(SIN_CHANNEL_IN_TYPE, NPC);
		
		int rd_ptr = 0,wr_ptr = 0;
		ap_int<13> i,j,k;
		
		ap_ufixed<32, 24> tmp_sum[1 << XF_BITSHIFT(NPC)];
		ap_ufixed<32, 24> sum = 0;
// clang-format off
    #pragma HLS ARRAY_PARTITION variable=tmp_sum complete 
	// clang-format on		
		
		for (j = 0; j < (1 << XF_BITSHIFT(NPC)); j++) {
// clang-format off
			#pragma HLS UNROLL
        // clang-format on
        tmp_sum[j] = 0;
    }

    ap_ufixed<16, 4> log_out;
rowLoop1:
    for (i = 0; i < rows; i++) {
// clang-format off
			#pragma HLS LOOP_TRIPCOUNT min=ROWS max=ROWS
			#pragma HLS LOOP_FLATTEN off
    // clang-format on

    colLoop1:
        for (j = 0; j < cols; j++) {
// clang-format off
				#pragma HLS LOOP_TRIPCOUNT min=TC max=TC
				#pragma HLS PIPELINE
            // clang-format on

            XF_TNAME(SIN_CHANNEL_IN_TYPE, NPC) val_src1;
            val_src1 = yimage.read(rd_ptr++);

        procLoop1:
            for (k = 0; k < NPC; k++) {
// clang-format off
					#pragma HLS UNROLL
                // clang-format on

                XF_DTUNAME(SIN_CHANNEL_IN_TYPE, NPC) pxl_val;
                pxl_val =
                    val_src1.range((k + 1) * PXL_WIDTH - 1, k * PXL_WIDTH); // Get bits from certain range of positions.

                logarithm(pxl_val, log_out);

                tmp_sum[k] = tmp_sum[k] + log_out;

                if (log_out > L_max) L_max = log_out;
                if (log_out < L_min) L_min = log_out;
            }
        }
    }

    for (j = 0; j < (1 << XF_BITSHIFT(NPC)); j++) {
// clang-format off
			#pragma HLS UNROLL
        // clang-format on
        sum = sum + tmp_sum[j];
    }
    ap_ufixed<32, 1> inv_divsion = (ap_ufixed<32, 1>)1 / (ap_ufixed<32, 32>)(rows * cols * NPC);
    mean_fixed = (sum * inv_divsion);
    return;
}

template <int SIN_CHANNEL_IN_TYPE,
          int SIN_CHANNEL_OUT_TYPE,
          int ROWS,
          int COLS,
          int NPC,
          int XFCVDEPTH_YIMAGE = _XFCVDEPTH_DEFAULT,
          int TC>
void xFcompute_mean_multi(xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_YIMAGE>& yimage,
                          ap_ufixed<16, 4>& mean_fixed,
                          ap_ufixed<16, 4>& L_max,
                          ap_ufixed<16, 4>& L_min,
                          int rows,
                          int cols,
                          unsigned short org_height,
                          int slc_id,
                          ap_ufixed<32, 24>& acc_sum) {
// clang-format off
		#pragma HLS INLINE OFF
	// clang-format on		
	   
        
		const int PXL_WIDTH = XF_PIXELWIDTH(SIN_CHANNEL_IN_TYPE, NPC);
		int rd_ptr = 0,wr_ptr = 0;
		ap_int<13> i,j,k;
		
		ap_ufixed<32, 24> tmp_sum[1 << XF_BITSHIFT(NPC)];
		ap_ufixed<32, 24> sum=0;
      
		
        
        if(slc_id==0) {
		
  
            acc_sum = 0;
        
        } 
// clang-format off
    #pragma HLS ARRAY_PARTITION variable=tmp_sum complete 
	// clang-format on		
		
		for (j = 0; j < (1 << XF_BITSHIFT(NPC)); j++) {
// clang-format off
			#pragma HLS UNROLL
        // clang-format on
        tmp_sum[j] = 0;
    }

    ap_ufixed<16, 4> log_out;
rowLoop1:
    for (i = 0; i < rows; i++) {
// clang-format off
			#pragma HLS LOOP_TRIPCOUNT min=ROWS max=ROWS
			#pragma HLS LOOP_FLATTEN off
    // clang-format on

    colLoop1:
        for (j = 0; j < cols; j++) {
// clang-format off
				#pragma HLS LOOP_TRIPCOUNT min=TC max=TC
				#pragma HLS PIPELINE
            // clang-format on

            XF_TNAME(SIN_CHANNEL_IN_TYPE, NPC) val_src1;
            val_src1 = yimage.read(rd_ptr++);

        procLoop1:
            for (k = 0; k < NPC; k++) {
// clang-format off
					#pragma HLS UNROLL
                // clang-format on

                XF_DTUNAME(SIN_CHANNEL_IN_TYPE, NPC) pxl_val;
                pxl_val =
                    val_src1.range((k + 1) * PXL_WIDTH - 1, k * PXL_WIDTH); // Get bits from certain range of positions.

                logarithm(pxl_val, log_out);

                tmp_sum[k] = tmp_sum[k] + log_out;

                if (log_out > L_max) L_max = log_out;
                if (log_out < L_min) L_min = log_out;
            }
        }
    }

    for (j = 0; j < (1 << XF_BITSHIFT(NPC)); j++) {
// clang-format off
			#pragma HLS UNROLL
        // clang-format on

        sum = sum + tmp_sum[j];
    }

    acc_sum += sum;

    ap_ufixed<32, 1> inv_divsion = (ap_ufixed<32, 1>)1 / (ap_ufixed<32, 32>)(org_height * cols * NPC);

    mean_fixed = (acc_sum * inv_divsion);

    return;
}

template <int SIN_CHANNEL_IN_TYPE,
          int SIN_CHANNEL_OUT_TYPE,
          int ROWS,
          int COLS,
          int NPC,
          int XFCVDEPTH_IN = _XFCVDEPTH_DEFAULT,
          int TC>
void xFcompute_xyzmapped(xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN>& ximage,
                         xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN>& yimage,
                         xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN>& zimage,
                         ap_ufixed<16, 4>& mean,
                         ap_ufixed<16, 4>& L_max,
                         ap_ufixed<16, 4>& L_min,
                         float c1,
                         float c2,
                         xf::cv::Mat<SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN>& xmapped,
                         xf::cv::Mat<SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN>& ymapped,
                         xf::cv::Mat<SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN>& zmapped,
                         int rows,
                         int cols) {
// clang-format off
		#pragma HLS INLINE OFF
		// clang-format on				
		
		const int PXL_WIDTH_IN = XF_PIXELWIDTH(SIN_CHANNEL_IN_TYPE, NPC);
		const int PXL_WIDTH_OUT = XF_PIXELWIDTH(SIN_CHANNEL_OUT_TYPE, NPC);
		
		int rd_ptr = 0,wr_ptr = 0;
		
		ap_int<13> i,j,k;
		
		ap_ufixed<16, 4> ld_nume = 2.4;
		ap_ufixed<16, 4> ld_dinom = L_max-L_min;
	
        
     
        
		ap_ufixed<16, 1> inv_L_range =  (ap_ufixed<16, 1>)1/ld_dinom;	
		
		ap_ufixed<16, 4> K1 = (ld_nume * inv_L_range);
		ap_fixed<16, 4> _k1 = 1- K1; 
		
		ap_ufixed<16, 4> c1_fixed = (ap_ufixed<16, 8>) c1;
		
		ap_ufixed<16, 4> ld_dinom_sq = (ld_dinom * ld_dinom);
		ap_ufixed<16, 1> inv_comp = (ap_ufixed<16, 1>)1/(ap_ufixed<16, 8>)(2 * ld_dinom_sq);
		
		ap_ufixed<16, 4> c1_sq = (c1_fixed * c1_fixed);
		ap_ufixed<16, 4> sigma_sq = (c1_sq * inv_comp);
		
		ap_ufixed<16, 4> log_out;
		ap_ufixed<16, 8> exp_out1, exp_out2;
		
		float val_out;
		
		XF_TNAME(SIN_CHANNEL_IN_TYPE, NPC) val_xin, val_zin, val_yin;
		XF_TNAME(SIN_CHANNEL_OUT_TYPE, NPC) val_xout, val_zout, val_yout;
		
		rd_ptr=0, wr_ptr=0;
		ap_ufixed<16, 4> c2_fixed = (ap_ufixed<16, 8>) c2;
	
	rowLoop2:
		for (i = 0; i < rows; i++) {
	// clang-format off
			#pragma HLS LOOP_TRIPCOUNT min=ROWS max=ROWS
			#pragma HLS LOOP_FLATTEN off
    // clang-format on

    colLoop2:
        for (j = 0; j < cols; j++) {
// clang-format off
				#pragma HLS LOOP_TRIPCOUNT min=TC max=TC
				#pragma HLS PIPELINE
            // clang-format on

            val_yin = yimage.read(rd_ptr);
            val_xin = ximage.read(rd_ptr);
            val_zin = zimage.read(rd_ptr);

        procLoop2:
            for (k = 0; k < NPC; k++) {
// clang-format off
					#pragma HLS UNROLL
                // clang-format on

                XF_DTUNAME(SIN_CHANNEL_IN_TYPE, NPC) pxl_valx, pxl_valy, pxl_valz;

                pxl_valx = val_xin.range((k + 1) * PXL_WIDTH_IN - 1, k * PXL_WIDTH_IN);
                pxl_valy = val_yin.range((k + 1) * PXL_WIDTH_IN - 1, k * PXL_WIDTH_IN);
                pxl_valz = val_zin.range((k + 1) * PXL_WIDTH_IN - 1, k * PXL_WIDTH_IN);

                logarithm(pxl_valy, log_out);

                ap_fixed<16, 4> pxl_val = log_out - mean;

                ap_fixed<16, 4> pxl_val_sq = pxl_val * pxl_val;
                ap_fixed<16, 4> exp_in1 = -pxl_val_sq * sigma_sq;

                exponential(exp_in1, exp_out1);

                ap_ufixed<16, 4> K2 = _k1 * exp_out1 + K1;
                ap_ufixed<16, 4> prod = c2_fixed * K2;
                ap_fixed<16, 4> exp_in2 = (prod * pxl_val + mean);

                exponential(exp_in2, exp_out2);

                float pxl_ratio = exp_out2.to_float() / pxl_valy;
                ap_ufixed<24, 1> ratio = (ap_ufixed<24, 1>)pxl_ratio;

                ap_ufixed<16, 8> pxl_x = ratio * (ap_ufixed<16, 16>)pxl_valx;
                ap_ufixed<16, 8> pxl_z = ratio * (ap_ufixed<16, 16>)pxl_valz;

                val_xout.range((k + 1) * PXL_WIDTH_OUT - 1, k * PXL_WIDTH_OUT) = hls::round(pxl_x);
                val_zout.range((k + 1) * PXL_WIDTH_OUT - 1, k * PXL_WIDTH_OUT) = hls::round(pxl_z);
                val_yout.range((k + 1) * PXL_WIDTH_OUT - 1, k * PXL_WIDTH_OUT) = hls::round(exp_out2);
            }

            ymapped.write(wr_ptr, val_yout);
            xmapped.write(wr_ptr, val_xout);
            zmapped.write(wr_ptr, val_zout);

            rd_ptr++;
            wr_ptr++;
        }
    }
    return;
}

template <int SRC_T,
          int DST_T,
          int SIN_CHANNEL_IN_TYPE,
          int SIN_CHANNEL_OUT_TYPE,
          int ROWS,
          int COLS,
          int NPC,
          int XFCVDEPTH_IN = _XFCVDEPTH_DEFAULT,
          int XFCVDEPTH_OUT = _XFCVDEPTH_DEFAULT>
void gtm(xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN>& src,
         xf::cv::Mat<DST_T, ROWS, COLS, NPC, XFCVDEPTH_OUT>& dst,
         ap_ufixed<16, 4>& mean1,
         ap_ufixed<16, 4>& mean2,
         ap_ufixed<16, 4>& L_max1,
         ap_ufixed<16, 4>& L_max2,
         ap_ufixed<16, 4>& L_min1,
         ap_ufixed<16, 4>& L_min2,
         float c1,
         float c2) {
#ifndef __SYNTHESIS__
    assert(((SRC_T == XF_16UC3) || (SRC_T == XF_14UC3)) && "Input TYPE must be XF_16UC3 or XF_14UC3");
    assert(((SIN_CHANNEL_IN_TYPE == XF_16UC1) || (SIN_CHANNEL_IN_TYPE == XF_14UC1)) &&
           "Input Single Channel TYPE must be XF_16UC1 or XF_14UC1");
    assert(((DST_T == XF_8UC3) || (SIN_CHANNEL_OUT_TYPE == XF_8UC1)) && "OUTPUT TYPE must be XF_8UC3");
    assert(((NPC == XF_NPPC1) || (NPC == XF_NPPC2)) && "NPC must be XF_NPPC1, XF_NPPC2 ");
    assert((src.rows <= ROWS) && (src.cols <= COLS) && "ROWS and COLS should be greater than input image size ");
#endif
    int rows = src.rows;
    int cols = src.cols;

    uint16_t cols_shifted = cols >> (XF_BITSHIFT(NPC));

    xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN> bgr2xyz(rows, cols);

    xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN> xyzimg1(rows, cols);
    xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN> xyzimg2(rows, cols);
    xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN> xyzimg3(rows, cols);
    xf::cv::Mat<DST_T, ROWS, COLS, NPC, XFCVDEPTH_IN> xyzoutput(rows, cols);

    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> ximage(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> yimage(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> yimage1(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> yimage2(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> zimage(rows, cols);

    xf::cv::Mat<SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> xmapped(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> ymapped(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> zmapped(rows, cols);

// clang-format off
		#pragma HLS DATAFLOW
    // clang-format on

    // Convert BGR to XYZ:
    xf::cv::bgr2xyz<SRC_T, SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN>(src, bgr2xyz);

    xf::cv::duplicateimages<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN, XFCVDEPTH_IN, XFCVDEPTH_IN>(
        bgr2xyz, xyzimg1, xyzimg2, xyzimg3);

    xf::cv::extractChannel<SRC_T, SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN>(xyzimg1, ximage, 0);

    xf::cv::extractChannel<SRC_T, SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN>(xyzimg2, yimage, 1);

    xf::cv::extractChannel<SRC_T, SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN>(xyzimg3, zimage, 2);

    xf::cv::duplicateMat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN, XFCVDEPTH_IN>(
        yimage, yimage1, yimage2);

    xFcompute_mean<SIN_CHANNEL_IN_TYPE, SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN,
                   (COLS >> (XF_BITSHIFT(NPC)))>(yimage1, mean1, L_max1, L_min1, rows, cols_shifted);

    xFcompute_xyzmapped<SIN_CHANNEL_IN_TYPE, SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN,
                        (COLS >> (XF_BITSHIFT(NPC)))>(ximage, yimage2, zimage, mean2, L_max2, L_min2, c1, c2, xmapped,
                                                      ymapped, zmapped, rows, cols_shifted);

    xf::cv::merge<SIN_CHANNEL_OUT_TYPE, DST_T, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN, XFCVDEPTH_IN>(
        zmapped, ymapped, xmapped, xyzoutput);

    xf::cv::xyz2bgr<DST_T, DST_T, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_OUT>(xyzoutput, dst);

    return;
}

template <int SRC_T,
          int DST_T,
          int SIN_CHANNEL_IN_TYPE,
          int SIN_CHANNEL_OUT_TYPE,
          int ROWS,
          int COLS,
          int NPC,
          int XFCVDEPTH_IN = _XFCVDEPTH_DEFAULT,
          int XFCVDEPTH_OUT = _XFCVDEPTH_DEFAULT>
void gtm_multi(xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN>& src,
               xf::cv::Mat<DST_T, ROWS, COLS, NPC, XFCVDEPTH_OUT>& dst,
               ap_ufixed<16, 4>& mean1,
               ap_ufixed<16, 4>& mean2,
               ap_ufixed<16, 4>& L_max1,
               ap_ufixed<16, 4>& L_max2,
               ap_ufixed<16, 4>& L_min1,
               ap_ufixed<16, 4>& L_min2,
               float c1,
               float c2,
               unsigned short org_height,
               int slc_id,
               ap_ufixed<32, 24>& acc_sum) {
#ifndef __SYNTHESIS__
    assert(((SRC_T == XF_16UC3) || (SRC_T == XF_14UC3)) && "Input TYPE must be XF_16UC3 or XF_14UC3");
    assert(((SIN_CHANNEL_IN_TYPE == XF_16UC1) || (SIN_CHANNEL_IN_TYPE == XF_14UC1)) &&
           "Input Single Channel TYPE must be XF_16UC1 or XF_14UC1");
    assert(((DST_T == XF_8UC3) || (SIN_CHANNEL_OUT_TYPE == XF_8UC1)) && "OUTPUT TYPE must be XF_8UC3");
    assert(((NPC == XF_NPPC1) || (NPC == XF_NPPC2)) && "NPC must be XF_NPPC1, XF_NPPC2 ");
    assert((src.rows <= ROWS) && (src.cols <= COLS) && "ROWS and COLS should be greater than input image size ");
#endif
    int rows = src.rows;
    int cols = src.cols;
    uint16_t cols_shifted = cols >> (XF_BITSHIFT(NPC));

    xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN> bgr2xyz(rows, cols);

    xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN> xyzimg1(rows, cols);
    xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN> xyzimg2(rows, cols);
    xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN> xyzimg3(rows, cols);
    xf::cv::Mat<DST_T, ROWS, COLS, NPC, XFCVDEPTH_IN> xyzoutput(rows, cols);

    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> ximage(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> yimage(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> yimage1(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> yimage2(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> zimage(rows, cols);

    xf::cv::Mat<SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> xmapped(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> ymapped(rows, cols);
    xf::cv::Mat<SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN> zmapped(rows, cols);

// clang-format off
		#pragma HLS DATAFLOW
    // clang-format on

    // Convert BGR to XYZ:
    xf::cv::bgr2xyz<SRC_T, SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN>(src, bgr2xyz);

    xf::cv::duplicateimages<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN, XFCVDEPTH_IN, XFCVDEPTH_IN>(
        bgr2xyz, xyzimg1, xyzimg2, xyzimg3);

    xf::cv::extractChannel<SRC_T, SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN>(xyzimg1, ximage, 0);

    xf::cv::extractChannel<SRC_T, SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN>(xyzimg2, yimage, 1);

    xf::cv::extractChannel<SRC_T, SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN>(xyzimg3, zimage, 2);

    xf::cv::duplicateMat<SIN_CHANNEL_IN_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN, XFCVDEPTH_IN>(
        yimage, yimage1, yimage2);

    xFcompute_mean_multi<SIN_CHANNEL_IN_TYPE, SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN,
                         (COLS >> (XF_BITSHIFT(NPC)))>(yimage1, mean1, L_max1, L_min1, rows, cols_shifted, org_height,
                                                       slc_id, acc_sum);

    xFcompute_xyzmapped<SIN_CHANNEL_IN_TYPE, SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN,
                        (COLS >> (XF_BITSHIFT(NPC)))>(ximage, yimage2, zimage, mean2, L_max2, L_min2, c1, c2, xmapped,
                                                      ymapped, zmapped, rows, cols_shifted);

    xf::cv::merge<SIN_CHANNEL_OUT_TYPE, DST_T, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_IN, XFCVDEPTH_IN>(
        zmapped, ymapped, xmapped, xyzoutput);

    xf::cv::xyz2bgr<DST_T, DST_T, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_OUT>(xyzoutput, dst);

    return;
}

template <int SRC_T,
          int DST_T,
          int SIN_CHANNEL_IN_TYPE,
          int SIN_CHANNEL_OUT_TYPE,
          int ROWS,
          int COLS,
          int NPC,
          int XFCVDEPTH_IN = _XFCVDEPTH_DEFAULT,
          int XFCVDEPTH_OUT = _XFCVDEPTH_DEFAULT>
void gtm_multistream(xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN>& src,
                     xf::cv::Mat<DST_T, ROWS, COLS, NPC, XFCVDEPTH_OUT>& dst,
                     ap_ufixed<16, 4>& mean1,
                     ap_ufixed<16, 4>& mean2,
                     ap_ufixed<16, 4>& L_max1,
                     ap_ufixed<16, 4>& L_max2,
                     ap_ufixed<16, 4>& L_min1,
                     ap_ufixed<16, 4>& L_min2,
                     float c1,
                     float c2,
                     bool& flag,
                     bool& eof,
                     unsigned short org_height,
                     int slc_id,
                     ap_ufixed<32, 24>& acc_sum) {
// clang-format off
#pragma HLS INLINE OFF
    // clang-format on

    if (!flag) {
        xf::cv::gtm_multi<SRC_T, DST_T, SIN_CHANNEL_IN_TYPE, SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN,
                          XFCVDEPTH_OUT>(src, dst, mean1, mean2, L_max1, L_max2, L_min1, L_min2, c1, c2, org_height,
                                         slc_id, acc_sum);

        if (eof) flag = 1;
    } else {
        xf::cv::gtm_multi<SRC_T, DST_T, SIN_CHANNEL_IN_TYPE, SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN,
                          XFCVDEPTH_OUT>(src, dst, mean2, mean1, L_max2, L_max1, L_min2, L_min1, c1, c2, org_height,
                                         slc_id, acc_sum);

        if (eof) flag = 0;
    }
    return;
}
template <int SRC_T,
          int DST_T,
          int SIN_CHANNEL_IN_TYPE,
          int SIN_CHANNEL_OUT_TYPE,
          int ROWS,
          int COLS,
          int NPC,
          int STREAMS = 2,
          int XFCVDEPTH_IN = _XFCVDEPTH_DEFAULT,
          int XFCVDEPTH_OUT = _XFCVDEPTH_DEFAULT>
void gtm_multi_wrap(xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN>& src,
                    xf::cv::Mat<DST_T, ROWS, COLS, NPC, XFCVDEPTH_OUT>& dst,
                    ap_ufixed<16, 4> mean1[STREAMS],
                    ap_ufixed<16, 4> mean2[STREAMS],
                    ap_ufixed<16, 4> L_max1[STREAMS],
                    ap_ufixed<16, 4> L_max2[STREAMS],
                    ap_ufixed<16, 4> L_min1[STREAMS],
                    ap_ufixed<16, 4> L_min2[STREAMS],
                    float c1[STREAMS],
                    float c2[STREAMS],
                    bool flag[STREAMS],
                    bool eof[STREAMS],
                    int strm_id,
                    unsigned short org_height,
                    int slc_id,
                    ap_ufixed<32, 24> acc_sum[STREAMS]) {
// clang-format off
#pragma HLS ARRAY_PARTITION variable= mean1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable= mean2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable= L_max1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable= L_max2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable= L_min1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable= L_min2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable= c1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable= c2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=flag dim=1 complete
#pragma HLS ARRAY_PARTITION variable=eof dim=1 complete
#pragma HLS ARRAY_PARTITION variable= acc_sum dim=1 complete
    // clang-format on  
   
    gtm_multistream<SRC_T, DST_T, SIN_CHANNEL_IN_TYPE, SIN_CHANNEL_OUT_TYPE, ROWS, COLS, NPC, XFCVDEPTH_IN, XFCVDEPTH_OUT>
              (src, dst, mean1[strm_id], mean2[strm_id], L_max1[strm_id], L_max2[strm_id], L_min1[strm_id], L_min2[strm_id],
                 c1[strm_id], c2[strm_id], flag[strm_id], eof[strm_id], org_height, slc_id, acc_sum[strm_id]);
         
}


////////

} // namespace cv
} // namespace xf

#endif
