-------------------------------------------------------------------------------
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-22
-- Universidad Rey Juan Carlos
-- 
--
-- Simulation model for AS5133 SSI: Synchronous Serial Interface
-- http://ams.com/eng/Products/Magnetic-Position-Sensors/Linear-Position/AS5311
--


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
--use IEEE.NUMERIC_STD.ALL;

library WORK;
use WORK.PKG_FUN.ALL;
use WORK.PKG_FPGA.ALL;
use WORK.PKG_AS5133.ALL;


entity SSI_AS5133 is
  port(
    ssi_cs_n        : in  std_logic;
    ssi_clk         : in  std_logic;
    ssi_data        : out std_logic
  );
end SSI_AS5133;


architecture BEHAV of SSI_AS5133 is


--------- constants from the datasheet ----------------
  -- Data Output Activated (logic high)
  -- MAX time between falling edge of ssi_cs_n and data 
  --  output activated
  constant  t_do_active : time :=  100 ns;
  -- First data shifted to output register
  -- MIN time between falling edge of cs_n and first falling
  --   edge of ssi_clk
  constant  t_clk_fe : time   :=  500 ns;
  -- Start of data output
  -- MIN Rising edge of ssi_clk shift out one bit at a time
  constant  t_clk_2 : time    :=  500 ns;
  -- Data output valid
  -- MAX time between rising edge of ssi_clk and data output
  --   valid
  constant  t_do_valid : time :=  413 ns;
  -- Data output tristate
  -- MAX After the last bit DO changes back to tristate
  constant  t_do_tristate : time := 100 ns;
  -- Pulse width of CSn
  -- MIN. ssi_cs_n= '1'. To initiate read-out of next 
  --   angular (linear) position
  constant  t_cs_n : time    :=  500 ns;

  --constant  pos_data : std_logic_vector 
  --       (c_data_bits-1 downto 0) := "101011001010";

  --constant  mag_data : std_logic_vector 
  --       (c_data_bits-1 downto 0) := "001111000011";

  type positions_type is array (natural range<>) of
                    std_logic_vector(c_data_bits-1 downto 0);
  constant  pos_data : positions_type := (
         "000000000000",
         "000000000001",
         "000000000010",
         "000000000100",
         "000000001000",
         "000000010000",
         "000000100000",
         "000001000000",
         "000010000000",
         "000100000000",
         "001000000000",
         "010000000000",
         "100000000000",
         "000000000011",
         "000000000110",
         "000000001100",
         "000000011000",
         "000000110000",
         "000001100000",
         "000011000000",
         "000110000000",
         "001100000000",
         "011000000000",
         "110000000000",
         "111111111111"
  );

  --constant  status : std_logic_vector 
  --       (c_status_bits-2 downto 0) := "10110";

  -- parity bit not included
  type status_type is array (natural range<>) of
                    std_logic_vector(c_status_bits-2 downto 0);
  constant  status_data : status_type := (
         "00000",
         "00001",
         "00010",
         "00011",
         "00100",
         "00101",
         "00110",
         "00111",
         "01000",
         "01001",
         "01010",
         "01011",
         "01100",
         "01101",
         "01110",
         "01111",
         "10000",
         "10001",
         "10010",
         "10011",
         "10100",
         "10101",
         "10110",
         "10111",
         "11000"
  );


begin

--  P_SSI_AS5133: Process (ssi_cs_n, ssi_clk)
--  begin
--    if ssi_cs_n = '1' then
--      ssi_data <= 'Z';
--    else

  P_SSI_AS5133: Process 
    -- indicates if the position ('1') or 
    -- the magnetic field ('0') is going to be sent
    variable position_fieldn : std_logic;
    variable all_ssi_bits    : std_logic_vector 
         (c_ssi_bits-1 downto 0) ;
    variable parity : std_logic := '0';
    variable index  : natural := 0;
  begin
    parity := '0';
    ssi_data <= 'Z';
    wait until ssi_cs_n = '0';
    if ssi_clk = '1' then
      position_fieldn := '1';
      all_ssi_bits := pos_data(index) & status_data(index);
      wait for t_do_active;
      ssi_data <= '1';
      wait for t_clk_fe - t_do_active;
      wait until ssi_clk = '0';
      ssi_data <= '0';
      wait for t_clk_2;
    else
      position_fieldn := '0';
      all_ssi_bits := pos_data(index) & status_data(index);
      wait for t_do_active;
      ssi_data <= '0';
    end if;
    -- data to be sent
    for i in c_ssi_bits-1 downto 0 loop
      wait until ssi_clk = '1';  -- rising edge
      wait for t_do_valid;
      ssi_data <= all_ssi_bits(i);
      parity := parity XOR all_ssi_bits(i);
      --falling edge t_clk_2 after the rising edge
      wait for t_clk_2 - t_do_valid;
      -- assert reports when condition is not met
      assert ssi_clk = '1' report "clock prematurely low";
    end loop;
    wait until ssi_clk = '1';  -- rising edge
    -- parity bit
    wait for t_do_valid;
    -- Even parity bit, if we have had an odd number of 1s
    -- parity will be '1', so the total number of 1s will
    -- be even. 
    ssi_data <= parity;
    wait until ssi_cs_n = '1';
    wait for t_do_tristate;
    index := index + 1;
    if index = status_data'length -1 then
      wait;
    end if;
  end process;

end BEHAV;
    
