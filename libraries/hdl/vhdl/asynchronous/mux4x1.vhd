-- a, b, c and d are the inputs
-- sel is the input selection
-- z is the output
ENTITY mux4x1 IS
   PORT (a:   IN std_logic;
         b:   IN std_logic;
         c:   IN std_logic;
         d:   IN std_logic;
         z:   OUT std_logic;
         sel: IN std_logic_vector(1 DOWNTO 0));
END mux4x1;

ARCHITECTURE rtl of mux4x1 IS
BEGIN
   process(a,b,c,d,sel) begin
      case sel is
         when "00" => z <= a;
         when "01" => z <= b;
         when "10" => z <= c;
         when "11" => z <= d;
      end case;
   end process;
END rtl;
