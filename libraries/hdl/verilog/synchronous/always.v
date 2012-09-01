always @ ( posedge clk ) begin
   if (rst == 1'b1 || CONDITION) begin
      // Initialize registers
      ...
   end
   else if (CONDITION) begin
      // Concurrent assignments
      ...
   end
end
