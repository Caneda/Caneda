process(CLK, RST)
begin
   if(RST = '1') then
      Q <= '0';
   elsif(CLK = '1' and CLK'event) then
      if(EN = '1') then
         Q <= D;
      end if;
   end if;
end process;
