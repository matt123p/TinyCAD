library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity InstMemory_test is
	port(	Address: in std_logic_vector(7 downto 0);
			Output: out std_logic_vector(7 downto 0));
end InstMemory_test;

architecture structural of InstMemory_test is
begin
	with Address select Output <= 
	-- MOVI R3, 0; R3 = 0
	"10001100" when x"00",
	-- LD R0, R3, R0 = 1
	"10100011" when x"01",
	-- ADDI R3, 1, R3 = 1
	"10011101" when x"02",
	-- LD R1, R3, R1 = 7
	"10100111" when x"03",
	-- ADDI R3, 1, R3 = 2
	"10011101" when x"04",
	-- LD R2, R3,  R2 = 9
	"10101011" when x"05",
	-- ADDI R3, 1, R3 = 3
	"10011101" when x"06",
	-- LD R3, R3, R3 = 15
	"10101111" when x"07",
	
	-- MOV R3, R0, R3 = 1
	"00001100" when x"08",
	-- ADD R1, R3, R1 = 8
	"00010111" when x"09",
	-- SUB R1, R3, R1 = 7
	"00100111" when x"0A",
	-- CMP R0, R2, 8
	"00110010" when x"0B",
	-- NAND R0, R3, R0 = 14
	"01000011" when x"0C",
	-- NOR R1, R2, R1 = 0
	"01010110" when x"0D",
	-- SRA R2, R2=12
	"01101000" when x"0E",
	-- SL R3, R3 = 2
	"01111100" when x"0F",
	-- MOVI R2, 2, R2 = 2
	"10001010" when x"10",
	-- ADDI R2, -1, R2 = 1
	"10011011" when x"11",
	-- LD R0, R1, R0 = 1
	"10100001" when x"12",
	-- ST R0, R3, ??
	"10111100" when x"13",
	-- LD R1, R0, R1 = 2
	"10100100" when x"14",
	-- J +3
	"11000011" when x"15",
	-- NOP
	"00000000" when x"16",
	-- NOP
	"00000000" when x"17",
	-- CMP R1, R1
	"00110101" when x"18",
	-- JZ +3
	"11010011" when x"19",
	-- NOP
	"00000000" when x"1A",
	-- NOP
	"00000000" when x"1B",
	-- JNZ +3
	"11100011" when x"1C",
	-- NOP
	"00000000" when x"1D",
	-- NOP
	"00000000" when x"1E",
	-- JN +3
	"11110011" when x"1F",
	-- NOP
	"00000000" when x"20",
	-- NOP
	"00000000" when x"21",
	-- CMP R0, R3, -1
	"00110011" when x"22",
	-- JN +3
	"11110011" when x"23",
	-- NOP
	"00000000" when x"24",
	-- NOP
	"00000000" when x"25",
	"00000000" when others;
end structural;
