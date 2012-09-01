-- in1, in2 and the cin (carry) are inputs
-- output is the sum output, cout is the carry out
ENTITY adder IS
   port (in1, in2 : IN bit;
         cin      : IN bit; 
         output   : OUT bit; 
         cout     : OUT bit);
END adder;
     
ARCHITECTURE rtl OF adder IS
BEGIN
   output <= in1 xor in2 xor cin;
   cout   <= (in1 and in2) or (in1 and cin) or (in2 and cin);
END rtl;
