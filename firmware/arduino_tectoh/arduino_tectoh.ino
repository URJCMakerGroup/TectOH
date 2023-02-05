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
#define ESTOP_INI_PIN 3         // INIT endstop x=0 (HIGH when pressed)
#define ESTOP_END_PIN 2         // END endstop (HIGH when pressed)


// ------------ Stepper motor
#define X_STEP_PIN 54       // STEP pin for the first stepper motor driver (axis X)
#define X_DIR_PIN 55        // DIR pin for the first stepper motor driver (axis X)
#define X_ENABLE_PIN 38     // ENABLE pin for the first stepper motor driver (axis X)

// defined in configuration file: tectoh_config.h
#if DIR_MOTOR_POSITIVE_HIGH
  #define DIR_MOTOR_POS HIGH
  #define DIR_MOTOR_NEG LOW
#else
  #define DIR_MOTOR_POS LOW
  #define DIR_MOTOR_NEG HIGH
#endif

#if ENABLE_MOTOR_LOW
  #define ENABLE_MOTOR  LOW
  #define DISABLE_MOTOR HIGH
#else
  #define ENABLE_MOTOR  HIGH
  #define DISABLE_MOTOR LOW
#endif

#if LCD_STOP_BTN_ACTIVE_HIGH
  #define STOP_BTN_ON   HIGH
  #define STOP_BTN_OFF  LOW
#else
  #define STOP_BTN_ON   LOW
  #define STOP_BTN_OFF  HIGH
#endif

// ------------ Linear position sensor LPS
  
#define LPS_ENC1_PIN  20  // Linear position encoder 1 pin
#define LPS_ENC2_PIN  21  // PIN del canal B del encoder


// in 400 mm there are about 2362 lines, so a int, or a short is enough
volatile short lps_line_cnt = 0;    // number of counted lines of the linear position sensor
const float mm_per_lps_line = 0.1693; // milimiters per line from linear pos sensor

// --- endstop values
byte estop_ini;    // endstop value at x=0
byte estop_end;    // endstop value at the end

// defined in configuration file: tectoh_config.h
#if ESTOP_ACTIVE_HIGH
  #define ESTOP_ON  HIGH
  #define ESTOP_OFF LOW
#else
  #define ESTOP_ON  LOW
  #define ESTOP_OFF HIGH
#endif

const int TOT_LEN = 270;    // leadscrew length in mm, maximum distance
const int MAX_VEL = 100;    // maximum velocity in mm/h

const int LEAD = 3;  // lead screw lead in mm. How many mm advances per revolution
const int STEP_REV = 200; // how many steps per revolution the motor has
const int HSTEP_REV = 2 * STEP_REV; // halfsteps per revolution

const float GEAR_R = 51.0f; // gear ratio of the motor

//advance in mm per halfstep  3 / (270 * 51) = 0.000 147
//const float ADVAN_HSTEP = float(LEAD) / (HSTEP_REV * GEAR_R);
const float ADVAN_HSTEP = 0.000147;

// ---- variables for keeping track the experiment timing

// and experiment could take 400 hours if 400 mm at 1mm/h, it is not likely
// but just in case, make it short, not byte
volatile short hour_cnt = 0;        // Keep track of the number of hours of the experiment
volatile byte minute_cnt = 0;       // Keep track of the number of minutes of the experiment
volatile byte sec_cnt = 0; // Keep track of the number of seconds of the experiment

// ----- Sandbox parameters

// EEPROM DATA

//#define EEPROM_DIR 0  // memory address of the EEPROM where the gantry position is

// ----------------- EEPROM addresses 

// 1 Valid, 0 not valid (defined by EEP_VALID_DATA, EEP_INVALID_DATA)
#define EEP_AD_VALID 0

// Actual position in mm: 0 to 400, short. 2 bytes
#define EEP_AD_POS_ACT 24

// Last experiment values
// Distance last experiment. 2 bytes
// with sign
#define EEP_AD_DIST  3

// original position of the previous experiment
#define EEP_AD_POS_PREV 5 // 2 bytes
// So we have: [EEP_AD_POS_ACT] = [EEP_AD_POS_PREV] + [EEP_AD_DIST]

// Speed last experiment. short. 2 bytes
// could be just 1 byte, but in case we get larger speeds
#define EEP_AD_VEL   7

// Time last experiment. 1 byte
#define EEP_AD_SECS  9
#define EEP_AD_MINS  10 // 1 byte
#define EEP_AD_HOURS 11 // 2 bytes

// distance calculated from halfsteps
#define EEP_AD_DIST_HS  13  // 2 bytes

// distance calculated from counting lines
#define EEP_AD_DIST_LINE  15  // 2 bytes

// number of halfsteps unsigned long 32 bits: 4 bytes
#define EEP_AD_HS_CNT  17  // 4 bytes

// number of lines short 16 bits: 2 bytes. less than 3000 lines
// can be positive or negative
#define EEP_AD_LINE_CNT  21  // 2 bytes

// state of the endstops
#define EEP_AD_ESTOP   23  // 1 bytes

