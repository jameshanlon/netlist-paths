module registered_output(input logic i_clk,
                         input logic [31:0] i_a,
                         output logic [31:0] o_b);
  always_ff @(posedge i_clk)
    o_b <= i_a;
endmodule
