library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
 
ENTITY counter IS
   GENERIC ( WIDTH : integer := 32);

   PORT (CLK  : IN std_logic;
         RST  : IN std_logic;
         LOAD : IN std_logic;
         DATA : IN  unsigned(WIDTH-1 DOWNTO 0);  
         Q    : OUT unsigned(WIDTH-1 DOWNTO 0));
END counter;
 
ARCHITECTURE rtl OF counter IS

signal count : unsigned(WIDTH-1 DOWNTO 0);

BEGIN
   process(RST, CLK) is
   begin
      if(RST = '1') then
         count <= (others => '0');
      elsif(CLK = '1' and CLK'event) then
         if(LOAD = '1') then
            count <= DATA;
         else
            count <= count + 1;
         end if;
      end if;
   end process;
 
   Q <= count;
 
END rtl;
