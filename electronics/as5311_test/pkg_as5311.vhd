--------------------------------------------------------------------------------
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-21
-- Universidad Rey Juan Carlos
-- 
-- Package of constants for the AS5133

library IEEE;
use IEEE.STD_LOGIC_1164.all;

-- las funciones tienen que estar en otro paquete para poder usarlo con modelsim
library WORK;
use WORK.PKG_FUN.all;
use WORK.PKG_FPGA.all;


package PKG_AS5133 is

  -- Max frequency for the AS5133 SSI
  --constant c_ssi_freq            : natural   := 10**6; -- SSI AS5133
  -- the period
  --constant c_ssi_period_ns      : natural   := 10**9/c_freq_ssi;

  -- the higher count for the frequence divisor for the SSI clock
  --constant c_cnt_ssiclk  : natural :=
  --                          div_redondea (c_fpga_freq_clk, c_ssi_freq);
  
  -- the number of bits (nb) necesary to represent c_cnt_ssiclk
  --constant c_nb_cnt_ssiclk : natural := log2i(c_cnt_ssiclk) + 1;

  -- depending on the FPGA buttons
  constant c_rst_on        : std_logic := c_btn_on;

  -- number of bits for the data: either the position or the magnetic field
  constant c_data_bits       : natural := 12;   
  -- number of bits for the status reg:
  -- Status regs are:
  -- status_reg[0] : Parity bit
  -- status_reg[1] : Mag DEC 
  -- status_reg[2] : Mag INC
  -- status_reg[3] : LIN
  -- status_reg[4] : COF
  -- status_reg[5] : OCF
  constant c_status_bits     : natural := 6;  

  -- the number of bits received (without the parity bit)
  -- 12 + 6 = 18, but not considering the parity bit: 17
  constant c_ssi_bits        : natural := c_data_bits + c_status_bits - 1;
  -- this is the number of bits we need to represent c_ssi_bits
  constant c_nb_ssi_bits     : natural := log2i(c_ssi_bits) + 1; 
  

end PKG_AS5133;

