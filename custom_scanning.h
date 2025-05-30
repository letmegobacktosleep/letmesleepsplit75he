/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "hal.h"

// Function prototypes
void multiplexer_init(void);
uint8_t graycode_col(uint8_t col);
bool select_multiplexer_channel(uint8_t channel);
bool actuation(
    const analog_config_t *config, 
    analog_key_t *key, 
    matrix_row_t *current_row, 
    const uint8_t current_col, 
    const uint8_t current, 
    const uint8_t max_key_displacement
);