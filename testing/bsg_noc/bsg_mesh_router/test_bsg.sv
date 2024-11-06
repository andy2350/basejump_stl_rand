/*verilator coverage_off*/

//`define DATA_WIDTH_P 4
//`define MESH_EDGE_P  2 // tests a 2^(MESH_EDGE_P) x 2^(MESH_EDGE_P) mesh network

// Need to reverse the field declaration of the C struct
typedef struct packed {
  bit valid;
  int data;
  int dest_y;
  int dest_x;
} Message;

import "DPI-C" function void get_message(input int i, input int j, output bit[$bits(Message)-1:0] m);
import "DPI-C" function void notify_on_receive(input int i, input int j, input bit [(`DATA_WIDTH_P)+(`MESH_EDGE_P)*2-1:0] rcv_data);
import "DPI-C" function void check_consume(input int i, input int j, output bit yumi);
import "DPI-C" function void notify_on_send(input int i, input int j);
import "DPI-C" function void reset_fn();

/************************** TEST RATIONALE *********************************
* 1. STATE SPACE
*
*   A n x n, where n is a power of 2, mesh network of UUTs is instantiated 
*   with fifos bridging them. Test data,(tile no.) ^ 0 1 2 .... (total no. 
*   of tiles) is continuously fed through proc fifos of each router.
*   Effectively, proc of every router sends data to proc of every other router
*   in the network making the network very congested.
* 
* 2. PARAMETERIZATION
*
*   DATA_WIDTH_P is the width of test data excluding the embedded coordinates.
*   MESH_EDGE_P is log(edge length of the mesh). A reasonable set tests would
*   be MESH_EDGE_P = 0 1 2 with sufficient DATA_WIDTH_P.
* **************************************************************************/

// import enum Dirs for directions
import bsg_noc_pkg::Dirs
       , bsg_noc_pkg::P  // proc (processor core)
       , bsg_noc_pkg::W  // west
       , bsg_noc_pkg::E  // east
       , bsg_noc_pkg::N  // north
       , bsg_noc_pkg::S; // south


