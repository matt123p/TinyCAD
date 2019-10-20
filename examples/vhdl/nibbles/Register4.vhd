library ieee;
use ieee.std_logic_1164.all;

entity Register4 is
	port(	D: in std_logic_vector(3 downto 0);
			Load: in std_logic;
			CLK: in std_logic;
			Q: out std_logic_vector(3 downto 0):="0000"
			);
end Register4;

architecture structural of Register4 is
begin
	Q <= D when CLK'event and CLK='1' and Load='1';
end structural;
