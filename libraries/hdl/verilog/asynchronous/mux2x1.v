// a, b are the inputs
// sel is the input selection
// out is the output
module mux2x1 ( a, b, sel, out );

   // Inputs and outputs
   input  a, b, sel;
   output out;

   // Data types
   wire a, b, sel;
   reg  out;

   always @ (a or b or sel) begin
      if (sel) begin
         out = a;
      end
      else begin
         out = b;
      end
   end

endmodule
