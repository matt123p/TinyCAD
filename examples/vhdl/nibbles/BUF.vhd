library ieee;
use ieee.std_logic_1164.all;

entity BUF is
	port(I: in std_logic; O: out std_logic);
end BUF;

architecture structural of BUF is
begin
	O <= I;
end structural;