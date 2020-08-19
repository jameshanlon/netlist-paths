module dtypes
  ();

  // Logic

  logic logic_bit;
  logic [3:0] logic_vector;
  logic [3:0] [2:0] logic_packarray_vector;
  logic [3:0] [2:0] [1:0] logic_packarray2d_vector;
  logic logic_unpackarray [3:0];
  logic logic_unpackarray2d [3:0] [2:0];
  logic logic_unpackarray3d [3:0] [2:0] [1:0];
  logic [3:0] logic_unpackarray_vector [1:0];
  logic [3:0] logic_unpackarray2d_vector [1:0] [2:0];

  logic [2129:1234] [81:74] logic_array_large [2134:1123] [74:23];

  // Structs

  struct packed {
    logic a;
    logic b;
  } packed_struct_1d_notypedef;

  typedef struct packed {
    logic c;
    logic d;
  } packed_struct_t;

  packed_struct_t        packed_struct_1d;
  packed_struct_t [7:0]  packed_struct_array_1d;
  packed_struct_t        unpacked_struct_array_1d [7:0];

  packed_struct_t [7:0] packed_unpacked_struct_array_2d [3:0];

  typedef struct packed {
    packed_struct_t e;
    logic f;
  } packed_struct_nested_t;

  typedef struct packed {
    packed_struct_nested_t g;
    logic h;
  } packed_struct_twice_nested_t;

  packed_struct_nested_t packed_struct_nested;
  packed_struct_twice_nested_t packed_struct_twice_nested;

  // Enums

  enum logic [2:0] {
    FOO_A = 3'b001,
    FOO_B = 3'b010,
    FOO_C = 3'b100
  } enum_no_typedef;

  typedef enum logic [2:0] {
    BAR_A,
    BAR_B,
    BAR_C
  } enum_auto_t;

  enum_auto_t enum_auto;

  typedef enum logic [2:0] {
    BAZ_A = 3'b001,
    BAZ_B = 3'b010,
    BAZ_C = 3'b100
  } enum_onehot_t;

  enum_onehot_t enum_onehot;

  // Unions

  union packed {
    logic a;
    logic b;
  } union_of_logics_no_typedef;

  union packed {
    packed_struct_t a;
    packed_struct_nested_t b;
  } union_of_structs_no_typedef;

endmodule
