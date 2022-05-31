--
-- Author: Felipe Machado, felipe.machado@urjc.es
-- Version: 0.1
-- Date: 2016-06-21
--
-- Interface for AS5133 SSI: Synchronous Serial Interface
-- http://ams.com/eng/Products/Magnetic-Position-Sensors/Linear-Position/AS5311
--
--  PORTS
--  -- signals from/to FPGA
--  rst             : in  std_logic; 
--                    Asynchronous reset of the circuit
--  clk             : in  std_logic;
--                    Clock of the FPGA board
--  read_ssi        : in  std_logic;
--                    request for a new reading of the AS5133, a pulse is enough
--  position_fieldn : in  std_logic;
--                    It asks for the linear position data '1'
--                    or for the magnetic field strength data '0' 
--  ssi_avail       : out std_logic;
--                    Indicates if the entity is available ('1') to receive data
--                    or if it is busy ('0') receiving data
--  data_ready      : out std_logic;
--                    Indicates that the data_reg and status_reg are ready
--                    It is just a clock cycle
--  parity_ok       : out std_logic. '1' if the even parity bit from the SSI is
--                    the same as the computed (signal parity_calc)
--                    '0': if they don´t match
--  data_reg        : out std_logic_vector(c_data_bits-1 downto 0);
--                    Data from the AS5311 SSI, could be the linear position
--                    or the magnetic field. Depending on what was requested
--                    by postion_fieldn
--  status_reg      : out std_logic_vector(c_status_bits-1 downto 0);
--                    Status bits from the transmision of the AS5133 SSI
--                    or the magnetic field. Depending on what was requested
--                    by postion_fieldn
--                    Status regs are:
--                    status_reg[0] : Parity bit
--                    status_reg[1] : Mag DEC 
--                    status_reg[2] : Mag INC
--                    status_reg[3] : LIN
--                    status_reg[4] : COF
--                    status_reg[5] : OCF
--
--  -- Signals to/from ssi
--  ssi_data_in     : in  std_logic;
--                    serial data from the AS5133
--  ssi_cs_n_out    : out std_logic 
--                    chip select for the AS5133 SSI, ´0´is active
--  ssi_clk_out     : out std_logic; -- clk for the SSI
--                    clock for the AS5133 SSI
--

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library WORK;
use WORK.PKG_FUN.all;
use WORK.PKG_FPGA.all;
use WORK.PKG_AS5133.all;

entity INTERF_SSI_AS5133 is
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
end INTERF_SSI_AS5133;


architecture RTL of INTERF_SSI_AS5133 is

  --constant c_freq_ssi     : natural := 1;    --MHz
  --constant c_freq_fpga    : natural := 100;  --MHz 
  -- cnt: count. The end of count to get the frequency
  --constant c_cnt_ssi      : natural := c_freq_fpga/c_freq_ssi;

  constant c_period_ns_ssi     : natural := 1000;   --ns
  --constant c_period_ns_fpga    : natural := 10;     --ns 
  -- time of first Falling Edge of ssiclk after cs_n is active (1st FE of cs_n) 
  constant c_ssiclk_ns_fe      : natural := 500;     --ns
  -- number of cycles to count each cycle of the SSI clk -- 100
  -- we don't substract 1, we will do it when we calculate the end of the count
  -- so we avoid doing the divisions wrong, like c_cnt_ssiclk_fe
  constant c_cnt_ssiclk     : natural := div_redondea
                                           (c_period_ns_ssi,c_period_ns_fpga); 
  -- the number of bits (nb) necesary to represent c_cnt_ssiclk
  constant c_nb_cnt_ssiclk : natural := log2i(c_cnt_ssiclk) + 1;

  -- number of cycles to count for the 1st falling edge of the SSI clk
  --  50
  constant c_cnt_ssiclk_1stfe : natural := c_ssiclk_ns_fe/c_period_ns_fpga;
  -- number of cycles to count for the rest of falling edge of the SSI clk
  -- which is the same as c_cnt_ssiclk_1stfe
  constant c_cnt_ssiclk_fe : natural := c_cnt_ssiclk/2;                 --  50

  -- signals that will be registered to be outputs to the AS5133 SSI
  signal ssi_clk         :  std_logic; -- ssi clock
  signal ssi_cs_n        :  std_logic; -- chip select
  signal ssi_data_rg     :  std_logic; -- serial data from the AS5133 SSI

  -- counter for the frequency divider for the SSI receiver
  signal cnt_ssiclk       : unsigned (c_nb_cnt_ssiclk-1 downto 0);
  -- combinatorial signal that indicates the end of the count for the frequency
  -- divider for the SSI receiver
  signal end_cnt_ssiclk   : std_logic;
  -- combinatorial signal that indicates the half of the count for the frequency
  -- divider for the SSI receiver. Falling edge
  signal half_cnt_ssiclk  : std_logic;
  -- it is ´0´ when it is on the first half of the count
  signal cnt_ssiclk_2ndhalf_rg : std_logic;

  -- this is the count of the bits received from the SSI (nb: number of bits)
  signal cnt_ssi_bits       : unsigned (c_nb_ssi_bits-1 downto 0);
  signal end_cnt_ssibits    : std_logic;

  type   recep_states is (IDLE, PREPARE_RECEP, RECEIVING_BITS,
                          PARITY, ENDING_RECEP);

  signal present_state, next_state  : recep_states;

  -- register for the position / magnetic field option. 
  -- to keep it during the whole reception, since the input position_fieldn
  -- may change
  signal pos_fieldn_rg : std_logic;

  -- shift register where the bits from the SSI are being stored
  -- we use c_ssi_bits, which means the number of bits transmited by the SSI
  -- (not including the parity bit)
  signal ssi_data_shrg  : std_logic_vector (c_ssi_bits-1 downto 0);

  -- '1' if it is not in IDLE
  signal ssi_receiving       : std_logic;
  -- '1' when all the bits have been received, except for the parity bit.
  -- Marks the transition from the state RECEIVING_BITS to PARITY
  signal end_receiving_bits  : std_logic;

  -- signal that calculates the parity
  signal parity_calc         : std_logic;
  

