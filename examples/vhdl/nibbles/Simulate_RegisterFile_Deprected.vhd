library ieee;
use ieee.std_logic_1164.all;

entity Simulate_RegisterFile is
end Simulate_RegisterFile;

architecture behavior of Simulate_RegisterFile is

component RegisterFile
	port(	D: in std_logic_vector(3 downto 0);
		DA: in std_logic_vector(1 downto 0);
		WR: in std_logic;
		AA: in std_logic_vector(1 downto 0);
		BA: in std_logic_vector(1 downto 0);
		CLK: in std_logic;
		A: out std_logic_vector(3 downto 0);
		B: out std_logic_vector(3 downto 0));
end component;

	signal clock: std_logic := '1';
	signal D, A, B: std_logic_vector(3 downto 0);
	signal DA, AA, BA: std_logic_vector(1 downto 0);
	signal WR: std_logic;
begin
	process
	begin
		wait for 50 ns;
		clock <= not clock;
	end process;
	
	process
	begin
		D <= "1001"; --9
		WR <= '0';
		
		-- Reads all the registers
		AA <= "00"; BA <= "01"; wait until clock'event and clock='0'; 
		AA <= "10"; BA <= "11"; wait until clock'event and clock='0'; 
		
		--store in register 2
		DA <= "10"; WR <= '1'; wait until clock'event and clock='0'; 
		WR <= '0';
		
		-- Reads all the registers in another order
		AA <= "01"; BA <= "10"; wait until clock'event and clock='0'; 
		AA <= "00"; BA <= "11"; wait until clock'event and clock='0'; 
		
		--store in register 0
		DA <= "00"; WR <= '1'; wait until clock'event and clock='0'; 
		WR <= '0';
		
		-- Reads all the registers
		AA <= "00"; BA <= "01"; wait until clock'event and clock='0'; 
		AA <= "10"; BA <= "11"; wait until clock'event and clock='0'; 
		
		--store in register 1
		D <= "0110";
		DA <= "01"; WR <= '1'; wait until clock'event and clock='0'; 
		WR <= '0';
		
		-- Reads all the registers
		AA <= "00"; BA <= "01"; wait until clock'event and clock='0'; 
		AA <= "10"; BA <= "11"; wait until clock'event and clock='0'; 
		
		
	end process;
	
	RF0: RegisterFile port map (D, DA, WR, AA, BA, clock, A, B);
	
end behavior;

