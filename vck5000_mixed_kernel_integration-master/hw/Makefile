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
	$(ECHO) ""
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<hw/hw_emu>"
	$(ECHO) ""
	$(ECHO) "  make clean"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""

PLATFORM := xilinx_vck5000_gen4x8_qdma_2_202220_1
TARGET := hw

XOCCLFLAGS := --kernel_frequency 200 --platform $(PLATFORM) -t $(TARGET)  -s -g

AIE_OBJ := ../aie_overlay/libadf.a
XOS     := ../krnl_rgb_mover/krnl_rgb_mover.xo 
XOS     += ../krnl_yuv_mover/krnl_yuv_mover_$(TARGET).xo
XOS     += ../krnl_jpeg/krnl_jpeg_$(TARGET).xo
XSA_OBJ := overlay_$(TARGET).xsa
XCLBIN  := overlay_$(TARGET).xclbin

.phony: clean

all: $(XCLBIN)

$(XCLBIN): $(XSA_OBJ) $(AIE_OBJ)
	v++ -p -t $(TARGET) -f $(PLATFORM) $^ -o $@ --package.boot_mode=ospi

$(XSA_OBJ): $(XOS) $(AIE_OBJ)
	v++ -l $(XOCCLFLAGS) --config xclbin_overlay.cfg -o $@ $^

clean:
	$(RM) -r _x .Xil .ipcache *.ltx *.log *.jou *.info *.xclbin *.xo.* *.str
	
