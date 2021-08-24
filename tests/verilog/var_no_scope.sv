package bar;

  typedef enum logic [1:0] {
    READ        = 0,
    READ_MARGIN = 1,
    WRITE       = 2,
    IDLE        = 3
  } bar_cmd;

  typedef logic [$clog2(128)-1:0] cntr_t;

  localparam cntr_t LATENCY_A = 3-1;
  localparam cntr_t LATENCY_B = 6-1;
  localparam cntr_t LATENCY_C = 65-1;

  localparam cntr_t LATENCIES [4] =
    '{LATENCY_A,
      LATENCY_B,
      LATENCY_C,
      cntr_t'(0)};

  function automatic cntr_t latency (input bar_cmd cmd);
    latency = LATENCIES[cmd];
  endfunction : latency

endpackage

module var_no_scope(output logic [6:0] out);
  assign out = bar::latency(bar::IDLE);
endmodule
