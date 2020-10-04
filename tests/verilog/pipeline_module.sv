// A single pipeline stage.
module pipestage
  #(
    parameter p_width = 32
  )(
    input logic  i_clk,
    input logic  i_rst,
    input logic  [p_width-1:0] i_data,
    output logic [p_width-1:0] o_data
  );

  logic [p_width-1:0] data_q;

  always_ff @(posedge i_clk or posedge i_rst)
    if (i_rst) begin
      data_q <= 'b0;
    end else begin
      data_q <= i_data;
    end

  assign o_data = data_q;

endmodule

// Pipeline a data signal over a number of stages.
module pipeline_module
  #(
    parameter p_width = 32,
              p_stages = 8
  )(
    input  logic i_clk,
    input  logic i_rst,
    input  logic [p_width-1:0] i_data,
    output logic [p_width-1:0] o_data
  );

  logic [p_width-1:0] routing [p_stages:0];

  generate
    for (genvar i=0; i<p_stages; i++) begin : g_pipestage
      pipestage
      u_pipestage(.*, 
        .i_data(routing[i]),
        .o_data(routing[i+1]));
    end
  endgenerate

  assign routing[0] = i_data;
  assign o_data = routing[p_stages];

endmodule

