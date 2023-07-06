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

`define BUFFER_BASE  64'h0000_0040_0001_0000
`define WIDTH        128
`define HEIGHT       64
`define WORD_NUM     `WIDTH * `HEIGHT


import axi_vip_pkg::*;
import axi4stream_vip_pkg::*;
import axi_vip_mst_pkg::*;
import axi_vip_slv_pkg::*;
import axis_vip_mst_pkg::*;

module tb_krnl_rgb_mover ();

// Kernel register address map
// Common Control Register 
parameter KRNL_CTRL_REG_ADDR     = 32'h00000000;
parameter CTRL_START_MASK        = 32'h00000001;
parameter CTRL_DONE_MASK         = 32'h00000002;
parameter CTRL_IDLE_MASK         = 32'h00000004;
parameter CTRL_READY_MASK        = 32'h00000008;
parameter CTRL_CONTINUE_MASK     = 32'h00000010; 

// kernel argument
parameter ARG_IMG_WIDTH       = 32'h0000_0010; 
parameter ARG_IMG_HEIGHT      = 32'h0000_0018; 
parameter ARG_IMG_BADDR0      = 32'h0000_0020; 
parameter ARG_IMG_BADDR1      = 32'h0000_0024; 

// clock frequency definition
parameter real CLK_PERIOD = 3.333; // 300MHz

//System Signals
logic ap_clk = 0;

initial begin: AP_CLK
  forever begin
    ap_clk = #(CLK_PERIOD/2) ~ap_clk;
  end
end
 
//System Signals
logic ap_rst_n = 0;

task automatic ap_rst_n_sequence(input integer unsigned width = 20);
  @(posedge ap_clk);
  #1ns;
  ap_rst_n = 0;
  repeat (width) @(posedge ap_clk);
  #1ns;
  ap_rst_n = 1;
endtask

initial begin: AP_RST
  ap_rst_n_sequence(50);
end


// connnection signal declare
  wire [31:0]   m_axi_awaddr   ;   
  wire          m_axi_awvalid  ;
  wire          m_axi_awready  ;
  wire [31:0]   m_axi_wdata    ;
  wire [3:0]    m_axi_wstrb    ;
  wire          m_axi_wvalid   ;
  wire          m_axi_wready   ;
  wire [1:0]    m_axi_bresp    ;
  wire          m_axi_bvalid   ;
  wire          m_axi_bready   ;
  wire [31:0]   m_axi_araddr   ;
  wire          m_axi_arvalid  ;
  wire          m_axi_arready  ;
  wire [31:0]   m_axi_rdata    ;
  wire [1:0]    m_axi_rresp    ;
  wire          m_axi_rvalid   ;
  wire          m_axi_rready   ;

  wire [63:0]   s_axi_awaddr    ; 
  wire [7:0]    s_axi_awlen     ; 
  wire [1:0]    s_axi_awburst   ; 
  wire          s_axi_awvalid   ; 
  wire          s_axi_awready   ; 
  wire [31:0]   s_axi_wdata     ; 
  wire [3:0]    s_axi_wstrb     ; 
  wire          s_axi_wlast     ; 
  wire          s_axi_wvalid    ; 
  wire          s_axi_wready    ; 
  wire [1:0]    s_axi_bresp     ; 
  wire          s_axi_bvalid    ; 
  wire          s_axi_bready    ; 
  wire [63:0]   s_axi_araddr    ; 
  wire [7:0]    s_axi_arlen     ; 
  wire [1:0]    s_axi_arburst   ; 
  wire          s_axi_arvalid   ; 
  wire          s_axi_arready   ; 
  wire [31:0]   s_axi_rdata     ; 
  wire [1:0]    s_axi_rresp     ; 
  wire          s_axi_rlast     ; 
  wire          s_axi_rvalid    ; 
  wire          s_axi_rready    ;  

  wire          axis_tvalid;
  wire          axis_tready;
  wire [31:0]   axis_tdata;

// instantiation of axis master/slave vips
  axis_vip_mst axis_vip_mst_inst (
    .aclk           (ap_clk),            
    .aresetn        (ap_rst_n),         
    .m_axis_tvalid  (axis_tvalid),       
    .m_axis_tready  (axis_tready),   
    .m_axis_tdata   (axis_tdata)   
);

