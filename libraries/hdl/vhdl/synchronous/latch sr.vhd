process(S,R)
begin
   if(R = '1') then
      Q <= '0';
   elsif(S = '1') then
      Q <= '1';
   end if;
end process;
