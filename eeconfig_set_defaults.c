/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>

#include "config.h"
#include "custom_matrix.h"
#include "eeconfig_set_defaults.h"

// External definitions
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];
extern analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS];
extern static_config_t static_config;

void set_default_analog_config(void){
    // loop through rows and columns
    for (uint8_t row = 0; row < MATRIX_ROWS; row++){
        for (uint8_t col = 0; col < MATRIX_COLS; col++){
#        ifdef DKS_ENABLE
            // normal keys
            if (
#            ifdef SPLIT_KEYBOARD
                (row != MATRIX_ROWS   - 1) && // last row on right
                (row != ROWS_PER_HAND - 1) // last row on left
#            else
                (row != MATRIX_ROWS   - 1) && // last row
                (row != MATRIX_ROWS   - 2) // second last row
#            endif
            )
            {
#        endif
                // rapid trigger
                analog_config[row][col].mode  = 2;
                // 1.5 mm
                analog_config[row][col].lower = 75;
                // 0.1 mm
                analog_config[row][col].upper = 5;
                // 0.5 mm
                analog_config[row][col].down  = 25;
                // 0.5 mm
                analog_config[row][col].up    = 25;
#        ifdef DKS_ENABLE
            }
            // extra keys for DKS
            else {
                // normal actuation
                analog_config[row][col].mode  = 0;
                // 0.5 mm + (max travel - 1 mm) * (col % 4) / 3
                analog_config[row][col].lower = (uint8_t) 25 + (static_config.displacement.max_output - 50) * (col % 4) / 3;
                // 0.1 mm
                analog_config[row][col].upper = 5;
                // actuation point
                analog_config[row][col].down  = analog_config[row][col].lower;
                // max travel - actuation point
                analog_config[row][col].up    = static_config.displacement.max_output - analog_config[row][col].lower;
            }
#        endif
        }
    }
    return;
}

void set_default_analog_key(void){
    // loop through rows and columns
    for (uint8_t row = 0; row < MATRIX_ROWS; row++){
        for (uint8_t col = 0; col < MATRIX_COLS; col++){
            analog_key[row][col].rest = 0;
            analog_key[row][col].down = 0;
            analog_key[row][col].mode = analog_config[row][col].mode;
            analog_key[row][col].old  = 0;
        }
    }
    return;
}

void set_default_calibration_parameters(void){
    
    static_config.displacement.lut_a =  4.2203566951856235;
    static_config.displacement.lut_b =  0.012188934550872534;
    static_config.displacement.lut_c =  3.1428764382410472;
    static_config.displacement.lut_d = -97.22596301964597;
    static_config.displacement.max_input  = ANALOG_CAL_MAX_VALUE;
    static_config.displacement.max_output = 200; // travel distance in mm * 50

    static_config.multiplier.lut_a =  0;
    static_config.multiplier.lut_b =  0;
    static_config.multiplier.lut_c =  0;
    static_config.multiplier.lut_d =  860;
    static_config.multiplier.max_input  = ANALOG_MULTIPLIER_LUT_SIZE; // lut_multiplier is a lookup table of uint16_t (2048 long)
    static_config.multiplier.max_output = ANALOG_RAW_MAX_VALUE;       // is the predicted absolute difference between rest and down
    
    return;
}

void set_default_virtual_axes(void){
#ifdef JOYSTICK_LEFT
    static_config.joystick_left  = JOYSTICK_LEFT;
#endif
#ifdef JOYSTICK_RIGHT
    static_config.joystick_right = JOYSTICK_RIGHT;
#endif
#ifdef MOUSE_MOVEMENT
    static_config.mouse_movement = MOUSE_MOVEMENT;
#endif
#ifdef MOUSE_SCROLL
    static_config.mouse_scroll   = MOUSE_SCROLL;
#endif
    return;
}