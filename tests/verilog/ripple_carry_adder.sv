module ripple_carry_adder
  #(parameter p_width = 32)
  ( input  logic               i_clk,
    input  logic               i_rst,
    input  logic [p_width-1:0] i_op1,
    input  logic [p_width-1:0] i_op2,
    output logic               o_co,
    output logic [p_width-1:0] o_sum );

  // Wires.
  logic [p_width-1:0] carry;
  logic [p_width-1:0] sum;
  // Registers.
  logic [p_width-1:0] sum_q;
  logic               co_q;
  // Variables.
  genvar i;

  assign carry[0] = 1'b0;
  assign {o_co, o_sum} = {co_q, sum_q};

  // Named generate block for per-bit continuous assignments.
  for (i = 0; i < p_width; i++) begin
    assign {carry[i+1], sum[i]} = i_op1[i] + i_op2[i] + carry[i];
  end

  always_ff @(posedge i_clk or posedge i_rst)
    if (i_rst) begin
      sum_q <= {p_width{1'b0}};
      co_q  <= 1'b0;
    end else begin
      sum_q <= sum;
      co_q  <= carry[p_width-1];
    end
endmodule
