/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// Macros to register, deregister keys
#define REGISTER_KEY(current_row, current_col) *current_row |= (1 << current_col)
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
bool actuation(struct key_settings *key, uint8_t current, uint8_t max_key_displacement);