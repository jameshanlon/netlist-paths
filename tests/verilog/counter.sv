module counter
  #(parameter width=32)
  (
    input  logic             i_clk,
    input  logic             i_rst,
    output logic [width-1:0] o_count,
    output logic             o_wrap
  );

  logic [width-1:0] counter_q;
  logic [width-1:0] counter;
  logic             co;

  assign {co, counter} = counter_q + 1;
  assign o_count = counter_q;
  assign o_wrap = co;

  always_ff @(posedge i_clk or posedge i_rst)
    if (i_rst)
      counter_q <= 0;
    else
      counter_q <= counter;

endmodule
