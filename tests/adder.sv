module adder 
  #(
    parameter p_width = 32
  )(
    input  logic [p_width-1:0] i_a,
    input  logic [p_width-1:0] i_b,
    output logic [p_width-1:0] o_sum,
    output logic               o_co
  );

  assign {o_co, o_sum} = i_a + i_b;

endmodule
