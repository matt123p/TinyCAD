library ieee;
use ieee.std_logic_1164.all;

entity ControlUnit is
	port(	F: in std_logic_vector(3 downto 0);
		CLK: in std_logic;
		DA: out std_logic_vector(1 downto 0);
		WR: out std_logic;
		AA: out std_logic_vector(1 downto 0);
		BA: out std_logic_vector(1 downto 0);
		MA: out std_logic;
		MB: out std_logic;
		KNS: out std_logic_vector(3 downto 0);
		FS: out std_logic_vector(2 downto 0);
		MD: out std_logic;
		MW: out std_logic);
end ControlUnit;

architecture structural of ControlUnit is
	component JumpControl
 		port(	BC: in std_logic_vector(2 downto 0);
			Offset: in std_logic_vector(3 downto 0);
			S: in std_logic_vector(3 downto 0);
			OldPC: in std_logic_vector(7 downto 0);
			NewPC: out std_logic_vector(7 downto 0));
 	end component;
	component StateRegister
 		port(	F: in std_logic_vector(3 downto 0);
			FW: in std_logic_vector(3 downto 0);
			CLK: in std_logic;
			S: out std_logic_vector(3 downto 0));
 	end component;
	component PCReg
 		port(	D: in std_logic_vector(7 downto 0);
			CLK: in std_logic;
			Q: out std_logic_vector(7 downto 0));
 	end component;
	component InstMemory
 		port(	Address: in std_logic_vector(7 downto 0);
			Output: out std_logic_vector(7 downto 0));
 	end component;
	component Decoder
 		port(	Inst: in std_logic_vector(7 downto 0);
			FW: out std_logic_vector(3 downto 0);
			BC: out std_logic_vector(2 downto 0);
			AA: out std_logic_vector(1 downto 0);
			BA: out std_logic_vector(1 downto 0);
			DA: out std_logic_vector(1 downto 0);
			WR: out std_logic;
			MA: out std_logic;
			MB: out std_logic;
			FS: out std_logic_vector(2 downto 0);
			MD: out std_logic;
			KNS: out std_logic_vector(3 downto 0);
			MW: out std_logic;
			Offset: out std_logic_vector(3 downto 0));
 	end component;
	signal N00:  std_logic_vector(7 downto 0);
	signal N01:  std_logic_vector(3 downto 0);
	signal N02:  std_logic_vector(7 downto 0);
	signal N03:  std_logic_vector(3 downto 0);
	signal N04:  std_logic_vector(2 downto 0);
	signal N05:  std_logic_vector(3 downto 0);
	signal PC:  std_logic_vector(7 downto 0);
begin
	U1: StateRegister port map (F(3 downto 0), N03(3 downto 0), CLK, N01(3 downto 0));
	U2: JumpControl port map (N04(2 downto 0), N05(3 downto 0), N01(3 downto 0), PC(7 downto 0), N00(7 downto 0));
	U3: PCReg port map (N00(7 downto 0), CLK, PC(7 downto 0));
	U4: InstMemory port map (PC(7 downto 0), N02(7 downto 0));
	U5: Decoder port map (N02(7 downto 0), N03(3 downto 0), N04(2 downto 0), AA(1 downto 0), BA(1 downto 0), DA(1 downto 0), WR, MA, MB, FS(2 downto 0), MD, KNS(3 downto 0), MW, N05(3 downto 0));
end structural;
