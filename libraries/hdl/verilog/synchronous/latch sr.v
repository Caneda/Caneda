always @ (S or R) begin
   if (R) begin
      Q <= 0;
   end
   else if (S) begin
      Q <= 1'b1;
   end
end
