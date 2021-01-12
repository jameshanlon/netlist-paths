module foo(input logic i_clk); /* verilator public_module */
endmodule

module top(input logic i_clk);
  foo f(.*);
endmodule
