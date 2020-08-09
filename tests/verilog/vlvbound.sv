module vlvbound_test
  (
    input logic [31:0]  i_foo_current,
    input logic [31:0]  i_foo_next,
    output logic [6:0]  o_foo_inactive,
    output logic [6:0]  o_next_foo_inactive
  );

  typedef logic [6:0] foo_active_t;

  function  automatic foo_active_t rd_sts_inactive(input logic [31:0] foo);
    foo_active_t ret;
    integer i;
    integer tmp;
    for (i=0 ; i < 7; i++) begin
      tmp    = foo >> (i*2);
      ret[i] = (tmp[1:0] == 2'b00) ;
    end
    return ret;
  endfunction

  assign o_foo_inactive       = rd_sts_inactive(i_foo_current);
  assign o_next_foo_inactive  = rd_sts_inactive(i_foo_next);

endmodule
