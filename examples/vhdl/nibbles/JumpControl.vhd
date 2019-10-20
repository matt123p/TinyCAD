library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity JumpControl is
	port(	S: in std_logic_vector(3 downto 0);
			BC: in std_logic_vector(2 downto 0);
			Offset: in std_logic_vector(3 downto 0);
			OldPC: in std_logic_vector(7 downto 0);
			NewPC: out std_logic_vector(7 downto 0));
end JumpControl;

architecture structural of JumpControl is
	signal Jump: std_logic;
begin
	-- flags ZNCV
	-- BC: NB, NB, NB, NB, B, BZ, BNZ, BLU
	with BC select Jump <= 
		'0' when "000" | "001" | "010" | "011",
		'1' when "100",
		S(3) when "101",
		not S(3) when "110",
		not S(1) when "111",
		'X' when others;
	
	with Jump select NewPC <= 
		std_logic_vector(signed(OldPC) + 1) when '0',
		std_logic_vector(signed(OldPC) + signed(Offset & "00")) when '1',
		"XXXXXXXX" when others;
	
end structural;
