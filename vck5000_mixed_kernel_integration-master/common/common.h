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
#include <cstdint>
#include <cstddef>

//Definitions that are needed by both the AIE and host sources

//S0 control packet, 16 word (64 bytes) total
struct overlay_S0_control
{
	//Parameters
	uint32_t IMG_W;  // image width
	uint32_t IMG_H;  // image height
	uint32_t OVL_R;  // overlay layer color, Red
	uint32_t OVL_G;  // overlay layer color, Green
	uint32_t OVL_B;  // overlay layer color, Blue
	uint32_t MCU_N;  // JPEG minimal code unit number of JPEG file
	uint32_t RSVD[10]; // reserved words to fill up the 64 byte packets
};