begin

  -- process to register the outputs/inputs to/from the AS5133 SSI
  -- the delay has to be considered
  P_register_ports: Process (rst, clk)
  begin
    if rst = c_rst_on then
      ssi_clk_out  <= '1';
      ssi_cs_n_out <= '1';
      ssi_data_rg  <= '0';
    elsif clk'event and clk='1' then
      ssi_clk_out  <= ssi_clk;
      ssi_cs_n_out <= ssi_cs_n;
      ssi_data_rg  <= ssi_data_in;    
    end if;
  end process;
   
-- frequency divider for the SSI clock

  P_SSI_freq_div: Process (rst, clk)
  begin
    if rst = c_rst_on then
      cnt_ssiclk <= (others => '0');
      cnt_ssiclk_2ndhalf_rg <= '0';
    elsif clk'event and clk= '1' then
      if ssi_receiving = '1' then
        if end_cnt_ssiclk = '1' then
          cnt_ssiclk <= (others => '0');
          cnt_ssiclk_2ndhalf_rg <= '0';
        else
          cnt_ssiclk <= cnt_ssiclk + 1;
        end if;
        if half_cnt_ssiclk = '1' then
          cnt_ssiclk_2ndhalf_rg <= '1';
        end if;
      else
        cnt_ssiclk <= (others => '0');
        cnt_ssiclk_2ndhalf_rg <= '0';
      end if;
    end if;
   end process;
     
  

  end_cnt_ssiclk <= '1' when cnt_ssiclk = c_cnt_ssiclk-1 else
                    '0' ;
  half_cnt_ssiclk <= '1' when cnt_ssiclk = c_cnt_ssiclk_fe-1 else
                     '0' ;
  -- this is just for the 1st falling edge, which happens to be the same
  -- as for the rest (half cycle)
--  fe1st_cnt_ssiclk <= '1' when cnt_ssiclk = c_cnt_ssiclk_1stfe else
--                      '0' ;


-- Process to keep the position/magnetic_field value for the whole reception
-- process. 
  P_position_fieldn: Process (clk, rst)
  begin 
    if rst = c_rst_on then
      -- by default, we ask for the position, but it doesn´t matter because
      -- it won't be used until it is assigned
      pos_fieldn_rg <= '1'; 
    elsif clk'event and clk='1' then
      if present_state = IDLE then
        if read_ssi = '1' then
          pos_fieldn_rg <= position_fieldn;
        else 
          -- when it is in IDLE and no command to read, set to the default value
          -- it is not really necesary, because always when in IDLE we receive
          -- read_ssi= '1', it will get the appropriate value
          pos_fieldn_rg <= '1';
        end if;
      end if;
    end if;
  end process;
        

-- process that counts the number of receiving bits from the SSI

  P_ssi_bits_cnt: Process(rst, clk)
  begin
    if rst = c_rst_on then
      cnt_ssi_bits <= (others => '0');
    elsif clk'event and clk = '1' then
      if present_state = RECEIVING_BITS then
        if end_cnt_ssiclk = '1' then
          if end_cnt_ssibits = '0' then
            cnt_ssi_bits <= cnt_ssi_bits + 1;
          else 
            cnt_ssi_bits <= (others => '0');
          end if;
        end if;
      else
        cnt_ssi_bits <= (others => '0');
      end if;
    end if;
  end process;
     
  
  end_cnt_ssibits <= '1' when (cnt_ssi_bits = c_ssi_bits - 1) else
                     '0';

  end_receiving_bits <= '1' when end_cnt_ssibits = '1' and
                                 end_cnt_ssiclk  = '1' else 
		        '0';

  P_SEQ_FSM: Process (rst, clk) 
  begin 
    if rst = c_rst_on then 
      present_state <= IDLE; 
    elsif clk'event and clk='1' then 
      present_state <= next_state; 
    end if; 
  end process; 


  P_COMB_FSM: Process (present_state, read_ssi, position_fieldn, read_ssi,
                       end_cnt_ssiclk, end_receiving_bits, half_cnt_ssiclk)
  begin
    case present_state is
      when IDLE => 
        next_state <= present_state;
        if read_ssi = '1' then  -- request for a new SSI read
          next_state <= PREPARE_RECEP;
        end if;
      when PREPARE_RECEP =>
        next_state <= present_state;
        if end_cnt_ssiclk = '1' then 
          next_state <= RECEIVING_BITS;
        end if;
      when RECEIVING_BITS =>
        next_state <= present_state;
        if end_receiving_bits = '1' then 
          next_state <= PARITY;
        end if;
      when PARITY =>
        next_state <= present_state;
        if end_cnt_ssiclk = '1' then 
          next_state <= ENDING_RECEP;
        end if;
      when ENDING_RECEP =>
        next_state <= present_state;
        if half_cnt_ssiclk = '1' then 
          next_state <= IDLE;
        end if;
    end case;
  end process;
        

