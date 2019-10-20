library ieee;
use ieee.std_logic_1164.all;

entity NAND2 is
	port(I1, I2: in std_logic; O: out std_logic);
end NAND2;

architecture structural of NAND2 is
begin
	O <= I1 nand I2;
end structural;