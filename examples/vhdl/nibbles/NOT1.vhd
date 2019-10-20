library ieee;
use ieee.std_logic_1164.all;

entity NOT1 is
	port(I: in std_logic; O: out std_logic);
end NOT1;

architecture structural of NOT1 is
begin
	O <= not I;
end structural;