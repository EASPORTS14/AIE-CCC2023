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


##################################### Step 1: create vivado project and add design sources

# create ip project with part name in command line argvs
create_project krnl_rgb_mover ./krnl_rgb_mover -part [lindex $argv 0]

# add design sources into project
add_files -norecurse \
       {                                    \
        ../rtl/axi_ctrl_slave.v             \
        ../rtl/krnl_rgb_mover.v             \
        ../rtl/rgb_s2mm.v                   \
       }

update_compile_order -fileset sources_1

# create IP packaging project
ipx::package_project -root_dir ./krnl_rgb_mover_ip -vendor xilinx.com -library user -taxonomy /UserIP -import_files -set_current true


##################################### Step 2: Inference clock, reset, AXI interfaces and associate them with clock

# inference clock and reset signals
ipx::infer_bus_interface ap_clk xilinx.com:signal:clock_rtl:1.0 [ipx::current_core]
ipx::infer_bus_interface ap_rst_n xilinx.com:signal:reset_rtl:1.0 [ipx::current_core]

# associate AXI/AXIS interface with clock
ipx::associate_bus_interfaces -busif s_axi_control  -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif axis_slv       -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif axi_wmst       -clock ap_clk [ipx::current_core]

# associate reset signal with clock
ipx::associate_bus_interfaces -clock ap_clk -reset ap_rst_n [ipx::current_core]


##################################### Step 3: Set the definition of AXI control slave registers, including CTRL and user kernel arguments

# Add RTL kernel registers
ipx::add_register CTRL      [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]
ipx::add_register IMG_W     [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]
ipx::add_register IMG_H     [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]
ipx::add_register BUFF_ADDR [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]

# Set RTL kernel registers property
set_property description    {Control Signals}   [ipx::get_registers CTRL    -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]
set_property address_offset {0x000}             [ipx::get_registers CTRL    -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]
set_property size           {32}                [ipx::get_registers CTRL    -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]

set_property description    {Image width   }    [ipx::get_registers IMG_W   -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]
set_property address_offset {0x010}             [ipx::get_registers IMG_W   -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]
set_property size           {32}                [ipx::get_registers IMG_W   -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]

set_property description    {Image height  }    [ipx::get_registers IMG_H   -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]
set_property address_offset {0x018}             [ipx::get_registers IMG_H   -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]
set_property size           {32}                [ipx::get_registers IMG_H   -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]

set_property description    {buffer addr   }    [ipx::get_registers BUFF_ADDR  -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]
set_property address_offset {0x020}             [ipx::get_registers BUFF_ADDR  -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]
set_property size           {64}                [ipx::get_registers BUFF_ADDR  -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]


##################################### Step 4: associate AXI master port to pointer argument and set data width
ipx::add_register_parameter ASSOCIATED_BUSIF [ipx::get_registers BUFF_ADDR -of_objects [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]]
set_property value          {axi_wmst}          [ipx::get_register_parameters ASSOCIATED_BUSIF     \
                                    -of_objects [ipx::get_registers BUFF_ADDR                      \
                                    -of_objects [ipx::get_address_blocks reg0                      \
                                    -of_objects [ipx::get_memory_maps s_axi_control                 \
                                    -of_objects [ipx::current_core]]]]]

ipx::add_bus_parameter DATA_WIDTH [ipx::get_bus_interfaces axi_wmst -of_objects [ipx::current_core]]
set_property value          {32} [ipx::get_bus_parameters DATA_WIDTH -of_objects [ipx::get_bus_interfaces axi_wmst -of_objects [ipx::current_core]]]

#### Step 5: Package Vivado IP and generate Vitis kernel file

# Set required property for Vitis kernel
set_property sdx_kernel true [ipx::current_core]
set_property sdx_kernel_type rtl [ipx::current_core]

# Packaging Vivado IP
ipx::update_source_project_archive -component [ipx::current_core]
ipx::save_core [ipx::current_core]

# Generate Vitis Kernel from Vivado IP
package_xo -force -xo_path ../krnl_rgb_mover.xo -kernel_name krnl_rgb_mover -ctrl_protocol ap_ctrl_hs -ip_directory ./krnl_rgb_mover_ip -output_kernel_xml ../krnl_rgb_mover.xml
