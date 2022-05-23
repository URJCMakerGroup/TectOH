
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

#define LCD_NUMCOLS 20  // 20 columns
#define LCD_NUMROWS 4   // 4 rows

#define LCD_PARAMS_COL 12 // column where the parameters are drawn, the left side 

// Rotary encoder of LCD
#define ROT_ENC1_PIN 31    // Quadrature rotary encoder signal 1
#define ROT_ENC2_PIN 33    // Quadrature rotary encoder signal 1
#define ROT_ENCPB_PIN 35   // Rotary encoder push button

// Beeper
//#define LCD_BEEPER_PIN 33

// LCD stop / kill button
#define LCD_STOP_PIN 41

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


volatile int lps_line_cnt = 0;    // number of counted lines of the linear position sensor
const float mm_per_lps_line = 0.160; // milimiters per line from linear pos sensor
float lps_mm = 0;    // milimeters count by the linear position sensor (lps)


const int TOT_LEN = 400;    // leadscrew length in mm, maximum distance
const int MAX_VEL = 100;    // maximum velocity in mm/h

const int LEAD = 3;  // lead screw lead in mm. How many mm advances per revolution
const int STEP_REV = 200; // how many steps per revolution the motor has
const int HSTEP_REV = 2 * STEP_REV; // halfsteps per revolution

const float GEAR_R = 51.0; // gear ratio of the motor

//advance in mm per halfstep
const float ADVAN_HSTEP = float(LEAD) / (HSTEP_REV * GEAR_R);

// ---- variables for keeping track the experiment timing

short hour_cnt = 0;        // Keep track of the number of hours of the experiment
byte minute_cnt = 0;       // Keep track of the number of minutes of the experiment
volatile byte sec_cnt = 0; // Keep track of the number of seconds of the experiment

// Sandbox parameters

// short are 16bits
// Speed of the Sandbox in mm/h, from 1 to 100, making it short, to have negative
// to make some operations
short vel_mmh = 1;   // Speed of the Sandbox in mm/h, from 1 to 100
short pos_x_ini = 0;  // Initial position of the gantry for the experiment
short pos_x_end = 0;  // Final position of the gantry for the experiment

byte usteps_cnt = 0;    // Number of usteps 0 to 15
volatile unsigned long tot_halfstep_cnt = 0;  // Number of half steps, from pos  0
volatile unsigned long halfstep_cnt = 0;     // Number of half steps, from initial position
float absol_pos_mm = 0;     // position from the x=0 in mm, absolute position
float relat_pos_mm = 0;     // position from the initial position of the experimet, relative

// El siguiente vector muestra los tiempos de cada medio micropaso dependiendo de la velocidad seleccionado. Este vector se encuentra en un .csv en el GitHub del autor del trabajo
float array_t_half_ustep[] = {0,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,199.33,196.95,194.64,192.37,190.16,188.00,185.89,183.82,181.80,179.83,177.89,176.00,174.15,172.33,170.56,168.82,167.11,165.44};

// El siguiente vector muestra los tiempos de cada medio paso dependiendo de la velocidad seleccionado. Este vector se encuentra en un .csv en el GitHub del autor del trabajo
float array_t_half_step[] = {0,529411.76,264705.88,176470.59,132352.94,105882.35,88235.29,75630.25,66176.47,58823.53,52941.18,48128.34,44117.65,40723.98,37815.13,35294.12,33088.24,31141.87,29411.76,27863.78,26470.59,25210.08,24064.17,23017.90,22058.82,21176.47,20361.99,19607.84,18907.56,18255.58,17647.06,17077.80,16544.12,16042.78,15570.93,15126.05,14705.88,14308.43,13931.89,13574.66,13235.29,12912.48,12605.04,12311.90,12032.09,11764.71,11508.95,11264.08,11029.41,10804.32,10588.24,10380.62,10181.00,9988.90,9803.92,9625.67,9453.78,9287.93,9127.79,8973.08,8823.53,8678.88,8538.90,8403.36,8272.06,8144.80,8021.39,7901.67,7785.47,7672.63,7563.03,7456.50,7352.94,7252.22,7154.21,7058.82,6965.94,6875.48,6787.33,6701.41,6617.65,6535.95,6456.24,6378.45,6302.52,6228.37,6155.95,6085.19,6016.04,5948.45,5882.35,5817.71,5754.48,5692.60,5632.04,5572.76,5514.71,5457.85,5402.16,5347.59,5294.12};


