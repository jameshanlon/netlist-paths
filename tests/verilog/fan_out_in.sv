module fan_out_in
  (
    input logic in,
    output logic out
  );

  logic a;
  logic b;
  logic c;

  always_ff @* begin
    a <= in;
    b <= in;
    c <= in;
  end

  assign out = a | b | c;

endmodule
