module basic_assign_chain
  (
    input logic in,
    output logic out
  );
  logic a;
  logic b;
  // Simple chain of dependencies.
  assign a = in;
  assign b = a;
  assign out = b;
endmodule
