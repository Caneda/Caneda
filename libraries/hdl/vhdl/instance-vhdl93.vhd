-- name is the name of the component to instantiate
-- TYPE must be those defined in the component entity
-- u_name can be any name for the instance
ARCHITECTURE rtl OF example IS
   signal sA: TYPE;
   signal sB: TYPE;
   ...

BEGIN
   u_name: ENTITY work.name(rtl)
      PORT MAP(A => sA,
               B => sB,
               ...
               );

END ARCHITECTURE rtl;
