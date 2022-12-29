library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity uart_tx_nopar is
  generic (
    gFrecClk      : integer := 100000000;   --100MHz
    gBaud         : integer := 115200 --9600bps 921600
  ); 
  port(
    rst           : in std_logic; 
    clk           : in std_logic;
    Transmite     : in std_logic;
    DatoTxIn      : in std_logic_vector (7 downto 0);    
    Transmitiendo : out std_logic;
    DatoSerieOut  : out std_logic
  );
end uart_tx_nopar;


architecture BEHAV of uart_tx_nopar is
  constant cFinCuenta     : natural := gFrecClk/gBaud-1;
  signal   Cuenta         : natural range 0 to cFinCuenta;
  signal   ClkBaud        : std_logic;
  signal   EnableCont     : std_logic;
  signal   Dsplza         : std_logic;
  signal   CargaDato      : std_logic;
  signal   CuentaBits     : natural range 0 to 7;
  signal   FinDsplza8bits : std_logic;
  signal   RegDsplza      : std_logic_vector (7 downto 0);

  type   estados is (eInit, eBitInit, eBitsDato, eBitFin);
  signal Estado    : estados;

begin

  P_DivFrec: Process (rst, clk)
  begin
    if rst = '1' then
      Cuenta   <=  0; 
      ClkBaud  <= '0';
    elsif clk'event and clk='1' then
      if EnableCont = '1' then
        if Cuenta = cFinCuenta then   
          Cuenta  <= 0;
          ClkBaud <= '1';
        else
          Cuenta  <= Cuenta + 1;
          ClkBaud <= '0';
        end if;
      else  --  inhabilitado
        Cuenta  <= 0;
        ClkBaud <= '0';
      end if;
    end if;
  end process;

  P_Control_FSM: Process (rst, clk)
  begin
    if rst = '1' then
      Estado <= eInit;
    elsif clk'event and clk='1' then
      case Estado is
        when eInit =>
          if Transmite = '1' then
            Estado <= eBitInit;
          end if;
        when eBitInit =>
          if ClkBaud = '1' then
            Estado <= eBitsDato;
          end if;
        when eBitsDato =>
          if FinDsplza8bits = '1' then
            Estado <= eBitFin;
          end if;
        when eBitFin =>
          if ClkBaud = '1' then
            Estado <= eInit;
          end if;
      end case;
    end if;
  end process;

  P_Control_Salidas: Process (Estado, Transmite, ClkBaud )
  begin
    Dsplza        <= '0';
    CargaDato     <= '0';
    EnableCont    <= '1';
    Transmitiendo <= '1';
    case Estado is
      when eInit =>
        EnableCont    <= '0';
        Transmitiendo <= '0';
        if Transmite = '1' then
          CargaDato <= '1';
          EnableCont <= '1';
        end if;
      when eBitInit =>
        null;
      when eBitsDato =>
        if ClkBaud = '1' then
          Dsplza <= '1';
        end if;
      when eBitFin =>
        if ClkBaud = '1' then
          EnableCont <= '0';
        end if;
    end case;
  end process; 

  P_CuentaBits: Process (rst, clk)
  begin
    if rst = '1' then
      CuentaBits     <= 0;
    elsif clk'event and clk='1' then
      if Estado = eBitsDato then
        if ClkBaud = '1' then
          if CuentaBits = 7 then
            CuentaBits <= 0;
          else
            CuentaBits <= CuentaBits + 1;
          end if;
        end if;
      else
        CuentaBits <= 0;
      end if;
    end if;
  end process;

  FinDsplza8bits <= '1' when CuentaBits = 7 and ClkBaud = '1' else '0';

  P_RegDsplza: Process (rst, clk)
  begin
    if rst = '1' then
      RegDsplza <= (others => '0');
    elsif clk'event and clk='1' then
      if CargaDato = '1' then
        RegDsplza <= DatoTxIn;
      elsif Dsplza = '1' then
        RegDsplza (6 downto 0) <= RegDsplza (7 downto 1);
      end if;
    end if;
  end process;

  -- Registrado
  P_MuxOut: Process (rst, clk)
  begin
    if rst = '1' then
      DatoSerieOut <= '1'; --1 es en reposo
    elsif clk'event and clk='1' then
      case Estado is
        when eInit =>
          DatoSerieOut <= '1'; 
        when eBitInit =>
          DatoSerieOut <= '0'; 
        when eBitsDato =>
          DatoSerieOut <= RegDsplza(0); 
        when eBitFin =>
          DatoSerieOut <= '1'; 
      end case; 
    end if;
  end process;

end BEHAV;
