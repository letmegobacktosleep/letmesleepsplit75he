/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "util.h"
#include "quantum.h"

// Use const if the same pins are used for left and right
#ifdef MATRIX_ROW_PINS_RIGHT
#    undef SPLIT_MUTABLE_ROW
#    define SPLIT_MUTABLE_ROW
#else
#    undef SPLIT_MUTABLE_ROW
#    define SPLIT_MUTABLE_ROW const
#endif
#ifdef MATRIX_COL_PINS_RIGHT
#    undef SPLIT_MUTABLE_COL
#    define SPLIT_MUTABLE_COL
#else
#    undef SPLIT_MUTABLE_COL
#    define SPLIT_MUTABLE_COL const
#endif
// Redefine rows per hand
#define ROWS_PER_HAND
#ifdef SPLIT_KEYBOARD
#    undef ROWS_PER_HAND
#    define ROWS_PER_HAND (MATRIX_ROWS / 2)
#else
#    undef ROWS_PER_HAND
#    define ROWS_PER_HAND (MATRIX_ROWS)
#endif

// Extern global variables
extern SPLIT_MUTABLE_ROW pin_t row_pins[ROWS_PER_HAND];
extern SPLIT_MUTABLE_COL pin_t col_pins[MATRIX_COLS];
// Extern joystick definitions
#ifdef ANALOG_KEY_VIRTUAL_AXES
extern uint8_t virtual_axes_from_self[2][8];
extern uint8_t virtual_axes_from_slave[2][8];
# ifdef JOYSTICK_COORDINATES
extern const uint8_t joystick_coordinates[8][2];
# endif
# ifdef MOUSE_COORDINATES
extern const uint8_t mouse_coordinates[8][2];
# endif
# ifdef MOUSE_COORDINATES_RIGHT
extern const uint8_t mouse_coordinates_right[8][2];
# endif
#endif

typedef struct PACKED { 

    // All the settings
    uint8_t mode;   // actuation mode // 0 = normal // 2 = rapid trigger // 5,6,7,8 = DKS
    uint8_t lower;  // actuation point
    uint8_t upper;  // deadzone
    uint8_t down;   // rapid trigger sensitivity
    uint8_t up;     // rapid trigger sensitivity

} analog_config_t; // 5 bytes
_Static_assert(sizeof(analog_config_t)*MATRIX_ROWS*MATRIX_COLS == EECONFIG_USER_DATA_SIZE, "Mismatch in user EECONFIG stored data size");
extern analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS];

typedef struct {

    // Calibration settings
    uint16_t rest; // analog value when key is at rest
    uint16_t down; // analog value when key is fully pressed

    // Stuff that changes
    uint8_t mode;   // copy over mode from analog_config in matrix_init
    uint8_t old;    // old displacement, initialize to zero
    
} analog_key_t; // 6 bytes
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];

typedef struct PACKED {

    // Get displacement from gauss
    double lut_a;        // 8 bytes
    double lut_b;        // 8 bytes
    double lut_c;        // 8 bytes
    double lut_d;        // 8 bytes
    // Define the maximum values
    uint16_t max_input;  // 2 bytes
    uint16_t max_output; // 2 bytes

} lookup_table_t; // 36 bytes

typedef struct PACKED {

    lookup_table_t displacement; // 36 bytes
    lookup_table_t joystick;     // 36 bytes
    lookup_table_t multiplier;   // 36 bytes

} calibration_parameters_t; // 108 bytes
_Static_assert(sizeof(calibration_parameters_t) == EECONFIG_KB_DATA_SIZE, "Mismatch in keyboard EECONFIG stored data size");
extern calibration_parameters_t calibration_parameters;

// Function prototypes
void generate_lookup_tables(uint8_t *lut_displacement, uint8_t *lut_joystick, uint16_t *lut_multiplier);
void matrix_init_custom(void);
bool matrix_scan_custom(matrix_row_t current_matrix[]);