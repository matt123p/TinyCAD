library ieee;
use ieee.std_logic_1164.all;

entity Adder4 is
	port(	A: in std_logic_vector(3 downto 0);
		B: in std_logic_vector(3 downto 0);
		Ci: in std_logic;
		Co: out std_logic;
		O: out std_logic_vector(3 downto 0);
		V: out std_logic);
end Adder4;

architecture structural of Adder4 is
	component FullAdder
 		port(	X: in std_logic;
			Y: in std_logic;
			CI: in std_logic;
			O: out std_logic;
			CO: out std_logic);
 	end component;
	component BUF
 		port(	I: in std_logic;
			O: out std_logic);
 	end component;
	component XOR2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	signal N00:  std_logic;
	signal N01:  std_logic;
	signal N02:  std_logic;
	signal N03:  std_logic;
begin
	Q2: BUF port map (N03, Co);
	Q3: XOR2 port map (N02, N03, V);
	U1: FullAdder port map (A(0), B(0), Ci, O(0), N00);
	U2: FullAdder port map (A(1), B(1), N00, O(1), N01);
	U3: FullAdder port map (A(2), B(2), N01, O(2), N02);
	U4: FullAdder port map (A(3), B(3), N02, O(3), N03);
end structural;
