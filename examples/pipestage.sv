  module pipestage
    #(parameter p_width=32)
    (
      input  logic               i_clk,
      input  logic               i_rst,
      input  logic [p_width-1:0] in,
      output logic [p_width-1:0] out
    );
    logic [p_width-1:0] data_q;
    always_ff @(posedge i_clk or posedge i_rst)
      if (i_rst) begin
        data_q <= '0;
      end else begin
        data_q <= in;
      end
    assign out = data_q;
  endmodule
