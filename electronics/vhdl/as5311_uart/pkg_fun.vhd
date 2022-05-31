--------------------------------------------------------------------------------
-- Felipe Machado Sanchez
-- Departameto de Tecnologia Electronica
-- Universidad Rey Juan Carlos
--
-- Functions to calculate ranges, bits, log

library IEEE;
use IEEE.STD_LOGIC_1164.all;

-- este paquete es para usar las funciones en el paquete UART_PKG_CTE
package PKG_FUN is

 --------------------------- funcion: log2i --------------------------------------
  -- Descripcion: funcion que calcula el logaritmo en base 2 de un numero entero 
  --              positivo. No calcula decimales, devuelve el entero menor o igual
  --              al resultado - por eso la i (de integer) del nombre log2i.
  --              P. ej: log2i(7) = 2, log2i(8) = 3.
  -- Entradas:
  --   * valor: numero entero positivo del que queremos calcular el logaritmo en
  -- Salida:
  --   * devuelve el logaritmo truncado al mayor entero menor o igual que el resultado
  function log2i (valor : positive) return natural;


 --------------------------- funcion: div_redondea-----------------------------------
  -- Descripcion: funcion que calcula la division entera con redondeo al numero
  --              entero mas cercano
  function div_redondea (dividendo, divisor: natural) return natural;

  function a_texto (arg : natural) return string;

end PKG_FUN;


Package body PKG_FUN is

 --------------------------- log2i ---------------------------------------
 -- Ejemplos de funcionamiento (valor = 6, 7 y 8)
 --  * valor = 6            |  * valor = 7          |  * valor = 8
 --      tmp = 6/2 = 3      |     tmp = 7/2 = 3     |     tmp = 8/2 = 4
 --      log2 = 0           |     log2 = 0          |     log2 = 0
 --    - loop 0: tmp 3 > 0  |   - loop 0: tmp 3>0   |   - loop 0: tmp 4>0
 --      tmp = 3/2 = 1      |     tmp = 3/2 = 1     |     tmp = 4/2 = 2
 --      log2 = 1           |     log2 = 1          |     log2 = 1
 --    - loop 1: tmp 1 > 0  |   - loop 1: tmp 1 > 0 |   - loop 1: tmp 2 > 0
 --      tmp = 1/2 = 0      |     tmp = 1/2 = 0     |     tmp = 2/2 = 1
 --      log2 = 2           |     log2 = 2          |     log2 = 2
 --    - end loop: tmp = 0  |   - end loop: tmp = 0 |   - loop 2: tmp 1 > 0
 --  * return log2 = 2      | * return log2 = 2     |     temp = 1/2 = 0
 --                                                 |     log2 = 3
 --                                                 |   - end loop: tmp = 0
 --                                                 | * return log2 = 3

  function log2i (valor : positive) return natural is
    variable tmp, log2: natural;
  begin
    tmp := valor / 2;  -- division entera, redondea al entero menor
    log2 := 0;
    while (tmp /= 0) loop
      tmp := tmp/2;
      log2 := log2 + 1;
    end loop;
    return log2;
  end function log2i;
  
  function div_redondea (dividendo, divisor: natural)
    return natural is
      variable division : integer;
      variable resto    : integer;
  begin
    division := dividendo/divisor;
    resto    := dividendo rem divisor; -- rem: calcula el resto de la division entera
    if (resto > (divisor/2)) then
      division := division + 1;
    end if;
    return (division);
  end;

-- funcion para los assert

  function a_texto (arg : natural) return string is
  begin
    case arg is
      when 0 => return "0";
      when 1 => return "1";
      when 2 => return "2";
      when 3 => return "3";
      when 4 => return "4";
      when 5 => return "5";
      when 6 => return "6";
      when 7 => return "7";
      when others => return "?";
    end case;
  end;

end PKG_FUN;

