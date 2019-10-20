library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity InstMemory is
	port(	Address: in std_logic_vector(7 downto 0);
			Output: out std_logic_vector(7 downto 0));
end InstMemory;

architecture structural of InstMemory is
begin
	with Address select Output <= 
	-- Sort ascending the first entries in memory.
	-- M[0] has the number of entries.
	-- R0 outer loop, R1 inner loop
	-- MOVI R0, 1
	"10000001" when x"00",
	-- MOV R1, R0
	"00000100" when x"01",
	-- ADDI R1, 1
	"10010101" when x"02",
	-- LD R2, R0
	"10101000" when x"03",
	-- LD R3, R1
	"10101101" when x"04",
	-- CMP R2, R3
	"00111011" when x"05",
	-- BN +4 ;(d.f.x./4)
	"11110001" when x"06",
	-- ST R0, R3 ;(d.f.x.fields)
	"10111100" when x"07",
	-- ST R1, R2 ;(d.f.x.fields)
	"10111001" when x"08",
	-- NOP
	"00000000" when x"09",
	-- MOVI R2, 0 ; BR R1<n
	"10001000" when x"0A",
	-- LD R2, R2
	"10101010" when x"0B",
	-- CMP R1, R2
	"00110110" when x"0C",
	-- NOP
	"00000000" when x"0D",
	-- BLU -12 ;(d.f.x./4)
	"11111101" when x"0E",
	-- ADDI R0, 1
	"10010001" when x"0F",
	-- MOVI R2, 0 ; BR R0<n
	"10001000" when x"10",
	-- LD R2, R2
	"10101010" when x"11",
	-- CMP R0, R2
	"00110010" when x"12",
	-- NOP
	"00000000" when x"13",
	-- NOP
	"00000000" when x"14",
	-- BLU -20 ;(d.f.x./4)
	"11111011" when x"15",
	-- MOVI R0, 0
	"10000000" when x"16",
	-- LD	R1, R0
	"10100100" when x"17",
	-- ADDI R0, 1
	"10010001" when x"18",
	-- NOP
	"00000000" when x"19",
	-- NOP
	"00000000" when x"1A",
	-- BNZ -4 
	"11101111" when x"1B",
	-- BR 0
	"11000000" when x"1C",
	"00000000" when others;
end structural;
