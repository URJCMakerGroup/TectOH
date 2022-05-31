--------------------------------------------------------------------------------
-- Felipe Machado Sanchez, felipe.machado@urjc.es
-- Universidad Rey Juan Carlos
-- 
-- Pakage for the FPGA constants

library IEEE;
use IEEE.STD_LOGIC_1164.all;

library WORK;
--use WORK.PKG_FUNCTION.all;


package PKG_FPGA is

  -- c_btn_on: shows if the logic of the push butons of the board is 
  --           low or high. So if I push (on) I get '1' or '0'
  -- if it is '1' means that is direct logica ->  NEXYS2 Board
  -- if it is '0' means that is reversed logic ->  XUPV2P Board
  constant c_btn_on        : std_logic := '1'; -- NEXYS4
  constant c_btn_off       : std_logic := not c_btn_on; 

  constant c_led_on        : std_logic := '1'; -- NEXYS4

  -- the NEXYS4 has a reset button, to be used for the FPGA, and can be 
  -- used for any purpose 
  constant c_rst_on    : std_logic := '0'; 
  
  -- The level of the anode of the 7 segment to be iluminated
  constant c_an7seg_on     : std_logic := '0'; -- Nexys4
  -- The level of the anode of the 7 segment to be iluminated
  constant c_ca7seg_on     : std_logic := '0'; -- Nexys4

  -- c_freq_clk: shows the FPGA board clock frequency
  -- For Nexys2 clock freq is  50MHz -> 5*10**7;
  -- For XUPV2P clock freq is 100MHz -> 10**8;  
  --constant c_fpga_freq_clk  : natural   := 10**8; -- XUPV2P
  constant c_period_ns_fpga   : natural := 10;  -- ns

  -- number of bits of a BCD number
  constant nb_bcd             : natural := 4;

  -- number of 7-segments displays
  constant c_7seg_units       : natural := 8;  -- Nexys 4
  -- number of 7-segments segments. 8: if includes the dot
  constant c_7seg_seg         : natural := 8;  -- Nexys 4

  -- number of switches in the FPGA
  constant c_sw              : natural := 16;  -- Nexys 4

  -- number of LEDs
  constant c_leds            : natural := 16;  -- Nexys 4

  -- el periodo del reloj en ns (pero de tipo natural)
  -- luego lo tendremos que multiplicar por un nanosegundo
  --constant c_fpga_period_ns_clk : natural := 10**9/c_fpga_freq_clk;
  

end PKG_FPGA;

