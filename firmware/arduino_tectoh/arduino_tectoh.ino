
// Libraries: https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries
// https://www.arduino.cc/reference/en/libraries/timerone
#include <TimerOne.h>
// https://www.arduino.cc/reference/en/libraries/timerthree
#include <TimerThree.h>
// https://www.arduino.cc/reference/en/libraries/timerfour
#include <TimerFour.h> 

// LCD library: https://www.arduino.cc/reference/en/libraries/liquidcrystal/
#include <LiquidCrystal.h>

// -------------- LCD and rotary encoder smart controller
// LCD pins
#define LCD_PINS_RS 16      // LCD control RS pin
#define LCD_PINS_ENABLE 17  // LCD enable pin
#define LCD_PINS_D4 23      // LCD data bit 4
#define LCD_PINS_D5 25      // LCD data bit 5
#define LCD_PINS_D6 27      // LCD data bit 6
#define LCD_PINS_D7 29      // LCD data bit 7

// creates a variable of type LiquidCrystal:
// LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(LCD_PINS_RS, LCD_PINS_ENABLE,
                  LCD_PINS_D4, LCD_PINS_D5,
                  LCD_PINS_D6, LCD_PINS_D7);

// LCD size

# define LCD_NUMCOLS 20  // 20 columns
# define LCD_NUMROWS 4   // 4 rows

// Rotary encoder of LCD
#define ROT_ENC1_PIN 31    // Quadrature rotary encoder signal 1
#define ROT_ENC2_PIN 33    // Quadrature rotary encoder signal 1
#define ROT_ENCPB_PIN 35   // Rotary encoder push button

// Beeper
// #define BEEPER_PIN 33

// ----------- Endstops 
#define X_MIN_PIN 3         // INIT endstop x=0 (True when pressed)
#define X_MAX_PIN 2         // END endstop (True when pressed)


// ------------ Stepper motor
#define X_STEP_PIN 54       // STEP pin for the first stepper motor driver (axis X)
#define X_DIR_PIN 55        // DIR pin for the first stepper motor driver (axis X)
#define X_ENABLE_PIN 38     // ENABLE pin for the first stepper motor driver (axis X)

// ------------ Linear position sensor LPS
  
#define LPS_ENC1_PIN  20  // Linear position encoder 1 pin
#define LPS_ENC2_PIN  21  // PIN del canal B del encoder

// not used, just in the interrupt
//int lps_enc1;          // Value of linear encoder channel 1
int lps_enc2;          // Value of linear position sensor (lps) encoder channel 2

volatile int lps_line_cnt = 0;    // number of counted lines of the linear position sensor
const float mm_per_lps_line = 0.160; // milimiters per line from linear pos sensor
float lps_mm = 0;    // milimeters count by the linear position sensor (lps)

bool fc_inic_X = true;      // Valor del final de carrera situacion al inicio del experimento
bool fc_fin_X = true;       // Valor del final de carrera situacion al final del experimento

const int TOT_LEN = 400;    // leadscrew length in mm, maximum distance

// VARIABLES MEDIDA TIEMPO 

int horas = 0;              // Variable que cuenta las horas del experimento
int minutos = 0;            // Variable que cuenta los minutos del experimento
volatile int segundos = 0;  // Variable que cuenta los segundos del experimento

// VARIABLES MOVIMIENTO MOTOR

unsigned long t_mediomicropaso = 0;   // Variable en la que introducimo el tiempo de cada medio micropaso
unsigned long t_mediopaso = 0;        // Variable en la que introducimo el tiempo de cada medio paso
byte vel_mmh = 1;                      // Speed of the Sandbox in mm/h, from 1 to 99
volatile int nivel = LOW;             // Nivel del motor paso a paso
unsigned long micropasos = 0;         // Numero de micropasos dados
unsigned long n_mediospasos = 0;      // Numero de medios pasos cuando el experimento empieza en 0 mm
volatile int n_mediospasos2 = 0;      // Numero de medios pasos cuando el experimento no empieza en 0 mm
float avance_mediospasos = 0;         // Avance a partir de los medios pasos dados por el motor conocido su avance por cada medio paso cuando el experimento empieza en 0 mm
float avance_mediospasos2 = 0;        // Avance a partir de los medios pasos dados por el motor conocido su avance por cada medio paso cuando el experimento no empieza en 0 mm
int di_X, df_X = 0;                   // Distancia inicial y final que queremos en el experimento

