--
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-29
--
-- Test bench of the rising edge detector and filtering of the push buttons
--
--  PORTS
--  -- signals from/to FPGA
--  rst             : in  std_logic; 
--                    Asynchronous reset of the circuit
--  clk             : in  std_logic;
--                    Clock of the FPGA board
--  btn             : in  std_logic_vector (c_btn-1 downto 0);
--                    vector of the push buttons of the FPGA
--  btn_filter      : out std_logic_vector (c_btn-1 downto 0);
--                    vector of the push buttons of the FPGA

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library WORK;
use WORK.PKG_FUN.ALL;
use WORK.PKG_FPGA.ALL;

entity TB_RISINGEDGE_DETECT is
end TB_RISINGEDGE_DETECT;


architecture TB of TB_RISINGEDGE_DETECT is

  component RISINGEDGE_DETECT is
    port(
      rst             : in  std_logic;
      clk             : in  std_logic;
      btn             : in  std_logic_vector (c_btn-1 downto 0);
      btn_filter      : out std_logic_vector (c_btn-1 downto 0)
    );
  end component;

  signal    rst               : std_logic;
  signal    clk               : std_logic;
  signal    btn               : std_logic_vector (c_btn-1 downto 0);
  signal    btn_filter        : std_logic_vector (c_btn-1 downto 0);

  signal   end_simul           : std_logic;
 

begin

  I_RISINGEDGE_DETECT: RISINGEDGE_DETECT
    port map (
      rst             => rst,
      clk             => clk,
      btn             => btn,
      btn_filter      => btn_filter
    );

  -------- clock process
  P_clk: Process
  begin
    clk <= '1';
    wait for (c_period_ns_fpga/2) * 1 ns;
    clk <= '0';
    wait for (c_period_ns_fpga/2) * 1 ns;
    if end_simul = '1' then
      wait;
    end if;
  end process;

  -------- reset process
  P_rst: Process
  begin
    rst <= NOT c_rst_on;
    wait for (3*c_period_ns_fpga + 1) * 1 ns;
    rst <= c_rst_on;
    wait for (2*c_period_ns_fpga + 1) * 1 ns;
    rst <= NOT c_rst_on;
    wait;
  end process;


  ------- process to generate the signals for the SSI interface
  P_gen_btn: Process
  begin
    end_simul <= '0';
    btn <= "010000";
    wait until rst = c_rst_on;
    wait until rst = NOT c_rst_on;
    for i in 0 to 5 loop
      wait until clk = '1';
    end loop;
    btn <= "001000";
    wait for 3 ns;
    btn <= "001100";
    wait for 3 ns;
    btn <= "001110";
    wait for 3 ns;
    btn <= "001000";
    wait for 3 ns;
    btn <= "001100";
    wait for 3 ns;
    btn <= "101110";
    wait for 3 ns;
    btn <= "101000";
    wait for 300 ns;
    btn <= "101101";
    wait for 300 ns;
    btn <= "101111";
    wait for 300 ns;
    btn <= "101001";
    wait for 300 ns;
    btn <= "101101";
    wait for 100 ms;  -- ms
    btn <= "101110";
    wait for 3 ns;
    btn <= "101000";
    wait for 300 ns;
    btn <= "101101";
    wait for 300 ns;
    btn <= "101111";
    wait for 300 ns;
    btn <= "101001";
    wait for 100 ms;  -- ms
    btn <= "101110";
    wait for 3 ns;
    btn <= "101000";
    wait for 300 ns;
    btn <= "101101";
    wait for 300 ns;
    btn <= "101111";
    wait for 300 ns;
    btn <= "101001";
    wait for 100 ms;  -- ms
    btn <= "000000";  -- "00000
    wait for 20 ns;
    btn <= "000111"; 
    wait for 100 ns;
    btn <= "000001"; 
    wait for 100 ns;
    btn <= "000111"; 
    wait for 60 ns;
    btn <= "000001"; 
    wait for 40 ns;
    btn <= "000110"; 
    wait for 60 ns;
    btn <= "000001"; 
    wait for 40 ns;
    btn <= "000010"; 
    wait for 40 ns;
    btn <= "000111"; 
    wait for 100 ms;  -- ms
    btn <= "000111"; 
    wait for 100 ms;  -- ms
    btn <= "001000"; 
    wait for 80 ns; 
    btn <= "000001";  -- "00000
    wait for 100 ns;
    btn <= "000111"; 
    wait for 10 ns; 
    btn <= "010001";  
    wait for 10 ns;  
    btn <= "111111"; 
    wait for 800 ns;  
    btn <= "010101"; 
    wait for 800 ns;  
    btn <= "000001"; 
    wait for 400 ms;
    end_simul <= '1';
    wait for 30 ns;  
    wait;
  end process;




end TB;
  
