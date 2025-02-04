#include <stdint.h>

typedef struct PACKED { 

    // All the settings
    uint8_t mode;   // actuation mode // 0 = normal // 2 = rapid trigger // 5,6,7,8 = DKS
    uint8_t lower;  // actuation point
    uint8_t upper;  // deadzone
    uint8_t down;   // rapid trigger sensitivity
    uint8_t up;     // rapid trigger sensitivity

} analog_config_t; // 5 bytes
_Static_assert(sizeof(analog_config_t)*MATRIX_ROWS*MATRIX_COLS == EECONFIG_USER_DATA_SIZE, "Mismatch in user EECONFIG stored data size");
extern analog_key_t analog_config[MATRIX_ROWS][MATRIX_COLS];

typedef struct {

    // Calibration settings
    uint16_t rest;  // analog value when key is at rest

    // Stuff that changes
    uint8_t mode;   // copy over mode from analog_config in matrix_init
    uint8_t old;    // old displacement, initialize to zero
    
} analog_key_t; // 4 bytes
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];

typedef struct PACKED {

    // Get displacement from gauss
    double lut_a;        // 8 bytes
    double lut_b;        // 8 bytes
    double lut_c;        // 8 bytes
    double lut_d;        // 8 bytes
    // Define the maximum values
    uint16_t max_input;  // 1 byte
    uint16_t max_output; // 1 byte

} lookup_table_t; // 35 bytes

typedef struct PACKED {

    lookup_table_t displacement; // 35 bytes
    lookup_table_t joystick;     // 35 bytes
    lookup_table_t multiplier;   // 35 bytes

} calibration_parameters_t; // 105 bytes
_Static_assert(sizeof(calibration_parameters_t) == EECONFIG_KB_DATA_SIZE, "Mismatch in keyboard EECONFIG stored data size");
extern calibration_parameters_t calibration_parameters;