// Endstops:

#define ENDSTOP_ACTIVE_HIGH true // Set to false to invert the logic of the endstop

// Pin X_DIR_PIN determines the rotation direction of the stepper.
// A HIGH will determine a direction and a LOW the opposite
// However, depending on how the motor coils are wired in the RAMPS,
// it will change also the rotation direction of the motor
// If the following is true, a HIGH in this pin will make the gantry
// move in positive direction, from x0 to xend.

#define DIR_MOTOR_POSIT_HIGH true // Set to false to invert the logic of the motor direction

// enable of motor are usually active low.
#define ENABLE_MOTOR_LOW true  // Set to false to make enable with a HIGH

// ----------------- EEPROM Directions

// -1 Not valid, 1 Valid, 0 unknown
#define EEP_DIR_VALID 0

// X position in mm: 0 to 400, short. 2 bytes
#define EEP_DIR_POS_X 1

// Last experiment values
// Distance last experiment. 2 bytes
#define EEP_DIR_DIST  3

// Speed last experiment. short. 2 bytes
// could be just 1 byte, but in case we get larger speeds
#define EEP_DIR_VEL   5

// Time last experiment. 1 byte
#define EEP_DIR_SECS  7
#define EEP_DIR_MINS  8 // 1 byte
#define EEP_DIR_HOURS 9 // 2 bytes




