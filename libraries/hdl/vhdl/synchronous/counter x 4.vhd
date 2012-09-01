library IEEE;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

ENTITY counter IS
   PORT (rst:    IN std_logic;
         clk:    IN std_logic;
         output: OUT std_logic_vector(3 DOWNTO 0));
END counter;
 
ARCHITECTURE rtl OF counter IS

signal aux: std_logic_vector(3 DOWNTO 0);

BEGIN
   process (clk, rst)
   begin
      if(rst = '1') then
         aux <= (others => '0');
      elsif(clk'event and clk = '1') then
         if(aux = "1111") then
            aux <= (others => '0');
         else
            aux <= aux + 1;
         end if;
      end if;

      output <= aux;
   end process;
END rtl;
