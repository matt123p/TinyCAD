library ieee;
use ieee.std_logic_1164.all;

entity MUX2to1 is
	port(	A: in std_logic;
		B: in std_logic;
		S: in std_logic;
		O: out std_logic);
end MUX2to1;

architecture structural of MUX2to1 is
	component NAND2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	component NOT1
 		port(	I: in std_logic;
			O: out std_logic);
 	end component;
	signal N00:  std_logic;
	signal N01:  std_logic;
	signal N02:  std_logic;
begin
	Q1: NAND2 port map (A, N02, N00);
	Q2: NOT1 port map (S, N02);
	Q3: NAND2 port map (N00, N01, O);
	Q4: NAND2 port map (S, B, N01);
end structural;
