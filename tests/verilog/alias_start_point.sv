module sub_module (
  output logic out
);
  assign out = 1;
endmodule

module alias_start_point (
  input logic i_clk
);

  logic x, y;
  logic y_q;

  sub_module u_a(x);

  assign y = x;

  always_ff @(posedge i_clk)
    y_q <= y;

endmodule
