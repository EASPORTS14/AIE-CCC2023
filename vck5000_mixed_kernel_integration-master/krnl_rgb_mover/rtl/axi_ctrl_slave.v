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
module axi_ctrl_slave (
    input           ACLK,
    input           ARESETn,
    // AXI signals
    input   [11:0]  AWADDR,
    input           AWVALID,
    output          AWREADY,
    input   [31:0]  WDATA,
    input   [3:0]   WSTRB,
    input           WVALID,
    output          WREADY,
    output  [1:0]   BRESP,
    output          BVALID,
    input           BREADY,
    input   [11:0]  ARADDR,
    input           ARVALID,
    output          ARREADY,
    output  [31:0]  RDATA,
    output  [1:0]   RRESP,
    output          RVALID,
    input           RREADY,
    // ap_ctrl_hs signals
    output reg      start,
    input           finish,
    // control register signals
    output  [15:0]  image_width,
    output  [15:0]  image_height,
    output  [63:0]  image_base_addr
);

//------------------------Register Address Map-------------------
// 0x000 : CTRL
// 0x010 : image_width
// 0x018 : image_height
// 0x020 : image_base_addr[31:0]
// 0x024 : image_base_addr[63:32]


//------------------------Parameter----------------------
localparam
    // register address map
    ADDR_CTRL       = 12'h000, 
    ADDR_IMG_WIDTH  = 12'h010, 
    ADDR_IMG_HEIGHT = 12'h018, 
    ADDR_IMG_BADDR0 = 12'h020, 
    ADDR_IMG_BADDR1 = 12'h024,
    
    // registers write state machine
    WRIDLE          = 2'd0,
    WRDATA          = 2'd1,
    WRRESP          = 2'd2,
    WRRESET         = 2'd3,
    
    // registers read state machine
    RDIDLE          = 2'd0,
    RDDATA          = 2'd1,
    RDRESET         = 2'd2;

//------------------------Signal Declaration----------------------
    // axi operation
    reg  [1:0]      wstate;
    reg  [1:0]      wnext;
    reg  [11:0]     waddr;
    wire [31:0]     wmask;
    wire            aw_hs;
    wire            w_hs;
    reg  [1:0]      rstate;
    reg  [1:0]      rnext;
    reg  [31:0]     rdata;
    wire            ar_hs;
    wire [11:0]     raddr;
    
    // control register bit
    reg             reg_ctrl_ap_idle;
    reg             reg_ctrl_ap_done;
    wire            reg_ctrl_ap_ready;      // copy version of ap_done
    reg             reg_ctrl_ap_start;
    reg             reg_ctrl_ap_continue;   // useless in ap_ctrl_hs mode
    
    reg  [31:0]     reg_img_width;  
    reg  [31:0]     reg_img_height;
    reg  [63:0]     reg_img_base_addr;        

    assign reg_ctrl_ap_ready = reg_ctrl_ap_done;

//------------------------AXI protocol control------------------

    //------------------------AXI write fsm------------------
    assign AWREADY = (wstate == WRIDLE);
    assign WREADY  = (wstate == WRDATA);
    assign BRESP   = 2'b00;  // OKAY
    assign BVALID  = (wstate == WRRESP);
    assign wmask   = { {8{WSTRB[3]}}, {8{WSTRB[2]}}, {8{WSTRB[1]}}, {8{WSTRB[0]}} };
    assign aw_hs   = AWVALID & AWREADY;
    assign w_hs    = WVALID & WREADY;

    // wstate
    always @(posedge ACLK) begin
        if (!ARESETn)
            wstate <= WRRESET;
        else
            wstate <= wnext;
    end
    
    // wnext
    always @(*) begin
        case (wstate)
            WRIDLE:
                if (AWVALID)
                    wnext = WRDATA;
                else
                    wnext = WRIDLE;
            WRDATA:
                if (WVALID)
                    wnext = WRRESP;
                else
                    wnext = WRDATA;
            WRRESP:
                if (BREADY)
                    wnext = WRIDLE;
                else
                    wnext = WRRESP;
            default:
                wnext = WRIDLE;
        endcase
    end
    
    // waddr
    always @(posedge ACLK) begin
        if (aw_hs)
            waddr <= AWADDR;
    end
    
    //------------------------AXI read fsm-------------------
    assign ARREADY = (rstate == RDIDLE);
    assign RDATA   = rdata;
    assign RRESP   = 2'b00;  // OKAY
    assign RVALID  = (rstate == RDDATA);
    assign ar_hs   = ARVALID & ARREADY;
    assign raddr   = ARADDR;
    
    // rstate
    always @(posedge ACLK) begin
        if (!ARESETn)
            rstate <= RDRESET;
        else
            rstate <= rnext;
    end
    
    // rnext
    always @(*) begin
        case (rstate)
            RDIDLE:
                if (ARVALID)
                    rnext = RDDATA;
                else
                    rnext = RDIDLE;
            RDDATA:
                if (RREADY & RVALID)
                    rnext = RDIDLE;
                else
                    rnext = RDDATA;
            default:
                rnext = RDIDLE;
        endcase
    end
    
    // rdata
    always @(posedge ACLK) begin
        if (ar_hs) begin
            case (raddr)
                ADDR_CTRL: begin
                    rdata[0] <= reg_ctrl_ap_start;
                    rdata[1] <= reg_ctrl_ap_done;
                    rdata[2] <= reg_ctrl_ap_idle;
                    rdata[3] <= reg_ctrl_ap_ready;  
                    rdata[4] <= reg_ctrl_ap_continue;
                    rdata[31:5] <= 'h0;
                end
                // --------------------------------------
                ADDR_IMG_WIDTH: begin
                    rdata <= reg_img_width;
                end
                // --------------------------------------
                ADDR_IMG_HEIGHT: begin
                    rdata <= reg_img_height;
                end              
                // --------------------------------------
                ADDR_IMG_BADDR0: begin
                    rdata <= reg_img_base_addr[31:0];
                end
                // --------------------------------------
                ADDR_IMG_BADDR1: begin
                    rdata <= reg_img_base_addr[63:32];
                end
            endcase
        end
    end
    

