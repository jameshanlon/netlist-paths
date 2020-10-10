module foo
  (
    input logic in,
    output logic out
  );

  logic x;

  always_ff @* begin
    x <= in;
  end

  assign out = x;

endmodule

module fan_out_in_modules
  (
    input logic in,
    output logic out
  );

  logic a;
  logic b;
  logic c;

  foo foo_a (
    .in(in),
    .out(a));

  foo foo_b (
    .in(in),
    .out(b));

  foo foo_c (
    .in(in),
    .out(c));

  assign out = a | b | c;

endmodule

