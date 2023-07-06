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

module rgb_s2mm (
    
    input           aclk,
    input           arst_n,

    // control signals
    input   [15:0]  image_width,
    input   [15:0]  image_height,
    input   [63:0]  image_base_addr,
    input           start,              // one cycle pulse to trigger the transfer of a frame
    output  reg     finish,             // one cycle pulse to signify the transfer finish


    // axi stream slave interface
    input           axis_slv_tvalid,
    output          axis_slv_tready,
    input   [31:0]  axis_slv_tdata,  

    // axi mm master
    // write channels
    output          m_axi_awvalid,
    input           m_axi_awready,
    output  [63:0]  m_axi_awaddr ,
    output  [7:0]   m_axi_awlen  ,
    output          m_axi_wvalid ,
    input           m_axi_wready ,
    output  [31:0]  m_axi_wdata  ,
    output  [3:0]   m_axi_wstrb  ,
    output          m_axi_wlast  ,
    input           m_axi_bvalid ,
    output          m_axi_bready ,
    // read channel
    output          m_axi_arvalid,
    input           m_axi_arready,
    output  [63:0]  m_axi_araddr,
    output  [7:0]   m_axi_arlen,
    input           m_axi_rvalid,
    output          m_axi_rready,
    input   [31:0]  m_axi_rdata,
    input           m_axi_rlast
);

localparam 
// axi master state
    ST_MST_IDLE     = 3'd0,
    ST_MST_WAIT     = 3'd1,
    ST_MST_WADDR    = 3'd2,
    ST_MST_WDATA    = 3'd3,
    ST_MST_WRESP    = 3'd4;
    
    reg         i_awvalid;
    reg  [63:0] i_awaddr;

    reg  [15:0]  col_cnt;
    reg  [15:0]  next_col_cnt;

    reg  [15:0]  row_cnt;
    reg  [15:0]  next_row_cnt;

    reg  [31:0]  pix_cnt;
    reg  [31:0]  next_pix_cnt;

    reg  [2:0]  mst_state;
    reg  [2:0]  mst_state_nxt;

    wire [31:0] total_pix_num;

    wire [63:0] pix_offset;

    assign total_pix_num = image_width * image_height;

// AXI master write channel assignment
    assign m_axi_awlen      = 8'b0;
    assign m_axi_wstrb      = 4'b1111;
    assign m_axi_bready     = 1'b1;
    assign m_axi_awcache    = 4'b1111;
    assign m_axi_awprot     = 3'b000;
    assign m_axi_wlast      = m_axi_wvalid;
    assign m_axi_awsize     = 3'b010;
    assign m_axi_awvalid    = i_awvalid;
    assign m_axi_awaddr     = i_awaddr;

// AXI master read channel assignment
    assign m_axi_arvalid    = 1'b0;
    assign m_axi_araddr     = 64'b0;
    assign m_axi_arlen      = 8'b0;
    assign m_axi_rready     = 1'b1;


