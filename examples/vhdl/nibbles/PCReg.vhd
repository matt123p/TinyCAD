library ieee;
use ieee.std_logic_1164.all;

entity PCReg is
	port(	D: in std_logic_vector(7 downto 0);
			CLK: in std_logic;
			Q: out std_logic_vector(7 downto 0):= "00000000"
			);
end PCReg;

architecture structural of PCReg is
begin
	Q <= D when CLK'event and CLK='1';
end structural;