// Control register update operation

    // reg_ctrl_ap_start
    always @(posedge ACLK) begin
        if (!ARESETn)
            reg_ctrl_ap_start <= 1'b0;
        else
            if (w_hs && waddr == ADDR_CTRL && WSTRB[0] && WDATA[0])
                reg_ctrl_ap_start <= 1'b1;
            else if (reg_ctrl_ap_ready)
                reg_ctrl_ap_start <= 1'b0; // clear when ap_ready asserted
    end
    
    // reg_ctrl_ap_done
    always @(posedge ACLK) begin
        if (!ARESETn)
            reg_ctrl_ap_done <= 1'b0;
        else
            if (finish)
                reg_ctrl_ap_done <= 1'b1;
            else if (ar_hs && raddr == ADDR_CTRL)
                reg_ctrl_ap_done <= 1'b0; // clear on read
    end
    
    // reg_ctrl_ap_idle
    always @(posedge ACLK) begin
        if (!ARESETn)
            reg_ctrl_ap_idle <= 1'b1;
        else if (!reg_ctrl_ap_idle && finish)
            reg_ctrl_ap_idle <= 1'b1;
        else if (reg_ctrl_ap_start)
            reg_ctrl_ap_idle <= 1'b0;
    end
    
    // reg_ctrl_ap_continue
    always @(posedge ACLK) begin
        if (!ARESETn)
            reg_ctrl_ap_continue <= 1'b0;
        else
            if (w_hs && waddr == ADDR_CTRL && WSTRB[0] && WDATA[4])
                reg_ctrl_ap_continue <= 1'b1;
            else
                reg_ctrl_ap_continue <= 1'b0; // self clear
    end
    
    // start generation
    always @(posedge ACLK) begin
        if (!ARESETn)
            start <= 1'b0;
        else
            if (w_hs && waddr == ADDR_CTRL && WSTRB[0] && WDATA[0])
                start <= 1'b1;
            else 
                start <= 1'b0;
    end

    always @(posedge ACLK) begin
        if (!ARESETn)
            reg_img_width <= 'h0;
        else
            if (w_hs && waddr == ADDR_IMG_WIDTH)
                reg_img_width <= (WDATA & wmask) | (reg_img_width & ~wmask);
    end
    
    always @(posedge ACLK) begin
        if (!ARESETn)
            reg_img_height <= 'h0;
        else
            if (w_hs && waddr == ADDR_IMG_HEIGHT)
                reg_img_height <= (WDATA & wmask) | (reg_img_height & ~wmask);
    end

    always @(posedge ACLK) begin
        if (!ARESETn)
            reg_img_base_addr[31:0] <= 'h0;
        else
            if (w_hs && waddr == ADDR_IMG_BADDR0)
                reg_img_base_addr[31:0] <= (WDATA & wmask) | (reg_img_base_addr[31:0] & ~wmask);
    end

    always @(posedge ACLK) begin
        if (!ARESETn)
            reg_img_base_addr[63:32] <= 'h0;
        else
            if (w_hs && waddr == ADDR_IMG_BADDR1)
                reg_img_base_addr[63:32] <= (WDATA & wmask) | (reg_img_base_addr[63:32] & ~wmask);
    end
    

//------------------------Control registers output-----------------
    assign ap_start     = reg_ctrl_ap_start;
    assign image_height = reg_img_height[15:0];
    assign image_width  = reg_img_width[15:0];
    assign image_base_addr = reg_img_base_addr;

endmodule
