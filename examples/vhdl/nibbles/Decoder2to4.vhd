library ieee;
use ieee.std_logic_1164.all;

entity Decoder2to4 is
	port(	I: in std_logic_vector(1 downto 0);
		Q0: out std_logic;
		Q1: out std_logic;
		Q2: out std_logic;
		Q3: out std_logic);
end Decoder2to4;

architecture structural of Decoder2to4 is
begin
	Q0 <= not I(1) and not I(0); -- 00
	Q1 <= not I(1) and I(0); 	-- 01
	Q2 <= I(1) and not I(0); 	-- 10
	Q3 <= I(1) and I(0); 		-- 11
end structural;
