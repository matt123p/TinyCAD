library ieee;
use ieee.std_logic_1164.all;

entity ZERO is
 	port(O0: out std_logic);
end zero;

architecture structural of ZERO is
begin
	O0 <= '0';
end structural;

