process (CLK, RST)
begin
   if(RST = '1') then
      -- Reset asignments
      ...
   elsif(CLK'event and CLK = '1') then
      -- Concurrent asignments
      ...
   end if;

   -- Concurrent asignments
   ...

end process;
