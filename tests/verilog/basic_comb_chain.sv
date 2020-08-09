module basic_comb_chain
  (
    input logic in,
    output logic out
  );
  logic a;
  logic b;
  // Simple chain of dependencies.
  always_comb begin
    a = in;
  end
  always_comb begin
    b = a;
  end
  always_comb begin
    out = b;
  end
endmodule

