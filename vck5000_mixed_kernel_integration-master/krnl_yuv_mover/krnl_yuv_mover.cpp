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

#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

//Combined data mover for S0 and S1 streams
void mm2s_2(ap_int<128>* mem0, hls::stream<qdma_axis<128,0,0,0> >& s0, int size0, ap_int<128>* mem1, hls::stream<qdma_axis<128,0,0,0> >& s1, int size1)
{
	for(int i=0; i<size0; i++)
	{
		#pragma HLS PIPELINE II=1
		qdma_axis<128,0,0,0> x;
		x.data=mem0[i];
		x.keep_all();
		s0.write(x);
	}

	for(int i=0; i<size1; i++)
	{
		#pragma HLS PIPELINE II=1
		qdma_axis<128,0,0,0> x;
		x.data=mem1[i];
		x.keep_all();
		s1.write(x);
	}
}

extern "C" {

	void krnl_yuv_mover(
		int          op_s0_size,
		int          op_s1_size,

		ap_int<128>* op_s0_ptr,
		ap_int<128>* op_s1_ptr,

		hls::stream<qdma_axis<128,0,0,0> >& stream_s0,
		hls::stream<qdma_axis<128,0,0,0> >& stream_s1
	)
	{
		#pragma HLS INTERFACE m_axi port=op_s0_ptr offset=slave bundle=gmem
		#pragma HLS INTERFACE m_axi port=op_s1_ptr offset=slave bundle=gmem

		#pragma HLS INTERFACE axis port=stream_s0
		#pragma HLS INTERFACE axis port=stream_s1

		#pragma HLS INTERFACE s_axilite port=op_s0_size bundle=control
		#pragma HLS INTERFACE s_axilite port=op_s1_size bundle=control

		#pragma HLS INTERFACE s_axilite port=op_s0_ptr  bundle=control
		#pragma HLS INTERFACE s_axilite port=op_s1_ptr  bundle=control

		#pragma HLS INTERFACE s_axilite port=return bundle=control

		#pragma HLS DATAFLOW
		//Preprocess op_*_size fields with a right shift followed by a left shift; allows HLS to optimise AXI burst sizes
		const int rshift=6;         //Right shift lengths by 6; all lengths are mutiples of 64 bytes
		const int lshift=rshift-4;  //Each 128bit word contains 16 bytes
		mm2s_2(op_s0_ptr, stream_s0, (op_s0_size>>rshift)<<lshift, op_s1_ptr, stream_s1, (op_s1_size>>rshift)<<lshift);
	}

}
