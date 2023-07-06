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

#pragma once
#include <adf.h>
#include "kernel_overlay.h"
#include "kernel_cvt.h"

using namespace adf;

class graph_overlay: public graph
{

private:
	kernel k_cvt;
	kernel k_overlay;

public:
	input_plio p_s0;
	input_plio p_s1;
	output_plio p_s2;

	graph_overlay()
	{
		// create kernel
		k_cvt =  kernel::create(kernel_cvt);
		k_overlay = kernel::create(kernel_overlay);

		// create port
		p_s0 = input_plio::create("s0", plio_128_bits, "data/s0.txt");
		p_s1 = input_plio::create("s1", plio_128_bits, "data/s1.txt");
		p_s2 = output_plio::create("s2", plio_32_bits, "data/s2_act.txt");

		// connect port and kernel
		connect<window<64>>(p_s0.out[0], async(k_overlay.in[0]));
		connect<window<64>>(p_s0.out[0], async(k_cvt.in[0]));
		connect<window<192>>(p_s1.out[0], async(k_cvt.in[1]));
		connect<window<192>>(async(k_cvt.out[0]), async(k_overlay.in[1]));
		connect<window<256>>(async(k_overlay.out[0]), p_s2.in[0]);

		// set kernel source codes
		source(k_cvt)     	= "src/kernel_cvt.cpp";
		source(k_overlay) 	= "src/kernel_overlay.cpp";
		headers(k_cvt) 	  	= {"src/kernel_cvt.h","../common/common.h"};
		headers(k_overlay)	= {"src/kernel_overlay.h","../common/common.h"};

		// set ratio
		runtime<ratio>(k_cvt)     =0.9;
		runtime<ratio>(k_overlay) =0.9;

	};

};
