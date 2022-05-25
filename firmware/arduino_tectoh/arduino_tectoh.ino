#include "tectoh_config.h"

// Libraries: https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries
// https://www.arduino.cc/reference/en/libraries/timerone
#include <TimerOne.h>
// https://www.arduino.cc/reference/en/libraries/timerthree
#include <TimerThree.h>
// https://www.arduino.cc/reference/en/libraries/timerfour
#include <TimerFour.h> 

// https://docs.arduino.cc/learn/built-in-libraries/eeprom
// The EEPROM memory keeps its value after powering off, it is very convenient
// to hold the position because:
//  - homing takes a lot of time due to the slow speeds
//  - Setting up the experiment may require hours, so the gantry can be set
//    to its position, then power off the Sandbox to setup the experiment, 
//    and when it is ready, power it up, and start.
//    Te gantry should not go home, because it would ruin the experimet
#include <EEPROM.h>

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

// column where the additional info of the actual position of the gantry according
// to the EEPROM
#define LCD_EEP_POS_COL 11

// Rotary encoder of LCD
#define ROT_ENC1_PIN 31    // Quadrature rotary encoder signal 1
#define ROT_ENC2_PIN 33    // Quadrature rotary encoder signal 1
#define ROT_ENCPB_PIN 35   // Rotary encoder push button

// Beeper
//#define LCD_BEEPER_PIN 33

// LCD stop / kill button
#define LCD_STOP_PIN 41

// ----------- Endstops 
#define X_MIN_PIN 3         // INIT endstop x=0 (HIGH when pressed)
#define X_MAX_PIN 2         // END endstop (HIGH when pressed)


// ------------ Stepper motor
#define X_STEP_PIN 54       // STEP pin for the first stepper motor driver (axis X)
#define X_DIR_PIN 55        // DIR pin for the first stepper motor driver (axis X)
#define X_ENABLE_PIN 38     // ENABLE pin for the first stepper motor driver (axis X)

// defined in configuration file: tectoh_config.h
#if DIR_MOTOR_POS_HIGH
  #define DIR_MOTOR_POS HIGH
  #define DIR_MOTOR_NEG LOW
#else
  #define DIR_MOTOR_POS LOW
  #define DIR_MOTOR_NEG HIGH
#endif


// ------------ Linear position sensor LPS
  
#define LPS_ENC1_PIN  20  // Linear position encoder 1 pin
#define LPS_ENC2_PIN  21  // PIN del canal B del encoder


// in 400 mm there are about 2362 lines, so a int, or a short is enough
volatile short lps_line_cnt = 0;    // number of counted lines of the linear position sensor
const float mm_per_lps_line = 0.1693; // milimiters per line from linear pos sensor
float lps_mm = 0;    // milimeters count by the linear position sensor (lps)

// --- endstop values
byte endstop_x_ini;    // endstop value at x=0
byte endstop_x_end;    // endstop value at the end

// defined in configuration file: tectoh_config.h
#if ENDSTOP_ACTIVE_HIGH
  #define ENDSTOP_ON  HIGH
  #define ENDSTOP_OFF LOW
#else
  #define ENDSTOP_ON  LOW
  #define ENDSTOP_OFF HIGH
#endif

const int TOT_LEN = 400;    // leadscrew length in mm, maximum distance
const int MAX_VEL = 100;    // maximum velocity in mm/h

const int LEAD = 3;  // lead screw lead in mm. How many mm advances per revolution
const int STEP_REV = 200; // how many steps per revolution the motor has
const int HSTEP_REV = 2 * STEP_REV; // halfsteps per revolution

const float GEAR_R = 51.0f; // gear ratio of the motor

//advance in mm per halfstep
const float ADVAN_HSTEP = float(LEAD) / (HSTEP_REV * GEAR_R);

// ---- variables for keeping track the experiment timing

// and experiment could take 400 hours if 400 mm at 1mm/h, it is not likely
// but just in case, make it short, not byte
volatile short hour_cnt = 0;        // Keep track of the number of hours of the experiment
volatile byte minute_cnt = 0;       // Keep track of the number of minutes of the experiment
volatile byte sec_cnt = 0; // Keep track of the number of seconds of the experiment

// ----- Sandbox parameters

// position of the gantry in mm, according to the value saved in the EEPROM
short pos_x_eeprom;

//#define EEPROM_DIR 0  // memory address of the EEPROM where the gantry position is

byte  eeprom_valid_read = 0;  // 0
short pos_x_eep_read    =-1;  // 1,2
short dist_eep_read     =-1;  // 3,4
short vel_eep_read      =-1;  // 5,6
byte  secs_eep_read;          // 7
byte  mins_eep_read;          // 8
short hours_eep_read    = -1; // 9,10

// short are 16bits
// Speed of the Sandbox in mm/h, from 1 to 100, making it short, to have negative
// to make some operations
short vel_mmh = MAX_VEL;  // Speed of the Sandbox in mm/h, from 1 to 100
short abs_dest = -1;  // absolute position of the destination, from x0 in mm
short rel_dist = 1; // relative position of the destination, in absolute value
short rel_dist_sign = 1; // relative position of the destination, including sign
bool  rel_dist_neg = false;  // sign of rel_dist: 0: positive, 1: negative
// relative position of the destination, from actual position, it is update when
// actual position changes
short rel_dest_updated = 0;

short abs_init = -1;  // absolute initial position, if -1 is unknown

volatile byte h_ustp_cnt = 0;    // Number of half usteps in a halfstep 0 to 31

// Number of half steps counted. a long has 32 bits -> being unsigned
// from 0 to 4,294,967,295
// 400 mm, with 3mm lead, 400 halfsteps per turn and 51 reduction,
// gives 0,147 um/halfstep, which is 
//               2,714,666.67 halfsteps (22 bits) So we can count them
volatile unsigned long hstp_cnt = 0;   // Number of half steps, from initial position

float absol_pos_mm = 0; // position from the x=0 in mm, absolute position
// position from the initial position of the experimet, relative
float relat_pos_mm_stp = 0; // calculated from halfsteps
float relat_pos_mm_lin = 0; // calculated from lines
 

// array for the time of a half of a microstep (half time low, half high)
// For  3mm lead, 400 halfsteps per turn and 51 reduction, the fastest
// speeds, from 83mm/h and up, the h_ustp take less than 200 us
// for speed from 1mm/h to 82 mm/h, the h_microstp take 200 us
// This vector should be integer, because we are not usign decimals AAAAA