// El siguiente vector muestra los tiempos de cada medio micropaso dependiendo de la velocidad seleccionado. Este vector se encuentra en un .csv en el GitHub del autor del trabajo
float vectort_mediomicropaso[] = {0,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,199.33,196.95,194.64,192.37,190.16,188.00,185.89,183.82,181.80,179.83,177.89,176.00,174.15,172.33,170.56,168.82,167.11,165.44};

// El siguiente vector muestra los tiempos de cada medio paso dependiendo de la velocidad seleccionado. Este vector se encuentra en un .csv en el GitHub del autor del trabajo
float vectort_mediopaso[] = {0,529411.76,264705.88,176470.59,132352.94,105882.35,88235.29,75630.25,66176.47,58823.53,52941.18,48128.34,44117.65,40723.98,37815.13,35294.12,33088.24,31141.87,29411.76,27863.78,26470.59,25210.08,24064.17,23017.90,22058.82,21176.47,20361.99,19607.84,18907.56,18255.58,17647.06,17077.80,16544.12,16042.78,15570.93,15126.05,14705.88,14308.43,13931.89,13574.66,13235.29,12912.48,12605.04,12311.90,12032.09,11764.71,11508.95,11264.08,11029.41,10804.32,10588.24,10380.62,10181.00,9988.90,9803.92,9625.67,9453.78,9287.93,9127.79,8973.08,8823.53,8678.88,8538.90,8403.36,8272.06,8144.80,8021.39,7901.67,7785.47,7672.63,7563.03,7456.50,7352.94,7252.22,7154.21,7058.82,6965.94,6875.48,6787.33,6701.41,6617.65,6535.95,6456.24,6378.45,6302.52,6228.37,6155.95,6085.19,6016.04,5948.45,5882.35,5817.71,5754.48,5692.60,5632.04,5572.76,5514.71,5457.85,5402.16,5347.59,5294.12};


// VARIABLES MAQUINA DE ESTADOS

int inicio = 0;                       // Variable que inicia el experimento cuando comienza en 0 mm
int inicio_experimento = 0;           // Variable que inicia el experimento cuando no comienza en 0 mm
int fin = 0;                          // Variable que para el experimento cuando llega al final de carrera 
int volatile estado = 0;              // Variables del estado


// LCD variables

byte lcd_rownum = 0; // a byte is enogh, 4 rows
byte lcd_colnum = 0; // a byte is enough, 20 lines

// LCD rotary encoder
bool rot_enc1, rot_enc2;
bool rot_enc1_prev, rot_enc2_prev;  // previous values
bool rot_encpb;  // push button of the rotary encoder

bool direccion = false;        // Direccion del codificador

bool rot_enc_rght = false;  // if LCD rotary encoder turned clocwise ->
bool rot_enc_left = false;  // if LCD rotary encoder turned counter cw <-
bool rot_enc_pb   = false;  // if LCD rotary encoder pushed

// AAA check
int i = 0;                 // Contador de pulsos

// New symbols

byte empty[8] =
{
  B00000,
  B00100,
  B01010,
  B10001,
  B10001,
  B01010,
  B00100,
};
byte full[8] =
{
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
};
byte micro[8] =
{
  B00000,
  B10001,
  B10001,
  B11011,
  B10101,
  B10000,
  B10000,
};
byte arrow[8] =
{
  B00000,
  B00001,
  B00001,
  B00001,
  B01001,
  B11111,
  B01000,
};

