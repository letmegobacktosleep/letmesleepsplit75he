/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "hal.h"

// Macros to register, deregister keys
#define REGISTER_KEY(current_row, current_col)   *current_row |=  (1 << current_col)
#define DEREGISTER_KEY(current_row, current_col) *current_row &= ~(1 << current_col)
/*
#define bitset(byte,nbit)   ((byte) |=  (1<<(nbit)))
#define bitclear(byte,nbit) ((byte) &= ~(1<<(nbit)))
#define bitflip(byte,nbit)  ((byte) ^=  (1<<(nbit)))
#define bitcheck(byte,nbit) ((byte) &   (1<<(nbit)))
*/

// Function prototypes
void multiplexer_init(void);
bool select_multiplexer_channel(uint8_t channel);
bool actuation(
    const analog_config_t *config, 
    analog_key_t *key, 
    matrix_row_t *current_row, 
    const uint8_t current_col, 
    const uint8_t current, 
    const uint8_t max_key_displacement
);