const float vec_t_h_ustp[] = {0,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,199.33,196.95,194.64,192.37,190.16,188.00,185.89,183.82,181.80,179.83,177.89,176.00,174.15,172.33,170.56,168.82,167.11,165.44};

// it is byte because is less than 200
byte t_half_ustp; // time that a half microstep takes, from the previous vector
const byte SLEW_VEL_T_USTP = 200; // microseconds lower than the speed will be continuos

// array for the time of a half step (h_stp), it is only used when the time
// a half microstep (h_ustp) is lower than 200. Otherwise, they would be out
// of sync
const float vec_t_h_stp[] = {0,529411.76,264705.88,176470.59,132352.94,105882.35,88235.29,75630.25,66176.47,58823.53,52941.18,48128.34,44117.65,40723.98,37815.13,35294.12,33088.24,31141.87,29411.76,27863.78,26470.59,25210.08,24064.17,23017.90,22058.82,21176.47,20361.99,19607.84,18907.56,18255.58,17647.06,17077.80,16544.12,16042.78,15570.93,15126.05,14705.88,14308.43,13931.89,13574.66,13235.29,12912.48,12605.04,12311.90,12032.09,11764.71,11508.95,11264.08,11029.41,10804.32,10588.24,10380.62,10181.00,9988.90,9803.92,9625.67,9453.78,9287.93,9127.79,8973.08,8823.53,8678.88,8538.90,8403.36,8272.06,8144.80,8021.39,7901.67,7785.47,7672.63,7563.03,7456.50,7352.94,7252.22,7154.21,7058.82,6965.94,6875.48,6787.33,6701.41,6617.65,6535.95,6456.24,6378.45,6302.52,6228.37,6155.95,6085.19,6016.04,5948.45,5882.35,5817.71,5754.48,5692.60,5632.04,5572.76,5514.71,5457.85,5402.16,5347.59,5294.12};


// ---------------- FINITE STATE MACHINES (FSM)
// There are 3:
//  - ui_state: the main FSM of the user interface (UI),
//               indicate the main activity
//  - selparam_st: indicates which parameter is being changed
//  - seldigit_st: indicates which digit is being changed

#define   ST_INI        0   // initial state, welcome
#define   ST_SEL_TASK   1   // Task selection: GO HOME - Move
#define   ST_SEL_PARAMS 2   // Selecting the parameters: speed, distance
#define   ST_SEL_DIGIT  3   // Setting the digit
#define   ST_SEL_VALUE  4   // Setting the digit value
#define   ST_CONFIRM    5   // Confirming to start
#define   ST_HOMING     6   // Homing
#define   ST_RUN        7   // Running the experiment
#define   ST_END        8   // Moving ended

byte ui_state = ST_INI; 

#define   TASK_HOME     0   // Task Go Home
#define   TASK_MOVE_REL 1   // Task Movement relative

byte task_st = TASK_HOME;
//

// indicates what parameter are we changing, moving on rows
#define   SELPARAM_VEL    0   // changing the experiment velocity
#define   SELPARAM_DIST   1   // Distance to move, + or - mm from actual pos
#define   SELPARAM_GO     2   // start the experiment
#define   SELPARAM_BACK   3   // Go to previous menu

byte selparam_st = SELPARAM_VEL;
               
// the column indicates the digit we are changing
#define   SELDIG_PARAM   0    // go back to parameter select
#define   SELDIG_SIGN    1    // changing the sign
#define   SELDIG_100s    2    // changing the hundreds
#define   SELDIG_TENS    3    // changing the tens
#define   SELDIG_UNITS   4    // changing the units

byte seldigit_st = SELDIG_PARAM; //default state, no change

#define   CONFIRM_YES    0    // confirmation
#define   CONFIRM_NO     1    // No confirmation

byte confirm_st = CONFIRM_YES;

// variables relative to the experiment
bool exp_homed = false;     // if the gantry has gone to the init endstop (home)
// if the gantry has pass to the init experiment X (after homing)
bool exp_pass_init = false;
int fin = 0;           // Variable que para el experimento cuando llega al final de carrera 

// LCD rotary encoder
bool rot_enc1, rot_enc2;

bool rot_enc_rght   = false;  // if LCD rotary encoder turned clocwise ->
bool rot_enc_left   = false;  // if LCD rotary encoder turned counter cw <-

// New symbols only 7

#define CHR_HL_DIAM   0
#define CHR_FL_DIAM   1

#define CHR_X0   0 // in different screens
#define CHR_XF   1

#define CHR_HL_HL_BOX  2
#define CHR_HL_FL_BOX  3
#define CHR_FL_HL_BOX  4
#define CHR_FL_FL_BOX  5

#define CHR_MM         6
#define CHR_PER_HOUR   7


#define CHR_MICRO        228 // 0xE4: it is in the LCD charset
#define CHR_RGHT_ARROW   126 // 0x7E: it is in the LCD charset
#define CHR_LEFT_ARROW   127 // 0x7F: it is in the LCD charset


const byte IC_HL_DIAM[8] = // icon hollow diamond
{
  B00000,
  B00100,
  B01010,
  B10001,
  B10001,
  B01010,
  B00100,
};
const byte IC_FL_DIAM[8] = // icon full diamond
{
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
};

const byte IC_HL_HL_BOX[8] = // icon two hollow boxes
{
  B11111,
  B10001,
  B10001,
  B11111,
  B10001,
  B10001,
  B11111,
};

const byte IC_HL_FL_BOX[8] = // icon one hollow one full box
{
  B11111,
  B10001,
  B10001,
  B11111,
  B11111,
  B11111,
  B11111,
};

const byte IC_FL_HL_BOX[8] = // icon one full and other hollow box
{
  B11111,
  B11111,
  B11111,
  B11111,
  B10001,
  B10001,
  B11111,
};

const byte IC_FL_FL_BOX[8] = // two full boxes
{
  B11111,
  B11111,
  B11111,
  B00000,
  B11111,
  B11111,
  B11111,
};

const byte IC_MM[8] = // millimiter
{
  B11010,
  B10101,
  B10101,
  B00000,
  B11010,
  B10101,
  B10101,
};

byte IC_PER_HOUR[8] = // icon per hour
{
  B00010,
  B00100,
  B01000,
  B10000,
  B00101,
  B00111,
  B00101,
};

byte IC_X0[8] = // X_0
{
  B10100,
  B01000,
  B10100,
  B00000,
  B00111,
  B00101,
  B00101,
  B00111,
};

