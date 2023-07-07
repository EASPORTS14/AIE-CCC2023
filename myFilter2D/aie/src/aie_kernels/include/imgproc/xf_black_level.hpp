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

#ifndef __XF_BLACK_LEVEL_HPP__
#define __XF_BLACK_LEVEL_HPP__

// =========================================================================
// Required files
// =========================================================================
#include "common/xf_common.hpp"

// =========================================================================
// Actual body
// =========================================================================

template <typename T>
T xf_satcast_bl(int in_val){};

template <>
inline ap_uint<8> xf_satcast_bl<ap_uint<8> >(int v) {
    v = (v > 255 ? 255 : v);
    v = (v < 0 ? 0 : v);
    return v;
};
template <>
inline ap_uint<10> xf_satcast_bl<ap_uint<10> >(int v) {
    v = (v > 1023 ? 1023 : v);
    v = (v < 0 ? 0 : v);
    return v;
};
template <>
inline ap_uint<12> xf_satcast_bl<ap_uint<12> >(int v) {
    v = (v > 4095 ? 4095 : v);
    v = (v < 0 ? 0 : v);
    return v;
};
template <>
inline ap_uint<14> xf_satcast_bl<ap_uint<14> >(int v) {
    v = (v > 16383 ? 16383 : v);
    v = (v < 0 ? 0 : v);
    return v;
};
template <>
inline ap_uint<16> xf_satcast_bl<ap_uint<16> >(int v) {
    v = (v > 65535 ? 65535 : v);
    v = (v < 0 ? 0 : v);
    return v;
};

namespace xf {
namespace cv {

template <int SRC_T,
          int _MAX_ROWS,
          int _MAX_COLS,
          int NPPC = XF_NPPC1,
          int MUL_VALUE_WIDTH = 16,
          int FL_POS = 15,
          int USE_DSP = 1,
          int XFCVDEPTH_IN = _XFCVDEPTH_DEFAULT,
          int XFCVDEPTH_OUT = _XFCVDEPTH_DEFAULT>
void blackLevelCorrection(xf::cv::Mat<SRC_T, _MAX_ROWS, _MAX_COLS, NPPC, XFCVDEPTH_IN>& _Src,
                          xf::cv::Mat<SRC_T, _MAX_ROWS, _MAX_COLS, NPPC, XFCVDEPTH_OUT>& _Dst,
                          unsigned short black_level,
                          float mul_value // ap_uint<MUL_VALUE_WIDTH> mul_value
                          ) {
// clang-format off
#pragma HLS INLINE OFF
    // clang-format on

    // max/(max-black)

    const uint32_t _TC = _MAX_ROWS * (_MAX_COLS >> XF_BITSHIFT(NPPC));

    const int STEP = XF_DTPIXELDEPTH(SRC_T, NPPC);

    uint32_t LoopCount = _Src.rows * (_Src.cols >> XF_BITSHIFT(NPPC));
    uint32_t rw_ptr = 0, wrptr = 0;

    uint32_t max_value = (1 << (XF_DTPIXELDEPTH(SRC_T, NPPC))) - 1;

    ap_ufixed<16, 1> mulval = (ap_ufixed<16, 1>)mul_value;

    int value = 0;

    for (uint32_t i = 0; i < LoopCount; i++) {
// clang-format off
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min=_TC max=_TC
        // clang-format on

        XF_TNAME(SRC_T, NPPC) wr_val = 0;
        XF_TNAME(SRC_T, NPPC) rd_val = _Src.read(rw_ptr++);

        for (uint8_t j = 0; j < NPPC; j++) {
// clang-format off
#pragma HLS UNROLL
            // clang-format on
            XF_CTUNAME(SRC_T, NPPC)
            in_val = rd_val.range(j * STEP + STEP - 1, j * STEP);

            int med_val = (in_val - black_level);

            if (in_val < black_level) {
                value = 0;
            } else {
                value = (int)(med_val * mulval);
            }

            wr_val.range(j * STEP + STEP - 1, j * STEP) = xf_satcast_bl<XF_CTUNAME(SRC_T, NPPC)>(value);
        }

        _Dst.write(wrptr++, wr_val);
    }
}

template <int SRC_T,
          int _MAX_ROWS,
          int _MAX_COLS,
          int NPPC = XF_NPPC1,
          int MUL_VALUE_WIDTH = 16,
          int FL_POS = 15,
          int USE_DSP = 1,
          int STREAMS = 2,
          int XFCVDEPTH_IN = _XFCVDEPTH_DEFAULT,
          int XFCVDEPTH_OUT = _XFCVDEPTH_DEFAULT>
void blackLevelCorrection_multi(xf::cv::Mat<SRC_T, _MAX_ROWS, _MAX_COLS, NPPC, XFCVDEPTH_IN>& _Src,
                                xf::cv::Mat<SRC_T, _MAX_ROWS, _MAX_COLS, NPPC, XFCVDEPTH_OUT>& _Dst,
                                unsigned short black_level[STREAMS],
                                int stream_id) {
// clang-format off
#pragma HLS ARRAY_PARTITION variable= black_level dim=1 complete
    // clang-format on
    float inputMax = (1 << (XF_DTPIXELDEPTH(SRC_T, NPPC))) - 1; // 65535.0f;
    float mul_value = (inputMax / (inputMax - black_level[stream_id]));
    blackLevelCorrection<SRC_T, _MAX_ROWS, _MAX_COLS, NPPC, MUL_VALUE_WIDTH, FL_POS, USE_DSP, XFCVDEPTH_IN,
                         XFCVDEPTH_OUT>(_Src, _Dst, black_level[stream_id], mul_value);
}
}
}

#endif // __XF_BLACK_LEVEL_HPP__