// instantiation of axi master vip
  axi_vip_mst axi_vip_mst_inst (
    .aclk           (ap_clk),           // input wire aclk
    .aresetn        (ap_rst_n),         // input wire aresetn
    .m_axi_awaddr   (m_axi_awaddr),     // output wire [31 : 0] m_axi_awaddr
    .m_axi_awvalid  (m_axi_awvalid),    // output wire m_axi_awvalid
    .m_axi_awready  (m_axi_awready),    // input wire m_axi_awready
    .m_axi_wdata    (m_axi_wdata),      // output wire [31 : 0] m_axi_wdata
    .m_axi_wstrb    (m_axi_wstrb),      // output wire [3 : 0] m_axi_wstrb
    .m_axi_wvalid   (m_axi_wvalid),     // output wire m_axi_wvalid
    .m_axi_wready   (m_axi_wready),     // input wire m_axi_wready
    .m_axi_bresp    (m_axi_bresp),      // input wire [1 : 0] m_axi_bresp
    .m_axi_bvalid   (m_axi_bvalid),     // input wire m_axi_bvalid
    .m_axi_bready   (m_axi_bready),     // output wire m_axi_bready
    .m_axi_araddr   (m_axi_araddr),     // output wire [31 : 0] m_axi_araddr
    .m_axi_arvalid  (m_axi_arvalid),    // output wire m_axi_arvalid
    .m_axi_arready  (m_axi_arready),    // input wire m_axi_arready
    .m_axi_rdata    (m_axi_rdata),      // input wire [31 : 0] m_axi_rdata
    .m_axi_rresp    (m_axi_rresp),      // input wire [1 : 0] m_axi_rresp
    .m_axi_rvalid   (m_axi_rvalid),     // input wire m_axi_rvalid
    .m_axi_rready   (m_axi_rready)      // output wire m_axi_rready
);

// instantiation of axi slave vip 
  axi_vip_slv axi_vip_slv_inst (
    .aclk           (ap_clk),           // input wire aclk
    .aresetn        (ap_rst_n),         // input wire aresetn
    .s_axi_awaddr   (s_axi_awaddr),     // input wire [63 : 0] s_axi_awaddr
    .s_axi_awlen    (s_axi_awlen),      // input wire [7 : 0] s_axi_awlen
    .s_axi_awburst  (s_axi_awburst),    // input wire [1 : 0] s_axi_awburst
    .s_axi_awvalid  (s_axi_awvalid),    // input wire s_axi_awvalid
    .s_axi_awready  (s_axi_awready),    // output wire s_axi_awready
    .s_axi_wdata    (s_axi_wdata),      // input wire [127 : 0] s_axi_wdata
    .s_axi_wstrb    (s_axi_wstrb),      // input wire [15 : 0] s_axi_wstrb
    .s_axi_wlast    (s_axi_wlast),      // input wire s_axi_wlast
    .s_axi_wvalid   (s_axi_wvalid),     // input wire s_axi_wvalid
    .s_axi_wready   (s_axi_wready),     // output wire s_axi_wready
    .s_axi_bresp    (s_axi_bresp),      // output wire [1 : 0] s_axi_bresp
    .s_axi_bvalid   (s_axi_bvalid),     // output wire s_axi_bvalid
    .s_axi_bready   (s_axi_bready),     // input wire s_axi_bready
    .s_axi_araddr   (s_axi_araddr),     // input wire [63 : 0] s_axi_araddr
    .s_axi_arlen    (s_axi_arlen),      // input wire [7 : 0] s_axi_arlen
    .s_axi_arburst  (s_axi_arburst),    // input wire [1 : 0] s_axi_arburst
    .s_axi_arvalid  (s_axi_arvalid),    // input wire s_axi_arvalid
    .s_axi_arready  (s_axi_arready),    // output wire s_axi_arready
    .s_axi_rdata    (s_axi_rdata),      // output wire [127 : 0] s_axi_rdata
    .s_axi_rresp    (s_axi_rresp),      // output wire [1 : 0] s_axi_rresp
    .s_axi_rlast    (s_axi_rlast),      // output wire s_axi_rlast
    .s_axi_rvalid   (s_axi_rvalid),     // output wire s_axi_rvalid
    .s_axi_rready   (s_axi_rready)      // input wire s_axi_rready
);

  assign s_axi_awburst = 2'b01;
  assign s_axi_arburst = 2'b01;


