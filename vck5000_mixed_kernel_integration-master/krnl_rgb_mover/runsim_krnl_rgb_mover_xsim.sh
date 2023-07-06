#!/bin/bash

#
# Copyright 2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

xvlog -f ./filelist_krnl_rgb_mover.f      \
      -L xilinx_vip                 \
      --sv  # -d DUMP_WAVEFORM
      
xelab tb_krnl_rgb_mover glbl      \
      -debug typical        \
      -L unisims_ver        \
      -L xpm                \
      -L xilinx_vip

xsim -t xsim.tcl --wdb work.tb_krnl_rgb_mover.wdb work.tb_krnl_rgb_mover#work.glbl

