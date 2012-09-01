always @ ( posedge clk or posedge reset ) begin
   if (reset) begin
      Q <= 0;
   end
   else if (en) begin
      Q <= D;
   end
end
