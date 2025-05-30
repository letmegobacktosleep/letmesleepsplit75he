/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "custom_matrix.h"
#include "custom_analog.h"
#include "custom_calibration.h"
#include "custom_scanning.h"
#include "eeconfig_set_defaults.h"
#include "letmesleepsplit75he.h"

#ifdef DEBUG_LAST_PRESSED
# include "print.h"
uint8_t last_pressed_row = 0;
uint8_t last_pressed_col = 0;
uint16_t last_pressed_value = 0;
#endif

// is_keyboard_left()
// returns a bool of whether it is left or right
// is_keyboard_master()
// returns a bool of whether it is connected by usb

// Create global variables storing the row and column pins
#if defined(MATRIX_ROW_PINS) && defined(MATRIX_COL_PINS)
#    ifdef MATRIX_ROW_PINS
SPLIT_MUTABLE_ROW pin_t row_pins[ROWS_PER_HAND] = MATRIX_ROW_PINS;
#    endif // MATRIX_ROW_PINS
#    ifdef MATRIX_COL_PINS
SPLIT_MUTABLE_COL pin_t col_pins[MATRIX_COLS]   = MATRIX_COL_PINS;
#    endif // MATRIX_COL_PINS
#endif

// row offset
static uint8_t row_offset = 0;

// Create array for custom matrix mask
static const matrix_row_t custom_matrix_mask[MATRIX_ROWS] = CUSTOM_MATRIX_MASK;

// Declare per-key variables
__attribute__((section(".ram0")))
analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS] = { 0 };
__attribute__((section(".ram0")))
analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS] = { 0 };
__attribute__((section(".ram0")))
static_config_t static_config = { 0 };

// Declare lookup tables in core-coupled memory (ram4)
__attribute__((section(".ram4")))
static uint8_t lut_displacement[ANALOG_CAL_MAX_VALUE+1]    = { 0 };
__attribute__((section(".ram4")))
static uint16_t lut_multiplier[ANALOG_MULTIPLIER_LUT_SIZE] = { 0 };

// Create global joystick variables
#ifdef ANALOG_KEY_VIRTUAL_AXES
uint8_t virtual_axes_toggle = 0;
uint8_t virtual_axes_from_self[4][4]  = { 0 };
uint8_t virtual_axes_from_slave[4][4] = { 0 };
#endif



// Generate lookup tables
void generate_lookup_tables(void){

    for (uint16_t i = 0; i < ANALOG_MULTIPLIER_LUT_SIZE; i++){
        // rest -> fully pressed value
        lut_multiplier[i] = rest_to_absolute_change(i, &static_config.multiplier);
    }

    for (uint16_t i = 0; i < ANALOG_CAL_MAX_VALUE+1; i++){
        // change in voltage from rest -> distance pressed
        lut_displacement[i] = analog_to_distance(i, &static_config.displacement);
    }

    return;
}

// Initialise matrix
void matrix_init_custom(void){
#ifdef SPLIT_KEYBOARD
    // Set pinout for right half if pinout for that half is defined
    if (!is_keyboard_left()) {
#    ifdef MATRIX_ROW_PINS_RIGHT
        const pin_t row_pins_right[ROWS_PER_HAND] = MATRIX_ROW_PINS_RIGHT;
        for (uint8_t i = 0; i < ROWS_PER_HAND; i++) {
            row_pins[i] = row_pins_right[i]; // row_pins is a global variable
        }
#    endif
#    ifdef MATRIX_COL_PINS_RIGHT
        const pin_t col_pins_right[MATRIX_COLS] = MATRIX_COL_PINS_RIGHT;
        for (uint8_t i = 0; i < MATRIX_COLS; i++) {
            col_pins[i] = col_pins_right[i]; // col_pins is a global variable
        }
#    endif
        // set row offset if right hand
        row_offset = ROWS_PER_HAND;
    }
#endif
    
    // Load default values (before real values are loaded)
    set_default_calibration_parameters();
    set_default_analog_config();
    set_default_analog_key();

    // Generate lookup tables
    generate_lookup_tables();

    // Initialize multiplexer GPIO pins
    multiplexer_init();
    // Initialize ADC pins
    initADCGroups();
    // Wait some time for ADCs to start
    wait_ms(100);
    return;
}

