module include_b(
                 input [1:0] data_i,
                 input       select_i,
                 output      data_o
                 );

   assign data_o = (select_i == 1'b1) ? data_i[1] : data_i[0];

endmodule
