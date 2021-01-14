package adder_pkg;
  parameter WIDTH = 32;
  // This function causes the package not to be removed as dead code.
  function logic get_value(logic a, logic b);
     return a+ b;
  endfunction : get_value
endpackage

module adder
  #(parameter p_width = adder_pkg::WIDTH)(
    input  logic [p_width-1:0] i_a,
    input  logic [p_width-1:0] i_b,
    output logic [p_width-1:0] o_sum,
    output logic               o_co
  );

  assign {o_co, o_sum} = i_a + i_b;

endmodule

module orphan_package
  // Wrapper not really necessary.
  #(parameter p_width = adder_pkg::WIDTH)(
    input  logic [p_width-1:0] i_a,
    input  logic [p_width-1:0] i_b,
    output logic [p_width-1:0] o_sum,
    output logic               o_co
  );

  adder u_adder(.*);

endmodule
