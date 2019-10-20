library ieee;
use ieee.std_logic_1164.all;

entity ArithmeticUnit is
	port(	A: in std_logic_vector(3 downto 0);
		B: in std_logic_vector(3 downto 0);
		S: in std_logic_vector(1 downto 0);
		O: out std_logic_vector(3 downto 0);
		V: out std_logic;
		C: out std_logic);
end ArithmeticUnit;

architecture structural of ArithmeticUnit is
	component Adder4
 		port(	A: in std_logic_vector(3 downto 0);
			B: in std_logic_vector(3 downto 0);
			CI: in std_logic;
			O: out std_logic_vector(3 downto 0);
			CO: out std_logic;
			V: out std_logic);
 	end component;
	component UALogic4
 		port(	A: in std_logic_vector(3 downto 0);
			B: in std_logic_vector(3 downto 0);
			S: in std_logic_vector(1 downto 0);
			X: out std_logic_vector(3 downto 0);
			Y: out std_logic_vector(3 downto 0));
 	end component;
	signal N00:  std_logic_vector(3 downto 0);
	signal N01:  std_logic_vector(3 downto 0);
begin
	U0: UALogic4 port map (A(3 downto 0), B(3 downto 0), S(1 downto 0), N00(3 downto 0), N01(3 downto 0));
	U1: Adder4 port map (N00(3 downto 0), N01(3 downto 0), S(1), O(3 downto 0), C, V);
end structural;
