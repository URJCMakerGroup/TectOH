--
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-24
--
-- Module that displays the 8 7-segments displays. It receives 8 BCD numbers
-- with a dot and an enable to indicate if a particular one should be off
--
--  PORTS
--  -- signals from/to FPGA
--  rst             : in  std_logic; 
--                    Asynchronous reset of the circuit
--  clk             : in  std_logic;
--                    Clock of the FPGA board
--  bcd0            : in  std_logic_vector(nb_bcd-1 downto 0);
--                    binary number of 4 digits, not necesarily BCD
--  bcd1,...,8      : in  std_logic_vector(nb_bcd-1 downto 0);
--                    binary number of 4 digits, not necesarily BCD
--  dot_v           : in   std_logic_vector(c_7seg_units-1 downto 0);
--                    Array with the dots of the 7 segments. Active High
--  en_bcd          : in   std_logic_vector(c_7seg_units-1 downto 0);
--                    Enable for each of the 7 segments. Active High
--  d7an_sel        : out  std_logic_vector(c_7seg_units-1 downto 0);
--                    Anodes of each of the 7segments units. Used to select
--  d7cat_seg       : out  std_logic_vector(c_7seg_seg-1 downto 0);
--                    Common cathodes of the 7segments units.

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library WORK;
use WORK.PKG_FUN.ALL;
use WORK.PKG_FPGA.ALL;

entity DISP7SEG_8UNIT is
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
end DISP7SEG_8UNIT;


architecture RTL of DISP7SEG_8UNIT is

  -- each display will be on once every 1ms
  -- refresh peridod 8ms
  constant c_period_ns_7seg  : natural := 1000000; --1000000 ns: 1000 us: 1ms

  -- the end of the count for the frequency divider for the 7 segment time mux
  constant c_cnt_7seg_freq   : natural := div_redondea
                                          (c_period_ns_7seg, c_period_ns_fpga);
  constant nb_cnt_7seg_freq  : natural := log2i(c_cnt_7seg_freq) + 1;

  -- counter for the frequency divider for the 7-segment time mulitplexer
  signal  cnt_7seg_freq      : unsigned(nb_cnt_7seg_freq-1 downto 0);
  signal  end_cnt_7seg_freq  : std_logic;

  -- 
  signal  decod_7seg_units    : std_logic_vector(c_7seg_units-1 downto 0);
  signal  en_decod_7seg_units : std_logic_vector(c_7seg_units-1 downto 0);

  -- the 7 segments to be sent to the catodes
  signal  segments           : std_logic_vector(7-1 downto 0);

  signal  bcd7seg            : std_logic_vector (nb_bcd-1 downto 0);
  signal  dot7seg            : std_logic;

  -- I could have done this for the ports, but I am guessing that I may have
  -- problems with the synthesis or simulation
  type bcd_array_type is array (c_7seg_units-1 downto 0) of
                                std_logic_vector(nb_bcd-1 downto 0);

  signal bcd_array   : bcd_array_type;
  

begin

  bcd_array(0) <= bcd0;
  bcd_array(1) <= bcd1;
  bcd_array(2) <= bcd2;
  bcd_array(3) <= bcd3;
  bcd_array(4) <= bcd4;
  bcd_array(5) <= bcd5;
  bcd_array(6) <= bcd6;
  bcd_array(7) <= bcd7;


  -- Frequency divider for the 7 segment refresh
  P_7seg_freqdiv: Process (rst, clk)
  begin
    if rst = c_rst_on then
      cnt_7seg_freq <= (others => '0');
    elsif clk'event and clk = '1' then
      if end_cnt_7seg_freq = '1' then
        cnt_7seg_freq <= (others => '0');
      else
        cnt_7seg_freq <= cnt_7seg_freq + 1;
      end if;
    end if;
  end process;

  end_cnt_7seg_freq <= '1' when (cnt_7seg_freq =  c_cnt_7seg_freq -1) else
                       '0';

  -- 7 segment units decodification, instead of counting and decoding, just
  -- a circular shift register
  P_7seg_unit_count: Process (rst, clk)
  begin
    if rst = c_rst_on then
      decod_7seg_units <= (others => '0');
      decod_7seg_units(0) <= '1';
    elsif clk'event and clk = '1' then
      if end_cnt_7seg_freq = '1' then
        decod_7seg_units(c_7seg_units-1 downto 1) <= 
                                    decod_7seg_units(c_7seg_units-2 downto 0); 
        decod_7seg_units(0) <= decod_7seg_units(c_7seg_units-1);
      end if;
    end if;
  end process;


  -- Multiplexing the bcd that will be decoded into 7 segments
  --P_mux_bcd: Process ( bcd0,bcd1, bcd2, bcd3, bcd4, bcd5, bcd6, bcd7, dot_v,
  --                     decod_7seg_units)
  P_mux_bcd: Process (bcd_array, dot_v, decod_7seg_units)
  begin
    bcd7seg <= bcd_array(0);
    dot7seg <= dot_v(0);
    -- since it is decoded, I think is better to use if
    for i in 1 to (c_7seg_units-1) loop
      if decod_7seg_units(i) = '1' then
        bcd7seg <= bcd_array(i);
        dot7seg <= dot_v(i);
      end if;
    end loop;
  end process;

  -- seven segment decode
               --ABCDEFG
   segments  <= "1111110" when bcd7seg="0000" else  -- 0
                "0110000" when bcd7seg="0001" else  -- 1
                "1101101" when bcd7seg="0010" else  -- 2
                "1111001" when bcd7seg="0011" else  -- 3
                "0110011" when bcd7seg="0100" else  -- 4
                "1011011" when bcd7seg="0101" else  -- 5
                "1011111" when bcd7seg="0110" else  -- 6
                "1110000" when bcd7seg="0111" else  -- 7
                "1111111" when bcd7seg="1000" else  -- 8
                "1111011" when bcd7seg="1001" else  -- 9
                "1110111" when bcd7seg="1010" else  -- A
                "0011111" when bcd7seg="1011" else  -- B
                "1001110" when bcd7seg="1100" else  -- C
                "0111101" when bcd7seg="1101" else  -- D
                "1001111" when bcd7seg="1110" else  -- E
                "1000111"; --  bcd7seg="1111"       -- F

  -- enabling the BCD unit selection
  en_decod_7seg_units <= decod_7seg_units AND en_bcd;

  -- making the outputs high or low level depending on the FPGA
  -- the anode selects which display
  d7an_sel <= en_decod_7seg_units when c_an7seg_on = '1' else
              NOT en_decod_7seg_units;
  -- the catode indicates which segments are on
  d7cat_seg <= (segments & dot7seg) when c_ca7seg_on = '1' else
               NOT (segments & dot7seg);


end RTL;
  
