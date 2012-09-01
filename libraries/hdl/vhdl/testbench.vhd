-- name is the name of the component to test
-- MODE and TYPE must be those defined in the component entity
ENTITY testbenchEntity IS
END testbenchEntity;

ARCHITECTURE behav OF testbenchEntity IS
   --  Declaration of the component
   COMPONENT name
      PORT(A: MODE TYPE;
           B: MODE TYPE;
           ...
           );
   END COMPONENT;

   signal sA: TYPE;
   signal sB: TYPE;
   ...

BEGIN
   --  Component instantiation
   u0: name PORT MAP(A => sA,
                     B => sB,
                     ...
                     );
                                 
   --  Clock definition               
   clk <= not clk after 5 ns;

   --  Set the inputs
   process
   begin
      wait for 1 ns;
      sA <= '1';
      sB <= '0';
      ...
      wait for 1 ns;
      sA <= '1';
      sB <= '1';
      ...
      --  Wait forever = finish the simulation
      wait;
   end process;
END behav;