// ---------------- FINITE STATE MACHINES (FSM)
// There are 3:
//  - ui_state: the main FSM of the user interface (UI),
//               indicate the main activity
//  - selparam_st: indicates which parameter is being changed
//  - seldigit_st: indicates which digit is being changed

#define   ST_INI        0   // initial state, welcome
#define   ST_SEL_PARAMS 1   // Selecting the parameters
#define   ST_SEL_DIGIT  2   // Selecting which digit to change
#define   ST_SEL_VALUE  3   // Setting the digit value
#define   ST_RUN        4   // Running the experiment

byte ui_state = ST_INI; 

// indicates what parameter are we changing, moving on rows
#define   SELPARAM_X0    0   // changing the initial position
#define   SELPARAM_XF    1   // changing the final position
#define   SELPARAM_VEL   2   // changing the experiment velocity
#define   SELPARAM_START 3   // start the experiment

byte selparam_st = SELPARAM_X0;
               
// the column indicates the digit we are changing
#define   SELDIG_PARAM   0    // go back to parameter select
#define   SELDIG_100s    1    // changing the hundreds
#define   SELDIG_TENS    2    // changing the tens
#define   SELDIG_UNITS   3    // changing the units

byte seldigit_st = SELDIG_PARAM; //default state, no change


bool homed = false;        // if the gantry has gone to the init endstop
int inicio_experimento = 0; // Variable que inicia el experimento cuando no comienza en 0 mm
int fin = 0;           // Variable que para el experimento cuando llega al final de carrera 

// LCD rotary encoder
bool rot_enc1, rot_enc2;

bool rot_enc_rght   = false;  // if LCD rotary encoder turned clocwise ->
bool rot_enc_left   = false;  // if LCD rotary encoder turned counter cw <-

// New symbols

# define HOL_DIAM    0
# define FUL_DIAM    1
# define MICRO       2
# define BACK_ARROW  3

byte hol_diam[8] = // 0: hollow diamond
{
  B00000,
  B00100,
  B01010,
  B10001,
  B10001,
  B01010,
  B00100,
};
byte ful_diam[8] = // 1: full diamond
{
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
};
byte micro[8] =  // 2: micro symbol
{
  B00000,
  B10001,
  B10001,
  B11011,
  B10101,
  B10000,
  B10000,
};
byte back_arrow[8] =  // 3: back arrow (return)
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

  // creation of new lcd characters
  lcd.createChar(HOL_DIAM, hol_diam); // 0: hollow diamond
  lcd.createChar(FUL_DIAM, ful_diam); // 1: full diamond
  lcd.createChar(MICRO,    micro);    // 3: micro symbol
  lcd.createChar(BACK_ARROW, back_arrow);  // 4: back arrow (return)
 
  digitalWrite(X_ENABLE_PIN , LOW);  // Stepper motor enable. Active-low

  // interrupt for the linear position sensor
  attachInterrupt(digitalPinToInterrupt(LPS_ENC1_PIN), encoder, RISING);

  // interrupt for the counter of seconds, to keep track of the time 
  Timer3.initialize(1000000);             // A million microseconds to count seconds
  Timer3.attachInterrupt(SecondsCounter); // Second counter
  
}

// ------ next ui state
// simple function that calculates the next ui state depending on the 
// rotation of the knob
// we could have used the global variable, but instead we use arguments:
// ui_state_arg
// right, left (indicates if the nob is turned right or left

byte nxt_ui_state(byte ui_state_arg, bool right, bool left)
{
  if (right == true) {
    if (ui_state_arg == ST_RUN) {
      ui_state_arg = ST_SEL_PARAMS;
    } else {
      ui_state_arg = ui_state_arg + 1;
    }
  } else if (left == true) {
    if (ui_state_arg == ST_SEL_PARAMS) { // dont go back to ST_INI
      ui_state_arg = ST_RUN;
    } else {
      ui_state_arg = ui_state_arg - 1;
    }
  }
  return ui_state_arg;
}

// ------ next parameter
// simple function that calculates the next parameter selection state
// depending on the  rotation of the knob
// we could have used the global variable, but instead we use arguments:
// selparam_st_arg
// right, left (indicates if the nob is turned right or left

