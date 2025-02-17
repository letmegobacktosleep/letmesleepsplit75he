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
        case 0: // normal, not pressed
            if ((current > config->lower) || (current > max_key_displacement - config->upper)){
                key->mode = 1;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            else {
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }

        case 1: // normal, pressed
            if ((current < config->lower - config->upper) || (current < config->upper)){
                key->mode = 0;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            else {
                REGISTER_KEY(current_row, current_col);
                return 1;
            }

        case 2: // rapid trigger, at top
            if ((current > config->lower) || (current > max_key_displacement - config->upper)){
                key->old = current;
                key->mode = 3;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            else {
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }

        case 3: // rapid trigger, pressed
            if (current < config->upper){ // top deadzone
                key->mode = 2;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            else if (current > key->old){ // update lowest position
                key->old = current;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            else if (current < key->old - config->up){ // rapid untrigger
                key->old = current;
                key->mode = 4;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            else {
                REGISTER_KEY(current_row, current_col);
                return 1;
            }

        case 4: // rapid trigger, not pressed
            if (current < config->upper){ // top deadzone
                key->mode = 2;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            else if (current < key->old){ // update highest position
                key->old = current;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            else if ((current > key->old + config->down) || (current > max_key_displacement - config->upper)){ // rapid trigger or bottom deadzone
                key->old = current;
                key->mode = 3;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            else {
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }

        default: // invalid mode
            DEREGISTER_KEY(current_row, current_col);
            return 0;
    }
}