void setup() {

  pinMode(ROT_ENC1_PIN, INPUT_PULLUP);     // LCD rotary encoder 1
  pinMode(ROT_ENC2_PIN, INPUT_PULLUP);     // LCD rotary encoder 2
  pinMode(ROT_ENCPB_PIN, INPUT_PULLUP);    // LCD rotary encoder pushbutton
  
  pinMode(X_MIN_PIN, INPUT_PULLUP);   // Endstop init position
  pinMode(X_MAX_PIN, INPUT_PULLUP);   // Endstop final position
 
  pinMode (LPS_ENC1_PIN, INPUT);     // linear position sensor quadrature encoder 1
  pinMode (LPS_ENC2_PIN, INPUT);     // linear position sensor quadrature encoder 2

  lcd.begin(LCD_NUMCOLS, LCD_NUMROWS);   // 20 columns x 4 lines

  // stepper motor outputs
  pinMode(X_STEP_PIN , OUTPUT);
  pinMode(X_DIR_PIN  , OUTPUT);
  pinMode(X_ENABLE_PIN , OUTPUT);

  lcd.createChar(0, empty);   // 0: numero de carácter; empty: matriz que contiene los pixeles del carácter
  lcd.createChar(1, full);    // 1: numero de carácter; full: matriz que contiene los pixeles del carácter
  lcd.createChar(2, micro);   // 2: numero de carácter; micro: matriz que contiene los pixeles del carácter
  lcd.createChar(3, arrow);   // 3: número de carácter; arrow: matriz que contiene los pixeles del carácter
 
  digitalWrite(X_ENABLE_PIN , LOW);  // Stepper motor enable. Active-low

  attachInterrupt(digitalPinToInterrupt(LPS_ENC1_PIN), encoder, RISING);     // Función de la interrupcion del encoder

  Timer3.initialize(1000000);              // Inicialización de la interrupcion del contador de segundos
  Timer3.attachInterrupt(Temporizador);    // Función de la interrupcion del contador de segundos
  
}

// ------------------ Rotary encoder read 

void read_rot_encoder_dir()
{
  rot_enc1 = digitalRead(ROT_ENC1_PIN);
  rot_enc2 = digitalRead(ROT_ENC2_PIN);
  digitalWrite(X_DIR_PIN, direccion); // AAAAAA: shouldnt be here

  if (rot_enc1 != rot_enc1_prev || rot_enc2 != rot_enc2_prev)
  {
    if (rot_enc2 == false & rot_enc1 == false & rot_enc2_prev == true & rot_enc1_prev == false)
    {
      rot_enc_rght = true;
      rot_enc_left = false;
    }
    else if ( rot_enc2 == false & rot_enc1 == false & rot_enc2_prev == false & rot_enc1_prev == true )
    {
      rot_enc_rght = false;
      rot_enc_left = true;
    }
    else
    {
      rot_enc_rght = false;
      rot_enc_left = false;
    }
  }
  else
  {
    rot_enc_rght = false;
    rot_enc_left = false;
  }
  rot_enc1_prev = rot_enc1;
  rot_enc2_prev = rot_enc2;
}

// AAA check this function
void read_rot_encoder_pb()
{
  rot_encpb = digitalRead(ROT_ENCPB_PIN);

  if (rot_encpb == false) // risging edge detector
  {
    i++;
  }
  if (i >= 80)
  {
    rot_enc_pb = true;
    i = 0;
    delay(200);
  }
  else
  {
    rot_enc_pb = false;
  }
}

//////////////////////// ESTADO 0 /////////////////////////

void pantalla_inicio()
{

  lcd.setCursor(7, 0);   // set cursor position in lcd_colnum 2 lcd_rownum 0
  lcd.print("TectOH");
  lcd.setCursor(3, 1);    // set cursor position in lcd_colnum 3 lcd_rownum 1
  lcd.print("Open Hardware");
  lcd.setCursor(2, 2);    // set cursor position in lcd_colnum 3 lcd_rownum 2
  lcd.print("Tectonics Sanbox");


  lcd.setCursor(0, 3);    // set cursor position in lcd_colnum 1 lcd_rownum 3
  lcd.print("Press knob to start");
  
}


//////////////// ESTADO 1 ////////////////

