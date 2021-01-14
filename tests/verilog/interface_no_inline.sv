interface bus #(parameter p_width=32) (input logic i_clk);
  logic               valid;
  logic [p_width-1:0] data;
endinterface

module producer(input logic i_clk, bus bus_if);
  assign bus_if.valid = 0;
  assign bus_if.data = 0;
endmodule

module consumer #(parameter p_width=32) (input logic i_clk, bus bus_if);
  logic valid_q;
  logic [p_width-1:0] data_q;
  always_ff @(posedge i_clk) begin
    valid_q <= bus_if.valid;
    data_q <= bus_if.data;
  end
endmodule

module top(input logic i_clk);
  bus bus_if(i_clk);
  producer p(.*);
  consumer c(.*);
endmodule
