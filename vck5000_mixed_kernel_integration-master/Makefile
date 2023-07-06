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

TARGET := hw
PLATFORM := xilinx_vck5000_gen4x8_qdma_2_202220_1

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make build_hw [TARGET=hw_emu]"
	$(ECHO) ""
	$(ECHO) "  make build_sw"
	$(ECHO) ""
	$(ECHO) "  make clean"
	$(ECHO) ""

# Build hareware (xclbin) objects
build_hw: compile_aie compile_krnl_jpeg compile_krnl_yuv_mover compile_krnl_rgb_mover hw_link

compile_aie:
	make -C ./aie_overlay aie_compile

compile_krnl_jpeg:
	make -C ./krnl_jpeg compile TARGET=$(TARGET) PLATFORM=$(PLATFORM)

compile_krnl_yuv_mover:
	make -C ./krnl_yuv_mover compile TARGET=$(TARGET) PLATFORM=$(PLATFORM)

compile_krnl_rgb_mover:
	make -C ./krnl_rgb_mover pack_kernel

hw_link:
	make -C ./hw all TARGET=$(TARGET) PLATFORM=$(PLATFORM)

# Build software object
build_sw: 
	make -C ./sw all

# Clean objects
clean: clean_aie clean_krnl_jpeg clean_krnl_yuv_mover clean_krnl_rgb_mover clean_hw clean_sw

clean_aie:
	make -C ./aie_overlay clean

clean_krnl_jpeg:
	make -C ./krnl_jpeg clean

clean_krnl_yuv_mover:
	make -C ./krnl_yuv_mover clean

clean_krnl_rgb_mover:
	make -C ./krnl_rgb_mover clean

clean_hw:
	make -C ./hw clean

clean_sw: 
	make -C ./sw clean
