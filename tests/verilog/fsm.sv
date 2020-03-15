module fsm
  (
    input  logic       i_clk,
    input  logic       i_rst,
    input  logic       i_start,
    input  logic       i_finish,
    input  logic       i_wait,
    output logic [2:0] o_state
  );

  typedef enum logic [2:0] {
    IDLE = 3'b001,
    BUSY = 3'b010,
    WAIT = 3'b100
  } state_enum_t;

  // One-hot encoding of states.
  typedef union packed {
    state_enum_t states;
    struct packed {
      logic waiting;
      logic busy;
      logic idle;
    } state;
  } state_t;

  // Registers.
  state_t state_q;

  // Wires.
  state_t next_state;

  always_comb begin
    next_state = state_q;
    unique case (1'b1)
      state_q.state.idle,
      state_q.state.waiting:
	if (i_start)
	  next_state = BUSY;
      state_q.state.busy: begin
	if (i_wait)
	  next_state = WAIT;
	if (i_finish)
	  next_state = IDLE;
      end
    endcase
  end

  always_ff @(posedge i_clk or posedge i_rst)
    if (i_rst)
      state_q <= IDLE;
    else
      state_q <= next_state;

  assign o_state = state_q;

endmodule
