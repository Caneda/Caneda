// filename is the file with the component description
// name is the name of the component to instantiate
// u_name can be any name for the instance
`include "filename.v"

module example();

  // Component instantiation 
  name u_name ( A, B, OUT, ... );

endmodule // testbench
