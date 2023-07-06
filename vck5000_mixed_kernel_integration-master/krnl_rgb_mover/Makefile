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
	$(ECHO) "  make gen_vip"
	$(ECHO) "      Command to generate the IPs used in this design"
	$(ECHO) ""
	$(ECHO) "  make pack_kernel"
	$(ECHO) "      Command to pack the module krnl_aes to Vitis kernel"
	$(ECHO) ""
	$(ECHO) "  make runsim"
	$(ECHO) "      Command to run the simulation"
	$(ECHO) ""
	$(ECHO) "  make clean"
	$(ECHO) "      Command to remove all the generated files."

PART := xcvc1902-vsvd1760-2MP-e-S
PLATFORM := xilinx_vck5000_gen4x8_xdma_2_202210_1

# TARGET: set the build target, can be hw or hw_emu
TARGET := hw


.phony: clean

################## resource generation and simulation 
gen_vip:
	rm -rf ip_generation; mkdir ip_generation; vivado -mode batch -source ./gen_vip.tcl -tclargs $(PART)


pack_kernel: krnl_rgb_mover.xo

krnl_rgb_mover.xo: rtl/*
	rm -rf vivado_pack_krnl_project; mkdir vivado_pack_krnl_project; cd vivado_pack_krnl_project; vivado -mode batch -source ../pack_kernel.tcl -tclargs $(PART)

runsim:
	./runsim_krnl_rgb_mover_xsim.sh

################## clean up
clean:
	$(RM) -rf ip_generation vivado_pack_krnl_project
	$(RM) -rf *.xo *.xclbin *.xclbin.info *.xclbin.link_summary *.jou *.log *.xo.compile_summary _x
	$(RM) -rf *.dat *.pb xsim.dir *.xml *.ltx *.csv *.json *.protoinst *.wdb *.wcfg host_krnl_aes_test
