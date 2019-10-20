library ieee;
use ieee.std_logic_1164.all;

entity NOR4Bus is
	port(I: in std_logic_vector(3 downto 0);
		D: out std_logic);
end NOR4Bus;

architecture structural of NOR4Bus is
	component NOR2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	component AND2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	signal N00:  std_logic;
	signal N01:  std_logic;
begin
	Q1: NOR2 port map (I(0), I(1), N00);
	Q2: AND2 port map (N00, N01, D);
	Q3: NOR2 port map (I(2), I(3), N01);
end structural;