#define EEP_BIT_ESTOP_INI 0 // in the bit 0 of address EEP_AD_ESTOP
#define EEP_BIT_ESTOP_END 1 // in the bit 1 of address EEP_AD_ESTOP

// bits to make the mask
#define EEP_MASK_ESTOP_INI 1<<EEP_BIT_ESTOP_INI // bit 0 of ad EEP_AD_ESTOP
#define EEP_MASK_ESTOP_END 1<<EEP_BIT_ESTOP_END // bit 1 of ad EEP_AD_ESTOP

// EEPPROM address EEP_AD_VALID
#define EEP_VALID_DATA   1 //data from EEPROM is valid
#define EEP_INVALID_DATA 0 //data from EEPROM is not valid
#define EEP_ESTOP_ERR    2 //postion and endstop info doesnt match

byte  eeprom_valid_read = EEP_INVALID_DATA;  // 0
short pos_act_eep_read  =  0;  // 1,2
short dist_eep_read     =  0;  // 3,4
short pos_prev_eep_read =  0;  // 5,6
short vel_eep_read      =  0;  // 7,8
byte  secs_eep_read     =  0;  // 9
byte  mins_eep_read     =  0;  // 10
short hours_eep_read     =  0; // 11,12
short dist_hs_eep_read   =  0; // 13,14 positive
short dist_line_eep_read =  0; // 15,16 pos o neg
// Number of half steps counted. a long has 32 bits -> being unsigned
// from 0 to 4,294,967,295
// 400 mm, with 3mm lead, 400 halfsteps per turn and 51 reduction,
// gives 0,147 um/halfstep, which is 
//               2,714,666.67 halfsteps (22 bits) So we can count them
unsigned long  hs_cnt_eep_read    =  0; // 17,18,19,20
short line_cnt_eep_read  =  0; // 21,22 pos o neg
byte estop_ini_eep_read = 0; //23 taken from the same address
byte estop_end_eep_read = 0; //23


// short are 16bits
// Speed of the Sandbox in mm/h, from 1 to 100, making it short, to have negative
// to make some operations
short vel_mmh = MAX_VEL;  // Speed of the Sandbox in mm/h, from 1 to 100
short pos_dest = 0;  // absolute position of the destination, from x0 in mm
//experiment's distance to the destination in mm, Magnitude (no sign)
short dist_dest_magn = 1; //experiment's distance to the destination in mm, no sign 
short dist_dest_wsign = 1; // relative position of the destination, with sign
bool  is_dist_dest_neg = false;  //sign of dist_dest_magn: false: positive, true: negative

volatile byte h_ustp_cnt = 0;    // Number of half usteps in a halfstep 0 to 31

// Number of half steps counted. a long has 32 bits -> being unsigned
// from 0 to 4,294,967,295
// 400 mm, with 3mm lead, 400 halfsteps per turn and 51 reduction,
// gives 0,147 um/halfstep, which is 
//               2,714,666.67 halfsteps (22 bits) So we can count them
volatile unsigned long hstp_cnt = 0;   // Number of half steps, from initial position

// traveled distance in the experiment
// Calculated from steps
float traveled_mm_hs_f = 0; // float version calculated from halfsteps
short traveled_mm_hs = 0; //  converted integer version

// Calculated from lines
float traveled_mm_lin_f = 0; // float version, will be converted calculated from lines
short traveled_mm_lin = 0; // calculated from lines
 

// array for the time of a half of a microstep (half time low, half high)
// For  3mm lead, 400 halfsteps per turn and 51 reduction, the fastest
// speeds, from 83mm/h and up, the h_ustp take less than 200 us
// for speed from 1mm/h to 82 mm/h, the h_microstp take 200 us
// This vector should be integer, because we are not usign decimals AAAAA

const float vec_t_h_ustp[] = {0,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,199.33,196.95,194.64,192.37,190.16,188.00,185.89,183.82,181.80,179.83,177.89,176.00,174.15,172.33,170.56,168.82,167.11,165.44};

// it is byte because is less than 200
byte t_half_ustp; // time that a half microstep takes, from the previous vector
const byte SLEW_VEL_T_H_USTP = 200; // microseconds lower than the speed will be continuos

// array for the time of a half step (h_stp), it is only used when the time
// a half microstep (h_ustp) is 200. i.e. for slow speeds.
// Otherwise, they would be out of sync
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
#define   ST_LAST_INFO  8   // Info last experiment  
#define   ST_END        9   // Moving ended

byte ui_state = ST_INI; 

#define   TASK_HOME      0   // Task Go Home
#define   TASK_MOVE_DIST 1   // Task Movement relative
#define   TASK_LAST_INFO 2   // Info last experiment

byte task_st = TASK_HOME;

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
#define CHR_ERR_ESTOP    174 // 0xAE: a kind of flipped E which can be error of endstop


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
  
  pinMode(ESTOP_INI_PIN, INPUT_PULLUP);   // Endstop init position
  pinMode(ESTOP_END_PIN, INPUT_PULLUP);   // Endstop final position

  pinMode(LCD_STOP_PIN, INPUT_PULLUP);   // Stop/kill button of the LCD
 
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
 
  digitalWrite(X_ENABLE_PIN , DISABLE_MOTOR); 
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

