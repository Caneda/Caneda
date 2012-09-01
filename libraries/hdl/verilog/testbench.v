// filename is the file with the component description
`include "filename.v"

module testbench();
  // Declaration of inputs and outputs in reverse order as originally defined in the component
  reg A, B;
  wire [7:0] OUT;
  ...

  // Component instantiation 
  // name is the name of the component to test
  name u0 ( A, B, OUT, ... );

  // Clock definition
  always begin
    #5 clock = ~clock;
  end

  // Set the inputs
  initial begin        
    A = 1;
    B = 0;
    ...
    
    #10 A = 1;
    #10 A = 0;
    ...
    
    #5 $finish;  // Finish the simulation
  end

  // Define the output file and the output variables
  initial begin
    $dumpfile("results.vcd");
    $dumpvars(0, testbench);
  end

endmodule // testbench
