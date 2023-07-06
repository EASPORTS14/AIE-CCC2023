# Copyright 2021 Xilinx Inc.
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

ECHO=@echo

.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make compile TARGET=<hw/hw_emu>"
	$(ECHO) ""
	$(ECHO) "  make compile_sim"
	$(ECHO) ""
	$(ECHO) "  make clean"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""


PLATFORM := xilinx_vck5000_gen4x8_xdma_2_202210_1
TARGET := hw

XOCCFLAGS := --platform $(PLATFORM) -t $(TARGET)  -s -g

KERNEL_FLAGS_JPEG_DECODER := --kernel krnl_jpeg -I. -I./include/hw/jpegDec --advanced.prop kernel.kernelJpegDecoder.kernel_flags="-std=c++0x -D__SDSVHLS__ -DHLS_NO_XIL_FPO_LIB"

.phony: clean

################## hardware build 
compile: krnl_jpeg_$(TARGET).xo

krnl_jpeg_$(TARGET).xo: krnl_jpeg.cpp include/hw/jpegDec/*
	v++ $(XOCCFLAGS) $(KERNEL_FLAGS_JPEG_DECODER) -c -o $@ $<

compile_sim: krnl_jpeg_test

krnl_jpeg_test: krnl_jpeg.cpp krnl_jpeg_test.cpp
	g++ -std=c++11 -I. -I./include/hw/jpegDec -I$(XILINX_HLS)/include -o $@ $^

################## clean up
clean:
	$(RM) -r _x .Xil *.ltx *.log *.jou *.info *.xclbin *.xclbin.* *.xo *.xo.* *.str
	
