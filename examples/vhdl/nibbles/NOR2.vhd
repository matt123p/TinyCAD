library ieee;
use ieee.std_logic_1164.all;

entity NOR2 is
	port(I1, I2: in std_logic; O: out std_logic);
end NOR2;

architecture structural of NOR2 is
begin
	O <= I1 nor I2;
end structural;