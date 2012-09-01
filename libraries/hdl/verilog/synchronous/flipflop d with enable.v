always @ ( posedge clk ) begin
   if (en) begin
      Q <= D;
   end
end
