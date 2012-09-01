module counter ( clk, reset, en, out );

   // Constants
   parameter bitWidth = 8;  // Counter bit width
   parameter finalValue = 255;  // Counter final value

   // Inputs and outputs
   input  en, clk, reset;
   output [bitWidth-1:0] out;

   // Data types
   wire   en, clk, reset;
   reg   [bitWidth-1:0] out;

   always @ ( posedge clk ) begin
      if (reset == 1'b1 || out == finalValue) begin
	       out <= 0;
      end
      else if (en == 1'b1) begin
      	 out <= out + 1;
      end
   end

endmodule // counter