void menu()
{
  lcd_rownum = 0;
  lcd_colnum = 0;

  di_X = 0;
  df_X = 0;
  vel_mmh = 1;

  lcd.setCursor(1, 0);
  lcd.print("dist.iniX");
  lcd.setCursor(12, 0);
  lcd.print(di_X);
  lcd.setCursor(18, 0);
  lcd.print("mm");
  lcd.setCursor(1, 1);
  lcd.print("dist.finX");
  lcd.setCursor(12, 1);
  lcd.print(df_X);
  lcd.setCursor(18, 1);
  lcd.print("mm");
  lcd.setCursor(1, 2);
  lcd.print("velocidad");
  lcd.setCursor(12, 2);
  lcd.print(vel_mmh);
  lcd.setCursor(16, 2);
  lcd.print("mm/h");
  lcd.setCursor(1, 3);
  lcd.print("Iniciar experimento");
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(0, 2);
  lcd.write(byte(0));
  lcd.setCursor(0, 3);
  lcd.write(byte(0));
  lcd.setCursor(0, lcd_rownum);
  lcd.write(byte(1));
}

//////////////// ESTADO 2 ////////////////

void DefinicionDeVariables()
{
  switch (lcd_rownum) // lcd_rownum es la variable vertical de la pantalla, indica que variable se esta manipulando
  {
    case 0: // modificar la distancia inicial
      // lcd_colnum is the column of the LCD, when 0, is at the left,
      // and we can select up-down in the menu
      if (rot_enc_pb == true  and lcd_colnum < 3) 
      {
        lcd_colnum++;
      }
      else if (rot_enc_pb == true and lcd_colnum == 3) //hay 4 opciones
      {
        lcd_colnum = 0;
      }
      switch (lcd_colnum)
      {
        case 0: //opción 1: seleccionar variable
          if (rot_enc_rght == true )
          {
            lcd_rownum = 1;
            lcd.setCursor(0, 0);
            lcd.write(byte(0));
            lcd.setCursor(0, 1);
            lcd.write(byte(1));
            lcd.setCursor(0, 2);
            lcd.write(byte(0));
            lcd.setCursor(0, 3);
            lcd.write(byte(0));
          }
          else if (rot_enc_left == true)
          {
            lcd_rownum = 3;
            lcd.setCursor(0, 0);
            lcd.write(byte(0));
            lcd.setCursor(0, 1);
            lcd.write(byte(0));
            lcd.setCursor(0, 2);
            lcd.write(byte(0));
            lcd.setCursor(0, 3);
            lcd.write(byte(1));
          }
          break;
        case 1: //opción 2: ir sumando y restando de 1 en 1
          if (rot_enc_rght == true and di_X + 1 < TOT_LEN)
          {
            di_X = di_X + 1;
            lcd.setCursor(12, 0);
            lcd.print("   ");
            lcd.setCursor(12, 0);
            lcd.print(di_X);
          }
          else if (rot_enc_left == true and di_X - 1 >= 0)
          {
            di_X = di_X - 1;
            lcd.setCursor(12, 0);
            lcd.print("   ");
            lcd.setCursor(12, 0);
            lcd.print(di_X);
          }
          break;
        case 2: //opción 3: ir sumando y restando de 10 en 10
          if (rot_enc_rght == true and di_X + 10 < TOT_LEN)
          {
            di_X = di_X + 10;
            lcd.setCursor(12, 0);
            lcd.print("   ");
            lcd.setCursor(12, 0);
            lcd.print(di_X);
          }
          else if (rot_enc_left == true and di_X - 10 >= 0)
          {
            di_X = di_X - 10;
            lcd.setCursor(12, 0);
            lcd.print("   ");
            lcd.setCursor(12, 0);
            lcd.print(di_X);
          }
          break;
        default: //opción 4: ir sumando y restando de 100 en 100
          if (rot_enc_rght == true and di_X + 100 < TOT_LEN)
          {
            di_X = di_X + 100;
            lcd.setCursor(12, 0);
            lcd.print("   ");
            lcd.setCursor(12, 0);
            lcd.print(di_X);
          }
          else if (rot_enc_left == true and di_X - 100 >= 0)
          {
            di_X = di_X - 100;
            lcd.setCursor(12, 0);
            lcd.print("   ");
            lcd.setCursor(12, 0);
            lcd.print(di_X);
          }
          break;
      }
      break;
    case 1: // modificar la distancia final
      // lcd_colnum is the column of the LCD, when 0, is at the left,
      // and we can select up-down in the menu
      if (rot_enc_pb == true  and lcd_colnum < 3) 
      {
        lcd_colnum++;
      }
      else if (rot_enc_pb == true and lcd_colnum == 3)
      {
        lcd_colnum = 0;
      }
      switch (lcd_colnum)
      {
        case 0: //opción 1: seleccionar variable
          if (rot_enc_rght == true )
          {
            lcd_rownum = 2;
            lcd.setCursor(0, 0);
            lcd.write(byte(0));
            lcd.setCursor(0, 1);
            lcd.write(byte(0));
            lcd.setCursor(0, 2);
            lcd.write(byte(1));
            lcd.setCursor(0, 3);
            lcd.write(byte(0));
          }
          else if (rot_enc_left == true)
          {
            lcd_rownum = 0;
            lcd.setCursor(0, 0);
            lcd.write(byte(1));
            lcd.setCursor(0, 1);
            lcd.write(byte(0));
            lcd.setCursor(0, 2);
            lcd.write(byte(0));
            lcd.setCursor(0, 3);
            lcd.write(byte(0));
          }
          break;
        case 1: //opción 2: ir sumando y restando de 1 en 1
          if (rot_enc_rght == true and df_X + 1 < TOT_LEN)
          {
            df_X = df_X + 1;
            lcd.setCursor(12, 1);
            lcd.print("   ");
            lcd.setCursor(12, 1);
            lcd.print(df_X);
          }
          else if (rot_enc_left == true and df_X - 1 >= 0)
          {
            df_X = df_X - 1;
            lcd.setCursor(12, 1);
            lcd.print("   ");
            lcd.setCursor(12, 1);
            lcd.print(df_X);
          }
          break;
        case 2: //opción 3: ir sumando y restando de 10 en 10
          if (rot_enc_rght == true and df_X + 10 < TOT_LEN)
          {
            df_X = df_X + 10;
            lcd.setCursor(12, 1);
            lcd.print("   ");
            lcd.setCursor(12, 1);
            lcd.print(df_X);
          }
          else if (rot_enc_left == true and df_X - 10 >= 0)
          {
            df_X = df_X - 10;
            lcd.setCursor(12, 1);
            lcd.print("   ");
            lcd.setCursor(12, 1);
            lcd.print(df_X);
          }
          break;
        default: //opción 4: ir sumando y restando de 100 en 100
          if (rot_enc_rght == true and df_X + 100 < TOT_LEN)
          {
            df_X = df_X + 100;
            lcd.setCursor(12, 1);
            lcd.print("   ");
            lcd.setCursor(12, 1);
            lcd.print(df_X);
          }
          else if (rot_enc_left == true and df_X - 100 >= 0)
          {
            df_X = df_X - 100;
            lcd.setCursor(12, 1);
            lcd.print("   ");
            lcd.setCursor(12, 1);
            lcd.print(df_X);
          }
          break;
      }
      break;
    case 2: // modificar la velocidad
      // lcd_colnum is the column of the LCD, when 0, is at the left,
      // and we can select up-down in the menu
      if (rot_enc_pb == true  and lcd_colnum < 1)
      {
        lcd_colnum++;
      }
      else if (rot_enc_pb == true and lcd_colnum == 1) //hay 2 opciones
      {
        lcd_colnum = 0;
      }
      switch (lcd_colnum)
      {
        case 0: //opción 1: seleccionar variable
          if (rot_enc_rght == true )
          {
            lcd_rownum = 3;
            lcd.setCursor(0, 0);
            lcd.write(byte(0));
            lcd.setCursor(0, 1);
            lcd.write(byte(0));
            lcd.setCursor(0, 2);
            lcd.write(byte(0));
            lcd.setCursor(0, 3);
            lcd.write(byte(1));
          }
          else if (rot_enc_left == true)
          {
            lcd_rownum = 1;
            lcd.setCursor(0, 0);
            lcd.write(byte(0));
            lcd.setCursor(0, 1);
            lcd.write(byte(1));
            lcd.setCursor(0, 2);
            lcd.write(byte(0));
            lcd.setCursor(0, 3);
            lcd.write(byte(0));
          }
          break;
        default: // opción 2: modificar la velocidad de 1 en 1 o de 10 en 10 en función de la velocidad
          if (rot_enc_left == true and vel_mmh - 1 >= 0)
          {
              vel_mmh = vel_mmh - 1;
              lcd.setCursor(12, 2);
              lcd.print(vel_mmh);
          }
          else if (rot_enc_rght == true and vel_mmh < 100)
          {
              vel_mmh = vel_mmh + 1;
              lcd.setCursor(12, 2);
              lcd.print(vel_mmh);
          }
          
        if (vel_mmh <10 && vel_mmh >=0){
        lcd.setCursor(13, 2);
        lcd.print(" ");
        }
        if (vel_mmh <100 && vel_mmh >=0){
        lcd.setCursor(14, 2);
        lcd.print(" ");
        }
          
          break;
      }
      break;
      
    default: // iniciar experimento
    
      if (rot_enc_pb == true)
      {
          t_mediomicropaso = ((unsigned long)vectort_mediomicropaso[vel_mmh]);     
          Timer1.attachInterrupt(Micropasos);             // Funcion de la interrupcion de los micropasos
          Timer1.initialize(t_mediomicropaso);            // Inicializacion de la interrupcion de los micropasos
          Timer4.attachInterrupt(MedioPaso);              // Funcion de la interrupcion de los medios pasos
          t_mediopaso = ((unsigned long)(vectort_mediopaso[vel_mmh]));  
          Timer4.initialize(t_mediopaso);                 // Inicializacion de la interrupcion de los medios pasos
         
          lcd.clear();
          estado = 3;
        
      }
      else if (rot_enc_rght == true )
      {
        lcd_rownum = 0;
        lcd_colnum = 0;
        lcd.setCursor(0, 0);
        lcd.write(byte(1));
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(0, 2);
        lcd.write(byte(0));
        lcd.setCursor(0, 3);
        lcd.write(byte(0));
      }
      else if (rot_enc_left == true)
      {
        lcd_rownum = 2;
        lcd_colnum = 0;
        lcd.setCursor(0, 0);
        lcd.write(byte(0));
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(0, 2);
        lcd.write(byte(1));
        lcd.setCursor(0, 3);
        lcd.write(byte(0));
      }
      break;
  }
}

