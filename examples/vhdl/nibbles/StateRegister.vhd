library ieee;
use ieee.std_logic_1164.all;

entity StateRegister is
	port(	F: in std_logic_vector(3 downto 0);
			FW: in std_logic_vector(3 downto 0);
			CLK: in std_logic;
			S: out std_logic_vector(3 downto 0));
end StateRegister;

architecture structural of StateRegister is
begin
	S(0) <= F(0) when CLK'event and CLK='1' and FW(0)='1';
	S(1) <= F(1) when CLK'event and CLK='1' and FW(1)='1';
	S(2) <= F(2) when CLK'event and CLK='1' and FW(2)='1';
	S(3) <= F(3) when CLK'event and CLK='1' and FW(3)='1';
end structural;