byte nxt_param(byte selparam_st_arg, bool right, bool left)
{
  if (right == true) {
    if (selparam_st_arg == SELPARAM_START) {
      selparam_st_arg = SELPARAM_X0;
    } else {
      selparam_st_arg = selparam_st_arg + 1;
    }
  } else if (left == true) {
    if (selparam_st_arg == SELPARAM_X0) { 
      selparam_st_arg = SELPARAM_START;
    } else {
      selparam_st_arg = selparam_st_arg - 1;
    }
  }
  return selparam_st_arg;
}

// ------ next digit
// simple function that calculates the next digit selection state
// depending on the  rotation of the knob
// we could have used the global variable, but instead we use arguments:
// seldigit_st_arg
// right, left (indicates if the nob is turned right or left

byte nxt_digit(byte seldigit_st_arg, bool right, bool left)
{
  if (right == true) {
    if (seldigit_st_arg == SELDIG_UNITS) {
      seldigit_st_arg = SELDIG_PARAM;
    } else {
      seldigit_st_arg = seldigit_st_arg + 1;
    }
  } else if (left == true) {
    if (seldigit_st_arg == SELDIG_PARAM) { 
      seldigit_st_arg = SELDIG_UNITS;
    } else {
      seldigit_st_arg = seldigit_st_arg - 1;
    }
  }
  return seldigit_st_arg;
}

// calculates the increment, depending on the knob rotation, and the digit
short calc_incr(byte seldigit_st_arg, bool right, bool left)
{
  short increment = 0;

  switch (seldigit_st_arg) {
    case SELDIG_UNITS:
      increment = 1;
      break;
    case SELDIG_TENS:
      increment = 10;
      break;
    case SELDIG_100s:
      increment = 100;
      break;
    default:
      increment = 0;
      break;
  }
  if (left == true) {
    increment = - increment;
  } else if (right == false) {
    increment = 0; // no turn, no increment
  }

  return increment;

}


// -------------- lcdprint_rght
// print an integer in the lcd aligned to the right
// number: number to print
// col: colum to start printing (most left)
// row: row to print
// max_digit : maximum number of digits to be printed. Needed to align right

void lcdprint_rght (int number, int max_digit)
{
  int index = 0;
  // write whitespaces to delete what is down
  
  int max_number;

  max_number = int(pow(10, max_digit-1));

  //lcd.setCursor(col, row);
  for (index = 0; index < max_digit-1; index++) {
    if (number >= max_number) {
      break;
    }
    max_number = int(max_number/10);
    lcd.print("0");
  }  
  lcd.print(number);

}

// ------------------ Rotary encoder read 

void read_rot_encoder_dir()
{
  // static variables are only initilized the first time and keep their value
  static bool rot_enc1_prev = false; // previous values
  static bool rot_enc2_prev = false;

  rot_enc1 = digitalRead(ROT_ENC1_PIN);
  rot_enc2 = digitalRead(ROT_ENC2_PIN);

  if (rot_enc1 != rot_enc1_prev || rot_enc2 != rot_enc2_prev)
  {
    if (rot_enc2      == false && rot_enc1      == false &&
        rot_enc2_prev == true  && rot_enc1_prev == false)
    {
      rot_enc_rght = true;
      rot_enc_left = false;
    } else if ( rot_enc2      == false && rot_enc1      == false &&
                rot_enc2_prev == false && rot_enc1_prev == true )
    {
      rot_enc_rght = false;
      rot_enc_left = true;
    } else {
      rot_enc_rght = false;
      rot_enc_left = false;
    }
  } else {
    rot_enc_rght = false;
    rot_enc_left = false;
  }
  rot_enc1_prev = rot_enc1;
  rot_enc2_prev = rot_enc2;
}

// -------------------- rot_encoder_pushed ---------------
// reads the value of the rotary encoder, true if pushed
// compares with the last time it has been read

bool rot_encoder_pushed()
{
  // only read in the menu user interface, not during the experiment

  // static variables are only initilized the first time and keep their value
  // first time to true because it seems that initially is HIGH, maybe due
  // to de pull-up input
  static byte rot_enc_pb_prev = HIGH;


  byte   rot_enc_pb;  // push button of the rotary encoder
  bool   pushed = false;

  rot_enc_pb = digitalRead(ROT_ENCPB_PIN);

  if (rot_enc_pb == HIGH && rot_enc_pb_prev == LOW) {
    pushed = true;
    delay(300);
  }
  rot_enc_pb_prev = rot_enc_pb;
  return pushed;
}

// ------ ST_INI: initial state

