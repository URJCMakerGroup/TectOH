-------------------------------------------------------------------------------
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-22
-- Universidad Rey Juan Carlos
-- 
--
-- Test bench for the interface to the AS5133 SSI: Synchronous Serial Interface
-- http://ams.com/eng/Products/Magnetic-Position-Sensors/Linear-Position/AS5311
--


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library WORK;
use WORK.PKG_FUN.ALL;
use WORK.PKG_FPGA.ALL;
use WORK.PKG_AS5133.ALL;


entity TB_INTERF_SSI_AS5133 is
end TB_INTERF_SSI_AS5133;

architecture TB of TB_INTERF_SSI_AS5133 is

  component SSI_AS5133 is
    port(
      ssi_cs_n        : in  std_logic;
      ssi_clk         : in  std_logic;
      ssi_data        : out std_logic
    );
  end component;
  
  component INTERF_SSI_AS5133 is
    port(
      -- signals from/to FPGA
      rst             : in  std_logic; 
      clk             : in  std_logic;
      read_ssi        : in  std_logic;
      position_fieldn : in  std_logic;
      ssi_avail       : out std_logic;
      data_ready      : out std_logic;
      parity_ok       : out std_logic;
      data_reg        : out std_logic_vector(c_data_bits-1 downto 0);
      status_reg      : out std_logic_vector(c_status_bits-1 downto 0);
      -- Signals to/from ssi
      ssi_data_in     : in  std_logic; -- SSI data from AS5133
      ssi_cs_n_out    : out std_logic; -- chip select
      ssi_clk_out     : out std_logic  -- clk for the SSI
    );
  end component;

  -- signals to be generated
  signal    rst               : std_logic;
  signal    clk               : std_logic;
  signal    read_ssi          : std_logic;
  signal    position_fieldn   : std_logic;
  -- signals to be read and be checked
  signal    ssi_avail         : std_logic;
  signal    data_ready        : std_logic;
  signal    parity_ok         : std_logic;
  signal    data_reg          : std_logic_vector(c_data_bits-1 downto 0);
  signal    status_reg        : std_logic_vector(c_status_bits-1 downto 0);


  -- signals between SSI_AS5133 and INTERF_SSI_AS5133
  signal    ssi_cs_n          : std_logic;
  signal    ssi_clk           : std_logic;
  signal    ssi_data          : std_logic;

  -- simulation ends
  signal    end_simul         : std_logic  := '0';

begin


  I_AS5133: SSI_AS5133
    port map (
      ssi_cs_n        => ssi_cs_n,
      ssi_clk         => ssi_clk,
      ssi_data        => ssi_data
    );
  
  I_INTERF_SSI_AS5133: INTERF_SSI_AS5133
    port map (
      -- signals from/to inside FPGA
      rst             => rst,
      clk             => clk,
      read_ssi        => read_ssi,
      position_fieldn => position_fieldn,
      ssi_avail       => ssi_avail,
      data_ready      => data_ready,
      parity_ok       => parity_ok,
      data_reg        => data_reg,
      status_reg      => status_reg,
      -- Signals to/from AS5133 SSI 
      ssi_data_in     => ssi_data,
      ssi_cs_n_out    => ssi_cs_n,
      ssi_clk_out     => ssi_clk
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
  P_gen_ssi_cmd: Process
  begin
    wait until rst = c_rst_on;
    wait until rst = NOT c_rst_on;
    for i in 0 to 5 loop
      wait until clk = '1';
    end loop;
    --------------- first read, position
    read_ssi <= '1';
    position_fieldn <= '1';
    wait until clk = '1';
    read_ssi <= '0';
    wait until data_ready = '1';
    -- check for data_reg and status_reg
    wait until ssi_avail = '1';
    --------------- second read, magnetic field
    read_ssi <= '1';
    position_fieldn <= '0';
    wait until clk = '1';
    --read_ssi <= '0';  to see what happens
    wait until data_ready = '1';
    read_ssi <= '0';
    -- check for data_reg and status_reg
    wait until ssi_avail = '1';
    for i in 0 to 5 loop
      wait until clk = '1';
    end loop;
    end_simul <= '1';
    wait;
  end process;


end TB;
    
