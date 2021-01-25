module foo(input logic i_clk,
           input logic [31:0] i_a,
           output logic [31:0] o_b);
  always_ff @(posedge i_clk)
    o_b <= i_a;
endmodule

module registered_output_nested(input logic i_clk,
                                input logic [31:0] i_a,
                                output logic [31:0] o_b);
  foo u_foo(i_clk, i_a, o_b);
endmodule
