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
  logic [3:0] logic_unpackarray_vector [2:0];
  logic [4:0] [3:0] logic_unpackarray2d_vector [2:0] [1:0];
  logic [212934:123421] logic_array_large [213412:112312];

  // Structs

  struct packed {
    logic a;
    logic b;
  } packstruct_notypedef;

  typedef struct packed {
    logic c;
    logic d;
  } packed_struct_t;

  packed_struct_t packstruct;
  packed_struct_t [7:0] packstruct_packarray;
  packed_struct_t [7:0] [6:0] packstruct_packarray2d;
  packed_struct_t packstruct_unpackarray [7:0];
  packed_struct_t packstruct_unpackarray2d [7:0] [6:0];
  packed_struct_t [7:0] packstruct_packarray_unpackarray [6:0];
  packed_struct_t [7:0] [6:0] packstruct_packarray2d_unpackarray2d [5:0] [4:0];

  typedef struct packed {
    packed_struct_t e;
    logic f;
  } packed_struct_nested_t;

  typedef struct packed {
    packed_struct_nested_t g;
    logic h;
  } packed_struct_nested2_t;

  typedef struct packed {
    packed_struct_nested_t g;
    packed_struct_nested2_t h;
    logic i;
    logic j;
    logic k;
  } packed_struct_nested3_t;

  packed_struct_nested_t packstruct_nested;
  packed_struct_nested2_t packstruct_nested2;
  packed_struct_nested3_t packstruct_nested3;

  // Enums

  enum logic {
    FOO_A,
    FOO_B
  } enum_notypedef;

  typedef enum logic [1:0] {
    BAR_A,
    BAR_B,
    BAR_C,
    BAR_D
  } enum_auto_t;

  enum_auto_t enum_auto;

  typedef enum logic [2:0] {
    BAZ_A = 3'b001,
    BAZ_B = 3'b010,
    BAZ_C = 3'b100
  } enum_onehot_t;

  enum_onehot_t enum_onehot;
  enum_onehot_t [5:0] [4:0] enum_onehot_packarray2d_unpackarray2d [3:0] [1:0];

  // Unions

  union packed {
    logic a;
    logic b;
  } union_logic_notypedef;

  union packed {
    packed_struct_t a;
    packed_struct_nested_t b;
  } union_struct_notypedef;

  typedef union packed {
    logic a;
    logic b;
    logic c;
  } union_logic_t;

  union_logic_t [4:0] [3:0] union_logic_packarray2d_unpackarray2d [2:0] [1:0];

  union packed {
    enum_onehot_t a;
    packed_struct_nested_t b;
  } union_enum_struct;

endmodule
