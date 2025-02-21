/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>

#include "config.h"
#include "custom_matrix.h"
#include "custom_scanning.h"

// External definitions
extern SPLIT_MUTABLE_COL pin_t col_pins[MATRIX_COLS];

// Local definitions
static uint8_t mux_pin_count = 0;

void multiplexer_init(void){
    mux_pin_count = 0; // reset to zero
    for (uint8_t i = 0; i < MATRIX_COLS; i++){
        if (col_pins[i] != NO_PIN){
            palSetLineMode(col_pins[i], PAL_MODE_OUTPUT_PUSHPULL); // gpio_set_pin_output(col_pins[i]);
            mux_pin_count += 1;
        }
    }
}

bool select_multiplexer_channel(uint8_t channel){
    if (channel > MATRIX_COLS){
        return 0;
    }
    for (uint8_t i = 0; i < mux_pin_count; i++){
        palWriteLine(col_pins[i], channel & (1 << i)); // gpio_write_pin(col_pins[i], channel & (1 << i));
    }
    return 1;
}

bool actuation(analog_config_t *config, analog_key_t *key, matrix_row_t *current_row, uint8_t current_col, uint8_t current, uint8_t max_key_displacement){
    switch (key->mode){
        case 0: // normal, moving down, not pressed
            if (
                (current > config->lower) ||
                (current > max_key_displacement - config->upper) // bottom deadzone
            )
            {
                key->mode = 1;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            DEREGISTER_KEY(current_row, current_col);
            return 0;

        case 1: // normal, moving up, pressed
            if (
                (current < config->lower - config->upper) || 
                (current < config->upper) // top deadzone
            )
            {
                key->mode = 0;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            REGISTER_KEY(current_row, current_col);
            return 1;

        case 2: // rapid trigger, moving down, at top
            if (
                (current > config->lower) ||
                (current > max_key_displacement - config->upper) // bottom deadzone
            )
            {
                key->old = current;
                key->mode = 3;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            DEREGISTER_KEY(current_row, current_col);
            return 0;

        case 3: // rapid trigger, moving up, pressed
            if (current < config->upper){ // top deadzone
                key->mode = 2;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            else if (current > key->old){ // update lowest position
                key->old = current;
            }
            else if (current < key->old - config->up){ // rapid untrigger
                key->old = current;
                key->mode = 4;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            REGISTER_KEY(current_row, current_col);
            return 1;

        case 4: // rapid trigger, moving down, not pressed
            if (current < config->upper){ // top deadzone
                key->mode = 2;
            }
            else if (current < key->old){ // update highest position
                key->old = current;
            }
            else if (
                (current > key->old + config->down) || // rapid trigger
                (current > max_key_displacement - config->upper) // bottom deadzone
            )
            {
                key->old = current;
                key->mode = 3;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            DEREGISTER_KEY(current_row, current_col);
            return 0;

        case 5: // inverted, moving down, not pressed
            if (current > config->lower){ // critical point
                key->mode = 6;
            }
            DEREGISTER_KEY(current_row, current_col);
            return 0;

        case 6: // inverted, moving up, not pressed
            if (current < config->lower - config->upper){ // went back above critical point
                key->mode = 7;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            DEREGISTER_KEY(current_row, current_col);
            return 0;

        case 7: // inverted, moving up, pressed
            if (
                (current < config->lower - config->up) || // rapid untrigger
                (current < config->upper) // top deadzone
            )
            {
                mode = 5;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            REGISTER_KEY(current_row, current_col);
            return 1;

        case 8: // inv rapid trigger, moving down, not pressed
            if (
                (current < config->upper) || // top deadzone
                (current > key->old) // update lowest position
            )
            {
                key->old = current;
            }
            else if (current < key->old - config->up){ // rapid un-untrigger
                key->old = current;
                key->mode = 9;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            DEREGISTER_KEY(current_row, current_col);
            return 0;

        case 9: // inv rapid trigger, moving up, pressed
            if (
                (current < config->upper) || // top deadzone
                (current > max_key_displacement - config->upper) || // bottom deadzone
                (current > key->old + config->down) // rapid un-trigger
            )
            {
                key->old = current;
                key->mode = 8;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            else if (current < key->old){ // update highest position
                key->old = current;
            }
            REGISTER_KEY(current_row, current_col);
            return 1;

        default: // invalid mode
            DEREGISTER_KEY(current_row, current_col);
            return 0;
    }
}