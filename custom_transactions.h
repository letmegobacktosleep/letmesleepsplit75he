/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>

#include "config.h"
#include "custom_matrix.h"

typedef struct {

    uint8_t row;
    uint8_t col;
    analog_config_t config;

} user_sync_a_t;

// Function prototypes
void kb_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data);
void user_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data);
void eeconfig_update_sync_user(uint8_t row, uint8_t col);