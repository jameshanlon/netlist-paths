module foo(input logic i_clk); /* verilator no_inline_module */
endmodule

module top(input logic i_clk);
  foo f(.*);
endmodule
