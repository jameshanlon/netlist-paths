module basic_ff_chain
  (
    input logic in,
    output logic out
  );
  logic a;
  logic b;
  // Simple chain of dependencies.
  always_ff @* begin
    a <= in;
    b <= a;
    out = b;
  end
endmodule


