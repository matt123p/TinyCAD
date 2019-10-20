library ieee;
use ieee.std_logic_1164.all;

entity UALogic4 is
	port(	A: in std_logic_vector(3 downto 0);
		B: in std_logic_vector(3 downto 0);
		S: in std_logic_vector(1 downto 0);
		X: out std_logic_vector(3 downto 0);
		Y: out std_logic_vector(3 downto 0));
end UALogic4;

architecture structural of UALogic4 is
	component UALogic
 		port(	A: in std_logic;
			B: in std_logic;
			S0: in std_logic;
			S1: in std_logic;
			X: out std_logic;
			Y: out std_logic);
 	end component;
begin
	U1: UALogic port map (A(0), B(0), S(0), S(1), X(0), Y(0));
	U2: UALogic port map (A(1), B(1), S(0), S(1), X(1), Y(1));
	U3: UALogic port map (A(2), B(2), S(0), S(1), X(2), Y(2));
	U4: UALogic port map (A(3), B(3), S(0), S(1), X(3), Y(3));
end structural;
