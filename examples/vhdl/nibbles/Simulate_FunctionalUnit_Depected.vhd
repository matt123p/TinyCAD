library ieee;
use ieee.std_logic_1164.all;

entity Simulate_FunctionalUnit is
end Simulate_FunctionalUnit;

architecture behavior of Simulate_FunctionalUnit is

component FunctionalUnit
	port(	A: in std_logic_vector(3 downto 0);
		B: in std_logic_vector(3 downto 0);
		S: in std_logic_vector(2 downto 0);
		O: out std_logic_vector(3 downto 0));
end component;

	signal clock: std_logic := '1';
	signal A, B, O: std_logic_vector(3 downto 0);
	signal S: std_logic_vector(2 downto 0);
begin
	process
	begin
		wait for 50 ns;
		clock <= not clock;
	end process;
	
	process
	begin
		A <= "1001"; --9 
		B <= "0010"; --2
		
		S <= "000"; wait until clock'event and clock='1'; 
		S <= "001"; wait until clock'event and clock='1'; 
		S <= "010"; wait until clock'event and clock='1'; 
		S <= "011"; wait until clock'event and clock='1'; 
		S <= "100"; wait until clock'event and clock='1'; 
		S <= "101"; wait until clock'event and clock='1'; 
		S <= "110"; wait until clock'event and clock='1'; 
		S <= "111"; wait until clock'event and clock='1'; 
		
	end process;
	
	FU0: FunctionalUnit port map (A, B, S, O);
	
end behavior;

