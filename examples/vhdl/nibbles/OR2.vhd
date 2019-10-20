library ieee;
use ieee.std_logic_1164.all;

entity OR2 is
	port(I1, I2: in std_logic; O: out std_logic);
end OR2;

architecture structural of OR2 is
begin
	O <= I1 or I2;
end structural;