// ------ next task
// simple function that calculetes the next task state according
// on the rotation of the knob
// we could have used the global variable, but instead we use arguments:
// task_st_arg
byte nxt_task(byte task_st_arg, bool right, bool left)
{
  byte task_st_res = task_st_arg;

  if (right == true) {
    if (task_st_res == TASK_LAST_INFO) {
      task_st_res = TASK_HOME;
    } else {
      task_st_res = task_st_res + 1;
    }
  } else if (left == true) {
    if (task_st_res == TASK_HOME) { 
      task_st_res = TASK_LAST_INFO;
    } else { 
      task_st_res = task_st_res - 1;
    }
  }
  return task_st_res;
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

// -------------- lcdprint_errcode
// prints the error code of the error from the eeprom,
// short_print is true, only 3 chars will be printed

void lcdprint_errcode(byte eeprom_valid_read_arg)
{
  switch (eeprom_valid_read_arg) {
    case EEP_VALID_DATA:
      lcd.print(" VALID  ");
      break;
    case EEP_INVALID_DATA:
      lcd.print("INVALID "); // data from the eeprom is not valid
      break;
    case EEP_ESTOP_ERR:
      lcd.print("erENDST "); // there is an error with the position and endstop
      break;
  }
} // lcdprint_errcode

// -------------- lcdprint_val_err
// prints the value or the error code
// short_print is true, only 3 chars will be printed

void lcdprint_val_err(int number, byte eeprom_valid_read_arg, int max_digit)
{
  int index = 0;

  switch (eeprom_valid_read_arg) {
    case EEP_VALID_DATA:
      lcdprint_rght(number, max_digit);
      break;
    case EEP_INVALID_DATA:
      for (index=0; index<max_digit-1; index++) {
        lcd.print(" ");
      }
      lcd.print("?"); // data from the eeprom is not valid
      break;
    case EEP_ESTOP_ERR:
      for (index=0; index<max_digit-1; index++) {
        lcd.print(" ");
      }
      lcd.write(CHR_ERR_ESTOP); // there is an error with the position and endstop
      break;
  }
} // lcdprint_val_err


// -------------- lcdprint_rght
// print an integer in the lcd aligned to the right
// number: number to print
// col: colum to start printing (most left)
// row: row to print
// max_digit : maximum number of digits to be printed. Needed to align right

void lcdprint_rght (int number, int max_digit)
{
  int index = 0;
  int max_number;

  max_number = round(pow(10, max_digit-1));

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
// lcdprint_rght


// -------------- lcdprint_rght_sign
// print an integer in the lcd aligned to the right
// number: number to print
// col: colum to start printing (most left)
// row: row to print
// max_digit : maximum number of digits to be printed. Needed to align right
// the + - sign is included, but does not count in the digit
// for example to print +003 , max_digit is 3

void lcdprint_rght_sign (int number, byte max_digit)
{
  byte index = 0;
  int  max_number;
  int  number_positive;

  if (number >= 0) {
    lcd.print("+");
    number_positive = number;
  } else {
    lcd.print("-");
    number_positive = - number;
  }

  max_number = int(pow(10, max_digit-1));

  //lcd.setCursor(col, row);
  for (index = 0; index < max_digit-1; index++) {
    if (number_positive >= max_number) {
      break;
    }
    max_number = int(max_number/10);
    lcd.print("0");
  }  
  lcd.print(number_positive);
}

// -------------- lcdprint_errsymbol
// print an error symbol:
//   - right arrow: EEP_VALID_DATA  
//   - ?:           EEP_INVALID_DATA
//   - back E:      EEP_ERR_STOP

void lcdprint_errsymbol (byte eeprom_valid_read_arg)
{
  switch (eeprom_valid_read_arg) {
    case EEP_VALID_DATA:
      lcd.write(CHR_RGHT_ARROW); // ok
      break;
    case EEP_INVALID_DATA:
      lcd.print("?"); // data from the eeprom is not valid
      break;
    case EEP_ESTOP_ERR:
      lcd.write(CHR_ERR_ESTOP); // there is an error with the position and endstop
      break;
  }
}
// lcdprint_errsymbol


// -------------- lcdprint_errcode
// print an integer in the lcd aligned to the right
// number: number to print
// max_digit : maximum number of digits to be printed. Needed to align right
// if the number is negative, it is an error code, it will print error

void lcdprint_errcode (int number, int max_digit)
{
  int index = 0;
  
  int max_number;
                     //0123456789  I think ten is more than enough
  char error_code[] = "Error     ";

  if (number >= 0) {
    max_number = int(pow(10, max_digit-1));
    for (index = 0; index < max_digit-1; index++) {
      if (number >= max_number) {
        break;
      }
      max_number = int(max_number/10);
      lcd.print("0");
    }  
    lcd.print(number);
  } else { // error code
    if (max_digit > 10) {
      max_digit = 10;
    }
    for (index = 0; index < max_digit-1; index++) {
      lcd.write(error_code[index]);
    }
  }

}
// lcdprint_errcode


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

// ---------- lcdprint_endstops_arg
// -- print the state of the eeprom endstops given by the arguments,
// -- the position in the LCD has to be set before the function call

void lcdprint_endstops_arg(byte estop_ini_arg, byte estop_end_arg)
{
  if (estop_ini_arg == ESTOP_ON) {
    if (estop_end_arg == ESTOP_ON) {
      lcd.write(byte(CHR_FL_FL_BOX));
    } else {
      lcd.write(byte(CHR_FL_HL_BOX));
    }
  } else {
    if (estop_end_arg == ESTOP_ON) {
      lcd.write(byte(CHR_HL_FL_BOX));
    } else {
      lcd.write(byte(CHR_HL_HL_BOX));
    }
  }
}


// ---------- lcdprint_endstops
// -- print the state of the endstops, the position in the LCD has to be
// -- set before the function call

void lcdprint_endstops()
{
  estop_ini = digitalRead(ESTOP_INI_PIN);
  estop_end = digitalRead(ESTOP_END_PIN);
  
  if (estop_ini == ESTOP_ON) {
    if (estop_end == ESTOP_ON) {
      lcd.write(byte(CHR_FL_FL_BOX));
    } else {
      lcd.write(byte(CHR_FL_HL_BOX));
    }
  } else {
    if (estop_end == ESTOP_ON) {
      lcd.write(byte(CHR_HL_FL_BOX));
    } else {
      lcd.write(byte(CHR_HL_HL_BOX));
    }
  }
}

// --------- endstop_hit_vel
// -- reads the value of the endstop that is in the direction of the velocity
// -- and returns true if it has bin hit

bool endstop_hit_vel ()
{

  if (is_dist_dest_neg) { // if the distance is negative
    estop_ini = digitalRead(ESTOP_INI_PIN); // read and save it in global var
    if (estop_ini == ESTOP_ON) {
      return true;
    }
  } else { // distance positive, check the max pin
    estop_end = digitalRead(ESTOP_END_PIN);
    if (estop_end == ESTOP_ON) {
      return true;
    }
  }
  return false;
} //endstop_hit_vel



// --------- stop_btn_pushed
// -- reads the value of the stop/kill button of the LCD and if it is pulsed
// -- returns true

bool stop_btn_pushed ()
{
  byte stop_btn;
  stop_btn = digitalRead(LCD_STOP_PIN);

  if (stop_btn == STOP_BTN_ON) {
    return true;
  } else {
    return false;
  }
} //stop_btn_pushed

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


// ---------------- run distance screen
// -- moving a relative distance
void run_dist_screen()
{
  byte row = 0;

  lcd.createChar(CHR_X0, IC_X0); //
  lcd.createChar(CHR_XF, IC_XF); //

  if (eeprom_valid_read  == EEP_VALID_DATA) {
    pos_dest = pos_act_eep_read + dist_dest_wsign; 
  }

  lcd.clear();
  // -- row 0 
  row = 0;
  lcd.setCursor(0, row);
           //1234567890123456789
  lcd.print("D:");
  lcdprint_rght_sign(dist_dest_wsign,3);

  lcd.print(" ");
  lcd.write(CHR_XF);
  lcd.print("=");
  lcdprint_val_err(pos_dest, eeprom_valid_read, 3);
  lcd.write(CHR_MM);

  lcd.print(" ");
  lcd.write(byte(CHR_X0));
  lcd.print("=");
  lcdprint_val_err(pos_act_eep_read, eeprom_valid_read, 3);
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
  lcd.print("ds= ");
  // no need to print sign, it will be printed with the number
  //if (is_dist_dest_neg == true) {
  //  lcd.print("-");
  //} else {
  //  lcd.print("+");
  //}
  lcd.print("  0");
  lcd.write(CHR_MM);
           //8901
  lcd.print("|hs=");

  // -- row 3 lines
  row = 3;
  lcd.setCursor(0, row);

         //  01234567
  lcd.print("dl= ");
  // no need to print sign, it will be printed with the number
  //if (is_dist_dest_neg == true) {
  //  lcd.print("-");
  //} else {
  //  lcd.print("+");
  //}
  lcd.print("  0");
  lcd.write(CHR_MM);

  lcd.print("|lin=    0");


  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();

  // init the displacement
  traveled_mm_hs = 0; // calculated from halfsteps
  traveled_mm_lin = 0; // calculated from lines

  if (is_dist_dest_neg == true) {
    digitalWrite(X_DIR_PIN, DIR_MOTOR_NEG); // motor back
  } else {
    digitalWrite(X_DIR_PIN, DIR_MOTOR_POS); 
  }
} //run_dist_screen

// -------------- run_distance  run experiment with relative distance
// update the run_dist variables on the screen
// this function stays in a loop until the distance is traversed,
// or any of the endstops are active

void run_distance() {

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

  // we will be running the experiment provided that:
  // - the traveled distance is less than the total distance
  // - we haven't hit the endstop on the direction we are moving
  // - if the stop/kill button has not been pushed
  while ((traveled_mm_hs < dist_dest_magn) &&
          !endstop_hit_vel() &&
          !stop_btn_pushed())  {

    // disable interrupts to make a copy to avoid corruption
    // for seconds minutes is not necessary because they are byte
    noInterrupts();
    hstp_cnt_copy     = hstp_cnt;
    lps_line_cnt_copy = lps_line_cnt;
    hour_cnt_copy     = hour_cnt;
    interrupts();

    if (hstp_cnt_copy != hstp_cnt_prev) {
      lcd.setCursor(4, 2);
      traveled_mm_hs_f = ADVAN_HSTEP * hstp_cnt_copy;
      // cannot be rounded, but floor, because it has to reach it
      traveled_mm_hs = floor(traveled_mm_hs_f); // cann
      lcdprint_rght(traveled_mm_hs,3);
      //lcd.setCursor(12, 2);
      //lcd.print(hstp_cnt_copy);
      hstp_cnt_prev = hstp_cnt_copy;
    }

    if (lps_line_cnt_copy != lps_line_cnt_prev) {
      lcd.setCursor(4, 3);
      traveled_mm_lin_f = mm_per_lps_line * lps_line_cnt_copy;
      // cannot be rounded, but floor, because it has to reach it
      traveled_mm_lin = floor(traveled_mm_lin_f);
      lcdprint_rght_sign(traveled_mm_lin,3);
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

    // endstop print, it reads the enstops (maybe it should not be here
    // because it is done also in the while)
    lcd.setCursor(19, 3);
    lcdprint_endstops();
  }

  // deactivate interrupts
  disable_isr();
  save2eeprom();
} // run_distance()


// ---------------- homing screen

void homing_screen()
{
  byte row = 0;

  lcd.createChar(CHR_X0, IC_X0); //
  lcd.createChar(CHR_XF, IC_XF); //

  pos_dest = 0;  // destination is 0

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
  lcdprint_val_err(pos_act_eep_read, eeprom_valid_read, 3);
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
  lcd.print("ds=-  0");
  lcd.write(CHR_MM);
           //8901
  lcd.print("|hs=");

  // -- row 3 lines
  row = 3;
  lcd.setCursor(0, row);

         //  01234567
  lcd.print("dl=   0");
  lcd.write(CHR_MM);

  lcd.print("|lin=    0");


  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();

  digitalWrite(X_DIR_PIN, DIR_MOTOR_NEG); //homing:  motor back
} //homing_screen()


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

  // estop_ini has been checked, and it is checked at the end of the while
  while ( estop_ini == ESTOP_OFF &&
         !stop_btn_pushed()) {

    // disable interrupts to make a copy to avoid corruption
    // for seconds minutes is not necessary because they are byte
    noInterrupts(); // -----------
    hstp_cnt_copy     = hstp_cnt;
    lps_line_cnt_copy = lps_line_cnt;
    hour_cnt_copy     = hour_cnt;
    interrupts();   // -----------

    if (hstp_cnt_copy != hstp_cnt_prev) {
      lcd.setCursor(4, 2);
      traveled_mm_hs_f = ADVAN_HSTEP * hstp_cnt_copy;
      // cannot be rounded, but floor, because it has to reach it
      traveled_mm_hs = floor(traveled_mm_hs_f);
      lcdprint_rght(traveled_mm_hs,3);
      //lcd.setCursor(12, 2);  // too much info to print
      //lcd.print(hstp_cnt_copy);
      hstp_cnt_prev = hstp_cnt_copy;
    }

    if (lps_line_cnt_copy != lps_line_cnt_prev) {
      lcd.setCursor(4, 3);
      traveled_mm_lin_f = mm_per_lps_line * lps_line_cnt_copy;
      // cannot be rounded, but floor, because it has to reach it
      traveled_mm_lin = floor(traveled_mm_lin_f);
      lcdprint_rght_sign(traveled_mm_lin,3);
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

    // lcdprint_endstops, it reads the enstops, for the while
    lcd.setCursor(19, 3);
    lcdprint_endstops();
  }

  // deactivate interrupts
  disable_isr();
  // the distance that have been traveled
  dist_dest_wsign = - traveled_mm_hs;
  // save values in EEPROM
  save2eeprom();
  // HOMED state
} //homing()


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
  // even numbers it will be LOW, odds number it will be HIGH
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

//---------------- save2eeprom

void save2eeprom()
{
  byte estop_eep_save = 0;

  estop_ini = digitalRead(ESTOP_INI_PIN);
  estop_end = digitalRead(ESTOP_END_PIN);

  estop_eep_save = ((estop_end << EEP_BIT_ESTOP_END) ||
                    (estop_ini << EEP_BIT_ESTOP_INI));
  
  EEPROM.put(EEP_AD_POS_ACT,  pos_dest);
  EEPROM.put(EEP_AD_DIST,     dist_dest_wsign);
  EEPROM.put(EEP_AD_POS_PREV, pos_act_eep_read);
  EEPROM.put(EEP_AD_VEL,      vel_mmh);
  EEPROM.put(EEP_AD_SECS,     sec_cnt);
  EEPROM.put(EEP_AD_MINS,     minute_cnt);
  EEPROM.put(EEP_AD_HOURS,    hour_cnt);
  EEPROM.put(EEP_AD_DIST_HS,   traveled_mm_hs);
  EEPROM.put(EEP_AD_DIST_LINE, traveled_mm_lin);
  EEPROM.put(EEP_AD_HS_CNT,    hstp_cnt);
  EEPROM.put(EEP_AD_LINE_CNT,  lps_line_cnt);
  EEPROM.put(EEP_AD_ESTOP,     estop_eep_save);

  EEPROM.put(EEP_AD_VALID,   EEP_VALID_DATA); // all the data has ben saved

  // --- now that all the data has been saved, put it to their initial values

  //pos_dest = 0;
  dist_dest_magn = 1;
  dist_dest_wsign = 1;
  is_dist_dest_neg = false;

  hstp_cnt  = 0;
  h_ustp_cnt = 0;

  traveled_mm_hs = 0;
  traveled_mm_lin = 0;

  lps_line_cnt = 0;

  sec_cnt = 0;
  minute_cnt = 0;
  hour_cnt = 0;
} //save2eeprom


// ------------------------ check_eeprom
// -- get the value of the gantry position saved in EEPROM
// -- AAA the congruence should be checked from the endstops and 
// -- the read positions

void check_eeprom()
{
  byte estop_eep_read;

  estop_ini = digitalRead(ESTOP_INI_PIN);
  estop_end = digitalRead(ESTOP_END_PIN);

  EEPROM.get(EEP_AD_VALID, eeprom_valid_read);

  if (eeprom_valid_read == EEP_VALID_DATA) {
    EEPROM.get(EEP_AD_POS_ACT,   pos_act_eep_read);
    EEPROM.get(EEP_AD_DIST,      dist_eep_read);
    EEPROM.get(EEP_AD_POS_PREV,  pos_prev_eep_read);
    EEPROM.get(EEP_AD_VEL,       vel_eep_read);
    EEPROM.get(EEP_AD_SECS,      secs_eep_read);
    EEPROM.get(EEP_AD_MINS,      mins_eep_read);
    EEPROM.get(EEP_AD_HOURS,     hours_eep_read);
    EEPROM.get(EEP_AD_DIST_HS,   dist_hs_eep_read);
    EEPROM.get(EEP_AD_DIST_LINE, dist_line_eep_read);
    EEPROM.get(EEP_AD_HS_CNT,    hs_cnt_eep_read);
    EEPROM.get(EEP_AD_LINE_CNT,  line_cnt_eep_read);
    EEPROM.get(EEP_AD_ESTOP,     estop_eep_read);

    // if the bit given by EEP_MASK_ESTOP_INI  is 1,
    // result 1, if it is 0, result 0
    estop_ini_eep_read = (estop_eep_read & EEP_MASK_ESTOP_INI);
    estop_ini_eep_read = estop_ini_eep_read >> EEP_BIT_ESTOP_INI;
    // this las sentence is equivalent to:
    // estop_ini_eep_read = min(estop_ini_eep_read, 1);

    // if the bit 1 is 1, result 1, if it is 0, result 0
    estop_end_eep_read = (estop_eep_read & EEP_MASK_ESTOP_END);
    estop_end_eep_read = estop_end_eep_read >> EEP_BIT_ESTOP_END;
    // this las sentence is equivalent to:
    // estop_end_eep_read = min(estop_end_eep_read, 1);

    if ((estop_ini_eep_read != estop_ini) ||
        (estop_end_eep_read != estop_end)) {
      eeprom_valid_read = EEP_ESTOP_ERR;
    }
  } else {
    eeprom_valid_read = EEP_INVALID_DATA;
  }
} //check_eeprom



//  ----- Print task menu on screen
//  select GO HOME o move x mm

void task_menu()
{

  lcd.createChar(CHR_HL_DIAM, IC_HL_DIAM); // 0: hollow diamond
  lcd.createChar(CHR_FL_DIAM, IC_FL_DIAM); // 1: full diamond
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Go Home");

  lcd.setCursor(1, 1);
  lcd.print("Move distance");

  lcd.setCursor(1, 2);
  lcd.print("Info LastExperiment");

  lcd.setCursor(LCD_EEP_POS_COL, 3);
  lcd.print("|x:");
  lcdprint_val_err(pos_act_eep_read, eeprom_valid_read, 3);
  lcd.write(CHR_MM);

  lcd.setCursor(0, 0);
  lcd.write(byte(CHR_HL_DIAM));
  lcd.setCursor(0, 1);
  lcd.write(byte(CHR_HL_DIAM));
  lcd.setCursor(0, 2);
  lcd.write(byte(CHR_HL_DIAM));
  switch (task_st) {
    case TASK_HOME:
      lcd.setCursor(0, 0);
      break;
    case TASK_MOVE_DIST:
      lcd.setCursor(0, 1);
      break;
    case TASK_LAST_INFO:
      lcd.setCursor(0, 2);
      break;
  }
  lcd.write(byte(CHR_FL_DIAM));

  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();

}//task_menu

// ------------------- update_task_menu

void update_task_menu()
{
  static byte task_st_prev = TASK_HOME;

  if (task_st_prev != task_st) {
    // draw the diamonds
    lcd.setCursor(0, 0);
    lcd.write(byte(CHR_HL_DIAM));
    lcd.setCursor(0, 1);
    lcd.write(byte(CHR_HL_DIAM));
    lcd.setCursor(0, 2);
    lcd.write(byte(CHR_HL_DIAM));
    switch (task_st) {
      case TASK_HOME:
        lcd.setCursor(0, 0);
        break;
      case TASK_MOVE_DIST:
        lcd.setCursor(0, 1);
        break;
      case TASK_LAST_INFO:
        lcd.setCursor(0, 2);
        break;
    }
    lcd.write(byte(CHR_FL_DIAM));

    // endstop print (just in case)
    lcd.setCursor(19, 3);
    lcdprint_endstops();
  }
  task_st_prev = task_st;
} //update_task_menu


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
    lcd.print("Dist(d): ");
    lcd.setCursor(11, 1);
    lcdprint_rght_sign(dist_dest_wsign,3);
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
  lcdprint_val_err(pos_act_eep_read, eeprom_valid_read, 3);
  lcd.write(CHR_MM);

  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();
} //param_menu

// -- updates the menu of parameters when they have been changed

void update_param_menu() {

  byte col, row;
  static byte ui_state_prev = ST_SEL_PARAMS;
  static byte selparam_st_prev = SELPARAM_VEL;
  static byte seldigit_st_prev = SELDIG_PARAM;
  // relative destination distance, no sign
  static short dist_dest_magn_prev = 0; // relative destination distance, no sign
  static bool  is_dist_dest_neg_prev = false; // sign
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
      if (dist_dest_magn != dist_dest_magn_prev) {
        lcd.setCursor(LCD_PARAMS_COL, SELPARAM_DIST);
        lcdprint_rght(dist_dest_magn,3); //3 is the number of digits
      }
      if (is_dist_dest_neg != is_dist_dest_neg_prev) {
        lcd.setCursor(LCD_PARAMS_COL-1, SELPARAM_DIST);
        if (is_dist_dest_neg) {
          lcd.print("-");
        } else {
          lcd.print("+");          
        }
      }
    }

    if  ((ui_state_prev         != ui_state       ) ||
         (selparam_st_prev      != selparam_st    ) ||
         (seldigit_st_prev      != seldigit_st    ) ||
         (dist_dest_magn_prev   != dist_dest_magn ) ||
         (is_dist_dest_neg_prev != is_dist_dest_neg  ) ||         
         (vel_mmh_prev          != vel_mmh        )) {  
      lcd.setCursor(col, selparam_st); //row is directly defined by selparam_st
    }
  }

  // uptade previous values
  ui_state_prev     = ui_state;
  selparam_st_prev  = selparam_st;
  seldigit_st_prev  = seldigit_st;
  dist_dest_magn_prev = dist_dest_magn;
  is_dist_dest_neg_prev = is_dist_dest_neg;
  vel_mmh_prev      = vel_mmh;
} //update_param_menu


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
    lcdprint_rght_sign(dist_dest_wsign,3);
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
  lcdprint_val_err(pos_act_eep_read, eeprom_valid_read, 3);
  lcd.write(CHR_MM);

  // endstop print
  lcd.setCursor(19, 3);
  lcdprint_endstops();
} // confirm_menu

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
} //update_confirm_menu

