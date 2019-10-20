library ieee;
use ieee.std_logic_1164.all;

entity BUF4 is
	port(I: in std_logic_vector(3 downto 0); O: out std_logic_vector(3 downto 0));
end BUF4;

architecture structural of BUF4 is
begin
	O <= I;
end structural;