-- process for the outputs of the Finite State Machine
  P_COMB_FSM_OUTS: Process (present_state, read_ssi, position_fieldn,
	                    pos_fieldn_rg, cnt_ssiclk_2ndhalf_rg)
  begin
    case present_state is
      when IDLE =>
        ssi_cs_n  <= '1';
        ssi_clk   <= '1';  
        ssi_receiving <= '0';
        if read_ssi = '1' then
          if position_fieldn = '1' then -- position requested
            ssi_clk  <= '1';  
          else                          -- magnetic field data requested
            ssi_clk  <= '0';  
          end if;
        end if;
      when PREPARE_RECEP =>  
        ssi_cs_n <= '0'; 
        ssi_receiving <= '1';
        if pos_fieldn_rg = '1' then   -- the registered one, the input may have
          if cnt_ssiclk_2ndhalf_rg = '0' then  -- the first half to '1'
            ssi_clk  <= '1';  
          else                          -- second half to '0'
            ssi_clk  <= '0';  
          end if;
        else
          -- for the magnetic field data request, clk ='0' the whole time
          ssi_clk  <= '0';  
        end if;
      when RECEIVING_BITS =>  
        ssi_cs_n <= '0'; 
        ssi_receiving <= '1';
        if cnt_ssiclk_2ndhalf_rg = '0' then  -- the first half to '1'
          ssi_clk  <= '1';  
        else                          -- second half to '0'
          ssi_clk  <= '0';  
        end if;
      when PARITY =>  
        ssi_cs_n <= '0'; 
        ssi_clk  <= '1';  
        ssi_receiving <= '1';
      when ENDING_RECEP =>  
        ssi_cs_n <= '1'; 
        ssi_clk  <= '1';  
        ssi_receiving <= '1';
    end case;
  end process;
 
  ssi_avail <= NOT ssi_receiving;
      
  -- Shift Register: SIPO: Serial In - Parallel Out
  P_shift_reg_data: Process (rst, clk)
  begin
    if rst = c_rst_on then
      ssi_data_shrg   <= (others =>'0');
      parity_calc     <= '0';
    elsif clk'event and clk='1' then
      if present_state = IDLE then
        parity_calc     <= '0';
      elsif present_state = RECEIVING_BITS then
        -- the data is available 413 ns after the rising edge. half_cnt_ssiclk
        -- is 500 ns after. Even taking away 3 cycles of 10ns due to port
        -- registration, its ok. If clock cycle were 20ns, it still would be ok
        -- with not much time left
        if half_cnt_ssiclk = '1' then 
          -- Shift register moving to the left because the first bit that
          -- comes in is the MSB (D11)
          ssi_data_shrg(c_ssi_bits-1 downto 1) <=
                                ssi_data_shrg(c_ssi_bits-2 downto 0);
          ssi_data_shrg(0) <= ssi_data_rg;
	  parity_calc <= parity_calc XOR ssi_data_rg;
        end if;
      end if; -- we don´t clean it
    end if;
  end process;


  -- moving all the data of the shift register into a data & status register
  P_move2reg: Process (rst, clk)
  begin
    if rst = c_rst_on then
      data_ready   <= '0';
      data_reg     <= (others => '0');
      status_reg   <= (others => '0');
      parity_ok    <= '0';
    elsif clk'event and clk='1' then
      data_ready   <= '0';  -- just for a clock cycle
      if present_state = PARITY then
        if half_cnt_ssiclk = '1' then -- move all the data to the registers
          data_ready   <= '1';
          data_reg <= ssi_data_shrg (c_ssi_bits - 1 downto
                                     c_ssi_bits - c_data_bits);
          status_reg (c_status_bits-1 downto 1) <= 
                                ssi_data_shrg (c_status_bits-2 downto 0);
          -- also: equivalent to above
          --             ssi_data_shrg (c_ssi_bits-c_data_bits-1 downto 0);

          ----- the parity bit
          status_reg (0) <= ssi_data_rg;
	  if parity_calc = ssi_data_rg then
            parity_ok <= '1';
	  else
            parity_ok <= '0';
	  end if;
        end if;
      end if;
    end if;
  end process;
      
end RTL;
  
