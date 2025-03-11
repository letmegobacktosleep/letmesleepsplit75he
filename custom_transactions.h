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