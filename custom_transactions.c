/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stdint.h>

#include "config.h"
#include "custom_matrix.h"

#include "transactions.h"
#include "custom_transactions.h"

// External definitions
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];
extern analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS];
extern uint8_t virtual_axes_from_self[2][8];
extern uint8_t virtual_axes_from_slave[2][8];

#ifdef SPLIT_KEYBOARD
void kb_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    // copy virtual_axes_from_self to the outbound buffer
    memcpy(out_data, virtual_axes_from_self, sizeof(virtual_axes_from_self));
}

void user_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    // Cast data to correct type
    const user_sync_a_t *new_config = (const user_sync_a_t*) in_data;

    // Extract data
    uint8_t row = *new_config->row;
    uint8_t col = *new_config->col;
    analog_config_t config = *new_config->config;

    // Copy to analog_config
    memcpy(&(analog_config[row][col]), config, sizeof(analog_config_t));
    
    // Update mode in analog_key
    analog_key[row][col].mode = analog_config[row][col].mode;

    // Save to eeprom
    EEPROM_USER_PARTIAL_UPDATE(analog_config, row, col);
}
#endif