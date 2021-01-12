interface bus #(parameter p_width=32) (input logic i_clk);
  logic               valid;
  logic [p_width-1:0] data;
endinterface

module producer(input logic i_clk, bus bus_if);
endmodule

module consumer(input logic i_clk, bus bus_if);
endmodule

module top(input logic i_clk);
  bus bus_if(i_clk);
  producer p(.*);
  consumer c(.*);
endmodule
