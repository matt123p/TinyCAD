library ieee;
use ieee.std_logic_1164.all;

entity FullAdder is
	port(	X: in std_logic;
		Y: in std_logic;
		CI: in std_logic;
		O: out std_logic;
		CO: out std_logic);
end FullAdder;

architecture structural of FullAdder is
	component XOR2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	component AND2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	component OR2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	signal N00:  std_logic;
	signal N01:  std_logic;
	signal N02:  std_logic;
begin
	Q1: XOR2 port map (X, Y, N00);
	Q2: XOR2 port map (N00, CI, O);
	Q3: AND2 port map (X, Y, N02);
	Q4: AND2 port map (N00, CI, N01);
	Q5: OR2 port map (N01, N02, CO);
end structural;
