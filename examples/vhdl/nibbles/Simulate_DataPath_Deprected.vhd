library ieee;
use ieee.std_logic_1164.all;

entity Simulate_DataPath is
end Simulate_DataPath;

architecture behavior of Simulate_DataPath is

component DataPath
	port(	DA: in std_logic_vector(1 downto 0);
		WR: in std_logic;
		AA: in std_logic_vector(1 downto 0);
		BA: in std_logic_vector(1 downto 0);
		MB: in std_logic;
		MA: in std_logic;
		FS: in std_logic_vector(2 downto 0);
		MD: in std_logic;
		K: in std_logic_vector(3 downto 0);
		CLK: in std_logic;
		MW: in std_logic;
		FL: out std_logic_vector(3 downto 0);
		D: out std_logic_vector(3 downto 0));

end component;

	signal DA: std_logic_vector(1 downto 0);
	signal WR: std_logic;
	signal AA: std_logic_vector(1 downto 0);
	signal BA: std_logic_vector(1 downto 0);
	signal MB: std_logic;
	signal MA: std_logic;
	signal FS: std_logic_vector(2 downto 0);
	signal MD: std_logic;
	signal K: std_logic_vector(3 downto 0);
	signal clock: std_logic := '0';
	signal MW: std_logic;
	signal FL: std_logic_vector(3 downto 0);
	signal D: std_logic_vector(3 downto 0);
begin
	process
	begin
		wait for 50 ns;
		clock <= not clock;
	end process;
	
	process
	begin
		wait until clock'event and clock='0';
		-- FS: A, A+B, A+1, A-B, NAND, NOR, SRA, SL
		-- R0 <- 1
		DA<="00"; WR<='1'; AA<="00"; BA<="00"; MB<='0'; MA<='1'; FS<="000"; MD<='0'; K<="0001"; MW<='0'; wait until clock'event and clock='0';
		-- R1 <- 3
		DA<="01"; WR<='1'; AA<="00"; BA<="00"; MB<='0'; MA<='1'; FS<="000"; MD<='0'; K<="0011"; MW<='0'; wait until clock'event and clock='0';
		-- R2 <- 5
		DA<="10"; WR<='1'; AA<="00"; BA<="00"; MB<='0'; MA<='1'; FS<="000"; MD<='0'; K<="0101"; MW<='0'; wait until clock'event and clock='0';
		-- R3 <- 7
		DA<="11"; WR<='1'; AA<="00"; BA<="00"; MB<='0'; MA<='1'; FS<="000"; MD<='0'; K<="0111"; MW<='0'; wait until clock'event and clock='0';
		-- R1 <- R2 + R3 = 12 (FL=0101)
		DA<="01"; WR<='1'; AA<="10"; BA<="11"; MB<='0'; MA<='0'; FS<="001"; MD<='0'; K<="0000"; MW<='0'; wait until clock'event and clock='0';
		-- R3 <- R1 + 1 = 13   (FL=0100)
		DA<="11"; WR<='1'; AA<="01"; BA<="00"; MB<='0'; MA<='0'; FS<="010"; MD<='0'; K<="0000"; MW<='0'; wait until clock'event and clock='0';
		-- R2 <- R3 - R2 = 13-5 = 8 (FL=0110)
		DA<="10"; WR<='1'; AA<="11"; BA<="10"; MB<='0'; MA<='0'; FS<="011"; MD<='0'; K<="0000"; MW<='0'; wait until clock'event and clock='0';
		-- R3 <- R3 NAND R1 = 13 and 12 = 3 (FL=00__)
		DA<="11"; WR<='1'; AA<="11"; BA<="01"; MB<='0'; MA<='0'; FS<="100"; MD<='0'; K<="0000"; MW<='0'; wait until clock'event and clock='0';
		-- R1 <- R1 NOR R2 = 12 NOR 8 = 3   (FL=00__)
		DA<="01"; WR<='1'; AA<="01"; BA<="10"; MB<='0'; MA<='0'; FS<="101"; MD<='0'; K<="0000"; MW<='0'; wait until clock'event and clock='0';
		-- R1 <- SRA 8 = 12	(FL=0100)
		DA<="01"; WR<='1'; AA<="00"; BA<="00"; MB<='0'; MA<='1'; FS<="110"; MD<='0'; K<="1000"; MW<='0'; wait until clock'event and clock='0';
		-- R0 <- SL R0 = 2  (FL=0000)
		DA<="00"; WR<='1'; AA<="00"; BA<="00"; MB<='0'; MA<='0'; FS<="111"; MD<='0'; K<="0000"; MW<='0'; wait until clock'event and clock='0';
		
		-- M[R2] <- R1 = 12 	(D:8) (FL=____)
		DA<="00"; WR<='0'; AA<="10"; BA<="01"; MB<='0'; MA<='0'; FS<="000"; MD<='0'; K<="0000"; MW<='1'; wait until clock'event and clock='0';
		-- M[R3] <- 9  			(D:3)
		DA<="00"; WR<='0'; AA<="11"; BA<="00"; MB<='1'; MA<='0'; FS<="000"; MD<='0'; K<="1001"; MW<='1'; wait until clock'event and clock='0';

		-- R0 <- M[R2] = 12
		DA<="00"; WR<='1'; AA<="10"; BA<="00"; MB<='0'; MA<='0'; FS<="000"; MD<='1'; K<="0000"; MW<='0'; wait until clock'event and clock='0';
		-- R1 <- M[R3] = 9
		DA<="01"; WR<='1'; AA<="11"; BA<="00"; MB<='0'; MA<='0'; FS<="000"; MD<='1'; K<="0000"; MW<='0'; wait until clock'event and clock='0';
		
		-- R2 <- SL R1 = 2 (FL=0011)
		DA<="10"; WR<='1'; AA<="01"; BA<="00"; MB<='0'; MA<='0'; FS<="111"; MD<='0'; K<="0000"; MW<='0'; wait until clock'event and clock='0';

	end process;
	
	DP0: DataPath port map (DA, WR, AA, BA,	MB, MA, FS, MD, K, clock, MW, FL, D);
end behavior;