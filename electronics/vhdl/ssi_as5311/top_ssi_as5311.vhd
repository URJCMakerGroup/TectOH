--
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-24
--

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library WORK;
use WORK.PKG_FUN.ALL;
use WORK.PKG_FPGA.ALL;
use WORK.PKG_AS5133.ALL;

entity TOP_SSI_AS5133 is
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
end TOP_SSI_AS5133;


architecture STRUCT of TOP_SSI_AS5133 is

  component DISP7SEG_8UNIT is
    port(
      rst             : in  std_logic; 
      clk             : in  std_logic;
      bcd0            : in  std_logic_vector(nb_bcd-1 downto 0);
      bcd1            : in  std_logic_vector(nb_bcd-1 downto 0);
      bcd2            : in  std_logic_vector(nb_bcd-1 downto 0);
      bcd3            : in  std_logic_vector(nb_bcd-1 downto 0);
      bcd4            : in  std_logic_vector(nb_bcd-1 downto 0);
      bcd5            : in  std_logic_vector(nb_bcd-1 downto 0);
      bcd6            : in  std_logic_vector(nb_bcd-1 downto 0);
      bcd7            : in  std_logic_vector(nb_bcd-1 downto 0);
      dot_v           : in  std_logic_vector(c_7seg_units-1 downto 0);
      en_bcd          : in  std_logic_vector(c_7seg_units-1 downto 0);
      d7an_sel        : out std_logic_vector(c_7seg_units-1 downto 0);
      d7cat_seg       : out std_logic_vector(c_7seg_seg-1 downto 0)
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

  component CONTROL_AS5133 is
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
  end component;
  
  component RISINGEDGE_DETECT is
    port(
      rst             : in  std_logic;
      clk             : in  std_logic;
      btn             : in  std_logic_vector (c_btn-1 downto 0);
      btn_filter      : out std_logic_vector (c_btn-1 downto 0)
    );
  end component;

  component STEPPER_CONTROL is
    port(
      rst             : in  std_logic;
      clk             : in  std_logic;
      stp_fwd         : in  std_logic;
      stp_bck         : in  std_logic;
      stp_16fwd       : in  std_logic;
      stp_16bck       : in  std_logic;
      --en_n_stp        : out std_logic;
      dir_stp         : out std_logic;
      step_stp        : out std_logic
    );
  end component;


  constant c_dot_v     : std_logic_vector := "00000000";
  constant c_bcd_zero  : std_logic_vector(nb_bcd-1 downto 0) := (others =>'0');

  signal   en_bcd      : std_logic_vector (c_7seg_units-1 downto 0);

  signal   cnt_milim   : std_logic_vector (nb_cnt_mm-1 downto 0);

  signal   ssi_avail   : std_logic;
  signal   data_ready  : std_logic;
  signal   parity_ok   : std_logic;
  signal   data_interf_ssi  : std_logic_vector(c_data_bits-1 downto 0);
  signal   pos_data    : std_logic_vector(c_data_bits-1 downto 0);
  signal   field_data  : std_logic_vector(c_data_bits-1 downto 0);
  signal   status_interf_ssi  : std_logic_vector(c_status_bits-1 downto 0);
  signal   read_ssi    : std_logic;
  signal   pos_fieldn  : std_logic;

  signal   btn_in      : std_logic_vector(c_btn-1 downto 0);
  signal   btn_filter  : std_logic_vector(c_btn-1 downto 0);
  -- filtered push buttons
  signal   btnc_f      : std_logic;
  signal   btnu_f      : std_logic;
  signal   btnl_f      : std_logic;
  signal   btnr_f      : std_logic;
  signal   btnd_f      : std_logic;

begin

  btn_in (0)  <= btnc;
  btn_in (1)  <= btnu;
  btn_in (2)  <= btnl;
  btn_in (3)  <= btnr;
  btn_in (4)  <= btnd;

  btnc_f   <= btn_filter(0);
  btnu_f   <= btn_filter(1);
  btnl_f   <= btn_filter(2);
  btnr_f   <= btn_filter(3);
  btnd_f   <= btn_filter(4);

  led(0) <= parity_ok;
  led (c_status_bits downto 1) <= status_interf_ssi;
  led (c_leds-1 downto c_status_bits +1) <= (others => NOT c_led_on);
 
  en_bcd   <= "11111111";

  I_DISP7SEG_8UNIT: DISP7SEG_8UNIT
    port map (
      rst          => rst,
      clk          => clk,
      bcd0         => pos_data (3 downto 0),
      bcd1         => pos_data (7 downto 4),
      bcd2         => pos_data (11 downto 8),
      bcd3         => cnt_milim (3 downto 0),
      bcd4         => cnt_milim (7 downto 4),
      bcd5         => field_data (3 downto 0),
      bcd6         => field_data (7 downto 4),
      bcd7         => field_data (11 downto 8),
      dot_v        => c_dot_v,
      en_bcd       => en_bcd,
      d7an_sel     => d7an_sel,
      d7cat_seg    => d7cat_seg
    );

  I_CONTROL_AS5133: CONTROL_AS5133
    port map (
      -- signals from/to FPGA
      rst             => rst,
      clk             => clk,
      init_milim      => btnc_f,  -- central push button to initialize
      pos_fieldn_in   => sw(0),
      cnt_milim_out   => cnt_milim,
      pos_data_out    => pos_data,
      field_data_out  => field_data,
      -- Signals to/from interfase ssi
      ssi_avail       => ssi_avail,
      data_ready      => data_ready,
      parity_ok       => parity_ok,
      data_ssi        => data_interf_ssi,
      status_ssi      => status_interf_ssi,
      pos_fieldn_2ssi => pos_fieldn,
      read_ssi        => read_ssi
    );
 
  I_INTERF_SSI_AS5133: INTERF_SSI_AS5133
    port map (
      -- signals from/to inside FPGA
      rst             => rst,
      clk             => clk,
      read_ssi        => read_ssi,
      position_fieldn => pos_fieldn,
      ssi_avail       => ssi_avail,
      data_ready      => data_ready,
      parity_ok       => parity_ok,
      data_reg        => data_interf_ssi,
      status_reg      => status_interf_ssi,
      -- Signals to/from AS5133 SSI 
      ssi_data_in     => ssi_data_in,
      ssi_cs_n_out    => ssi_cs_n_out,
      ssi_clk_out     => ssi_clk_out
    );

  I_RISINGEDGE_DETECT: RISINGEDGE_DETECT 
    port map (
      rst             => rst,
      clk             => clk,
      btn             => btn_in,
      btn_filter      => btn_filter
    );


  I_STEPPER_CONTROL: STEPPER_CONTROL 
    port map(
      rst             => rst,
      clk             => clk,
      stp_fwd         => btnr_f,
      stp_bck         => btnl_f,
      stp_16fwd       => btnu_f,
      stp_16bck       => btnd_f,
      --en_n_stp        : out std_logic;
      dir_stp         => dir_stp,
      step_stp        => step_stp
    );

end STRUCT;
  
