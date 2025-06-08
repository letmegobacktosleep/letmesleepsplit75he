/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stdint.h>

#include "config.h"
#include "custom_matrix.h"

#include "transactions.h"
#include "custom_transactions.h"

#include "eeconfig_set_defaults.h"

#ifdef SPLIT_KEYBOARD

// External definitions
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];
extern analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS];

# ifdef ANALOG_KEY_VIRTUAL_AXES
extern uint8_t virtual_axes_from_self[4][4];
extern uint8_t virtual_axes_from_slave[4][4];
extern uint8_t virtual_axes_toggle;
# endif

void kb_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    // copy data into a buffer
    uint8_t raw_hid_data[RPC_M2S_BUFFER_SIZE] = { 0 };
    memcpy(raw_hid_data, in_data, in_buflen);

    // run rawhid processing on the slave
    raw_hid_receive_kb(raw_hid_data, in_buflen);

    // set the first byte of out_data to not zero
    if (out_buflen > 0) {
        ((uint8_t*)out_data)[0] = 1;
    }
}

void user_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    // set the virtual axes toggle
    if (in_buflen >= sizeof(uint8_t)) {
        virtual_axes_toggle = *(const uint8_t*)in_data;
    }

    // copy virtual_axes_from_self to the outbound buffer
    memcpy(out_data, virtual_axes_from_self, sizeof(virtual_axes_from_self));
}
#endif