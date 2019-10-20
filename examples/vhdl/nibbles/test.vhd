library ieee;
use ieee.std_logic_1164.all;

entity test is
end test;

architecture behavior of test is
	signal clock: std_logic := '0';
	signal ola: std_logic;
begin
	process
	begin
		wait for 50 ns;
		clock <= not clock;
	end process;
	
	ola <= '0' when clock = '1' else
	'1' when clock = '0';

end behavior;