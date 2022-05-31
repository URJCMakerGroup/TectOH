--
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-22
--
-- Module that controls the interface of the AS5133 (INTERF_SSI_AS5133)
-- and calculates the position
-- http://ams.com/eng/Products/Magnetic-Position-Sensors/Linear-Position/AS5311
--
--  PORTS
--  -- signals from/to FPGA
--  rst             : in  std_logic; 
--                    Asynchronous reset of the circuit
--  clk             : in  std_logic;
--                    Clock of the FPGA board
--  init_milim      : in  std_logic;
--                    Initialization of the milimiter count. To set the zero
--  pos_fieldn_in   : in  std_logic;
--                    Input that determines if we are requesting the position
--                    or the magnetic field to the SSI, it will then be 
--                    requested to the AS5133 SSI
--  cnt_milim_out   : out std_logic_vector (nb_cnt_mm-1 downto 0);
--                    The milimiter count. Now it is 2 mm step
--  ssi_avail       : in std_logic;
--                    Indicates if the ssi is available ('1') to receive data
--                    or if it is busy ('0') receiving data
--  data_ready      : in std_logic;
--                    Indicates that the data_ssi and status_ssi are ready
--                    It is just a clock cycle
--  parity_ok       : in std_logic. '1' if the even parity bit from the SSI is
--                    the same as the computed (signal parity_calc)
--                    '0': if they don´t match
--  data_ssi        : in std_logic_vector(c_data_bits-1 downto 0);
--                    Data from the AS5311 SSI, could be the linear position
--                    or the magnetic field. Depending on what was requested
--                    by postion_fieldn
--  status_ssi      : in std_logic_vector(c_status_bits-1 downto 0);
--                    Status bits from the transmision of the AS5133 SSI
--                    or the magnetic field. Depending on what was requested
--                    by postion_fieldn
--                    Status regs are:
--                    status_ssi[0] : Parity bit
--                    status_ssi[1] : Mag DEC 
--                    status_ssi[2] : Mag INC
--                    status_ssi[3] : LIN
--                    status_ssi[4] : COF
--                    status_ssi[5] : OCF
--  pos_fieldn_2ssi : out std_logic;
--                    The same signal from pos_fieldn_in that will be sent
--                    to the AS5133 SSI to either ask for the position or the
--                    magnetic field strength
--  read_ssi        : out  std_logic;
--                    request for a new reading of the AS5133, a pulse is enough
--

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library WORK;
use WORK.PKG_FUN.ALL;
use WORK.PKG_FPGA.ALL;
use WORK.PKG_AS5133.ALL;

entity CONTROL_AS5133 is
  port(
    -- signals from/to FPGA
    rst             : in  std_logic;
    clk             : in  std_logic;
    init_milim      : in  std_logic;
    pos_fieldn_in   : in  std_logic;  -- from a switch
    cnt_milim_out   : out std_logic_vector (nb_cnt_mm-1 downto 0);
    pos_data_out    : out std_logic_vector (c_data_bits-1 downto 0);
    field_data_out  : out std_logic_vector (c_data_bits-1 downto 0);

    -- Signals to/from interfase ssi
    ssi_avail       : in  std_logic;
    data_ready      : in  std_logic;
    parity_ok       : in  std_logic;
    data_ssi        : in  std_logic_vector(c_data_bits-1 downto 0);
    status_ssi      : in  std_logic_vector(c_status_bits-1 downto 0);
    pos_fieldn_2ssi : out std_logic;
    read_ssi        : out std_logic
  );
end CONTROL_AS5133;


architecture RTL of CONTROL_AS5133 is

 
  signal    cnt_milim       : signed (nb_cnt_mm-1 downto 0);

  -- The least significant bit is used to calculate when a zero crossing
  -- has happened. But maybe we don´t want to use all the least significant
  -- bits because maybe some positions are skipped if it is going too fast.
  -- It seems that going around 25mm/s you don´t skip any position of 488nm
  constant  c_lsbcomp_datssi  : natural := 2;
  signal    pos_ssi_rg       : std_logic_vector(c_data_bits-1 downto 0);
  signal    field_ssi_rg     : std_logic_vector(c_data_bits-1 downto 0);
  signal    milim_less       : std_logic;
  signal    milim_more       : std_logic;
  signal    read_ssi_aux     : std_logic;
  signal    pos_fieldn_rg    : std_logic;
  signal    valid_pos_ssi_rg : std_logic;

