library ieee;
use ieee.std_logic_1164.all;

entity Shifter is
	port(	A: in std_logic_vector(3 downto 0);
		S: in std_logic;
		O: out std_logic_vector(3 downto 0);
		C: out std_logic;
		V: out std_logic);
end Shifter;

architecture structural of Shifter is
	component MUX2to1
 		port(	A: in std_logic;
			B: in std_logic;
			S: in std_logic;
			O: out std_logic);
 	end component;
	component ZERO
 		port(	O0: out std_logic);
 	end component;
	component XOR2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	signal N00:  std_logic;
	signal N01:  std_logic;
	signal N02:  std_logic;
begin
	Q1: XOR2 port map (A(3), A(2), N02);
	U1: MUX2to1 port map (A(1), N00, S, O(0));
	U2: ZERO port map (N00);
	U3: MUX2to1 port map (A(2), A(0), S, O(1));
	U4: MUX2to1 port map (A(3), A(1), S, O(2));
	U6: MUX2to1 port map (A(3), A(2), S, O(3));
	U7: MUX2to1 port map (A(0), A(3), S, C);
	U8: ZERO port map (N01);
	U9: MUX2to1 port map (N01, N02, S, V);
end structural;
