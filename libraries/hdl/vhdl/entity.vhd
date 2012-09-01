-- name can be any name chosen for this entity

-- MODE specifies the port direction, and is one of the following:
-- in: a port that can be read from but not written to
-- out: a port that can be written to but not read from
-- in/out: a port that can be read and written to (tristate buses)
-- buffer: an out port whose current value can be read from

-- TYPE specifies the data type, and is one of the following:
-- bit: the signal has only the states "1" or "0"
-- boolean: the signal has only the states True or False
-- std_logic: the signal has extended values as "1", "0", "Z", "-"
-- integer: the signal is an integer
-- bit_vector: the signal is a concatenation of bits
-- std_logic_vector: the signal is a concatenation of std_logic signals
-- character: the signal may be any ISO character

ENTITY name IS
   PORT(A: MODE TYPE;
        B: MODE TYPE;
        ...
   );
END name;
