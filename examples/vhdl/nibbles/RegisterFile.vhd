library ieee;
use ieee.std_logic_1164.all;

entity RegisterFile is
	port(	D: in std_logic_vector(3 downto 0);
		DA: in std_logic_vector(1 downto 0);
		WR: in std_logic;
		AA: in std_logic_vector(1 downto 0);
		BA: in std_logic_vector(1 downto 0);
		CLK: in std_logic;
		A: out std_logic_vector(3 downto 0);
		B: out std_logic_vector(3 downto 0));
end RegisterFile;

architecture structural of RegisterFile is
	component Register4
 		port(	D: in std_logic_vector(3 downto 0);
			CLK: in std_logic;
			Load: in std_logic;
			Q: out std_logic_vector(3 downto 0));
 	end component;
	component MUX4to1_4
 		port(	A: in std_logic_vector(3 downto 0);
			B: in std_logic_vector(3 downto 0);
			S: in std_logic_vector(1 downto 0);
			C: in std_logic_vector(3 downto 0);
			D: in std_logic_vector(3 downto 0);
			O: out std_logic_vector(3 downto 0));
 	end component;
	component Decoder2to4
 		port(	I: in std_logic_vector(1 downto 0);
			Q0: out std_logic;
			Q1: out std_logic;
			Q2: out std_logic;
			Q3: out std_logic);
 	end component;
	component AND2
 		port(	I1: in std_logic;
			I2: in std_logic;
			O: out std_logic);
 	end component;
	signal N00:  std_logic_vector(3 downto 0);
	signal N01:  std_logic_vector(3 downto 0);
	signal N02:  std_logic_vector(3 downto 0);
	signal N03:  std_logic;
	signal N04:  std_logic;
	signal N05:  std_logic;
	signal N06:  std_logic;
	signal N07:  std_logic;
	signal N08:  std_logic_vector(3 downto 0);
	signal N09:  std_logic;
	signal N10:  std_logic;
	signal N11:  std_logic;
begin
	Q1: AND2 port map (N03, WR, N07);
	Q2: AND2 port map (N04, WR, N09);
	Q3: AND2 port map (N05, WR, N10);
	Q4: AND2 port map (N06, WR, N11);
	U0: Register4 port map (D(3 downto 0), CLK, N07, N00(3 downto 0));
	U1: Register4 port map (D(3 downto 0), CLK, N09, N01(3 downto 0));
	U2: Register4 port map (D(3 downto 0), CLK, N10, N02(3 downto 0));
	U3: Register4 port map (D(3 downto 0), CLK, N11, N08(3 downto 0));
	U4: Decoder2to4 port map (DA(1 downto 0), N03, N04, N05, N06);
	U5: MUX4to1_4 port map (N00(3 downto 0), N01(3 downto 0), AA(1 downto 0), N02(3 downto 0), N08(3 downto 0), A(3 downto 0));
	U6: MUX4to1_4 port map (N00(3 downto 0), N01(3 downto 0), BA(1 downto 0), N02(3 downto 0), N08(3 downto 0), B(3 downto 0));
end structural;
