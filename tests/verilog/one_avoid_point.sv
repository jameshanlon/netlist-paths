module one_avoid_point(
  input logic i_a,
  output logic o_a
);

  logic foo;

  assign foo = i_a;
  assign o_a = foo;

endmodule
