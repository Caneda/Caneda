module shiftRegister ( clk, d, q );

   // Constants
   parameter bitWidth = 8;

   // Inputs and outputs
   input  d, clk;
   output q;

   // Data types
   reg q;
   reg [bitWidth-1:0] m_reg;

   always @ (posedge clk) begin
     {q, m_reg} = {m_reg, d};
   end

endmodule
