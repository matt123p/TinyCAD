library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Memory is
	port(	Data, Address: in std_logic_vector(3 downto 0);
			WR: in std_logic;
			CLK: in std_logic;
			Output: out std_logic_vector(3 downto 0));
end Memory;

architecture structural of Memory is
type storage_type is array (0 to 15) of std_logic_vector(3 downto 0);
signal storage: storage_type := (
	 0 => "1000", 		-- 8 (number of elements)
	 1 => "0001",	   	-- 1
	 2 => "0111",	   	-- 7
	 3 => "1010",	   	-- 10
	 4 => "1111",	   	-- 15
	 5 => "0100",	   	-- 4
	 6 => "1110",	   	-- 14
	 7 => "0000",	   	-- 0
	 8 => "1000",	   	-- 8
	 others => "0000");
begin
	Output <= storage(to_integer(unsigned(Address))) when not Is_X(Address) else "XXXX";
	storage(to_integer(unsigned(Address)))<=Data when CLK'event and CLK='1' and WR='1' and not Is_X(Address);
end structural;
