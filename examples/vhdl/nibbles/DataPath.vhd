-- File Created by TinyCad VHDL
library ieee;
use ieee.std_logic_1164.all;

entity DataPath is
	port(	DA: in std_logic_vector(1 downto 0);
		WR: in std_logic;
		AA: in std_logic_vector(1 downto 0);
		BA: in std_logic_vector(1 downto 0);
		MB: in std_logic;
		MA: in std_logic;
		FS: in std_logic_vector(2 downto 0);
		MD: in std_logic;
		K: in std_logic_vector(3 downto 0);
		CLK: in std_logic;
		MW: in std_logic;
		FL: out std_logic_vector(3 downto 0);
		D: out std_logic_vector(3 downto 0));
end DataPath;

architecture structural of DataPath is
	component RegisterFile
 		port(	D: in std_logic_vector(3 downto 0);
			DA: in std_logic_vector(1 downto 0);
			AA: in std_logic_vector(1 downto 0);
			BA: in std_logic_vector(1 downto 0);
			WR: in std_logic;
			CLK: in std_logic;
			A: out std_logic_vector(3 downto 0);
			B: out std_logic_vector(3 downto 0));
 	end component;
	component MUX2to1_4
 		port(	A: in std_logic_vector(3 downto 0);
			B: in std_logic_vector(3 downto 0);
			S: in std_logic;
			O: out std_logic_vector(3 downto 0));
 	end component;
	component FunctionalUnit
 		port(	A: in std_logic_vector(3 downto 0);
			B: in std_logic_vector(3 downto 0);
			FS: in std_logic_vector(2 downto 0);
			F: out std_logic_vector(3 downto 0);
			FL: out std_logic_vector(3 downto 0));
 	end component;
	component Memory
 		port(	Address: in std_logic_vector(3 downto 0);
			Data: in std_logic_vector(3 downto 0);
			WR: in std_logic;
			CLK: in std_logic;
			Output: out std_logic_vector(3 downto 0));
 	end component;
	component Buf4
 		port(	I: in std_logic_vector(3 downto 0);
			O: out std_logic_vector(3 downto 0));
 	end component;
	signal N00:  std_logic_vector(3 downto 0);
	signal N01:  std_logic_vector(3 downto 0);
	signal N02:  std_logic_vector(3 downto 0);
	signal N03:  std_logic_vector(3 downto 0);
	signal N04:  std_logic_vector(3 downto 0);
	signal N05:  std_logic_vector(3 downto 0);
	signal N06:  std_logic_vector(3 downto 0);
begin
	U1: Memory port map (N02, N03, MW, CLK, N06);
	U2: MUX2to1_4 port map (N00, K, MA, N03);
	U3: MUX2to1_4 port map (N04, N06, MD, N05);
	U4: RegisterFile port map (N05, DA, AA, BA, WR, CLK, N00, N01);
	U5: FunctionalUnit port map (N03, N02, FS, N04, FL);
	U6: MUX2to1_4 port map (N01, K, MB, N02);
	U7: Buf4 port map (N05, D);
end structural;
