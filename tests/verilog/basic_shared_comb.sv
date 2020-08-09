module basic_shared_comb
  (
    input logic ain,
    input logic bin,
    output logic aout,
    output logic bout
  );
  // No depdendency should exist between aout and bin, and bout and ain.
  always_comb begin
    aout = ain;
    bout = bin;
  end
endmodule