void init_screen()
{

  lcd.setCursor(7, 0);   // set cursor position in col 7 row 0
  lcd.print("TectOH");
  lcd.setCursor(3, 1);   // set cursor position in col 3 row 1
  lcd.print("Open Hardware");
  lcd.setCursor(2, 2);
  lcd.print("Tectonics Sanbox");

  lcd.setCursor(0, 3);
  lcd.print("Press knob to start");
  
}


//  Print menu on screen

void menu()
{
  pos_x_ini = 0;
  pos_x_end = 0;
  vel_mmh = 1;

  lcd.setCursor(1, 0);
  lcd.print("Init posX");
  lcd.setCursor(12, 0);
  lcdprint_rght(pos_x_ini,3);
  lcd.setCursor(18, 0);
  lcd.print("mm");
  lcd.setCursor(1, 1);
  lcd.print("End posX");
  lcd.setCursor(12, 1);
  lcdprint_rght(pos_x_end,3);
  lcd.setCursor(18, 1);
  lcd.print("mm");
  lcd.setCursor(1, 2);
  lcd.print("velocity");
  lcd.setCursor(12, 2);
  lcdprint_rght(vel_mmh,3);
  lcd.setCursor(16, 2);
  lcd.print("mm/h");
  lcd.setCursor(1, 3);
  lcd.print("Start Experimet");
  lcd.setCursor(0, 0);
  lcd.write(byte(FUL_DIAM));
  lcd.setCursor(0, 1);
  lcd.write(byte(HOL_DIAM));
  lcd.setCursor(0, 2);
  lcd.write(byte(HOL_DIAM));
  lcd.setCursor(0, 3);
  lcd.write(byte(HOL_DIAM));
}


//////////////// ESTADO 3 ////////////////

void experimento() {

  byte endstop_x_ini;    // endstop value at x=0
  byte endstop_x_end;    // endstop value at the end

  endstop_x_ini = digitalRead(X_MIN_PIN);
  endstop_x_end = digitalRead(X_MAX_PIN);
  
  if (endstop_x_ini == HIGH ) {
    homed = 1;
  }

  if (endstop_x_end == HIGH ) {
    fin = 1;
  }

  if (homed == true) {
    if (pos_x_ini == 0) {
      inicio_experimento = 1;
    }
    else if (pos_x_ini == (ADVAN_HSTEP * tot_halfstep_cnt)) {
      inicio_experimento = 1;
    }
  }  

// MOVIMIENTO MOTOR

  if (homed == true && fin == 0) {  
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
    lcd.print(tot_halfstep_cnt);

    absol_pos_mm = (ADVAN_HSTEP * tot_halfstep_cnt);   // each halfstep
    relat_pos_mm = (ADVAN_HSTEP * halfstep_cnt);

    lcd.setCursor(0, 3);
    lcd.print("Avance: ");
    lcd.setCursor(7, 3);
    
    if (pos_x_ini == 0){     
    lcd.print(absol_pos_mm);}
    else{
    lcd.print(relat_pos_mm);
    }
    
    lcd.setCursor(11, 3);
    lcd.print("mm");
      
  }
  
// TIEMPO 
  
  if (homed == false && fin ==0) {
    lcd.setCursor(0, 0);
    lcd.print("MOVIENDO");
    lcd.setCursor(0, 1);
    lcd.print("A CERO");
  } else if (inicio_experimento == 1 && fin ==0 && (absol_pos_mm < pos_x_end)){  
      if (sec_cnt == 60)    {
      sec_cnt = 0;
      minute_cnt ++;           }
      if (minute_cnt == 60)     {
      minute_cnt = 0;
      hour_cnt ++;             }
      lcd.setCursor(0, 1);
      if (hour_cnt < 10)        {
      lcd.print("0");      }
      lcd.print(hour_cnt);
      lcd.print(":");
      lcd.setCursor(3, 1);
      if (minute_cnt < 10)      {
      lcd.print("0");      }
      lcd.print(minute_cnt);
      lcd.print(":");
      lcd.setCursor(6, 1);
      if (sec_cnt < 10)     {
      lcd.print("0");        }
      lcd.print(sec_cnt);
    }
}


// -------------- Interrupt function to count seconds

void SecondsCounter() {
  
  if (inicio_experimento == 1 && fin ==0) {
    sec_cnt ++;
  }
  
}

// -------------- Interrupt function to count the microsteps and generate
// ------ the pulses for the motor

