--
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-29
--
-- Direction and steps for the stepper motor
--
--  PORTS
--  -- signals from/to FPGA
--  rst             : in  std_logic; 
--                    Asynchronous reset of the circuit
--  clk             : in  std_logic;
--                    Clock of the FPGA board
--  stp_fwd         : in  std_logic;
--                    To make just a step in one direction. A pulse is required
--  stp_bck         : in  std_logic;
--                    To make just a step in the other direction. A pulse
--  stp16_fwd       : in  std_logic;
--                    To make 10 steps in one direction. A pulse is required
--  stp16_bck       : in  std_logic;
--                    To make 10 steps in the other direction. A pulse
--  Signals to the Pololu driver, A4988 or DRV8825
--  en_n_stp        : out std_logic;  
--                    enable signal for the stepper. Low level enable is on
--  dir_stp         : out std_logic;  
--  step_stp        : out std_logic;  

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library WORK;
use WORK.PKG_FUN.ALL;
use WORK.PKG_FPGA.ALL;

entity STEPPER_CONTROL is
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
end STEPPER_CONTROL;


architecture RTL of STEPPER_CONTROL is

  signal   stp_fwd_rg        : std_logic;
  signal   stp_bck_rg        : std_logic;
  signal   stp_16fwd_rg        : std_logic;
  signal   stp_16bck_rg        : std_logic;

  -- period of the motor, if done continously, it will define the speed
  constant c_period_stp       : natural := 100000;  -- 1 ms, in ns

  constant c_cnt_stp          : natural := div_redondea
                                          (c_period_stp, c_period_ns_fpga);
  constant nb_cnt_stp         : natural := log2i(c_cnt_stp-1)+1;

  constant c_cnt_half_stp     : natural := c_cnt_stp;

  signal   cnt_stp            : unsigned (nb_cnt_stp-1 downto 0);

  signal   end_cnt_stp        : std_logic;
  signal   half_cnt_stp       : std_logic;
  signal   active_st           : std_logic;
  signal   s_setup            : std_logic;
  signal   start_moving       : std_logic;
  signal   start_hold         : std_logic;
  signal   moving16           : std_logic;
  signal   moving1            : std_logic;
  signal   end_moving         : std_logic;

  -- time necessary to change MS1, MS2, MS3, RESETN, or DIR on the 
  -- Allegro A4988
  constant  c_setup_time      : natural := 200;  -- 200 ns
  constant  c_hold_time       : natural := 200;  -- 200 ns

  constant c_cnt_setup        : natural := div_redondea
                                           (c_setup_time, c_period_ns_fpga);
  constant c_cnt_hold                  : natural := div_redondea
                                           (c_hold_time, c_period_ns_fpga);
  constant nb_cnt_setup       : natural := log2i(c_cnt_setup-1)+1;
  constant nb_cnt_hold        : natural := log2i(c_cnt_hold-1)+1;

  signal   cnt_setup          : unsigned (nb_cnt_setup-1 downto 0);
  signal   cnt_hold           : unsigned (nb_cnt_hold-1 downto 0);

  signal   end_cnt_setup      : std_logic;
  signal   end_cnt_hold       : std_logic;
                                           
  -- count of 16 steps
  constant c_cnt_16stp        : natural := 16;
  signal   end_cnt_16stp      : std_logic;
  -- just a clock cycle
  signal   end_cnt_16stp_pls  : std_logic;
  signal   cnt_16stp          : unsigned (4-1 downto 0);

  type moving_states is (IDLE_ST, SETUP_ST, MOVING_ST, HOLD_ST);
  signal mov_pres_st, mov_next_st  : moving_states;

