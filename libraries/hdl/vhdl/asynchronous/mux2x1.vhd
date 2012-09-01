-- a, b are the inputs
-- sel is the input selection
-- z is the output
ENTITY mux2x1 IS
   PORT (a:   IN std_logic;
         b:   IN std_logic;
         sel: IN std_logic;
         z:   OUT std_logic);
END mux2x1;
 
ARCHITECTURE rtl of mux2x1 IS
BEGIN
   z <= a when sel='0' else b;
END rtl;
