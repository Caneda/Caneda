always @ ( posedge clk or posedge reset ) begin
   if (reset) begin
      Q <= 0;
   end
   else begin
      Q <= D;
   end
end
