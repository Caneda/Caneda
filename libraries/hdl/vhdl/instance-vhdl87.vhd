-- name is the name of the component to instantiate
-- MODE and TYPE must be those defined in the component entity
-- u_name can be any name for the instance
ARCHITECTURE rtl OF example IS

   COMPONENT name IS
      PORT(A: MODE TYPE;
           B: MODE TYPE;
           ...
           );
   END COMPONENT name;

   signal sA: TYPE;
   signal sB: TYPE;
   ...

BEGIN
   u_name: name PORT MAP(A => sA,
                         B => sB,
                         ...
                         );

END ARCHITECTURE rtl;
