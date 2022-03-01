`include "include_a.sv"

module single_include(
               input data_i,
               output data_o
               );

   include_a i_include_a(
                         .data_i(data_i),
                         .data_o(data_o)
                         );
endmodule
