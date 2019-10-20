library ieee;
use ieee.std_logic_1164.all;

entity LogicUnit is
	port(	A: in std_logic_vector(3 downto 0);
		B: in std_logic_vector(3 downto 0);
		S: in std_logic;
		O: out std_logic_vector(3 downto 0));
end LogicUnit;

architecture structural of LogicUnit is
	component MUX2to1
 		port(	A: in std_logic;
			B: in std_logic;
			S: in std_logic;
			O: out std_logic);
 	end component;
	component NAND2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	component NOR2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	signal N00:  std_logic;
	signal N01:  std_logic;
	signal N02:  std_logic;
	signal N03:  std_logic;
	signal N04:  std_logic;
	signal N05:  std_logic;
	signal N06:  std_logic;
	signal N07:  std_logic;
begin
	Q1: NAND2 port map (A(0), B(0), N00);
	Q2: NOR2 port map (A(0), B(0), N01);
	Q3: NAND2 port map (A(1), B(1), N02);
	Q4: NOR2 port map (A(1), B(1), N03);
	Q5: NAND2 port map (A(2), B(2), N04);
	Q6: NOR2 port map (A(2), B(2), N05);
	Q7: NAND2 port map (A(3), B(3), N06);
	Q8: NOR2 port map (A(3), B(3), N07);
	U1: MUX2to1 port map (N00, N01, S, O(0));
	U2: MUX2to1 port map (N02, N03, S, O(1));
	U3: MUX2to1 port map (N04, N05, S, O(2));
	U4: MUX2to1 port map (N06, N07, S, O(3));
end structural;