module test_bsg
#(
  parameter cycle_time_p = 20,
  parameter reset_cycles_lo_p=1,
  parameter reset_cycles_hi_p=5
)(
  input wire clk
);

  // clock and reset generation
  wire reset;
    
  bsg_nonsynth_reset_gen #(  .num_clocks_p     (1)
                           , .reset_cycles_lo_p(reset_cycles_lo_p)
                           , .reset_cycles_hi_p(reset_cycles_hi_p)
                          )  reset_gen
                          (  .clk_i        (clk) 
                           , .async_reset_o(reset)
                          ); 

  localparam medge_lp     = 2**(`MESH_EDGE_P);         // edge length of the mesh 
  localparam msize_lp     = (medge_lp)**2;             // area (total no. of routers)
  localparam x_cord_width_lp = `BSG_SAFE_CLOG2(medge_lp); // width of x coordinate
  localparam y_cord_width_lp = `BSG_SAFE_CLOG2(medge_lp); // width of y coordinate
  
  // data width including embedded coordinates
  localparam width_lp = (`DATA_WIDTH_P) + x_cord_width_lp + y_cord_width_lp;
  
  localparam dims_lp = 2;
  localparam dirs_lp = (dims_lp*2)+1;
  
  initial
  begin
    $display("\n\n\n");
    $display("===========================================================");
    $display("testing  %0d x %0d network with...", medge_lp, medge_lp);
    $display("DATA_WIDTH = %0d\n", (`DATA_WIDTH_P));
  end

  
  // I/O signal of UUTs

  logic [msize_lp-1:0][dirs_lp-1:0] test_input_valid;
  logic [msize_lp-1:0][dirs_lp-1:0] test_input_ready;
  logic [msize_lp-1:0][dirs_lp-1:0][width_lp-1:0] test_input_data;

  logic [dirs_lp-1:0] test_output_yumi[msize_lp-1:0];
  logic [msize_lp-1:0][dirs_lp-1:0] test_output_valid;
  logic [msize_lp-1:0][dirs_lp-1:0][width_lp-1:0] test_output_data;
  
  logic [width_lp-1:0] proc_output_data [msize_lp-1:0];
  logic proc_output_valid[msize_lp-1:0];
  logic proc_output_yumi[msize_lp-1:0];
  
  /*******************************************************
  * Instantiation of medge_lp x medge_lp mesh network
  * -- medge_lp is a power of 2
  ********************************************************/ 
  
  genvar i, j; 
  
  for(i=0; i<msize_lp; i=i+1)
    bsg_mesh_router #( .dims_p     (dims_lp)
                      ,.width_p    (width_lp)
                      ,.x_cord_width_p(x_cord_width_lp)
                      ,.y_cord_width_p(y_cord_width_lp)
                     ) uut
                     ( .clk_i  (clk)
                      ,.reset_i(reset)
                      
                      ,.data_i (test_input_data[i])
                      ,.v_i    (test_input_valid[i])
                      ,.yumi_o (test_output_yumi[i])

                      ,.data_o (test_output_data[i])
                      ,.v_o(test_output_valid[i])
                      ,.ready_and_i(test_input_ready[i])

                      ,.my_x_i(x_cord_width_lp'(i%medge_lp))
                      ,.my_y_i(y_cord_width_lp'(i/medge_lp))
                     );

  // disables the peripheral ports of the mesh
  for(i=0; i<msize_lp; i=i+1)
  begin
    if(i/medge_lp == 0)
      begin
        assign test_input_valid[i][N] = 1'b0;
        assign test_input_ready[i][N] = 1'b0;
      end

    if(i/medge_lp == medge_lp-1) 
      begin
        assign test_input_valid[i][S] = 1'b0;
        assign test_input_ready[i][S] = 1'b0;
      end

    if(i%medge_lp == 0) 
      begin
        assign test_input_valid[i][W] = 1'b0;
        assign test_input_ready[i][W] = 1'b0;
      end

    if(i%medge_lp == medge_lp-1) 
      begin
        assign test_input_valid[i][E] = 1'b0;
        assign test_input_ready[i][E] = 1'b0;
      end
  end


  
  /*********************************************
  * Instantiation of fifos bridging the routers
  /*********************************************/
  
  // vertical fifos => data flow N to S or vice-versa
  for(i=0; i<((medge_lp)*(medge_lp-1)); i=i+1)
  begin
    bsg_fifo_1r1w_small #( .width_p(width_lp)
                          ,.els_p  (`FIFO_SIZE_P)
                          ,.ready_THEN_valid_p(0)
                         ) fifo_up // north to south data flow
                         ( .clk_i  (clk)
                          ,.reset_i(reset)
                          
                          ,.data_i (test_output_data[i][S])
                          ,.v_i    (test_output_valid[i][S])
                          ,.ready_param_o(test_input_ready[i][S])

                          ,.data_o(test_input_data[i+medge_lp][N])
                          ,.v_o   (test_input_valid[i+medge_lp][N])
                          ,.yumi_i(test_output_yumi[i+medge_lp][N])
                         );

    bsg_fifo_1r1w_small #( .width_p(width_lp)
                          ,.els_p  (`FIFO_SIZE_P)
                          ,.ready_THEN_valid_p(0)
                         ) fifo_down // south to north data flow
                         ( .clk_i  (clk)
                          ,.reset_i(reset)
                          
                          ,.data_i (test_output_data[i+medge_lp][N])
                          ,.v_i    (test_output_valid[i+medge_lp][N])
                          ,.ready_param_o(test_input_ready[i+medge_lp][N])

                          ,.data_o(test_input_data[i][S])
                          ,.v_o   (test_input_valid[i][S])
                          ,.yumi_i(test_output_yumi[i][S])
                         );
  end

  // horizontal fifos => data flow E to W or vice versa
  for(i=0; i<medge_lp; i=i+1)
  begin
    for(j=0; j<medge_lp-1; j=j+1)
    begin
      bsg_fifo_1r1w_small #( .width_p(width_lp)
                            ,.els_p  (`FIFO_SIZE_P)
                            ,.ready_THEN_valid_p(0)
                           ) fifo_right // west to east data flow
                           ( .clk_i  (clk)
                            ,.reset_i(reset)
                            
                            ,.data_i (test_output_data[i*medge_lp + j][E])
                            ,.v_i    (test_output_valid[i*medge_lp + j][E])
                            ,.ready_param_o(test_input_ready[i*medge_lp + j][E])

                            ,.data_o(test_input_data[i*medge_lp+j+1][W])
                            ,.v_o   (test_input_valid[i*medge_lp+j+1][W])
                            ,.yumi_i(test_output_yumi[i*medge_lp+j+1][W])
                           );
  
      bsg_fifo_1r1w_small #( .width_p(width_lp)
                            ,.els_p  (`FIFO_SIZE_P)
                            ,.ready_THEN_valid_p(0)
                           ) fifo_left // east to west data flow
                           ( .clk_i  (clk)
                            ,.reset_i(reset)
                            
                            ,.data_i (test_output_data[i*medge_lp+j+1][W])
                            ,.v_i    (test_output_valid[i*medge_lp+j+1][W])
                            ,.ready_param_o(test_input_ready[i*medge_lp+j+1][W])

                            ,.data_o(test_input_data[i*medge_lp+j][E])
                            ,.v_o   (test_input_valid[i*medge_lp+j][E])
                            ,.yumi_i(test_output_yumi[i*medge_lp+j][E])
                           );
    end
  end


  // proc fifo
  
  // actual test data;
  // fed through input proc fifo
  logic [msize_lp-1:0][width_lp-1:0] test_stim_data_in;
  logic [msize_lp-1:0] test_stim_valid_in;
  logic [msize_lp-1:0] test_stim_ready_out;

  for(i=0; i<msize_lp; i=i+1)
  begin
    bsg_fifo_1r1w_small #( .width_p(width_lp)
                          ,.els_p  (`FIFO_SIZE_P)
                          ,.ready_THEN_valid_p(0)
                         ) fifo_proc_in
                         ( .clk_i  (clk)
                          ,.reset_i(reset)
                          
                          ,.data_i (test_stim_data_in[i])
                          ,.v_i    (test_stim_valid_in[i])
                          ,.ready_param_o(test_stim_ready_out[i])

                          ,.data_o(test_input_data[i][P])
                          ,.v_o   (test_input_valid[i][P])
                          ,.yumi_i(test_output_yumi[i][P])
                         );

    bsg_fifo_1r1w_small #( .width_p(width_lp)
                          ,.els_p  (`FIFO_SIZE_P)
                          ,.ready_THEN_valid_p(0)
                         ) fifo_proc_out
                         ( .clk_i  (clk)
                          ,.reset_i(reset)
                          
                          ,.data_i (test_output_data[i][P])
                          ,.v_i    (test_output_valid[i][P])
                          ,.ready_param_o(test_input_ready[i][P])

                          ,.data_o(proc_output_data[i])
                          ,.v_o   (proc_output_valid[i])
                          ,.yumi_i(proc_output_yumi[i])
                         );
  end

  /**************************************************
  * Test Stimuli
  ***************************************************/

  bit [$bits(Message)-1:0] msg_bits[medge_lp-1:0][medge_lp-1:0];
  Message msg[medge_lp-1:0][medge_lp-1:0];

  always_ff @(posedge clk)
  begin
    if (reset)
      reset_fn();
  end
  
  for(i=0; i<medge_lp; i=i+1)
  begin
    for(j=0;j<medge_lp; j=j+1)
    begin
      int coord = i * medge_lp + j;
      assign msg[i][j] = Message'(msg_bits[i][j]);
      // assign test_input_ready[coord][P] = 1'b1;
      assign test_stim_data_in[coord] = { msg[i][j].data[`DATA_WIDTH_P-1:0],
                                      msg[i][j].dest_x[x_cord_width_lp-1:0], 
                                      msg[i][j].dest_y[y_cord_width_lp-1:0]};
      assign test_stim_valid_in[coord] = 1'(msg[i][j].valid);

      always_ff @(posedge clk)
      begin
        if (reset)
        begin
            msg_bits[i][j] = 0;
        end
        else
        begin
            // Note the order of notify_on_send() and get_message()
            // Since I cannot make verilator to put notify_on_send() to the end of a cycle
            if (!reset && test_stim_valid_in[coord] && test_stim_ready_out[coord])
            begin
              // $display("GOOD");
              notify_on_send(i, j);
            end
            get_message(i, j, msg_bits[i][j]);
            // $display("%b", msg_bits[i][j]);
        end
      end
    end
  end

  /**************************************************
  * Verification
  ***************************************************/

  for(i=0;i<medge_lp;i+=1)
  begin
    for(j=0;j<medge_lp;j+=1)
    begin
      int coord = i*medge_lp+j;
      // assign proc_output_yumi[coord] = proc_output_valid[coord];
      always_comb 
      begin
        if (!reset && proc_output_valid[coord])
          check_consume(i, j, proc_output_yumi[coord]);
        else
          proc_output_yumi[coord] = 1'b0;
      end

      // Note: The proc_output_data comes in the next cycle after check_consume returns 1
      // because of the scheduling of verilator 
      // `notify_on_receive` should actually be named as `receive`
      always_ff @(posedge clk)
      begin
        if(!reset & proc_output_valid[coord])
        begin
          notify_on_receive(i, j, proc_output_data[coord]);
        end
      end
    end
  end
endmodule
