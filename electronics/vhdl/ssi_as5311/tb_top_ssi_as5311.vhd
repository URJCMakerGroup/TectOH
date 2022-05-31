-------------------------------------------------------------------------------
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-27
-- Universidad Rey Juan Carlos
-- 
--
-- Test bench for the top module that has:
--          - the interface to the AS5133 SSI 
--          - the control module
--          - the display
-- http://ams.com/eng/Products/Magnetic-Position-Sensors/Linear-Position/AS5311
--


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library WORK;
use WORK.PKG_FUN.ALL;
use WORK.PKG_FPGA.ALL;
use WORK.PKG_AS5133.ALL;


entity TB_TOP_SSI_AS5133 is
end TB_TOP_SSI_AS5133;

architecture TB of TB_TOP_SSI_AS5133 is


  component TOP_SSI_AS5133 is
    port(
      rst             : in  std_logic; 
      clk             : in  std_logic;
      sw              : in  std_logic_vector(c_sw-1 downto 0);
      -- push buttons
      btnu            : in  std_logic;
      btnd            : in  std_logic;
      btnr            : in  std_logic;
      btnl            : in  std_logic;
      btnc            : in  std_logic;
      -- LEDS
      led             : out std_logic_vector(c_leds-1 downto 0);
      -- 7 segments
      d7an_sel        : out std_logic_vector(c_7seg_units-1 downto 0);
      d7cat_seg       : out std_logic_vector(c_7seg_seg-1 downto 0);
      --en_n_stp        : out std_logic;
      dir_stp         : out std_logic;
      step_stp        : out std_logic;
      -- AS5133 SSI
      ssi_data_in     : in  std_logic; -- SSI data from AS5133
      ssi_cs_n_out    : out std_logic; -- chip select
      ssi_clk_out     : out std_logic  -- clk for the SSI
    );
  end component;


  component SSI_AS5133 is
    port(
      ssi_cs_n        : in  std_logic;
      ssi_clk         : in  std_logic;
      ssi_data        : out std_logic
    );
  end component;
  
  -- signals to be generated
  signal    rst               : std_logic;
  signal    clk               : std_logic;
  signal    sw                : std_logic_vector(c_sw-1 downto 0);
  signal    btnc              : std_logic;

  signal    btnu              : std_logic;  -- 16f
  signal    btnd              : std_logic;  -- 16b
  signal    btnr              : std_logic;  -- 1 step forward
  signal    btnl              : std_logic;  -- 1 step back
  -- signals to be read and be checked
  signal    led               : std_logic_vector(c_leds-1 downto 0);
  signal    d7an_sel          : std_logic_vector(c_7seg_units-1 downto 0);
  signal    d7cat_seg         : std_logic_vector(c_7seg_seg-1 downto 0);

  signal    dir_stp           : std_logic;
  signal    step_stp          : std_logic;

  -- signals between SSI_AS5133 and INTERF_SSI_AS5133
  signal    ssi_cs_n          : std_logic;
  signal    ssi_clk           : std_logic;
  signal    ssi_data          : std_logic;

  -- simulation ends
  signal    end_simul         : std_logic  := '0';

begin

  sw(c_sw-1 downto 1) <= (others => '0');

  I_AS5133: SSI_AS5133
    port map (
      ssi_cs_n        => ssi_cs_n,
      ssi_clk         => ssi_clk,
      ssi_data        => ssi_data
    );
  
  I_TOP_SSI_AS5133: TOP_SSI_AS5133
    port map (
      -- signals from/to inside FPGA
      rst             => rst,
      clk             => clk,
      sw              => sw, -- sw(0) pos_fieldn_in
      --btnd            => btnd, 
      btnu            => btnu,  -- 16 steps forward
      btnd            => btnd,  -- 16 steps back
      btnr            => btnr,  -- 1 step forward
      btnl            => btnl,  -- 1 step back
      btnc            => btnc,  -- init_milim
      led             => led,
      d7an_sel        => d7an_sel,
      d7cat_seg       => d7cat_seg,
      dir_stp         => dir_stp,
      step_stp        => step_stp,
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


  ------- process to generate the signals for control
  P_gen_control: Process
  begin
    wait until rst = c_rst_on;
    btnu <= '0'; 
    btnd <= '0'; 
    btnr <= '0'; 
    btnl <= '0'; 
    sw(0) <= '0'; --position_fieldn <= position;
    wait until rst = NOT c_rst_on;
    for i in 0 to 5 loop
      wait until clk = '1';
    end loop;
    wait for 7 ns;
    btnu <= '1'; 
    btnd <= '0'; 
    btnr <= '0'; 
    btnl <= '0'; 
    wait for 500 ns;
    btnu <= '0'; 
    btnd <= '1'; 
    btnr <= '0'; 
    btnl <= '0'; 
    wait for 500 ns;
    btnu <= '0'; 
    btnd <= '0'; 
    btnr <= '1'; 
    btnl <= '0'; 
    wait for 500 ns;
    btnu <= '0'; 
    btnd <= '0'; 
    btnr <= '0'; 
    btnl <= '1'; 
    wait for 500 ns;
    btnu <= '1'; 
    btnd <= '0'; 
    btnr <= '0'; 
    btnl <= '1'; 
    wait for 500 ns;
    btnu <= '0'; 
    btnd <= '0'; 
    btnr <= '0'; 
    btnl <= '0'; 
    wait for 1 ms;
    btnu <= '1'; 
    btnd <= '1'; 
    btnr <= '0'; 
    btnl <= '0'; 
    wait for 3 ms;
    btnu <= '0'; 
    btnd <= '0'; 
    btnr <= '1'; 
    btnl <= '1'; 
    wait for 3 ms;
    btnu <= '0'; 
    btnd <= '1'; 
    btnr <= '0'; 
    btnl <= '0'; 
    wait;
  end process;


  ------- process to generate the signals for the SSI interface
  P_gen_ssi_cmd: Process
    --variable position : std_logic := '0';
  begin
    btnc <= '0';  -- init_milim
    wait until rst = c_rst_on;
    btnc <= '0';  -- init_milim
    sw(0) <= '0'; --position_fieldn <= position;
    wait until rst = NOT c_rst_on;
    for i in 0 to 5 loop
      wait until clk = '1';
    end loop;
    sw(0) <= '1'; --position_fieldn <= position;
    btnc <= '1';  -- init_milim
    wait for 10 ms;
    sw(0) <= '0'; --position_fieldn <= position;
    btnc <= '1';  -- init_milim
    wait for 9 ms;
    sw(0) <= '0'; --position_fieldn <= position;
    btnc <= '0';  -- init_milim
    wait for 30 ms;
    sw(0) <= '1'; --position_fieldn <= position;
    btnc <= '0';  -- init_milim
    wait for 60 ms;
    end_simul <= '1';
    wait;
  end process;


end TB;
    
