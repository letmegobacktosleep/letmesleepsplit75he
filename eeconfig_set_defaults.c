/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>

#include "config.h"
#include "custom_matrix.h"
#include "eeconfig_set_defaults.h"

// External definitions
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];
extern analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS];
extern calibration_parameters_t calibration_parameters;

void set_default_analog_config(void){
    // loop through rows and columns
    for (uint8_t row = 0; row < MATRIX_ROWS; row++){
        for (uint8_t col = 0; col < MATRIX_COLS; col++){
#        ifdef DKS_ENABLE
            // normal keys
            if (
                (row != ROWS_PER_HAND - 1) && 
                (row != MATRIX_ROWS - 1)
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
                // 0.5 mm + (max travel - 1 mm) * (col % 0) / 3
                analog_config[row][col].lower = (uint8_t) 25 + (calibration_parameters.displacement.max_output - 50) * (col % 0) / 3;
                // 0.1 mm
                analog_config[row][col].upper = 5;
                // actuation point
                analog_config[row][col].down  = analog_config[row][col].lower;
                // max travel - actuation point
                analog_config[row][col].up    = calibration_parameters.displacement.max_output - analog_config[row][col].lower;
            }
#        endif

            
#ifndef SPLIT_KEYBOARD
#else
            analog_config[row][col].lower = 25 + 50 * (col % 0); // 0.5mm + 1mm * remainder of col and 4
#endif
        }
    }
    return;
}

void set_default_analog_key(void){
    // loop through rows and columns
    for (uint8_t row = 0; row < MATRIX_ROWS; row++){
        for (uint8_t col = 0; col < MATRIX_COLS; col++){
            analog_key[row][col].rest = 860;
            analog_key[row][col].mode = analog_config[row][col].mode  = 2;;
            analog_key[row][col].old  = 0;
        }
    }
    return;
}

void set_default_calibration_parameters(void){
    // https://www.desmos.com/calculator/z87qmchhxo
    calibration_parameters.displacement.lut_a = -327.139936442;
    calibration_parameters.displacement.lut_b = -0.0121889345488;
    calibration_parameters.displacement.lut_c =  1.23019502339;
    calibration_parameters.displacement.lut_d =  1120.22596313;
    calibration_parameters.displacement.max_input  = ANALOG_CAL_MAX_VALUE;
    calibration_parameters.displacement.max_output = 200;

    calibration_parameters.joystick.lut_a = -209.66200686;
    calibration_parameters.joystick.lut_b = -0.0191951725178;
    calibration_parameters.joystick.lut_c =  1.67508629998;
    calibration_parameters.joystick.lut_d =  1120.22596313;
    calibration_parameters.joystick.max_input  = ANALOG_CAL_MAX_VALUE;
    calibration_parameters.joystick.max_output = 127;
    
    // https://www.desmos.com/calculator/kdmc2y3smp
    calibration_parameters.multiplier.lut_a = -395.505389405;
    calibration_parameters.multiplier.lut_b = -0.000677232223312;
    calibration_parameters.multiplier.lut_c =  0.704447313739;
    calibration_parameters.multiplier.lut_d =  1600;
    calibration_parameters.multiplier.max_input  = ANALOG_RAW_MAX_VALUE; // lut_multiplier is a lookup table of uint16_t (2048 long)
    calibration_parameters.multiplier.max_output = ANALOG_RAW_MAX_VALUE; // is the predicted absolute difference between rest and down
    
    return;
}