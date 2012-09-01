process(CLK)
begin
   if(CLK = '1' and CLK'event) then
      if EN = '1' then
         Q <= D;
      end if;
   end if;
end process;