begin

  P_SEQ_FSM_MOV: Process (rst, clk) 
  begin 
    if rst = c_rst_on then 
      mov_pres_st <= IDLE_ST; 
    elsif clk'event and clk='1' then 
      mov_pres_st <= mov_next_st; 
    end if; 
  end process; 

  P_COMB_FSM_MOV: Process (mov_pres_st, mov_next_st, stp_fwd, stp_bck, 
                           stp_16fwd, stp_16bck, end_cnt_setup, end_moving,
                           end_cnt_hold)
  begin
    case mov_pres_st is
      when IDLE_ST => 
        mov_next_st <= mov_pres_st;
        if stp_fwd   = '1' OR stp_bck   = '1' OR 
           stp_16fwd = '1' OR stp_16bck = '1' then
          mov_next_st <= SETUP_ST;
        end if;
      when SETUP_ST =>
        mov_next_st <= mov_pres_st;
        if end_cnt_setup = '1' then
          mov_next_st <= MOVING_ST;
        end if;
      when MOVING_ST =>
        mov_next_st <= mov_pres_st;
        if end_moving = '1' then
          mov_next_st <= HOLD_ST;
        end if;
      when HOLD_ST =>
        mov_next_st <= mov_pres_st;
        if end_cnt_hold = '1' then
          mov_next_st <= IDLE_ST;
        end if;
    end case;
  end process;           

 P_COMB_FSM_OUT: Process (mov_pres_st, mov_next_st)
  begin
    case mov_pres_st is
      when IDLE_ST => 
        active_st <= '0';
      when SETUP_ST =>
        active_st <= '1';
      when MOVING_ST =>
        active_st <= '1';
      when HOLD_ST =>
        active_st <= '1';
    end case;
  end process;           

  P_Reg: Process (rst, clk)
  begin
    if rst = c_rst_on then
      stp_fwd_rg   <= '0';
      stp_bck_rg   <= '0';
      stp_16fwd_rg <= '0';
      stp_16bck_rg <= '0';
    elsif clk'event and clk = '1' then
      if active_st = '1' then
        -- Here, we don't care about the inptus stp_fwd, ...
        if stp_fwd_rg = '1' and end_cnt_stp = '1' then
          stp_fwd_rg <= '0';
        elsif stp_bck_rg = '1' and end_cnt_stp = '1' then
          stp_bck_rg <= '0';
        elsif stp_16fwd_rg = '1' and end_cnt_16stp = '1' then
          stp_16fwd_rg <= '0';
        elsif stp_16bck_rg = '1' and end_cnt_16stp = '1' then
          stp_16bck_rg <= '0';
        end if;
      else   -- idle, check if there is any command
        -- it has to be a precedence, but it doesn´t matter who. It is 
        -- unlikely to happen at the same time
        if stp_fwd = '1' then
          stp_fwd_rg <= '1';
        elsif stp_bck = '1' then
          stp_bck_rg <= '1';
        elsif stp_16fwd = '1' then
          stp_16fwd_rg <= '1';
        elsif stp_16bck = '1' then
          stp_16bck_rg <= '1';
        end if;
      end if;
    end if;
  end process;

  --active_st   <= stp_fwd_rg   OR stp_bck_rg OR stp_16fwd_rg OR stp_16bck_rg;
  moving1  <= stp_fwd_rg   OR stp_bck_rg;
  moving16 <= stp_16fwd_rg OR stp_16bck_rg;

  end_moving <= (moving1 AND end_cnt_stp) OR (moving16 AND end_cnt_16stp);

  P_setup_cnt: Process (rst, clk)
  begin
    if rst = c_rst_on then
      cnt_setup <= (others => '0');
    elsif clk'event and clk = '1' then
      if mov_pres_st = SETUP_ST then
        if end_cnt_setup = '1' then
          cnt_setup <= (others => '0');
        else
          cnt_setup <= cnt_setup + 1;
        end if;
      else
        cnt_setup <= (others => '0');
      end if;
    end if;
  end process;

  end_cnt_setup <= '1' when cnt_setup = c_cnt_setup -1 else '0';
 
  P_hold_cnt: Process (rst, clk)
  begin
    if rst = c_rst_on then
      cnt_hold <= (others => '0');
    elsif clk'event and clk = '1' then
      if mov_pres_st = HOLD_ST then
        if end_cnt_hold = '1' then
          cnt_hold <= (others => '0');
        else
          cnt_hold <= cnt_hold + 1;
        end if;
      else
        cnt_hold <= (others => '0');
      end if;
    end if;
  end process;

  end_cnt_hold <= '1' when cnt_hold = c_cnt_hold -1 else '0';
 

  P_period_stp_count: Process (rst, clk)
  begin
    if rst = c_rst_on then
      cnt_stp <= (others => '0');
    elsif clk'event and clk = '1' then
      if mov_pres_st = MOVING_ST then
        if end_cnt_stp = '1' then
          cnt_stp <= (others => '0');
        else
          cnt_stp <= cnt_stp + 1;
        end if;
      else
        cnt_stp <= (others => '0');
      end if;
    end if;
  end process;

  end_cnt_stp  <= '1' when (cnt_stp = c_cnt_stp -1)      else '0';
  half_cnt_stp <= '1' when (cnt_stp = c_cnt_half_stp -1) else '0';

  P_stp_16count: Process (rst, clk)
  begin
    if rst = c_rst_on then
      cnt_16stp <= (others => '0');
    elsif clk'event and clk = '1' then
      if mov_pres_st = MOVING_ST then
        if end_cnt_stp = '1' then
          if (end_cnt_16stp = '1') then
            cnt_16stp <= (others => '0');
          else
            cnt_16stp <= cnt_16stp + 1;
          end if;
        end if;
      else
        cnt_16stp <= (others => '0');
      end if;
    end if;
  end process;

  end_cnt_16stp      <= '1' when (cnt_16stp = c_cnt_16stp -1) else 
                        '0';
  end_cnt_16stp_pls  <= end_cnt_16stp AND end_cnt_stp;


  P_stepper_signal: Process (rst, clk)
  begin
    if rst = c_rst_on then
      step_stp   <= '0';
    elsif clk'event and clk = '1' then
      if end_cnt_setup = '1' then
        step_stp <= '1'; -- to start on
      elsif mov_pres_st = MOVING_ST then
        if half_cnt_stp = '1' then
          step_stp <= '0';
        elsif end_cnt_stp = '1' then
          step_stp <= '1';
        end if;
      else
        step_stp   <= '0';
      end if;
    end if;
  end process;

  -- direction 1 when forward, all the time, including SETUP_ST and HOLD_ST
  dir_stp <= stp_fwd_rg OR stp_16fwd_rg;
    
  

end RTL;
  
