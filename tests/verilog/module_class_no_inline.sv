class foo;
  int x;
 
  task set(int i);
    x = i;
  endtask
 
  function int get();
    return x;
  endfunction

endclass

module module_class_no_inline(input logic i_clk);
  foo f;
  initial begin
    f = new();
    f.set(1);
  end
endmodule
