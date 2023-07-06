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

`timescale 1ns/1ps

module krnl_rgb_mover (
// System Signals
    input             ap_clk,
    input             ap_rst_n,

// AXI4-Lite slave interface
    input             s_axi_control_awvalid,
    output            s_axi_control_awready,
    input   [11:0]    s_axi_control_awaddr ,
    input             s_axi_control_wvalid ,
    output            s_axi_control_wready ,
    input   [31:0]    s_axi_control_wdata  ,
    input   [3:0]     s_axi_control_wstrb  ,
    input             s_axi_control_arvalid,
    output            s_axi_control_arready,
    input   [11:0]    s_axi_control_araddr ,
    output            s_axi_control_rvalid ,
    input             s_axi_control_rready ,
    output  [31:0]    s_axi_control_rdata  ,
    output  [1:0]     s_axi_control_rresp  ,
    output            s_axi_control_bvalid ,
    input             s_axi_control_bready ,
    output  [1:0]     s_axi_control_bresp,
	
// AXI4-Stream (slave) interface axis_slv0
    input             axis_slv_tvalid     ,
    output            axis_slv_tready     ,
    input   [31:0]    axis_slv_tdata      ,
    
// AXI write master interface
    // write channels
    output            axi_wmst_awvalid,
    input             axi_wmst_awready,
    output  [63:0]    axi_wmst_awaddr,
    output  [7:0]     axi_wmst_awlen,
    output            axi_wmst_wvalid,
    input             axi_wmst_wready,
    output  [31:0]    axi_wmst_wdata,
    output  [3:0]     axi_wmst_wstrb,
    output            axi_wmst_wlast,
    input             axi_wmst_bvalid,
    output            axi_wmst_bready,
    // read channels (not used)
    output            axi_wmst_arvalid,
    input             axi_wmst_arready,
    output  [63:0]    axi_wmst_araddr,
    output  [7:0]     axi_wmst_arlen,
    input             axi_wmst_rvalid,
    output            axi_wmst_rready,
    input   [31:0]    axi_wmst_rdata,
    input             axi_wmst_rlast    

);


    wire start;
    wire finish;
    
    wire [15:0] image_width;
    wire [15:0] image_height;
    wire [63:0] image_base_addr;
    	
  axi_ctrl_slave axi_ctrl_slave_inst (
    .ACLK               (ap_clk),
    .ARESETn            (ap_rst_n),

    .AWADDR             (s_axi_control_awaddr),
    .AWVALID            (s_axi_control_awvalid),
    .AWREADY            (s_axi_control_awready),
    .WDATA              (s_axi_control_wdata),
    .WSTRB              (s_axi_control_wstrb),
    .WVALID             (s_axi_control_wvalid),
    .WREADY             (s_axi_control_wready),
    .BRESP              (s_axi_control_bresp),
    .BVALID             (s_axi_control_bvalid),
    .BREADY             (s_axi_control_bready),
    .ARADDR             (s_axi_control_araddr),
    .ARVALID            (s_axi_control_arvalid),
    .ARREADY            (s_axi_control_arready),
    .RDATA              (s_axi_control_rdata),
    .RRESP              (s_axi_control_rresp),
    .RVALID             (s_axi_control_rvalid),
    .RREADY             (s_axi_control_rready),

    .start              (start),
    .finish             (finish),

    .image_width        (image_width),
    .image_height       (image_height),
    .image_base_addr    (image_base_addr)
);


  rgb_s2mm rgb_s2mm_inst (
    .arst_n              (ap_rst_n),
    .aclk                (ap_clk),
	
    .image_width         (image_width),
    .image_height        (image_height),
    .image_base_addr     (image_base_addr),
    .start               (start),
    .finish              (finish),
	
    .axis_slv_tvalid     (axis_slv_tvalid),
    .axis_slv_tready     (axis_slv_tready),
    .axis_slv_tdata      (axis_slv_tdata),
	
    .m_axi_awvalid       (axi_wmst_awvalid),
    .m_axi_awready       (axi_wmst_awready),
    .m_axi_awaddr        (axi_wmst_awaddr),
    .m_axi_awlen         (axi_wmst_awlen),
    .m_axi_wvalid        (axi_wmst_wvalid),
    .m_axi_wready        (axi_wmst_wready),
    .m_axi_wdata         (axi_wmst_wdata),
    .m_axi_wstrb         (axi_wmst_wstrb),
    .m_axi_wlast         (axi_wmst_wlast),
    .m_axi_bvalid        (axi_wmst_bvalid),
    .m_axi_bready        (axi_wmst_bready),
	
    .m_axi_arvalid       (axi_wmst_arvalid),
    .m_axi_arready       (axi_wmst_arready),
    .m_axi_araddr        (axi_wmst_araddr),
    .m_axi_arlen         (axi_wmst_arlen),
    .m_axi_rvalid        (axi_wmst_rvalid),
    .m_axi_rready        (axi_wmst_rready),
    .m_axi_rdata         (axi_wmst_rdata),
    .m_axi_rlast         (axi_wmst_rlast)
);


endmodule
