library ieee;
use ieee.std_logic_1164.all;

entity Decoder is
	port(	Inst: in std_logic_vector(7 downto 0);
		Offset: out std_logic_vector(3 downto 0);
		FW: out std_logic_vector(3 downto 0);
		BC: out std_logic_vector(2 downto 0);
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
end Decoder;

architecture structural of Decoder is
signal Z, C, MOVI, MBi: std_logic;

begin
	Offset(3 downto 0) <= Inst(3 downto 0);
	Z <= (not Inst(7) and Inst(5))  or  (not Inst(7) and Inst(6)) or  (not Inst(6) and not Inst(5) and Inst(4));
	FW(3) <= Z;
	FW(2) <= Z;
	C <= (not Inst(7) and Inst(5))  or  (not Inst(6) and not Inst(5) and Inst(4));
	FW(1) <= C;
	FW(0) <= C;
	BC(0) <= Inst(4) and Inst(7);
	BC(1) <= Inst(5) and Inst(7);
	BC(2) <= Inst(6) and Inst(7);
	DA <= Inst(3 downto 2);
	WR <= (not Inst(7) and Inst(6)) or (not Inst(6) and not Inst(5)) or (not Inst(6) and not Inst(4));
	AA <= Inst(3 downto 2);
	BA <= Inst(1 downto 0);
	MOVI <= Inst(7) and not Inst(6) and not Inst(5) and not Inst(4);
	MBi <= Inst(7) and not Inst(6) and not Inst(5);
	with MOVI select KNS <= 
		"00" & Inst(1 downto 0) when '1',
		Inst(1) & Inst(1) & Inst(1 downto 0) when '0',
		"XXXX" when others;
	MA <= '0';
	MB <= MBi;
	FS <= Inst(6 downto 4);
	MD <= Inst(7) and not Inst(6) and Inst(5) and not Inst(4);
	MW <= Inst(7) and not Inst(6) and Inst(5) and Inst(4);
end structural;
