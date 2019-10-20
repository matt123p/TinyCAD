library ieee;
use ieee.std_logic_1164.all;

entity XOR2 is
	port(I1, I2: in std_logic; O: out std_logic);
end XOR2;

architecture structural of XOR2 is
begin
	O <= I1 xor I2;
end structural;