byte IC_XF[8] = // X_F
{
  B10100,
  B01000,
  B10100,
  B00000,
  B00111,
  B00100,
  B00110,
  B00100,
};


byte IC_PER_HOUR2[8] = // per hour
{
  B01000,
  B01000,
  B10000,
  B10100,
  B00100,
  B00111,
  B00101,
};

byte IC_PER_HOUR3[8] = // per hour
{
  B00100,
  B01000,
  B10000,
  B00100,
  B00100,
  B00111,
  B00101,
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
  //             char number     icon
  lcd.createChar(CHR_HL_DIAM, IC_HL_DIAM); // 0: hollow diamond
  lcd.createChar(CHR_FL_DIAM, IC_FL_DIAM); // 1: full diamond

  lcd.createChar(CHR_HL_HL_BOX, IC_HL_HL_BOX); // 2: hollow + hollow box
  lcd.createChar(CHR_HL_FL_BOX, IC_HL_FL_BOX); // 3: hollow + full box
  lcd.createChar(CHR_FL_HL_BOX, IC_FL_HL_BOX); // 4: full + hollow box
  lcd.createChar(CHR_FL_FL_BOX, IC_FL_FL_BOX); // 5: full + full box

  lcd.createChar(CHR_MM, IC_MM);               // 6: milimiter
  lcd.createChar(CHR_PER_HOUR, IC_PER_HOUR);   // 7: per hour
 
  digitalWrite(X_ENABLE_PIN , HIGH);  // Stepper motor disable. Active-low
  digitalWrite(X_STEP_PIN , LOW);  // dont step at start

  init_screen();
  
}

// -- sign_neg
//  if >= 0 ->  1
//  if <0   -> -1

short sign_neg (int num)
{
  if (num >= 0) {
    return 1;
  } else {
    return -1;
  }
}

// ------ next ui state. CAN BE DELETED
// simple function that calculates the next ui state depending on the 
// rotation of the knob
// we could have used the global variable, but instead we use arguments:
// ui_state_arg
// right, left (indicates if the nob is turned right or left