// --------------- last_info_menu
// -- displays the information of the last experiment.
// -- it doesnt read from the EEPROM because it has been already read

void last_info_menu()
{
  byte row = 0;

  lcd.createChar(CHR_X0, IC_X0); //
  lcd.createChar(CHR_XF, IC_XF); //

  lcd.clear();
  // -- row 0 
  row = 0;
  lcd.setCursor(0, row);
  lcdprint_errsymbol(eeprom_valid_read);

  lcd.print("v");
  lcdprint_rght(vel_eep_read,3);
  lcd.write(CHR_MM);
  lcd.write(CHR_PER_HOUR);

  lcd.print(" ");
  lcd.write(CHR_XF);
  lcd.print("=");
  lcdprint_errcode(pos_act_eep_read,3);
  lcd.write(CHR_MM);

  lcd.print(" ");
  lcd.write(byte(CHR_X0));
  lcd.print("=");
  lcdprint_errcode(pos_prev_eep_read,3);
  lcd.write(CHR_MM);

  // -- row 1 
  row = 1;
  lcd.setCursor(0, row);
  lcd.print("d=");
  lcdprint_rght_sign(dist_eep_read,3);
  lcd.write(CHR_MM);

           //7890123456789
  lcd.print("  ");
  lcdprint_rght(hours_eep_read,2);
  lcd.print("h ");
  lcdprint_rght(mins_eep_read,2);
  lcd.print("m ");
  lcdprint_rght(secs_eep_read,2);
  lcd.print("s");

  // -- row 2 steps
  row = 2;
  lcd.setCursor(0, row);

  lcd.print("ds=");
  lcdprint_rght_sign(dist_hs_eep_read,3);

  lcd.write(CHR_MM);
  lcd.print(" hs=");
  lcd.print(hs_cnt_eep_read);

  // -- row 3 lines
  row = 3;
  lcd.setCursor(0, row);
         //  01234567

  lcd.print("dl=");
  lcdprint_rght_sign(dist_line_eep_read,3);
  lcd.write(CHR_MM);

  lcd.print(" ln=");
  lcd.print(lps_line_cnt);


  // EEPROM endstop print
  lcd.setCursor(18, 3);
  lcdprint_endstops_arg(estop_ini_eep_read, estop_end_eep_read);

  // endstop print
  lcdprint_endstops();

} //last_info_menu

