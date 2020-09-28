// Pipeline a data signal over a number of stages.
module pipeline_loops
  #(
    parameter p_width = 32,
              p_stages = 8
  )(
    input  logic i_clk,
    input  logic i_rst,
    input  logic [p_width-1:0] i_data,
    output logic [p_width-1:0] o_data
  );

  logic [p_width-1:0] data_q [p_stages-1:0];

  int i;

  always_ff @(posedge i_clk or posedge i_rst)
    if (i_rst) begin
      for (i=0; i<p_stages; i++) begin
        data_q[i] <= 'b0;
      end
    end else begin
      for (i=0; i<p_stages; i++) begin
        if (i == 0) begin
          data_q[0] <= i_data;
        end else begin
          data_q[i] <= data_q[i - 1];
        end
      end
    end

  assign o_data = data_q[p_stages - 1];

endmodule