byte nxt_ui_state(byte ui_state_arg, bool right, bool left)
{
  if (right == true) {
    if (ui_state_arg == ST_END) {
      ui_state_arg = ST_SEL_PARAMS;
    } else {
      ui_state_arg = ui_state_arg + 1;
    }
  } else if (left == true) {
    if (ui_state_arg == ST_SEL_PARAMS) { // dont go back to ST_INI
      ui_state_arg = ST_END;
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

byte nxt_param (byte selparam_st_arg, bool right, bool left, byte task_st_arg)
{
  byte selparam_st_res = selparam_st_arg;

  if (right == true) {
    if (selparam_st_res == SELPARAM_BACK) {
      selparam_st_res = SELPARAM_VEL;
    } else {
      selparam_st_res = selparam_st_res + 1;
      if (task_st_arg == TASK_HOME && selparam_st_res == SELPARAM_DIST) {
        selparam_st_res = selparam_st_res + 1; // no SELPARAM_DIST when homing
      }
    }
  } else if (left == true) {
    if (selparam_st_res == SELPARAM_VEL) { 
      selparam_st_res = SELPARAM_BACK;
    } else { 
      selparam_st_res = selparam_st_res - 1;
      if (task_st_arg == TASK_HOME && selparam_st_res == SELPARAM_DIST) {
        selparam_st_res = selparam_st_res - 1; // no SELPARAM_DIST when homing
      }
    }
  }
  return selparam_st_res;
}

// ------ next digit
// simple function that calculates the next digit selection state
// depending on the  rotation of the knob
// we could have used the global variable, but instead we use arguments:
// seldigit_st_arg
// right, left (indicates if the nob is turned right or left

byte nxt_digit(byte seldigit_st_arg, bool right, bool left, byte selparam_st_arg)
{
  byte seldigit_st_res = seldigit_st_arg;

  if (right == true) {
    if (seldigit_st_res == SELDIG_UNITS) {
      seldigit_st_res = SELDIG_PARAM;
    } else {
      seldigit_st_res = seldigit_st_res + 1;
      if (seldigit_st_res == SELDIG_SIGN && selparam_st_arg == SELPARAM_VEL) {
        seldigit_st_res = seldigit_st_res + 1; // velocity has no sign
      }
    }
  } else if (left == true) {
    if (seldigit_st_res == SELDIG_PARAM) { 
      seldigit_st_res = SELDIG_UNITS;
    } else {
      seldigit_st_res = seldigit_st_res - 1;
      if (seldigit_st_res == SELDIG_SIGN && selparam_st_arg == SELPARAM_VEL) {
        seldigit_st_res = seldigit_st_res - 1; // velocity has no sign
      }
    }
  }
  return seldigit_st_res;
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

// ---------- lcdprint_endstops
// -- print the state of the endstops, the position in the LCD has to be
// -- set before the function call

void lcdprint_endstops()
{
  endstop_x_ini = digitalRead(X_MIN_PIN);
  endstop_x_end = digitalRead(X_MAX_PIN);
  
  if (endstop_x_ini == ENDSTOP_ON) {
    if (endstop_x_end == ENDSTOP_ON) {
      lcd.write(byte(CHR_FL_FL_BOX));
    } else {
      lcd.write(byte(CHR_FL_HL_BOX));
    }
  } else {
    if (endstop_x_end == ENDSTOP_ON) {
      lcd.write(byte(CHR_HL_FL_BOX));
    } else {
      lcd.write(byte(CHR_HL_HL_BOX));
    }
  }
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

  // print the state of the endstops
  lcd.setCursor(19, 3);
  lcdprint_endstops();
  
}


// ---------------- run relative screen
// -- moving a relative distancie
void runrel_screen()
{
  byte row = 0;

  lcd.createChar(CHR_X0, IC_X0); //
  lcd.createChar(CHR_XF, IC_XF); //

  lcd.clear();
  // -- row 0 
  row = 0;
  lcd.setCursor(0, row);
           //1234567890123456789
  lcd.print("RUNREL");
  lcd.write(CHR_XF);
  lcd.print("=");
  if (pos_x_eep_read == -1) {
    lcd.print("  ?");
    abs_dest = -1;
  } else {
    abs_dest = pos_x_eep_read + rel_dist_sign;
    lcdprint_rght(abs_dest,3);
  }
  lcd.write(CHR_MM);

  lcd.print("|");
  lcd.write(byte(CHR_X0));
  lcd.print("=");
  if (pos_x_eep_read == -1) {
    lcd.print("  ?");
  } else {
    lcdprint_rght(pos_x_eep_read,3);
  }
  lcd.write(CHR_MM);

  // -- row 1 
  row = 1;
  lcd.setCursor(0, row);
  lcd.print("v=");
  lcdprint_rght(vel_mmh,3);
  lcd.write(CHR_MM);
  lcd.write(CHR_PER_HOUR);

           //7890123456789
  lcd.print(" |00h 00m 00s");

  // -- row 2 steps
  row = 2;
  lcd.setCursor(0, row);
         //  0123456
  lcd.print("dS=");
  if (rel_dist_neg == false) {
    lcd.print("+");
  } else {
    lcd.print("-");
  }
  lcd.print("  0");
  lcd.write(CHR_MM);
           //8901
  lcd.print("|hs=");

  // -- row 3 lines
  row = 3;
  lcd.setCursor(0, row);

         //  01234567
  lcd.print("dL=");
  if (rel_dist_neg == false) {
    lcd.print("+");
  } else {
    lcd.print("-");
  }
  lcd.print("  0");
  lcd.write(CHR_MM);

  lcd.print("|lin=    0");


  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();

  // init the displacement
  relat_pos_mm_stp = 0; // calculated from halfsteps
  relat_pos_mm_lin = 0; // calculated from lines

  if (rel_dist_neg == false) {
    digitalWrite(X_DIR_PIN, DIR_MOTOR_POS); 
  } else {
    digitalWrite(X_DIR_PIN, DIR_MOTOR_NEG); // motor back
  }
}

// -------------- running_rel  run experiment with relative distance
// update the runrel variables on the screen
// this function stays in a loop until the distance is traversed,
// or any of the endstops are active

void running_rel() {

  // no need to use global vars o static, we remain in this function

  // local copies to work with them
  unsigned long hstp_cnt_copy;
  short         lps_line_cnt_copy;
  short         hour_cnt_copy;
  // for seconds minutes is not necessary because they are byte

  // to compare
  unsigned long hstp_cnt_prev     = 0;
  short         lps_line_cnt_prev = 0;
  byte          sec_cnt_prev      = 0;
  byte          minute_cnt_prev   = 0;
  short         hour_cnt_prev     = 0;

  while ((relat_pos_mm_stp <= rel_dist) &&
        !(relat_pos_mm_stp > 1 && (  // stop if any endstop is high, after starting, it can be made better
          endstop_x_ini == ENDSTOP_ON ||
          endstop_x_end == ENDSTOP_ON))) {

    // disable interrupts to make a copy to avoid corruption
    // for seconds minutes is not necessary because they are byte
    noInterrupts();
    hstp_cnt_copy     = hstp_cnt;
    lps_line_cnt_copy = lps_line_cnt;
    hour_cnt_copy     = hour_cnt;
    interrupts();

    if (hstp_cnt_copy != hstp_cnt_prev) {
      lcd.setCursor(4, 2);
      relat_pos_mm_stp = ADVAN_HSTEP * hstp_cnt_copy;
      lcdprint_rght(int(relat_pos_mm_stp),3);
      lcd.setCursor(12, 2);
      lcd.print(hstp_cnt_copy);
      hstp_cnt_prev = hstp_cnt_copy;
    }

    if (lps_line_cnt_copy != lps_line_cnt_prev) {
      lcd.setCursor(4, 3);
      relat_pos_mm_lin = mm_per_lps_line * lps_line_cnt_copy;
      lcdprint_rght(int(relat_pos_mm_lin),3);
      lcd.setCursor(12, 3);
      lcd.print(lps_line_cnt_copy);
      lps_line_cnt_prev = lps_line_cnt_copy;
    }
    
    if (sec_cnt != sec_cnt_prev) {
      if (minute_cnt != minute_cnt_prev) {
        if (hour_cnt_copy != hour_cnt_prev) {
          lcd.setCursor(9, 1);
          lcdprint_rght(hour_cnt_copy,2);
          hour_cnt_prev = hour_cnt_copy;
        }
        lcd.setCursor(13, 1);
        lcdprint_rght(minute_cnt,2);
        minute_cnt_prev = minute_cnt;
      }
      lcd.setCursor(17, 1);
      lcdprint_rght(sec_cnt,2);
      sec_cnt_prev = sec_cnt;
    }

    // endstop print, it reads the enstops, for the while
    lcd.setCursor(19, 3);
    lcdprint_endstops();
  }

  // deactivate interrupts
  disable_isr();
  // save values in EEPROM
  // HOMED state
}




// ---------------- homing screen

void homing_screen()
{
  byte row = 0;

  lcd.createChar(CHR_X0, IC_X0); //
  lcd.createChar(CHR_XF, IC_XF); //

  abs_dest = 0;  // destination is 0

  lcd.clear();
  // -- row 0 
  row = 0;
  lcd.setCursor(0, row);
  lcd.write(CHR_RGHT_ARROW);
           //1234567890123456789
  lcd.print("HOME:");
  lcd.write(CHR_XF);
  lcd.print("=  0");
  lcd.write(CHR_MM);

  lcd.print("|");
  lcd.write(byte(CHR_X0));
  lcd.print("=");
  if (pos_x_eep_read == -1) {
    lcd.print("  ?");
  } else {
    lcdprint_rght(pos_x_eep_read,3);
  }
  lcd.write(CHR_MM);

  // -- row 1 
  row = 1;
  lcd.setCursor(0, row);
  lcd.print("v=");
  lcdprint_rght(vel_mmh,3);
  lcd.write(CHR_MM);
  lcd.write(CHR_PER_HOUR);

           //7890123456789
  lcd.print(" |00h 00m 00s");

  // -- row 2 steps
  row = 2;
  lcd.setCursor(0, row);
         //  0123456
  lcd.print("dS=-  0");
  lcd.write(CHR_MM);
           //8901
  lcd.print("|hs=");

  // -- row 3 lines
  row = 3;
  lcd.setCursor(0, row);

         //  01234567
  lcd.print("dL=   0");
  lcd.write(CHR_MM);

  lcd.print("|lin=    0");


  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();

  digitalWrite(X_DIR_PIN, DIR_MOTOR_NEG); //homing:  motor back
}


// -------------- homing
// update the homing variables on the screen
// this function stays in a loop until the inital enstop is active

void homing() {

  // no need to use global vars o static, we remain in this function

  // local copies to work with them
  unsigned long hstp_cnt_copy;
  short         lps_line_cnt_copy;
  short         hour_cnt_copy;
  // for seconds minutes is not necessary because they are byte

  // to compare
  unsigned long hstp_cnt_prev     = 0;
  short         lps_line_cnt_prev = 0;
  byte          sec_cnt_prev      = 0;
  byte          minute_cnt_prev   = 0;
  short         hour_cnt_prev     = 0;

  while (endstop_x_ini == ENDSTOP_OFF) {

    // disable interrupts to make a copy to avoid corruption
    // for seconds minutes is not necessary because they are byte
    noInterrupts();
    hstp_cnt_copy     = hstp_cnt;
    lps_line_cnt_copy = lps_line_cnt;
    hour_cnt_copy     = hour_cnt;
    interrupts();

    if (hstp_cnt_copy != hstp_cnt_prev) {
      lcd.setCursor(4, 2);
      relat_pos_mm_stp = ADVAN_HSTEP * hstp_cnt_copy;
      lcdprint_rght(int(relat_pos_mm_stp),3);
      lcd.setCursor(12, 2);
      lcd.print(hstp_cnt_copy);
      hstp_cnt_prev = hstp_cnt_copy;
    }

    if (lps_line_cnt_copy != lps_line_cnt_prev) {
      lcd.setCursor(4, 3);
      relat_pos_mm_lin = mm_per_lps_line * lps_line_cnt_copy;
      lcdprint_rght(int(relat_pos_mm_lin),3);
      lcd.setCursor(12, 3);
      lcd.print(lps_line_cnt_copy);
      lps_line_cnt_prev = lps_line_cnt_copy;
    }
    
    if (sec_cnt != sec_cnt_prev) {
      if (minute_cnt != minute_cnt_prev) {
        if (hour_cnt_copy != hour_cnt_prev) {
          lcd.setCursor(9, 1);
          lcdprint_rght(hour_cnt_copy,2);
          hour_cnt_prev = hour_cnt_copy;
        }
        lcd.setCursor(13, 1);
        lcdprint_rght(minute_cnt,2);
        minute_cnt_prev = minute_cnt;
      }
      lcd.setCursor(17, 1);
      lcdprint_rght(sec_cnt,2);
      sec_cnt_prev = sec_cnt;
    }

    // endstop print, it reads the enstops, for the while
    lcd.setCursor(19, 3);
    lcdprint_endstops();
  }

  // deactivate interrupts
  disable_isr();
  save2eeprom();
  // save values in EEPROM
  // HOMED state
}

void save2eeprom()
{
  byte eeprom_valid = 1;

  EEPROM.put(EEP_DIR_VALID, eeprom_valid);
  EEPROM.put(EEP_DIR_POS_X, abs_dest);
  EEPROM.put(EEP_DIR_DIST,  rel_dist);
  EEPROM.put(EEP_DIR_VEL,   vel_mmh);
  EEPROM.put(EEP_DIR_SECS,  sec_cnt);
  EEPROM.put(EEP_DIR_MINS,  minute_cnt);
  EEPROM.put(EEP_DIR_HOURS, hour_cnt);

}


// -------------- Interrupt function to count seconds

void second_cnt_isr() {
  
  if (sec_cnt == 59) {
    sec_cnt = 0;
    if (minute_cnt == 59) {
      minute_cnt = 0;
      hour_cnt ++;
    } else {
      minute_cnt ++;
    }
  } else {
    sec_cnt ++;
  }
}


// -------------- Interrupt function to count the microsteps and generate
// ------ the pulses for the motor
// -- this is for the fast speeds, when the halfsteps interrupt is not needed

void h_ustp_fast_isr() {
  // this is local, independent from the half_step count (hstp_isr)
  // unlike the h_ustp_slow_isr, that is initialized by hstp_isr, so they
  // use a global variable
  static byte h_ustp_cnt_loc = 0;

  // a half step has 16 microsteps, but 32 half of a ustep, half of the time
  // low, and the other high
  if (h_ustp_cnt_loc == 31){
    h_ustp_cnt_loc = 0;
    hstp_cnt ++;  // increment halfsteps
  } else {
    h_ustp_cnt_loc ++;
  }
  //first time, it was 0 until the interruption comes
  digitalWrite(X_STEP_PIN , bitRead(h_ustp_cnt_loc,0));


  /* other option
  static byte step_value = LOW; // signal to send to stepper motor pin
  // this is local, independent from the half_step count (hstp_isr)
  // unlike the h_ustp_slow_isr, that is initialized by hstp_isr, so they
  // use a global variable
  static h_ustp_cnt_loc = 0;

  step_value = !step_value;
  digitalWrite(X_STEP_PIN , step_value); //first time, it was 0, for the interval
  // a half step has 16 microsteps, but 32 half of a ustep, half of the time
  // low, and the other high
  if (h_ustp_cnt_loc == 31){
    h_ustp_cnt_loc = 0;
    hstp_cnt ++;  // increment halfsteps
  } else {
    h_ustp_cnt_loc ++;
  }
  */

}

// -------------- Interrupt function to count the microsteps and generate
// ------ the pulses for the motor

void h_ustp_slow_isr() {
  // this function uses a global volatile h_ustp_cnt variable, unlike
  // h_ustp_fast_isr
  // here is global because it has to be initialized by hstp_isr

  if (h_ustp_cnt < 32) {
    h_ustp_cnt ++;
    //first time, it was 0, for the interval
    digitalWrite(X_STEP_PIN, bitRead(h_ustp_cnt,0));
  }


  /* another option
  static byte step_value = LOW; // signal to send to stepper motor pin
  // this function uses a global h_ustp_cnt variable, unlike h_ustp_fast_isr
  // here is global because it has to be initialized by hstp_isr

  if (h_ustp_cnt < 32) {
    step_value = !step_value;
    digitalWrite(X_STEP_PIN , step_value); //first time, it was 0, for the interval
    h_ustp_cnt ++;
  }
  */
}

// -------------- Interrupt function to count the number of halfsteps

void hstp_isr() {
  h_ustp_cnt  = 0; // initialize the ustpes

  hstp_cnt ++;
}

// -------------  linear encoder interrupt to read lines
void lin_encoder_isr() {
  byte lps_enc2;   // Value of linear position sensor (lps) encoder channel 2

  // linear position sensor encoder channel 2
  lps_enc2 = digitalRead (LPS_ENC2_PIN); // linear position sensor encoder 
  if (lps_enc2 == LOW){
    lps_line_cnt++;
  } else{
    lps_line_cnt--;
  }
}



// ------------------------ check_pos_x_eeprom
// -- get the value of the gantry position saved in EEPROM

void check_pos_x_eeprom()
{
  endstop_x_ini = digitalRead(X_MIN_PIN);
  endstop_x_end = digitalRead(X_MAX_PIN);

  EEPROM.get(EEP_DIR_VALID, eeprom_valid_read);

  if (eeprom_valid_read == 1) {
    EEPROM.get(EEP_DIR_POS_X, pos_x_eep_read);
    EEPROM.get(EEP_DIR_DIST,  dist_eep_read);
    EEPROM.get(EEP_DIR_VEL,   vel_eep_read);
    EEPROM.get(EEP_DIR_SECS,  secs_eep_read);
    EEPROM.get(EEP_DIR_MINS,  mins_eep_read);
    EEPROM.get(EEP_DIR_HOURS, hours_eep_read);
  } else {
    pos_x_eep_read = -1;  
  }
  /*
  if (endstop_x_ini == ENDSTOP_ON) {
    pos_x_eeprom = 0;
    EEPROM.put(EEPROM_DIR, pos_x_eeprom);
  } else if (endstop_x_end == ENDSTOP_ON) {
    pos_x_eeprom = TOT_LEN; // AAA: this value has to be calculated
    EEPROM.put(EEPROM_DIR, pos_x_eeprom);
  }
  */
}



//  ----- Print task menu on screen
//  select GO HOME o move x mm

void task_menu()
{

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Go Home");

  lcd.setCursor(1, 1);
  lcd.print("Relative move");

  lcd.setCursor(LCD_EEP_POS_COL, 3);
  lcd.print("|x:");
  if (pos_x_eep_read == -1) {
    lcd.print("  ?");
  } else {
    lcdprint_rght(pos_x_eep_read,3);
  }
  lcd.write(CHR_MM);

  lcd.setCursor(0, 0);
  if (task_st == TASK_HOME) {
    lcd.write(byte(CHR_FL_DIAM));
    lcd.setCursor(0, 1);
    lcd.write(byte(CHR_HL_DIAM));
  } else {
    lcd.write(byte(CHR_HL_DIAM));
    lcd.setCursor(0, 1);
    lcd.write(byte(CHR_FL_DIAM));
  }

  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();

}

// ------------------- update_task_menu

void update_task_menu()
{

  static byte task_st_prev = TASK_HOME;

  if (task_st_prev != task_st) {
    // draw the diamonds
    lcd.setCursor(0, 0);
    if (task_st == TASK_HOME) {
      lcd.write(byte(CHR_FL_DIAM));
      lcd.setCursor(0, 1);
      lcd.write(byte(CHR_HL_DIAM));
    } else {
      lcd.write(byte(CHR_HL_DIAM));
      lcd.setCursor(0, 1);
      lcd.write(byte(CHR_FL_DIAM));
    }
    // endstop print (just in case)
    lcd.setCursor(19, 3);
    lcdprint_endstops();
  }
  task_st_prev = task_st;
}



void param_menu ()
{

  lcd.clear();
  // -- row 0
  lcd.setCursor(0, 0);
  if (selparam_st == SELPARAM_VEL) {
    lcd.write(byte(CHR_FL_DIAM));
  } else {
    lcd.write(byte(CHR_HL_DIAM));
  }

  lcd.setCursor(1, 0);
  lcd.print("Speed:");
  lcd.setCursor(12, 0);
  lcdprint_rght(vel_mmh,3);
  lcd.setCursor(16, 0);
  lcd.write(CHR_MM);
  lcd.write(CHR_PER_HOUR);

  // -- row 1 
  lcd.setCursor(1, 1);
  if (task_st == TASK_HOME) {
    lcd.print("Go Home: xf= 0 ");
    lcd.write(CHR_MM);
  } else {
    lcd.setCursor(0, 1);
    if (selparam_st == SELPARAM_DIST) {
      lcd.write(byte(CHR_FL_DIAM));
    } else {
      lcd.write(byte(CHR_HL_DIAM));
    }
    lcd.print("Dist(d):");
    lcd.setCursor(11, 1);
    if (rel_dist_neg == true) {
      lcd.print("-");
    } else {
      lcd.print("+");
    }
    lcd.setCursor(12, 1);
    lcdprint_rght(rel_dist,3);
    lcd.setCursor(16, 1);
    lcd.write(CHR_MM);
  }

  // -- row 2 GO
  lcd.setCursor(0, 2);
  if (selparam_st == SELPARAM_GO) {
    lcd.write(byte(CHR_FL_DIAM));
  } else {
    lcd.write(byte(CHR_HL_DIAM));
  }
  lcd.setCursor(1, 2);
  lcd.print("Go!");


  // -- row 3, go back
  lcd.setCursor(0, 3);
  if (selparam_st == SELPARAM_BACK) {
    lcd.write(byte(CHR_FL_DIAM));
  } else {
    lcd.write(byte(CHR_HL_DIAM));
  }
  lcd.setCursor(1, 3);
  lcd.print("Back");
  lcd.write(CHR_LEFT_ARROW);

  // aditional info:
  lcd.setCursor(LCD_EEP_POS_COL, 3);
  lcd.print("|x:");
  if (pos_x_eep_read == -1) {
    lcd.print("  ?");
  } else {
    lcdprint_rght(pos_x_eep_read,3);
  }
  lcd.write(CHR_MM);

  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();
}


void update_param_menu() {

  byte col, row;
  static byte ui_state_prev = ST_SEL_PARAMS;
  static byte selparam_st_prev = SELPARAM_VEL;
  static byte seldigit_st_prev = SELDIG_PARAM;
  static short rel_dist_prev = 0; // relative destination
  static bool  rel_dist_neg_prev = false; // sign
  static short vel_mmh_prev  = MAX_VEL;  

  if (ui_state >= ST_SEL_PARAMS && ui_state <= ST_SEL_VALUE) {
    if (selparam_st != selparam_st_prev ) {

      lcd.setCursor(0, 0);
      if (selparam_st == SELPARAM_VEL) {
        lcd.write(byte(CHR_FL_DIAM));
      } else {
        lcd.write(byte(CHR_HL_DIAM));
      }
      
      if (task_st != TASK_HOME) {
        lcd.setCursor(0, 1);
        if (selparam_st == SELPARAM_DIST) {
          lcd.write(byte(CHR_FL_DIAM));
        } else {
          lcd.write(byte(CHR_HL_DIAM));
        }
      }

      lcd.setCursor(0, 2);
      if (selparam_st == SELPARAM_GO) {
        lcd.write(byte(CHR_FL_DIAM));
      } else {
        lcd.write(byte(CHR_HL_DIAM));
      }

      lcd.setCursor(0, 3);
      if (selparam_st == SELPARAM_BACK) {
        lcd.write(byte(CHR_FL_DIAM));
      } else {
        lcd.write(byte(CHR_HL_DIAM));
      }
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
      case SELDIG_SIGN:
        col = LCD_PARAMS_COL-1;
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
    if (vel_mmh != vel_mmh_prev) {
      lcd.setCursor(LCD_PARAMS_COL, SELPARAM_VEL); // row 0
      lcdprint_rght(vel_mmh,3); //3 is the number of digits
    }
    if (task_st != TASK_HOME) {
      if (rel_dist != rel_dist_prev) {
        lcd.setCursor(LCD_PARAMS_COL, SELPARAM_DIST);
        lcdprint_rght(rel_dist,3); //3 is the number of digits
      }
      if (rel_dist_neg != rel_dist_neg_prev) {
        lcd.setCursor(col, SELPARAM_DIST);
        if (rel_dist_neg == true) {
          lcd.print("-");
        } else {
          lcd.print("+");          
        }
      }
    }

    if  ((ui_state_prev    != ui_state       ) ||
         (selparam_st_prev != selparam_st    ) ||
         (seldigit_st_prev != seldigit_st    ) ||
         (rel_dist_prev    != rel_dist       ) ||
         (rel_dist_neg_prev != rel_dist_neg  ) ||         
         (vel_mmh_prev     != vel_mmh        )) {  
      lcd.setCursor(col, selparam_st); //row is directly defined by selparam_st
    }
  }

  // uptade previous values
  ui_state_prev     = ui_state;
  selparam_st_prev  = selparam_st;
  seldigit_st_prev  = seldigit_st;
  rel_dist_prev     = rel_dist;
  rel_dist_neg_prev = rel_dist_neg;
  vel_mmh_prev      = vel_mmh;
}


//----------- confirm_menu

void confirm_menu ()
{

  lcd.clear();
  // -- row 0

  lcd.setCursor(1, 0);
  lcd.print("Speed:");
  lcd.setCursor(12, 0);
  lcdprint_rght(vel_mmh,3);
  lcd.setCursor(16, 0);
  lcd.write(CHR_MM);
  lcd.write(CHR_PER_HOUR);

  // -- row 1 
  lcd.setCursor(1, 1);
  if (task_st == TASK_HOME) {
    lcd.print("Go Home: xf= 0 ");
    lcd.write(CHR_MM);
  } else {
    lcd.setCursor(1, 1);
    lcd.print("Dist(d):");
    lcd.setCursor(11, 1);
    if (rel_dist_neg == true) {
      lcd.print("-");
    } else {
      lcd.print("+");
    }
    lcd.setCursor(12, 1);
    lcdprint_rght(rel_dist,3);
    lcd.setCursor(16, 1);
    lcd.write(CHR_MM);
  }

  // -- row 2 GO
  lcd.setCursor(0, 2);
  if (confirm_st == CONFIRM_YES) {
    lcd.write(byte(CHR_FL_DIAM));
  } else {
    lcd.write(byte(CHR_HL_DIAM));
  }
  lcd.setCursor(1, 2);
  lcd.print("Confirm");

  // -- row 3, go back
  lcd.setCursor(0, 3);
  if (confirm_st == CONFIRM_NO) {
    lcd.write(byte(CHR_FL_DIAM));
  } else {
    lcd.write(byte(CHR_HL_DIAM));
  }
  lcd.setCursor(1, 3);
  lcd.print("Cancel");
  lcd.write(CHR_LEFT_ARROW);

  // aditional info:
  lcd.setCursor(LCD_EEP_POS_COL, 3);
  lcd.print("|x:");
  if (pos_x_eep_read == -1) {
    lcd.print("  ?");
  } else {
    lcdprint_rght(pos_x_eep_read,3);
  }
  lcd.write(CHR_MM);

  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();
}



//----------- update_confirm_menu

void update_confirm_menu ()
{

  // -- row 0

  // -- row 1 

  // -- row 2 GO
  lcd.setCursor(0, 2);
  if (confirm_st == CONFIRM_YES) {
    lcd.write(byte(CHR_FL_DIAM));
  } else {
    lcd.write(byte(CHR_HL_DIAM));
  }

  // -- row 3, Cancel
  lcd.setCursor(0, 3);
  if (confirm_st == CONFIRM_NO) {
    lcd.write(byte(CHR_FL_DIAM));
  } else {
    lcd.write(byte(CHR_HL_DIAM));
  }

}

// ---------------- disable interrupts
// when finishing the experiment

void disable_isr()
{
  byte t_half_ustp; // time that a half microstep takes
  unsigned long t_half_stp; // time that a half of a step takes

  Timer1.detachInterrupt(); // microstep interrupt
  if (t_half_ustp == 200) { // slow speed
    Timer4.detachInterrupt(); // there has no interrupt for halfsteps
  }
  // interrupt for the linear position sensor
  detachInterrupt(digitalPinToInterrupt(LPS_ENC1_PIN));
  Timer3.detachInterrupt(); // Second counter
}


// ---------------- enable interrupts
// when starting the experiment

void enable_isr()
{
  unsigned long t_half_stp = 0; // time that a half of a step takes

  t_half_ustp = ((byte)vec_t_h_ustp[vel_mmh]);

  // interrupt for the linear position sensor
  attachInterrupt(digitalPinToInterrupt(LPS_ENC1_PIN),
                  lin_encoder_isr, RISING);

  // interrupt for the counter of seconds, to keep track of the time 
  Timer3.attachInterrupt(second_cnt_isr); // Second counter
  Timer3.initialize(1000000);   // A million microseconds to count seconds


  if (t_half_ustp == 200) { // slow speed
    t_half_stp = ((unsigned long)(vec_t_h_stp[vel_mmh]));

    // attach function for half micro steps interruption
    Timer1.attachInterrupt(h_ustp_slow_isr);
    // attach function for halfsteps interruption
    Timer4.attachInterrupt(hstp_isr); 

    Timer4.initialize(t_half_stp);

  } else { // fast speed,no half step timer
    // attach function for half micro steps interruption
    Timer1.attachInterrupt(h_ustp_fast_isr);

  }
  Timer1.initialize(t_half_ustp);
}



void loop() {

  short val_incr;
  short aux_val;
  bool rot_enc_pushed;  // if LCD rotary encoder pushed
  unsigned long t_half_ustep = 0;   // us that takes half of a microstep
  unsigned long t_half_step = 0;    // us that takes a half step

  switch (ui_state){
    case ST_INI:
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == true) { 
        // get the EEPROM value (only once)
        check_pos_x_eeprom();
        task_menu();
        ui_state = ST_SEL_TASK;
      }
      break;
    case ST_SEL_TASK: // Choosing between going home or moving distance
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == true) { // select parameters
        param_menu();
        ui_state = ST_SEL_PARAMS;
      } else {
        read_rot_encoder_dir();
        if (rot_enc_rght == true || rot_enc_left == true) {
          // only 2, so the other
          if (task_st == TASK_HOME) {
            task_st = TASK_MOVE_REL;
          } else {
            task_st = TASK_HOME;
          }
          selparam_st = SELPARAM_VEL; // start selecting speed
          update_task_menu();
        }
      }
      break;
    case ST_SEL_PARAMS: // Selecting speed, distance (if not homing) & go back
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == true) { 
        switch (selparam_st) {
          case SELPARAM_VEL:
          case SELPARAM_DIST:
            ui_state = ST_SEL_DIGIT;
            // when comming from ST_SEL_PARAMS, start with units
            seldigit_st = SELDIG_UNITS;
            update_param_menu();
            break;
          case SELPARAM_GO:
            ui_state = ST_CONFIRM;
            confirm_menu();
            break;
          case SELPARAM_BACK:
            ui_state = ST_SEL_TASK;
            task_menu();
            break;
          default:
            break;
        }
      } else {
        read_rot_encoder_dir();
        selparam_st = nxt_param(selparam_st, rot_enc_rght, rot_enc_left, task_st);
        update_param_menu();
      }
      break;
    case ST_SEL_DIGIT: // Selecting digit to change
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == true) { // digit selected -> select value
        if (seldigit_st == SELDIG_PARAM) { // go back to ST_SEL_PARAMS
          ui_state = ST_SEL_PARAMS;
        } else {
          ui_state  = ST_SEL_VALUE;
        }
      } else { // knob not pushed
        read_rot_encoder_dir();
        seldigit_st = nxt_digit(seldigit_st, rot_enc_rght, rot_enc_left, selparam_st);
      } 
      update_param_menu();
      break;
    case ST_SEL_VALUE: // Setting/selecting the digit value
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == true) { // value selected, go back
        // we can only be here if we are changing values (units, 10s, 100s, sign)
        // of vel and dist
        ui_state = ST_SEL_DIGIT;
      } else {
        read_rot_encoder_dir();
        val_incr = calc_incr(seldigit_st, rot_enc_rght, rot_enc_left);
        if (rot_enc_rght == true) { // adding
          switch (selparam_st) {
            case SELPARAM_VEL: // changing the velocity
              aux_val = vel_mmh + val_incr;
              if (aux_val > MAX_VEL) {
                vel_mmh = MAX_VEL;
              } else {
                vel_mmh = aux_val;
              }
              break;
            case SELPARAM_DIST: // changing the destination
              if (seldigit_st == SELDIG_SIGN) {
                rel_dist_neg = ! rel_dist_neg;
              } else {
                aux_val = rel_dist + val_incr;
                if (aux_val > TOT_LEN) {
                  rel_dist = TOT_LEN;
                } else {
                  rel_dist = aux_val;
                }
                if (rel_dist_neg) {
                  rel_dist_sign = - rel_dist;
                } else {
                  rel_dist_sign = rel_dist;
                }
              }
              break;
            default: // would be an error to be here
              break;
          }
        } else if (rot_enc_left == true) { // substracting
           switch (selparam_st) {
            case SELPARAM_VEL: // velocity
              aux_val = vel_mmh + val_incr; // val_incr is negative
              if (aux_val < 1) {
                vel_mmh = 1;
              } else {
                vel_mmh = aux_val;
              }
              break;
            case SELPARAM_DIST: // changing the destination
              if (seldigit_st == SELDIG_SIGN) {
                rel_dist_neg = ! rel_dist_neg;
              } else {
                aux_val = rel_dist + val_incr;
                if (aux_val < 1) {
                  rel_dist = 1;  // no 0 distance
                } else {
                  rel_dist = aux_val;
                }
              }
              break;
            default: // would be an error to be here
              break;
          }
        }
      }
      update_param_menu();
      break;
    case ST_CONFIRM: 
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == true) { // confirmation or go back
        if (confirm_st == CONFIRM_YES) {
          if (task_st == TASK_HOME) {
            ui_state = ST_HOMING;
            homing_screen();
          } else {
            ui_state = ST_RUN;
            runrel_screen();
          }
          digitalWrite(X_ENABLE_PIN , LOW);  // Stepper motor enable. Active-low
          enable_isr(); // enable interrupts for the experiment
        } else {
          ui_state = ST_SEL_PARAMS;
          param_menu();
        }
      } else {
        read_rot_encoder_dir();
        if (rot_enc_rght == true || rot_enc_left == true) {
          // only 2 options
          if (confirm_st == CONFIRM_YES) {
            confirm_st = CONFIRM_NO;
          } else {
            confirm_st = CONFIRM_YES;
          }
          update_confirm_menu();
        }
      }
      break;
    case ST_HOMING: 
      homing();
      ui_state = ST_END;
      break;

    case ST_RUN: 
      running_rel();
      ui_state = ST_END;
      break;    
    case ST_END:
      digitalWrite(X_ENABLE_PIN , HIGH); // Stepper motor disable. Active-low
      rot_enc_pushed = rot_encoder_pushed();
        if (rot_enc_pushed == true) { // confirmation or go back
          ui_state = ST_INI;
          lcd.createChar(CHR_HL_DIAM, IC_HL_DIAM); // 0: hollow diamond
          lcd.createChar(CHR_FL_DIAM, IC_FL_DIAM); // 1: full diamond
          lcd.clear();
        }
      break;
  }
  
}

          
  