// ---------------- disable interrupts
// when finishing the experiment.
// Motors are also disabled

void disable_isr()
{

  t_half_ustp; // time that a half microstep takes
  digitalWrite(X_ENABLE_PIN , DISABLE_MOTOR);
  Timer1.detachInterrupt(); // microstep interrupt
  if (t_half_ustp == SLEW_VEL_T_H_USTP) { // slow speed
    Timer4.detachInterrupt(); // there was interrupt for halfsteps
  }
  // interrupt for the linear position sensor
  detachInterrupt(digitalPinToInterrupt(LPS_ENC1_PIN));
  Timer3.detachInterrupt(); // Second counter
}


// ---------------- enable interrupts
// when starting the experiment.
// the motor is also enabled
// the EEPROM values are no longer valid

void enable_isr()
{
  unsigned t_half_stp = 0; // time that a half of a step takes

  t_half_ustp = round(vec_t_h_ustp[vel_mmh]);

  digitalWrite(X_ENABLE_PIN , ENABLE_MOTOR);

  // EEPROM data is not going to be valid if it is powered off from now
  EEPROM.put(EEP_AD_VALID, EEP_INVALID_DATA);

  // interrupt for the linear position sensor
  attachInterrupt(digitalPinToInterrupt(LPS_ENC1_PIN),
                  lin_encoder_isr, RISING);

  // interrupt for the counter of seconds, to keep track of the time 
  Timer3.attachInterrupt(second_cnt_isr); // Second counter
  Timer3.initialize(1000000);   // A million microseconds to count seconds


  if (t_half_ustp == SLEW_VEL_T_H_USTP) { // slow speed
    t_half_stp = round(vec_t_h_stp[vel_mmh]);

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
        check_eeprom();
        task_menu();
        ui_state = ST_SEL_TASK;
      }
      break;
    case ST_SEL_TASK:
      // Choosing between going home, moving distance, or info last experiment
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == true) { // select parameters
        if (task_st == TASK_LAST_INFO) {
          last_info_menu();
          ui_state = ST_LAST_INFO;
        } else {
          selparam_st = SELPARAM_VEL; // start selecting speed
          param_menu();
          ui_state = ST_SEL_PARAMS;
        }
      } else {
        read_rot_encoder_dir();
        task_st = nxt_task(task_st, rot_enc_rght, rot_enc_left);
        update_task_menu();
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
                is_dist_dest_neg = ! is_dist_dest_neg;
              } else {
                aux_val = dist_dest_magn + val_incr;
                if (aux_val > TOT_LEN) {
                  dist_dest_magn = TOT_LEN;
                } else {
                  dist_dest_magn = aux_val;
                }
              }
              if (is_dist_dest_neg) {
                dist_dest_wsign = - dist_dest_magn;
              } else {
                dist_dest_wsign = dist_dest_magn;
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
                is_dist_dest_neg = ! is_dist_dest_neg;
              } else {
                aux_val = dist_dest_magn + val_incr;
                if (aux_val < 1) {
                  dist_dest_magn = 1;  // no 0 distance
                } else {
                  dist_dest_magn = aux_val;
                }
              }
              if (is_dist_dest_neg) {
                dist_dest_wsign = - dist_dest_magn;
              } else {
                dist_dest_wsign = dist_dest_magn;
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
            is_dist_dest_neg = true; // when homing, distance is negative
          } else {
            ui_state = ST_RUN;
            run_dist_screen();
          }
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
      run_distance();
      ui_state = ST_END;
      break;    
    case ST_LAST_INFO:
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == true) { // confirmation or go back
        ui_state = ST_SEL_TASK;
        task_menu();
      }
      break;
    case ST_END:
      rot_enc_pushed = rot_encoder_pushed();
      if (rot_enc_pushed == true) { // confirmation or go back
        ui_state = ST_INI;
        lcd.clear();
        init_screen();
      }
      break;
  }
}
