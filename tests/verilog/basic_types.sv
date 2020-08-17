module basic_types
  ();

  // Structs

  struct packed {
    logic a;
    logic b;
  } packed_struct_1d_notypedef;

  typedef struct packed {
    logic c;
    logic d;
  } packed_struct_t;

  logic [7:0] packed_array_1d;
  logic       unpacked_array_1d [7:0];

  logic [7:0] [3:0] packed_array_2d;
  logic             unpacked_array_2d [7:0] [3:0];

  logic [7:0] packed_unpacked_array_2d [3:0];

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
