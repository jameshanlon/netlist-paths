module foo(input logic i_clk); /* verilator no_inline_module */
endmodule

module module_no_inline_pragma(input logic i_clk);
  foo f(.*);
endmodule
