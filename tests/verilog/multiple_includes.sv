`include "include_a.sv"
`include "include_b.sv"

module multiple_includes(
               input data_i,
               output data_o
               );

   wire data_s;

   include_a i_include_a(
                         .data_i(data_i),
                         .data_o(data_s)
                         );
   include_b i_include_b(
                         .data_i({data_i, data_s}),
                         .select_i(1'b0),
                         .data_o(data_o)
                         );

endmodule
