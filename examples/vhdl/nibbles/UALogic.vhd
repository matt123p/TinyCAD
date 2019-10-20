library ieee;
use ieee.std_logic_1164.all;

entity UALogic is
	port(	A: in std_logic;
		B: in std_logic;
		S0: in std_logic;
		S1: in std_logic;
		X: out std_logic;
		Y: out std_logic);
end UALogic;

architecture structural of UALogic is
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
begin
	Q1: XOR2 port map (S1, B, Y);
	Q2: AND2 port map (A, N00, X);
	Q3: OR2 port map (S0, S1, N00);
end structural;
