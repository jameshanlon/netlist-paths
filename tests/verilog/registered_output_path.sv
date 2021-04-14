
module foo(input logic i_clk,
           input logic [31:0] i_a,
           output logic [31:0] o_b);
  // Register the output.
  always_ff @(posedge i_clk)
    o_b <= i_a;
endmodule

module registered_output_path(input logic i_clk,
                              input logic [31:0] in,
                              output logic [31:0] out);
  logic [31:0] data1;
  logic [31:0] data2;
  // Paths in (port) -> data1 (o_b reg) -> data2 (o_b reg) -> out (port).
  foo u_foo1(i_clk, in, data1);
  foo u_foo2(i_clk, data1, data2);
  foo u_foo3(i_clk, data2, out);
endmodule