void gen_usteps() {
  static byte step_value = LOW;     // signal to send to stepper motor pin

  if (ui_state == ST_RUN && (absol_pos_mm < pos_x_end) && fin == 0){
    if (usteps_cnt < 16){
      digitalWrite(X_STEP_PIN , step_value);
      if (step_value == LOW){
         step_value = HIGH;
         usteps_cnt++;
      } else{
         step_value = LOW;
      }
    }
  }
}

// -------------- Interrupt function to count the number of halfsteps

void MedioPaso() {
  usteps_cnt = 0; // initialize the ustpes
  
  if (homed == true && fin ==0 && (absol_pos_mm < pos_x_end)){ 
    tot_halfstep_cnt ++;
  }
    
  if (ADVAN_HSTEP * tot_halfstep_cnt > pos_x_ini){
    halfstep_cnt ++;
  }
}

// linear encoder interrupt to read lines
void encoder() {
    // not used, just in the interrupt
    //int lps_enc1;          // Value of linear encoder channel 1
    byte lps_enc2;          // Value of linear position sensor (lps) encoder channel 2

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

void update_menu() {

  byte col, row;
  static byte ui_state_prev = ST_SEL_PARAMS;
  static byte selparam_st_prev = SELPARAM_X0;
  static byte seldigit_st_prev = SELDIG_PARAM;
  static short pos_x_ini_prev = 0;
  static short pos_x_end_prev = 0;
  static short vel_mmh_prev   = 1;  

  if (ui_state > ST_INI && ui_state < ST_RUN) {
    if (selparam_st != selparam_st_prev ) {
      // draw all the hollow diamonds
      lcd.setCursor(0, 0);
      lcd.write(byte(HOL_DIAM));
      lcd.setCursor(0, 1);
      lcd.write(byte(HOL_DIAM));
      lcd.setCursor(0, 2);
      lcd.write(byte(HOL_DIAM));
      lcd.setCursor(0, 3);
      lcd.write(byte(HOL_DIAM));
      lcd.setCursor(0, selparam_st); // the full diamond
      lcd.write(byte(FUL_DIAM));
    }  
    if (ui_state != ui_state_prev ) {
      switch (ui_state) {
        case ST_SEL_PARAMS:
          lcd.noCursor(); // no cursor when setting params
          lcd.noBlink();  // no blink when setting params
          break;
        case ST_SEL_DIGIT:
          lcd.cursor();  // cursor when selecting digits
          lcd.noBlink(); // no blink when selecting digits
          break;
        case ST_SEL_VALUE:
          lcd.cursor();  // cursor when selecting digits
          lcd.blink();   // blink when selecting digits
          break;
        default:
          lcd.noCursor(); // no cursor
          lcd.noBlink();  // no blink
          break;
      }
    }

    switch (seldigit_st) {
      case SELDIG_PARAM:
        col = 0; // selection at first col (diamonds)
        break;
      case SELDIG_100s:
        col = LCD_PARAMS_COL;
        break;
      case SELDIG_TENS:
        col = LCD_PARAMS_COL+1;
        break;
      case SELDIG_UNITS:
        col = LCD_PARAMS_COL+2;
        break;
    }
    // draw the parameters if changed
    if (pos_x_ini != pos_x_ini_prev) {
      lcd.setCursor(LCD_PARAMS_COL, SELPARAM_X0);
      lcdprint_rght(pos_x_ini,3); //3 is the number of digits
    }
    if (pos_x_end != pos_x_end_prev) {
      lcd.setCursor(LCD_PARAMS_COL, SELPARAM_XF);
      lcdprint_rght(pos_x_end,3); //3 is the number of digits
    }
    if (vel_mmh != vel_mmh_prev) {
      lcd.setCursor(LCD_PARAMS_COL, SELPARAM_VEL);
      lcdprint_rght(vel_mmh,3); //3 is the number of digits
    }

    if  ((ui_state_prev    != ui_state    ) ||
         (selparam_st_prev != selparam_st ) ||
         (seldigit_st_prev != seldigit_st ) ||
         (pos_x_ini_prev   != pos_x_ini   ) ||
         (pos_x_end_prev   != pos_x_end   ) ||
         (vel_mmh_prev     != vel_mmh     )) {  
      lcd.setCursor(col, selparam_st); //row is directly defined by selparam_st
    }
  }

  // uptade previous values
  ui_state_prev    = ui_state;
  selparam_st_prev = selparam_st;
  seldigit_st_prev = seldigit_st;
  pos_x_ini_prev    = pos_x_ini;
  pos_x_end_prev    = pos_x_end;
  vel_mmh_prev      = vel_mmh;  
}


void loop() {

  short val_incr;
  short aux_val;
  bool rot_enc_pushed;  // if LCD rotary encoder pushed
  unsigned long t_half_ustep = 0;   // us that takes half of a microstep
  unsigned long t_half_step = 0;    // us that takes a half step
  
  switch (ui_state){
    case ST_INI:
      init_screen();
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == HIGH) { 
        lcd.clear();
        menu();
        ui_state = ST_SEL_PARAMS;
      }
      break;
    case ST_SEL_PARAMS: // Choosing between setting x0, xfin, vel, or start
      update_menu();
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == HIGH) {
        // we go to select the digit, unless the parameter selected is to start
        if (selparam_st == SELPARAM_START) {
          ui_state = ST_RUN;
          lcd.clear();
        } else {
          // when comming from ST_SEL_PARAMS, we start with the units
          ui_state    = ST_SEL_DIGIT;
          seldigit_st = SELDIG_UNITS; // start with units
        }
      } else {
        read_rot_encoder_dir();
        selparam_st = nxt_param(selparam_st, rot_enc_rght, rot_enc_left);
      }
      break;
    case ST_SEL_DIGIT: // Choosing between the digit
      update_menu();
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == HIGH) { // digit selected -> select value
        if (seldigit_st == SELDIG_PARAM) { // go back to ST_SEL_PARAMS
          ui_state = ST_SEL_PARAMS;
        } else {
          ui_state  = ST_SEL_VALUE;
        }
      } else { // knob not pushed
        read_rot_encoder_dir();
        seldigit_st = nxt_digit(seldigit_st, rot_enc_rght, rot_enc_left);
      }
      break;
    case ST_SEL_VALUE: // Setting/selecting the digit value
      update_menu();
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == HIGH) { // value selected, go back
        // we can only be here if we are changing values (units, 10s, 100s)
        // of X0, XF, VEL
        ui_state = ST_SEL_DIGIT;
      } else {
        read_rot_encoder_dir();
        val_incr = calc_incr(seldigit_st, rot_enc_rght, rot_enc_left);
        if (rot_enc_rght == true) { // adding
          switch (selparam_st) {
            case SELPARAM_X0: // changing the initial position
              aux_val = pos_x_ini + val_incr;
              if (aux_val > TOT_LEN) {
                pos_x_ini = TOT_LEN;
              } else {
                pos_x_ini = aux_val;
              }
              break;
            case SELPARAM_XF: // changing the final position
              aux_val = pos_x_end + val_incr;
              if (aux_val > TOT_LEN) {
                pos_x_end = TOT_LEN;
              } else {
                pos_x_end = aux_val;
              }
              break;
            case SELPARAM_VEL: // changing the velocity
              aux_val = vel_mmh + val_incr;
              if (aux_val > MAX_VEL) {
                vel_mmh = MAX_VEL;
              } else {
                vel_mmh = aux_val;
              }
              break;
            default: // would be an error to be here
              break;
          }
        } else if (rot_enc_left == true) { // substracting
           switch (selparam_st) {
            case SELPARAM_X0: // changing the initial position
              aux_val = pos_x_ini + val_incr; // val_incr is negative
              if (aux_val < 0) {
                pos_x_ini = 0;
              } else {
                pos_x_ini = aux_val;
              }
              break;
            case SELPARAM_XF: // changing the final position
              aux_val = pos_x_end + val_incr;
              if (aux_val < 0) {
                pos_x_end = 0;
              } else {
                pos_x_end = aux_val;
              }
              break;
            case SELPARAM_VEL: // changing the velocity
              aux_val = vel_mmh + val_incr;
              if (aux_val < 1) {
                vel_mmh = 1;
              } else {
                vel_mmh = aux_val;
              }
              break;
            default: // would be an error to be here
              break;
          }
        }
      }
      break;
    case ST_RUN: 
      lcd.clear();
      t_half_ustep = ((unsigned long)array_t_half_ustep[vel_mmh]);     
      // attach function for half micro steps interruption
      Timer1.attachInterrupt(gen_usteps);     
      Timer1.initialize(t_half_ustep);

      // attach function for halfsteps interruption
      Timer4.attachInterrupt(MedioPaso); 
      t_half_step = ((unsigned long)(array_t_half_step[vel_mmh]));  
      Timer4.initialize(t_half_step);
     
      experimento();
      break;    
  }
  
}

          
  
