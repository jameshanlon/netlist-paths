module foo(input logic i_clk); /* verilator public_module */
endmodule

module public_module_no_inline(input logic i_clk);
  foo f(.*);
endmodule
