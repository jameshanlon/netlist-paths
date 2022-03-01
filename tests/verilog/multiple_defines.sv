module multiple_defines(
               input data_i,
               output data_o
               );
   wire data_s;

   assign data_s = `EXPR_A;
   `ifdef MY_DEFINE
       assign `EXPR_B = data_s;
   `endif

endmodule
