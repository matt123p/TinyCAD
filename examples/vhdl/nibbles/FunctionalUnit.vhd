-- File Created by TinyCad VHDL
library ieee;
use ieee.std_logic_1164.all;

entity FunctionalUnit is
	port(	A: in std_logic_vector(3 downto 0);
		B: in std_logic_vector(3 downto 0);
		FS: in std_logic_vector(2 downto 0);
		F: out std_logic_vector(3 downto 0);
		FL: out std_logic_vector(3 downto 0));
end FunctionalUnit;

architecture structural of FunctionalUnit is
	component ArithmeticUnit
 		port(	A: in std_logic_vector(3 downto 0);
			B: in std_logic_vector(3 downto 0);
			S: in std_logic_vector(1 downto 0);
			O: out std_logic_vector(3 downto 0);
			C: out std_logic;
			V: out std_logic);
 	end component;
	component LogicUnit
 		port(	A: in std_logic_vector(3 downto 0);
			B: in std_logic_vector(3 downto 0);
			S: in std_logic;
			O: out std_logic_vector(3 downto 0));
 	end component;
	component Shifter
 		port(	A: in std_logic_vector(3 downto 0);
			S: in std_logic;
			O: out std_logic_vector(3 downto 0);
			C: out std_logic;
			V: out std_logic);
 	end component;
	component MUX2to1_4
 		port(	A: in std_logic_vector(3 downto 0);
			B: in std_logic_vector(3 downto 0);
			S: in std_logic;
			O: out std_logic_vector(3 downto 0));
 	end component;
	component Buf4
 		port(	I: in std_logic_vector(3 downto 0);
			O: out std_logic_vector(3 downto 0));
 	end component;
	component NOR4Bus
 		port(	I: in std_logic_vector(3 downto 0);
			D: out std_logic);
 	end component;
	component BUF
 		port(	I: in std_logic;
			O: out std_logic);
 	end component;
	component MUX2to1
 		port(	A: in std_logic;
			B: in std_logic;
			S: in std_logic;
			O: out std_logic);
 	end component;
	signal C_Arit:  std_logic;
	signal C_Shift:  std_logic;
	signal D:  std_logic_vector(3 downto 0);
	signal N00:  std_logic_vector(3 downto 0);
	signal N01:  std_logic_vector(3 downto 0);
	signal N02:  std_logic_vector(3 downto 0);
	signal N03:  std_logic_vector(3 downto 0);
	signal V_Arit:  std_logic;
	signal V_Shift:  std_logic;
begin
	Q1: BUF port map (D(3), FL(2));
	U1: MUX2to1_4 port map (N00, N03, FS(2), D);
	U2: ArithmeticUnit port map (A, B, FS(1 downto 0), N00, C_Arit, V_Arit);
	U3: MUX2to1_4 port map (N01, N02, FS(1), N03);
	U4: LogicUnit port map (A, B, FS(0), N01);
	U5: Shifter port map (A, FS(0), N02, C_Shift, V_Shift);
	U6: Buf4 port map (D, F);
	U7: NOR4Bus port map (D, FL(3));
	U8: MUX2to1 port map (C_Arit, C_Shift, FS(2), FL(1));
	U9: MUX2to1 port map (V_Arit, V_Shift, FS(2), FL(0));
end structural;