// create a previous matrix
static matrix_row_t previous_matrix[MATRIX_ROWS];
// do a "lite" custom matrix
bool matrix_scan_custom(matrix_row_t current_matrix[]){
    // update previous matrix
    memcpy(previous_matrix, current_matrix, sizeof(previous_matrix));

    // store virtual axes
    static uint8_t virtual_axes_temp[4][4] = { 0 };

    // row, col that can be modified - static because maybe it goes faster? I honestly don't know...
    static uint8_t row = 0;
    static uint8_t col = 0;
    static uint8_t next_col = 0;

    // create variables to track time
    static bool save_rest_values = false;
    static bool time_to_be_updated = false;
    static uint32_t time_current = 0;
    static uint32_t time_next_calibration = 0;
    time_current = timer_read32();
    
    // check if keyboard should be calibrated
    if (time_current > time_next_calibration){
        save_rest_values = true;
        time_to_be_updated = true;
    }

    // switch multiplexer to first column
    select_multiplexer_channel(0);
    // start first adc scan
    adcStartAllConversions(0);

    // loop through columns
    for (uint8_t current_col = 0; current_col < MATRIX_COLS; current_col++){

        // wait for adc to finish
        adcWaitForConversions();

        // fetch adc values
        static uint16_t raw_values[ROWS_PER_HAND];
        for (uint8_t current_row = 0; current_row < ROWS_PER_HAND; current_row++){
            raw_values[current_row] = getADCSample(current_row + row_offset);
        }

        // start next adc read
        if (current_col < MATRIX_COLS - 1){
            // graycode the col
            next_col = graycode_col(current_col + 1);
            // switch multiplexer to next column
            select_multiplexer_channel(graycoded_col);
            // start next adc scan
            adcStartAllConversions(graycoded_col);
        }

        // iterate through rows
        for (uint8_t current_row = 0; current_row < ROWS_PER_HAND; current_row++){

            // update modifable row, col
            row = current_row + row_offset;
            col = graycode_col(current_col);

            // if the key should be scanned
            if (BIT_GET(custom_matrix_mask[row], col)){
                
                // get raw adc value
                uint16_t raw = raw_values[current_row];
                // run analog filter
                raw = sma_filter_set(raw, current_row, col);
                // account for magnet polarity (bipolar sensor, 12-bit reading)
                if (raw <= ANALOG_RAW_MAX_VALUE){
                    raw = ANALOG_RAW_MAX_VALUE - raw;
                }
                else { // raw > ANALOG_RAW_MAX_VALUE
                    raw = raw - ANALOG_RAW_MAX_VALUE - 1;
                }

                // run calibration (output 0-1023)
                uint16_t calibrated = scale_raw_value(raw, analog_key[row][col].rest, lut_multiplier);

                // run lookup table (output 0-200, where 200=4mm)
                uint8_t displacement = lut_displacement[calibrated];

                if (
                    // run actuation
                    actuation(
                        &analog_config[row][col], 
                        &analog_key[row][col], 
                        &current_matrix[row], 
                        col,
                        displacement, 
                        static_config.displacement.max_output
                    )
                )
                {
                    // update time
                    time_to_be_updated = true;
                }

#            ifdef DKS_ENABLE
                // handle DKS
                if (
                    analog_key[row][col].mode >= 10 && 
                    analog_key[row][col].mode != 255
                )
                {
                    // find out which key it is assigned to
                    uint8_t dks_index = analog_key[row][col].mode - 10; // index, starts at zero
                    uint8_t dks_base_col = (dks_index * 4) % MATRIX_COLS; // starting column
                    uint8_t dks_base_row = dks_index / (MATRIX_COLS / 4); // how many rows from the end

#                ifdef SPLIT_KEYBOARD
                    // alternate between rows on the left and the right
                    dks_base_row = (dks_base_row / 2) + (ROWS_PER_HAND * (1 - (dks_base_row % 2)));
#                endif

                    // run actuation on four keys
                    for (uint8_t k = 0; k < 4; k++){

                        // get dks row and column
                        static uint8_t dks_row = MATRIX_ROWS - dks_base_row - 1;
                        static uint8_t dks_col = dks_base_col + k;
                        
                        if (
                            // run actuation
                            actuation(
                                &analog_config[dks_row][dks_col], 
                                &analog_key[dks_row][dks_col], 
                                &current_matrix[dks_row], 
                                dks_col,
                                displacement, 
                                static_config.displacement.max_output
                            )
                        )
                        {
                            // update time
                            time_to_be_updated = true;
                        }
                    }
                }
#            endif
#            ifdef ANALOG_KEY_VIRTUAL_AXES
                // handle joystick
                if (
                    BIT_GET(virtual_axes_toggle, va_joystick) || 
                    BIT_GET(virtual_axes_toggle, va_mouse)
                )
                {
                    // get value from 0 to 127 (scaled, close enough is good enough)
                    uint8_t joystick_value = (uint16_t) (
                        (displacement < static_config.virtual_axes_deadzone) ? 0 : 
                        (displacement - static_config.virtual_axes_deadzone)
                    ) * 127 / (static_config.displacement.max_output - static_config.virtual_axes_deadzone);

                    // check if it is supposed to be a joystick key
                    for (uint8_t k = 0; k < 4; k++){
#                    ifdef JOYSTICK_COORDINATES     
                        if (
                            BIT_GET(virtual_axes_toggle, va_joystick)
                        )
                        {
                            if (
                                col == static_config.joystick_left.col[k] && 
                                row == static_config.joystick_left.col[k]
                            )
                            {
                                virtual_axes_temp[0][k] += joystick_value;
                            }
                            if (
                                col == static_config.joystick_right.col[k] && 
                                row == static_config.joystick_right.col[k]
                            )
                            {
                                virtual_axes_temp[1][k] += joystick_value;
                            }
                        }
#                    endif
#                    ifdef MOUSE_COORDINATES
                        if (
                            BIT_GET(virtual_axes_toggle, va_mouse)
                        )
                        {
                            if (
                                col == static_config.mouse_movement.col[k] && 
                                row == static_config.mouse_movement.row[k]
                            )
                            {
                                virtual_axes_temp[2][k] += joystick_value;
                            }
                            if (
                                col == static_config.mouse_scroll.col[k] && 
                                row == static_config.mouse_scroll.row[k]
                            )
                            {
                                virtual_axes_temp[3][k] += joystick_value;
                            }
                        }
#                    endif
                    }
                }
#            endif

                // save rest values
                if (save_rest_values) {
                    analog_key[row][col].rest = MIN(raw, ANALOG_MULTIPLIER_LUT_SIZE - 1);
                }
                
#            ifdef DEBUG_SAVE_REST_DOWN
                analog_key[row][col].down = MAX(raw, analog_key[row][col].down);
#            endif
#            ifdef DEBUG_LAST_PRESSED
                if (
                    row == last_pressed_row &&
                    col == last_pressed_col
                )
                {
                    last_pressed_value = raw;
                }
#            endif
            }
        }
    }

    if (time_to_be_updated){

        if (save_rest_values){
            // next calibration in 1 minute
            time_next_calibration = time_current + (1 * 60000);
        }
        else {
            // next calibration in 5 minutes
            time_next_calibration = time_current + (5 * 60000);
        }
        
        // reset booleans
        time_to_be_updated = false;
        save_rest_values = false;
    }

    sma_filter_increment_pointer();

#ifdef ANALOG_KEY_VIRTUAL_AXES
    // copy over virtual axes
    memcpy(virtual_axes_from_self, virtual_axes_temp, sizeof(virtual_axes_temp));
    // clear virtual axes temp
    memset(virtual_axes_temp, 0, sizeof(virtual_axes_temp));
#endif

    // compare current matrix against previous matrix
    return memcmp(previous_matrix, current_matrix, sizeof(previous_matrix)) != 0;
}



