module basic_types
  ();

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

  enum logic [2:0] {
    FOO_A = 3'b001,
    FOO_B = 3'b010,
    FOO_C = 3'b100
  } enum_foo_notypedef;

  typedef enum logic [2:0] {
    BAR_A = 3'b001,
    BAR_B = 3'b010,
    BAR_C = 3'b100
  } bar_enum_t;

  bar_enum_t enum_bar;

endmodule
