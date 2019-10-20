library ieee;
use ieee.std_logic_1164.all;

entity AND2 is
	port(I1, I2: in std_logic; O: out std_logic);
end AND2;

architecture structural of AND2 is
begin
	O <= I1 and I2;
end structural;