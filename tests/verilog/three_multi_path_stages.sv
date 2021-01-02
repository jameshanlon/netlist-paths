module three_multi_path_stages
  (
    input logic in,
    output logic out
  );

  logic a1, a2, a3;
  logic all_a;
  logic b1, b2, b3;
  logic all_b;
  logic c1, c2, c3;

  always_comb begin
    a1 = in;
    a2 = in;
    a3 = in;
  end

  assign all_a = a1 | a2 | a3;

  always_comb begin
    b1 = all_a;
    b2 = all_a;
    b3 = all_a;
  end

  assign all_b = b1 | b2 | b3;

  always_comb begin
    c1 = all_b;
    c2 = all_b;
    c3 = all_b;
  end

  assign out = c1 | c2 | c3;

endmodule
