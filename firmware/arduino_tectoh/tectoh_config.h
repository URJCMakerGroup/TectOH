// Endstops:

#define ESTOP_ACTIVE_HIGH true // Set to false to invert the logic of the pushbutton

// kill/stop button of the LCD
#define LCD_STOP_BTN_ACTIVE_HIGH  false // Set to true to have direct logic


// Pin X_DIR_PIN determines the rotation direction of the stepper.
// A HIGH will determine a direction and a LOW the opposite/
// However, depending on how the motor coils are wired in the RAMPS,
// it will change also the rotation direction of the motor
// If the following is true, a HIGH in this pin will make the gantry
// move in positive direction, from x0 to xend.

#define DIR_MOTOR_POSITIVE_HIGH true // Set to false to invert the logic of the motor direction

// enable of motor are usually active low.
#define ENABLE_MOTOR_LOW true  // Set to false to make enable with a HIGH