// sync fifo instantiation    
    wire        fifo_wr_en;
    wire [31:0] fifo_din;
    wire        fifo_rd_en;
    wire [31:0] fifo_dout;
    wire        fifo_data_valid;
    
    wire        fifo_empty;
    wire        fifo_full;
    wire        fifo_overflow;  
    wire        fifo_underflow;

    wire [11:0] wr_data_cnt;
    wire [11:0] rd_data_cnt;
    
    assign fifo_wr_en = (!fifo_full) & axis_slv_tvalid;
    assign fifo_din   = axis_slv_tdata;
    assign fifo_rd_en = m_axi_bvalid;
    
    assign m_axi_wdata  = fifo_dout;
    assign m_axi_wvalid = fifo_data_valid & (mst_state == ST_MST_WDATA);

    assign axis_slv_tready = !fifo_full;

    xpm_fifo_sync # (
        .FIFO_MEMORY_TYPE    ( "auto"   ) , // string; "auto", "block", "distributed", or "ultra";
        .ECC_MODE            ( "no_ecc" ) , // string; "no_ecc" or "en_ecc";
        .SIM_ASSERT_CHK      ( 0        ) ,
        .CASCADE_HEIGHT      ( 0        ) , 
        .FIFO_WRITE_DEPTH    ( 2048     ) , // positive integer
        .WRITE_DATA_WIDTH    ( 32       ) , // positive integer
        .WR_DATA_COUNT_WIDTH ( 12       ) , // positive integer
        .FULL_RESET_VALUE    ( 0        ) , // positive integer; 0 or 1
        .USE_ADV_FEATURES    ( "1F1F"   ) , // string; "0000" to "1F1F";
        .READ_MODE           ( "fwft"   ) , // string; "std" or "fwft";
        .FIFO_READ_LATENCY   ( 1        ) , // positive integer;
        .READ_DATA_WIDTH     ( 32       ) , // positive integer
        .RD_DATA_COUNT_WIDTH ( 12       ) , // positive integer
        .DOUT_RESET_VALUE    ( "0"      ) , // string, don't care
        .WAKEUP_TIME         ( 0        ) // positive integer; 0 or 2;
    )
    inst_rd_xpm_fifo_sync (
        .sleep         ( 1'b0           ) ,
        .rst           ( !arst_n        ) ,
        .wr_clk        ( aclk           ) ,
        .wr_en         ( fifo_wr_en     ) ,
        .din           ( fifo_din       ) ,
        .full          ( fifo_full      ) ,
        .overflow      ( fifo_overflow  ) ,
        .prog_full     (                ) ,
        .wr_data_count ( wr_data_cnt    ) ,
        .almost_full   (                ) ,
        .wr_ack        (                ) ,
        .wr_rst_busy   (                ) ,
        .rd_en         ( fifo_rd_en     ) ,
        .dout          ( fifo_dout      ) ,
        .empty         ( fifo_empty     ) ,
        .prog_empty    (                ) ,
        .rd_data_count ( rd_data_cnt    ) ,
        .almost_empty  (                ) ,
        .data_valid    ( fifo_data_valid) ,
        .underflow     ( fifo_underflow ) ,
        .rd_rst_busy   (                ) ,
        .injectsbiterr ( 1'b0           ) ,
        .injectdbiterr ( 1'b0           ) ,
        .sbiterr       (                ) ,
        .dbiterr       (                )
    ) ;

// state machine control
    always @ (posedge aclk or negedge arst_n) begin
        if (!arst_n) begin
            mst_state <= ST_MST_IDLE;
            col_cnt   <= 'h0;
            row_cnt   <= 'h0;
            pix_cnt   <= 'h0;
        end else begin
            mst_state <= mst_state_nxt;
            col_cnt   <= next_col_cnt;
            row_cnt   <= next_row_cnt;
            pix_cnt   <= next_pix_cnt;
        end
    end

    always @ (*) begin
        mst_state_nxt = mst_state;
        next_col_cnt = col_cnt;
        next_row_cnt = row_cnt;
        next_pix_cnt = pix_cnt;
        finish = 0;

        case (mst_state)
            ST_MST_IDLE : begin
                if (start) begin
                    mst_state_nxt = ST_MST_WAIT;
                end
            end

            ST_MST_WAIT : begin
                if (!fifo_empty) begin
                    mst_state_nxt = ST_MST_WADDR;
                end
            end

            ST_MST_WADDR : begin
                if (m_axi_awready & m_axi_awvalid) begin
                    mst_state_nxt = ST_MST_WDATA;
                end
            end

            ST_MST_WDATA : begin
                if (m_axi_wready & m_axi_wvalid) begin
                    mst_state_nxt = ST_MST_WRESP;
                end
            end

            ST_MST_WRESP : begin
                if (m_axi_bvalid) begin
                    if ((total_pix_num - 1) == pix_cnt) begin
                        mst_state_nxt = ST_MST_IDLE;
                        next_pix_cnt = 0;
                        next_col_cnt = 0;
                        next_row_cnt = 0;
                        finish = 1;
                    end else begin
                        mst_state_nxt = ST_MST_WAIT;
                        next_pix_cnt = pix_cnt + 1;
                        if ((row_cnt[2:0] == 3'd7) && (col_cnt == (image_width - 1))) begin
                            next_col_cnt = 0;
                            next_row_cnt = row_cnt + 1;
                        end else if (row_cnt[2:0] == 3'd7) begin
                            next_row_cnt = row_cnt - 7;
                            next_col_cnt = col_cnt + 1;
                        end else begin
                            next_row_cnt = row_cnt + 1;
                        end
                    end
                end
            end
        endcase
    end

    assign pix_offset = row_cnt * image_width + col_cnt ; 

    always @ (posedge aclk or negedge arst_n) begin
        if (!arst_n) begin 
            i_awvalid <= 1'b0;
            i_awaddr  <= 64'b0;
        end else if (mst_state_nxt == ST_MST_WADDR) begin
            i_awvalid <= 1'b1;
            i_awaddr  <= image_base_addr + pix_offset * 4;
        end else begin
            i_awvalid <= 1'b0;
        end
    end



endmodule
