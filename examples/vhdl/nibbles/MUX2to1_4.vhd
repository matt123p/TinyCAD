library ieee;
use ieee.std_logic_1164.all;

entity MUX2to1_4 is
	port(	A: in std_logic_vector(3 downto 0);
		B: in std_logic_vector(3 downto 0);
		S: in std_logic;
		O: out std_logic_vector(3 downto 0));
end MUX2to1_4;

architecture structural of MUX2to1_4 is
	component MUX2to1
 		port(	A: in std_logic;
			B: in std_logic;
			S: in std_logic;
			O: out std_logic);
 	end component;
begin
	U1: MUX2to1 port map (A(0), B(0), S, O(0));
	U2: MUX2to1 port map (A(1), B(1), S, O(1));
	U3: MUX2to1 port map (A(2), B(2), S, O(2));
	U4: MUX2to1 port map (A(3), B(3), S, O(3));
end structural;