begin

  -- Process that continously ask for new data to the SSI
  P_SSI_CONTROL: Process (rst, clk)
  begin
    if rst = c_rst_on then
      read_ssi_aux  <= '0';
      pos_fieldn_rg <= '1';
    elsif clk'event and clk = '1' then
      read_ssi_aux <= '0';
      -- ask for a new data, just a clock cycle
      if ssi_avail = '1' AND read_ssi_aux = '0' then  
        read_ssi_aux <= '1';
        -- to keep the value of pos_fieldn that was asked 
        pos_fieldn_rg <= pos_fieldn_in;
      end if;
    end if;
  end process;

  read_ssi <= read_ssi_aux;

  pos_fieldn_2ssi <= pos_fieldn_rg;

  cnt_milim_out <= std_logic_vector(cnt_milim);

  -- Process that registers the data coming from the SSI, so it can be compared
  -- with the previous one. To see if it has been a cross over zero, in any 
  -- way (from 4095 to 0, or from 0 to 4095)
  P_reg_data_ssi: Process (rst, clk)
  begin
    if rst = c_rst_on then
      pos_ssi_rg   <= (others => '0');
      field_ssi_rg <= (others => '0');
      -- indicates if there is pos_ssi_rg is valid
      valid_pos_ssi_rg <= '0';  
    elsif clk'event and clk = '1' then
      -- data_ready is a pulse. 
      -- pos_field_rg is checked to see if it is a position data and not a
      -- magnetic field data. If it is a magnetic field data it will not be
      -- registered
      if data_ready = '1' then
        if pos_fieldn_rg = '1' then  
          pos_ssi_rg  <= data_ssi;
          valid_pos_ssi_rg <= '1';  
        else
          field_ssi_rg <= data_ssi;
        end if;
      end if;
    end if;
  end process;

  field_data_out <= field_ssi_rg;
  pos_data_out   <= pos_ssi_rg;

  -- process that generates the signals for tracking the change in milimiters
  P_milim_change: Process (data_ready, pos_fieldn_rg, pos_ssi_rg, data_ssi,
                           valid_pos_ssi_rg)
  begin
    milim_more <= '0';
    milim_less <= '0';
    if data_ready = '1' and pos_fieldn_rg = '1' then
      if valid_pos_ssi_rg = '1' then  
        if (data_ssi(c_data_bits-1 downto c_lsbcomp_datssi) = 
            (c_data_bits-1 downto c_lsbcomp_datssi => '0'))
               -- (others=>'0')) -- Not allowed
            and	
            (pos_ssi_rg(c_data_bits-1 downto c_lsbcomp_datssi) =
                (c_data_bits-1 downto c_lsbcomp_datssi => '1')) --(others=>'1'))
            then
          milim_more <= '1';
        end if;
        if (data_ssi(c_data_bits-1 downto c_lsbcomp_datssi) = 
            (c_data_bits-1 downto c_lsbcomp_datssi => '1')) --(others=>'1'))
            and
           (pos_ssi_rg(c_data_bits-1 downto c_lsbcomp_datssi) =
               (c_data_bits-1 downto c_lsbcomp_datssi => '0')) --(others=>'0'))
            then
          milim_less <= '1';
        end if;
      end if;
    end if;
  end process;
 

  -- Process that counts the milimiters
  P_count_milimiters: Process (rst, clk)
  begin
    if rst = c_rst_on then
      cnt_milim <= (others => '0');
    elsif clk'event and clk = '1' then
      if init_milim = '1' then
        cnt_milim <= (others => '0');
      elsif milim_more = '1' then
        cnt_milim <= cnt_milim + 1;
      elsif milim_less = '1' then
        cnt_milim <= cnt_milim - 1;
      end if;
    end if;
  end process;
      
end RTL;
  
