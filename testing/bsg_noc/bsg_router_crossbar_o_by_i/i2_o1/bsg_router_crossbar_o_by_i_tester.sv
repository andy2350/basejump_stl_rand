/*verilator coverage_off*/
//
// Paul Gao 06/2019
//
//

`timescale 1ps/1ps
`include "bsg_noc_links.svh"

class item;
  rand logic proc_v;
  rand logic [10-1:0] proc_data;

  // 添加约束，确保生成合理的数据
  constraint data_c {
    proc_data inside {[0:2**10-1]};
    proc_v inside {0, 1};
  }
endclass

module bsg_router_crossbar_o_by_i_tester();
   localparam nodes_lp = 8;
   localparam i_els_lp = 2;
   localparam o_els_lp = 1;
   localparam flit_width_lp = 10;

   genvar i;
   logic [3:0] clk_i;
   logic reset_i;

   wire [nodes_lp:0] v_li;
   wire [nodes_lp:0][flit_width_lp-1:0] data_li;
   wire [nodes_lp:0] ready_lo;

   logic [nodes_lp-1:0] proc_v_lo;
   wire [nodes_lp-1:0][flit_width_lp-1:0] proc_data_lo;
   wire [nodes_lp-1:0] proc_ready_li;

   item it[nodes_lp];
   
   assign v_li[0]    = 1'b0;
   assign data_li[0] = 'X;

   for (i = 0; i < nodes_lp; i++) begin: node
      bsg_router_crossbar_o_by_i
         #(.i_els_p(i_els_lp)
           ,.o_els_p(o_els_lp)
           ,.i_width_p(flit_width_lp)
         ) xbar (
           .clk_i              (clk_i[0])
           ,.reset_i           (reset_i)
           ,.valid_i           ({ proc_v_lo   [i],     v_li[i] })
           ,.data_i            ({ proc_data_lo[i],  data_li[i] })
           ,.credit_ready_and_o({ proc_ready_li[i], ready_lo[i] })
           ,.valid_o           (v_li[i+1])
           ,.data_o            (data_li[i+1])
           ,.ready_and_i       (ready_lo[i+1])
         );

      always_ff @(negedge clk_i[0]) begin
         if (!reset_i) begin
            // 在每个时钟周期随机化数据
            it[i] = new();
            if (!it[i].randomize()) begin
               $error("Randomization failed at node %0d", i);
               $finish;
            end
            proc_v_lo[i] <= it[i].proc_v;
            proc_data_lo[i] <= it[i].proc_data;
         end
      end
   end
   
   assign ready_lo[nodes_lp] = 1'b1;

   integer j;
   always @(negedge clk_i[0]) begin
      for (j = 0; j < nodes_lp; j++) begin
         if (v_li[j+1]) 
            $write("%1x ", data_li[j+1]);
         else 
            $write("- ");
      end
      $display(" CYCLE");
   end

   always #10 clk_i = clk_i + 1;

   initial begin
      reset_i = 1;
      clk_i = 0;
      $display("Start Simulation\n");

      #25;
      reset_i = 0;

      #5000;
      $finish;     
   end
endmodule
