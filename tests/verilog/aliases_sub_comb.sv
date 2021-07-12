module sub_comb (
  input  logic clk,
  input  logic in,
  output logic client_out,
  input  logic client_in,
  output logic out
);
  assign client_out = in;
  assign out = client_in;
endmodule

module aliases_sub_comb (
  input logic i_clk
);
  logic a__to__client_a, client_a__to__a, a__to__b;
  logic b__to__client_b, client_b__to__b, b__to__a;
  sub_comb u_a(.clk(i_clk), .in(b__to__a), .client_out(), .client_in(), .out(a__to__b));
  sub_comb u_b(.clk(i_clk), .in(a__to__b), .client_out(), .client_in(), .out(b__to__a));
endmodule
