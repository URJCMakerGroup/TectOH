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
  constant c_btn_on        : std_logic := '0'; -- XUPV2P
  constant c_btn_off       : std_logic := not c_btn_on; 
  
  -- c_freq_clk: shows the FPGA board clock frequency
  -- For Nexys2 clock freq is  50MHz -> 5*10**7;
  -- For XUPV2P clock freq is 100MHz -> 10**8;  
  --constant c_fpga_freq_clk  : natural   := 10**8; -- XUPV2P
  constant c_period_ns_fpga   : natural := 10;  -- ns

  -- el periodo del reloj en ns (pero de tipo natural)
  -- luego lo tendremos que multiplicar por un nanosegundo
  --constant c_fpga_period_ns_clk : natural := 10**9/c_fpga_freq_clk;
  

end PKG_FPGA;