//////////////// ESTADO 3 ////////////////

void experimento() {

  fc_inic_X = digitalRead(X_MIN_PIN);
  fc_fin_X = digitalRead(X_MAX_PIN);
  
  if (fc_inic_X == true )
  {
    inicio = 1;
  }

  if (fc_fin_X == true )
  {
    fin = 1;
  }

  if (inicio == 1) {
    if (di_X == 0) {
      inicio_experimento = 1;
    }
    else if (di_X == (0.000147*n_mediospasos)) {
      inicio_experimento = 1;
    }
  }  

// MOVIMIENTO MOTOR

  if (inicio == 1 && fin == 0) {  
       digitalWrite(X_DIR_PIN , HIGH);
  }
  else { 
       digitalWrite(X_DIR_PIN , LOW);
  }   

// POSITION

  if (inicio_experimento == 1) {
    
    lcd.setCursor(0, 0);
    lcd.print("LINEAS: ");

    lcd.setCursor(7, 0);
    lcd.print(lps_line_cnt);

    // millimeters observed by linear position sensor
    lps_mm = mm_per_lps_line*lps_line_cnt;
    
    lcd.setCursor(11, 0);
    lcd.print("->"); 
    lcd.setCursor(14, 0);
    lcd.print(lps_mm);
    lcd.setCursor(18, 0);
    lcd.print("mm"); 
     
      if (lps_line_cnt <10 && lps_line_cnt >=0){
      lcd.setCursor(8, 0);
      lcd.print(" ");
      }
      if (lps_line_cnt <100 && lps_line_cnt >=0){
      lcd.setCursor(9, 0);
      lcd.print(" ");
      }
      if (lps_line_cnt <1000 && lps_line_cnt >=0){
      lcd.setCursor(10, 0);
      lcd.print(" ");
      }
    
    lcd.setCursor(0, 2);
    lcd.print("Mediospasos: ");
    lcd.setCursor(12, 2);    
    lcd.print(n_mediospasos);

    avance_mediospasos = (0.000147*n_mediospasos);       // Avance de cada medio paso del motor = 0.000147 mm, ya que el eje del motor tiene 3 mm/vuelta y una reduccion de 51
    avance_mediospasos2 = (0.000147*n_mediospasos2);

    lcd.setCursor(0, 3);
    lcd.print("Avance: ");
    lcd.setCursor(7, 3);
    
    if (di_X == 0){     
    lcd.print(avance_mediospasos);}
    else{
    lcd.print(avance_mediospasos2);
    }
    
    lcd.setCursor(11, 3);
    lcd.print("mm");
      
  }
  
// TIEMPO 
  
  if (inicio == 0 && fin ==0) {
    lcd.setCursor(0, 0);
    lcd.print("MOVIENDO");
    lcd.setCursor(0, 1);
    lcd.print("A CERO");
    }
  else if (inicio_experimento == 1 && fin ==0 && (avance_mediospasos < df_X)){  
      if (segundos == 60)    {
      segundos = 0;
      minutos++;           }
      if (minutos == 60)     {
      minutos = 0;
      horas++;             }
      lcd.setCursor(0, 1);
      if (horas < 10)        {
      lcd.print("0");      }
      lcd.print(horas);
      lcd.print(":");
      lcd.setCursor(3, 1);
      if (minutos < 10)      {
      lcd.print("0");      }
      lcd.print(minutos);
      lcd.print(":");
      lcd.setCursor(6, 1);
      if (segundos < 10)     {
      lcd.print("0");        }
      lcd.print(segundos);
    }
}

