library ieee;
use ieee.std_logic_1164.all;

entity Simulate_CPU is
end Simulate_CPU;

architecture behavior of Simulate_CPU is

component CPU
	port(CLK: in std_logic;
	Data: out std_logic_vector(3 downto 0));
end component;

	signal clock: std_logic := '0';
	signal Data: std_logic_vector(3 downto 0);
begin
	process
	begin
		wait for 50 ns;
		clock <= not clock;
	end process;
	
	CPU0: CPU port map (clock, data);
end behavior;