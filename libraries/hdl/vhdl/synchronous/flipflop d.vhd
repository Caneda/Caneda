process(CLK)
begin
   if(CLK = '1' and CLK'event) then
      Q <= D;
   end if;
end process;
