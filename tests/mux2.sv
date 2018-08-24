module mux2
  #(parameter p_width = 8)
  (
    input  logic [p_width-1:0] i_op_a,
    input  logic [p_width-1:0] i_op_b,
    input                      i_sel,
    output logic [p_width-1:0] o_res
  );

  always_comb begin
    unique case (i_sel)
    1'b0:    o_res = i_op_a;
    1'b1:    o_res = i_op_b;
    default: o_res = {p_width{1'bx}}
    endcase
  end

endmodule
