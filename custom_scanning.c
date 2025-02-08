/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>

#include "config.h"
#include "matrix.h"
#include "custom_scanning.h"

// External definitions
extern col_pins[MATRIX_COLS];

// Local definitions
static uint8_t multiplexer_number_of_pins = 0;

void multiplexer_init(void){
    multiplexer_number_of_pins = 0;
    for (uint8_t i = 0; i < MATRIX_COLS; i++){
        if (col_pins[i] != NO_PIN){
            gpio_set_pin_output(pin)
            multiplexer_number_of_pins += 1;
        }
    }
}

bool select_multiplexer_channel(uint8_t channel){
    if (channel > MATRIX_COLS){
        return 0;
    }
    for (uint8_t i = 0; i < multiplexer_number_of_pins; i++){
        gpio_write_pin(col_pins[i], channel & (1 << i));
    }
    return 1
}

bool actuation(analog_config_t *config, analog_key_t *key, matrix_row_t *current_row, uint8_t current_col, uint8_t current, uint8_t max_key_displacement){
    switch (key->mode){
        case 0: // normal, not pressed
            if (current > config->lower){
                key->mode = 1;
                REGISTER_KEY(current_row, current_col);
                return 1;
            }
            else {
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }

        case 1: // normal, pressed
            if (current < config->lower - config->upper){
                key->mode = 0;
                DEREGISTER_KEY(current_row, current_col);
                return 0;
            }
            else {
                REGISTER_KEY(current_row, current_col);
                return 1;
            }

        case 2: // rapid trigger, at top
            if (current > config->lower){
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