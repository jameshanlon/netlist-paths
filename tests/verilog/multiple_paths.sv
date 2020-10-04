module multiple_paths
  (
    input logic in,
    output logic out
  );

  logic a;
  logic b;
  logic c;

  always_comb begin
    a = in;
    b = in;
    c = in;
  end

  assign out = a | b | c;

endmodule
