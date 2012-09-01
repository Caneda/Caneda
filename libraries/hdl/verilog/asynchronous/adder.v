// a, b and cin (carry) are inputs
// out is the sum output, cout is the carry out
module adder ( out, cout, a, b, cin );

   // Inputs and outputs
   input  a, b, cin;
   output out, cout;

   // Data types
   reg out, cout;

   always @ (a or b or cin) begin
      {cout, out} = a + b + cin;
   end
  
endmodule
