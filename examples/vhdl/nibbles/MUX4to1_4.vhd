library ieee;
use ieee.std_logic_1164.all;

entity MUX4to1_4 is
	port(	A: in std_logic_vector(3 downto 0);
		B: in std_logic_vector(3 downto 0);
		C: in std_logic_vector(3 downto 0);
		D: in std_logic_vector(3 downto 0);
		S: in std_logic_vector(1 downto 0);
		O: out std_logic_vector(3 downto 0));
end MUX4to1_4;

architecture structural of MUX4to1_4 is
begin
O <=	A when S="00" else
		B when S="01" else
		C when S="10" else
		D when S="11" else
		"XXXX";
end structural;
