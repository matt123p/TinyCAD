library ieee;
use ieee.std_logic_1164.all;

entity CPU is
	port(CLK: in std_logic;
	Data: out std_logic_vector(3 downto 0));
end CPU;

architecture structural of CPU is
component DataPath is
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
end component;

component ControlUnit is
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
end component;

signal  DA: std_logic_vector(1 downto 0);
signal 	WR: std_logic;
signal 	AA: std_logic_vector(1 downto 0);
signal 	BA: std_logic_vector(1 downto 0);
signal 	MB: std_logic;
signal 	MA: std_logic;
signal 	FS: std_logic_vector(2 downto 0);
signal 	MD: std_logic;
signal 	K: std_logic_vector(3 downto 0);
signal 	MW: std_logic;
signal 	FL: std_logic_vector(3 downto 0);

begin
	DP0: DataPath port map (DA => DA, WR => WR, AA => AA, BA => BA, MA => MA, MB => MB, FS => FS, MD => MD, K => K, CLK => CLK, MW => MW, FL => FL, D => Data);
	CU0: ControlUnit port map (DA => DA, WR => WR, AA => AA, BA => BA, MA => MA, MB => MB, FS => FS, MD => MD, KNS => K, CLK => CLK, MW => MW, F => FL);
end structural;