// instantiation of DUT
  krnl_rgb_mover krnl_rgb_mover_inst (
// System Signals
    .ap_clk                   (ap_clk),
    .ap_rst_n                 (ap_rst_n),
// AXI4-Lite slave interface
    .s_axi_control_awvalid    (m_axi_awvalid),
    .s_axi_control_awready    (m_axi_awready),
    .s_axi_control_awaddr     (m_axi_awaddr),
    .s_axi_control_wvalid     (m_axi_wvalid),
    .s_axi_control_wready     (m_axi_wready),
    .s_axi_control_wdata      (m_axi_wdata),
    .s_axi_control_wstrb      (m_axi_wstrb),
    .s_axi_control_arvalid    (m_axi_arvalid),
    .s_axi_control_arready    (m_axi_arready),
    .s_axi_control_araddr     (m_axi_araddr),
    .s_axi_control_rvalid     (m_axi_rvalid),
    .s_axi_control_rready     (m_axi_rready),
    .s_axi_control_rdata      (m_axi_rdata),
    .s_axi_control_rresp      (m_axi_rresp),
    .s_axi_control_bvalid     (m_axi_bvalid),
    .s_axi_control_bready     (m_axi_bready),
    .s_axi_control_bresp      (m_axi_bresp),
	
// AXI4-Stream (slave) interface axis_slv0
    .axis_slv_tvalid          (axis_tvalid),
    .axis_slv_tready          (axis_tready),
    .axis_slv_tdata           (axis_tdata),
    
// AXI write master interface
    .axi_wmst_awvalid         (s_axi_awvalid),
    .axi_wmst_awready         (s_axi_awready),
    .axi_wmst_awaddr          (s_axi_awaddr),
    .axi_wmst_awlen           (s_axi_awlen),
    .axi_wmst_wvalid          (s_axi_wvalid),
    .axi_wmst_wready          (s_axi_wready),
    .axi_wmst_wdata           (s_axi_wdata),
    .axi_wmst_wstrb           (s_axi_wstrb),
    .axi_wmst_wlast           (s_axi_wlast),
    .axi_wmst_bvalid          (s_axi_bvalid),
    .axi_wmst_bready          (s_axi_bready),
    .axi_wmst_arvalid         (s_axi_arvalid),
    .axi_wmst_arready         (s_axi_arready),
    .axi_wmst_araddr          (s_axi_araddr),
    .axi_wmst_arlen           (s_axi_arlen),
    .axi_wmst_rvalid          (s_axi_rvalid),
    .axi_wmst_rready          (s_axi_rready),
    .axi_wmst_rdata           (s_axi_rdata),
    .axi_wmst_rlast           (s_axi_rlast)

);

  axi_vip_mst_mst_t ctrl;
  axi_vip_slv_slv_mem_t buffer;
  axis_vip_mst_mst_t axis_mst;

`include "tb_krnl_rgb_mover.vh"

initial begin : main_test_routine
        
    bit [31:0] payload[`WORD_NUM];
    bit [31:0] reference[`WORD_NUM];
    bit [31:0] recieved[`WORD_NUM];
    int i, j, k, index;

    for (i = 0; i < `WORD_NUM; i = i + 1) begin
        payload[i] = $random;
    end

    index = 0;
    for (i = 0; i < (`HEIGHT); i = i + 8) begin
      for (j = 0; j < `WIDTH; j++) begin
        for (k = 0; k < 8; k++) begin
          reference[`WIDTH * (i + k) + j] = payload[index];
          index = index + 1;
        end
      end
    end


    #2000
    init_vips();
    
    blocking_write_register (ctrl, ARG_IMG_WIDTH, `WIDTH);
    blocking_write_register (ctrl, ARG_IMG_HEIGHT, `HEIGHT);
    blocking_write_register (ctrl, ARG_IMG_BADDR0, `BUFFER_BASE & 32'hffff_ffff);
    blocking_write_register (ctrl, ARG_IMG_BADDR1, `BUFFER_BASE >> 32);
    
    poll_start_register (ctrl);
    set_start_register (ctrl);

    start_stream_traffic(axis_mst, `WORD_NUM, payload);
    
    poll_done_register (ctrl);
    
    buffer_dump_memory(buffer, `BUFFER_BASE, recieved, 0, `WORD_NUM);

    if (words_compare(reference, recieved, `WORD_NUM)) begin
      $write("%c[1;32m",27);
      $display($time,, "      [CHECK] Data check SUCCEED!");
      $write("%c[0m",27); 
    end else begin
      $write("%c[1;31m",27);
      $display($time,, "      [CHECK] Data check FAIL!");
      $write("%c[0m",27); 
    end   

    #1000 $finish;

end




// Waveform dump
`ifdef DUMP_WAVEFORM
  initial begin
    $dumpfile("tb_krnl_rgb_mover.vcd");
    $dumpvars(0,tb_krnl_rgb_mover);
  end
`endif


endmodule
