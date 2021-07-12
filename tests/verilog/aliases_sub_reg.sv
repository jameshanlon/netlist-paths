module sub_reg (
  input  logic clk,
  input  logic in,
  output logic client_out,
  input  logic client_in,
  output logic out
);
  always_ff @(posedge clk) begin
     client_out <= in;
     out <= client_in;
  end
endmodule

module aliases_sub_reg (
  input logic i_clk
);
  logic a__to__client_a, client_a__to__a, a__to__b;
  logic b__to__client_b, client_b__to__b, b__to__a;
  sub_reg u_a(.clk(i_clk), .in(b__to__a), .client_out(), .client_in(), .out(a__to__b));
  sub_reg u_b(.clk(i_clk), .in(a__to__b), .client_out(), .client_in(), .out(b__to__a));
endmodule
