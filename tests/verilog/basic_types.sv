module basic_types
  ();

  typedef struct packed {
    logic a;
    logic b;
  } packed_struct_t;

  logic [7:0] packed_array_1d;
  logic       unpacked_array_1d [7:0];

  //assign packed_array_1d[0] = packed_array_1d[7];
  //assign unpacked_array_1d[0] = unpacked_array_1d[7];

  logic [7:0] [3:0] packed_array_2d;
  logic             unpacked_array_2d [7:0] [3:0];

  //assign packed_array_2d[0][1] = packed_array_2d[6][3];
  //assign unpacked_array_2d[0][1] = unpacked_array_2d[6][3];

  logic [7:0] packed_unpacked_array_2d [3:0];

  packed_struct_t       packed_struct_1d;
  packed_struct_t [7:0] packed_struct_array_1d;
  packed_struct_t       unpacked_struct_array_1d [7:0];

  packed_struct_t [7:0] packed_unpacked_struct_array_2d [3:0];

endmodule
