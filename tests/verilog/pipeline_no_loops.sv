// Pipeline a data signal over a number of stages.
module three_stage_pipeline
  #(
    parameter p_width = 32
  )(
    input  logic i_clk,
    input  logic i_rst,
    input  logic [p_width-1:0] i_data,
    output logic [p_width-1:0] o_data
  );

  logic [p_width-1:0] data_q [2:0];

  always_ff @(posedge i_clk or posedge i_rst)
    if (i_rst) begin
      data_q[0] <= 'b0;
      data_q[1] <= 'b0;
      data_q[2] <= 'b0;
    end else begin
      data_q[0] <= i_data;
      data_q[1] <= data_q[0];
      data_q[2] <= data_q[1];
    end

  assign o_data = data_q[2];

endmodule
