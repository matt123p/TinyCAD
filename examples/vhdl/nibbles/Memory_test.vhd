library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Memory_teste is
	port(	Data, Address: in std_logic_vector(3 downto 0);
			WR: in std_logic;
			CLK: in std_logic;
			Output: out std_logic_vector(3 downto 0));
end Memory_teste;

architecture structural of Memory_teste is
type storage_type is array (0 to 15) of std_logic_vector(3 downto 0);
signal storage: storage_type := (
	 0 => "0001",	   	-- 1
	 1 => "0111",	   	-- 7
	 2 => "1001",	   	-- -7
	 3 => "1111",	   	-- -1
	 others => "0000");
begin
	Output <= storage(to_integer(unsigned(Address))) when not Is_X(Address) else "XXXX";
	storage(to_integer(unsigned(Address)))<=Data when CLK'event and CLK='1' and WR='1' and not Is_X(Address);
end structural;
