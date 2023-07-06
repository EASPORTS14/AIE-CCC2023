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

ECHO=@echo

.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) ""
	$(ECHO) "  make compile"
	$(ECHO) "      Command to generate xo kernel file"
	$(ECHO) ""
	$(ECHO) "  make clean"
	$(ECHO) "      Command to remove all the generated files."

PLATFORM := xilinx_vck5000_gen4x8_xdma_2_202210_1

TARGET := hw


.phony: clean

################## hardware build 

XOCCFLAGS := --platform $(PLATFORM) -t $(TARGET)  -s -g

compile: krnl_yuv_mover_$(TARGET).xo

krnl_yuv_mover_$(TARGET).xo: ./krnl_yuv_mover.cpp
	v++ $(XOCCFLAGS) --kernel krnl_yuv_mover -c -o $@ $<

################## clean up
clean:
	$(RM) -rf *.xo *.xclbin *.xclbin.info *.xclbin.link_summary *.jou *.log *.xo.compile_summary _x