void Temporizador() {
  
  if (inicio_experimento == 1 && fin ==0) {//                             <- INTERRUPCION CRONOMETRO
    segundos++;
  }
  
}

void Micropasos() {
    if (estado == 3 && (avance_mediospasos < df_X) && fin == 0){
      if (micropasos < 16){
        digitalWrite(X_STEP_PIN , nivel);
        if (nivel == LOW){       //                             <- INTERRUPCION MOVIMIENTO MOTOR TIEMPO MEDIO MICROPASO
           nivel = HIGH;
           micropasos++;}
        else{
           nivel = LOW;
        }
      }
    }
}

void MedioPaso() {
  
  micropasos = 0;
  
    if (inicio == 1 && fin ==0 && (avance_mediospasos < df_X)){ //        <- INTERRUPCION MOVIMIENTO MOTOR TIEMPO TOTAL MEDIO PASO
    n_mediospasos++;
    }
    
    if (0.000147*n_mediospasos > di_X){
    n_mediospasos2++;
    }
}

// linear encoder interrupt to read lines
void encoder() {

    if (inicio_experimento == 1 && fin == 0) {
      // linear position sensor encoder channel 2
      lps_enc2 = digitalRead (LPS_ENC2_PIN); // linear position sensor encoder 
      if (lps_enc2 == LOW){
        lps_line_cnt++;
      }
      else{
        lps_line_cnt--;
      }
    }
}

void loop() {
  
  switch (estado){
    case 0:
      pantalla_inicio();
      estado = 1;
      break;
            
    case 1:
      read_rot_encoder_pb();
  
      if (rot_enc_pb == true) { 
        lcd.clear();
        menu();
        estado = 2;
      }
      
      break;

    case 2:
      read_rot_encoder_pb();
      read_rot_encoder_dir();
      DefinicionDeVariables();
      break;
   
    case 3: 
      experimento();
      break;    
  }
  
}

          
  