#ifdef BOOTMAGIC_ENABLE
void bootmagic_scan(void){

    uint16_t bootmagic_key_value = 0;

# if (defined(BOOTMAGIC_ROW) && defined(BOOTMAGIC_COLUMN))
    if (is_keyboard_left()){
        select_multiplexer_channel(BOOTMAGIC_COLUMN);
        adcStartAllConversions(BOOTMAGIC_COLUMN);
        adcWaitForConversions();
        bootmagic_key_value = getADCSample(BOOTMAGIC_ROW);
    }
# endif
# if (defined(BOOTMAGIC_ROW_RIGHT) && defined(BOOTMAGIC_COLUMN_RIGHT))
    if (!is_keyboard_left()){
        select_multiplexer_channel(BOOTMAGIC_COLUMN_RIGHT);
        adcStartAllConversions(BOOTMAGIC_COLUMN_RIGHT);
        adcWaitForConversions();
        bootmagic_key_value = getADCSample(BOOTMAGIC_ROW_RIGHT);
    }
#endif

    if (bootmagic_key_value <= ANALOG_RAW_MAX_VALUE){
        bootmagic_key_value = ANALOG_RAW_MAX_VALUE - bootmagic_key_value ;
    }
    else { // bootmagic_key_value > 2047
        bootmagic_key_value = bootmagic_key_value - ANALOG_RAW_MAX_VALUE - 1;
    }

    // greater than the max rest value
    if (bootmagic_key_value > ANALOG_MULTIPLIER_LUT_SIZE) {
        bootloader_jump();
    }
}
#endif