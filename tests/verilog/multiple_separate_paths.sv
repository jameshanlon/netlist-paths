module multiple_separate_paths
  (
    input logic i_a,
    input logic i_b,
    input logic i_c,
    output logic o_a,
    output logic o_b,
    output logic o_c
  );

  logic a;
  logic b;
  logic c;

  assign a = i_a;
  assign b = i_b;
  assign c = i_c;

  assign o_a = a;
  assign o_b = b;
  assign o_c = c;

endmodule
