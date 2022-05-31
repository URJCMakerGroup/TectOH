--
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-29
--
-- Rising edge detector and filtering of the push buttons
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

entity RISINGEDGE_DETECT is
  port(
    rst             : in  std_logic;
    clk             : in  std_logic;
    btn             : in  std_logic_vector (c_btn-1 downto 0);
    btn_filter      : out std_logic_vector (c_btn-1 downto 0)
  );
end RISINGEDGE_DETECT;


architecture RTL of RISINGEDGE_DETECT is

  signal   btn_rg1           : std_logic_vector (c_btn-1 downto 0);
  signal   btn_rg2           : std_logic_vector (c_btn-1 downto 0);

  -- instead of having a counter from the clock period to the period we want
  -- for each button. We are going to have a counter from the clock period to
  -- 1ms, and then, each button will have a counter from 1 ms to 255 ms.
  -- For example:
  --    FPGA period: 10 ns
  --    1ms period: 1000000 ns
  --    count of 100000: 17 bits. Just one
  -- For each btn, we will count to 255: 8 bits. 
  --   So it will be (17 + 8xN) bits. If N= 5 => 57 
  -- Having a counter for each will be:
  --   a count of 25500000 will be 25 bits for each one: 25xN. => 125
  constant c_period_cnt1     : natural := 1000000;   -- 1 ms in ns
  -- For simulation, to shorten the time, coment this in synthesis
  --constant c_period_cnt1     : natural := 100;   -- 1 ms in ns

  constant c_cnt1            : natural := div_redondea 
                                          (c_period_cnt1, c_period_ns_fpga);
  constant nb_cnt1           : natural := log2i(c_cnt1-1) + 1;

  constant c_cnt2            : natural := 256;
  constant nb_cnt2           : natural := log2i(c_cnt2-1) + 1;

  signal   cnt1              : unsigned (nb_cnt1-1 downto 0);
  type     cnt2_array_type   is array (c_btn-1 downto 0) of 
                                       unsigned (nb_cnt2-1 downto 0);
  signal   end_cnt1          : std_logic;

  signal   cnt2              : cnt2_array_type;
  signal   counting2         : std_logic_vector (c_btn-1 downto 0);
  
  signal   btn_risedge       : std_logic_vector (c_btn-1 downto 0);
 

begin

  -- Registering the inputs. Array operations
  P_Reg: Process (rst, clk)
  begin
    if rst = c_rst_on then
      btn_rg1 <= (others => '0');
      btn_rg2 <= (others => '0');
    elsif clk'event and clk = '1' then
      btn_rg1 <= btn;
      btn_rg2 <= btn_rg1;
    end if;
  end process;

  -- this is an array operation, done bit by bit
  -- rising edge
  btn_risedge <= btn_rg1 AND (NOT btn_rg2);


  P_1st_count: Process (rst, clk)
  begin
    if rst = c_rst_on then
      cnt1 <= (others => '0');
    elsif clk'event and clk = '1' then
      if end_cnt1 = '1' then
        cnt1 <= (others => '0');
      else
        cnt1 <= cnt1 + 1;
      end if;
    end if;
  end process;

  end_cnt1 <= '1' when (cnt1 = c_cnt1 -1) else '0';

  -- This is a process that operates with cnt2: array of unsigned
  P_2st_count: Process (rst, clk)
  begin
    if rst = c_rst_on then
      cnt2 <= (others => (others => '0'));
      counting2 <= (others => '0');
    elsif clk'event and clk = '1' then
      for i in c_btn-1 downto 0 loop
        -- if counting, continue counting until the end
        if counting2(i) = '1' then
          if end_cnt1 = '1' then
            if cnt2(i) = c_cnt2-1 then
              cnt2(i) <= (others => '0');
              counting2(i) <= '0';
            else
              cnt2(i) <= cnt2(i) + 1;
            end if;
          end if;
        elsif btn_risedge(i) = '1' then  -- counting2(i) = '0'
          -- start counting (we will loose less than a milisecond, depending
          -- on where cnt1 is
          counting2(i) <= '1';      
        end if;
      end loop;
    end if;
  end process;
          
  -- this is an array operation, done bit by bit
  btn_filter <= btn_risedge AND (NOT counting2);

end RTL;
  
