/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>
#include <stdlib.h>

#include "quantum.h"
#include "custom_matrix.h"
#include "custom_transactions.h"

// External definitions
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];
extern analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS];
extern static_config_t static_config;

// Call this when a new value is set
void eeconfig_update_sync_user(uint8_t row, uint8_t col){
#ifdef SPLIT_KEYBOARD
    user_sync_a_t *new_config = {
        .row = row,
        .col = col,
        .config = analog_config[row][col]
    }
    if (is_keyboard_master()){
        uint8_t literally_zero = 0;
        transaction_rpc_exec(
            USER_SYNC_A,
            sizeof(user_sync_a_t),
            &new_config,
            sizeof(literally_zero),
            &literally_zero
        );
    }
#endif
    // Save to EEPROM
    EEPROM_USER_PARTIAL_UPDATE(analog_config, row, col);
}

#if defined(VIA_ENABLE)

# if defined(VIAL_ENABLE)

enum letmesleep_cmd {
    id_custom_get_key_config = 1,
    id_custom_set_key_config,
    id_custom_get_lut_config,
    id_custom_set_lut_config,
    id_custom_save_lut_config,
};

enum letmesleep_lut_id {
    id_lut_multiplier = 1,
    id_lut_displacement,
    id_lut_joystick,
};

enum letmesleep_lut_config {
    id_lut_a = 1,
    id_lut_b,
    id_lut_c,
    id_lut_d,
    id_lut_max_input,
    id_lut_max_output,
};

void letmesleep_get_key_config(uint8_t *data){
    uint8_t *row   = &(data[0]);
    uint8_t *col   = &(data[1]);
    uint8_t *mode  = &(data[2]);
    uint8_t *lower = &(data[3]);
    uint8_t *upper = &(data[4]);
    uint8_t *down  = &(data[5]);
    uint8_t *up    = &(data[6]);

    *mode  = analog_config[*row][*col].mode;
    *lower = analog_config[*row][*col].lower;
    *upper = analog_config[*row][*col].upper;
    *down  = analog_config[*row][*col].down;
    *up    = analog_config[*row][*col].up;
}

void letmesleep_set_key_config(uint8_t *data){
    uint8_t *row   = &(data[0]);
    uint8_t *col   = &(data[1]);
    uint8_t *mode  = &(data[2]);
    uint8_t *lower = &(data[3]);
    uint8_t *upper = &(data[4]);
    uint8_t *down  = &(data[5]);
    uint8_t *up    = &(data[6]);

    analog_key[*row][*col].mode     = *mode;
    analog_config[*row][*col].mode  = *mode;
    analog_config[*row][*col].lower = *lower;
    analog_config[*row][*col].upper = *upper;
    analog_config[*row][*col].down  = *down;
    analog_config[*row][*col].up    = *up;

    eeconfig_update_sync_user(*row, *col)
    // EEPROM_USER_PARTIAL_UPDATE(analog_config, *row, *col);
    // eeconfig_update_user_datablock(&analog_config);
}

void letmesleep_get_lut_config(uint8_t *data){
    uint8_t *lut_id     = &(data[0]);
    uint8_t *value_id   = &(data[1]);
    double  *value_data = (double *) &(data[2]);

    lookup_table_t *lut_config = NULL;
    switch (*lut_id) {
        case id_lut_multiplier:
            lut_config = &static_config.multiplier;
            break;
        case id_lut_displacement:
            lut_config = &static_config.displacement;
            break;
        default:
            break;
    }

    double temp_value = 0.0;
    switch (*value_id) {
        case id_lut_a:
            temp_value = lut_config->lut_a;
            break;
        case id_lut_b:
            temp_value = lut_config->lut_b;
            break;
        case id_lut_c:
            temp_value = lut_config->lut_c;
            break;
        case id_lut_d:
            temp_value = lut_config->lut_d;
            break;
        case id_lut_max_input:
            temp_value = (double) lut_config->max_input;
            break;
        case id_lut_max_output:
            temp_value = (double) lut_config->max_output;
            break;
        default:
            break;
    }

    memcpy(value_data, &temp_value, sizeof(double));
}

void letmesleep_set_lut_config(uint8_t *data){
    uint8_t *lut_id     = &(data[0]);
    uint8_t *value_id   = &(data[1]);
    double  *value_data = (double *) &(data[2]);
    
    lookup_table_t *lut_config = NULL;
    switch (*lut_id) {
        case id_lut_multiplier:
            lut_config = &static_config.multiplier;
            break;
        case id_lut_displacement:
            lut_config = &static_config.displacement;
            break;
        default:
            break;
    }

    double temp_value = 0.0;
    memcpy(&temp_value, value_data, sizeof(double));
    switch (*value_id) {
        case id_lut_a:
            lut_config->lut_a = temp_value;
            break;
        case id_lut_b:
            lut_config->lut_b = temp_value;
            break;
        case id_lut_c:
            lut_config->lut_c = temp_value;
            break;
        case id_lut_d:
            lut_config->lut_d = temp_value;
            break;
        case id_lut_max_input:
            lut_config->max_input = (uint16_t) temp_value;
            break;
        case id_lut_max_output:
            lut_config->max_output = (uint16_t) temp_value;
            break;
        default:
            break;
    }
}

void letmesleep_save_lut_config(uint8_t *data){
    /* for some reason the partial update doesn't work
    uint8_t *lut_id = &(data[0]);
    switch (*lut_id) {
        case id_lut_multiplier:
            EEPROM_KB_PARTIAL_UPDATE(static_config, multiplier);
            break;
        case id_lut_displacement:
            EEPROM_KB_PARTIAL_UPDATE(static_config, displacement);
            break;
        default:
            break;
    }
    */
    eeconfig_update_kb_datablock(&static_config);
}

void letmesleep_custom_command_kb(uint8_t *data, uint8_t length){
    /* data = [ command_id, channel_id, custom_data ] */
    uint8_t *sub_command_id = &(data[0]);
    uint8_t *channel_id     = &(data[1]);
    uint8_t *custom_data    = &(data[2]);

    if (*channel_id == id_custom_channel) {
        switch (*sub_command_id) {
            case id_custom_get_key_config: {
                letmesleep_get_key_config(custom_data);
                break;
            }
            case id_custom_set_key_config: {
                letmesleep_set_key_config(custom_data);
                break;
            }
            case id_custom_get_lut_config: {
				letmesleep_get_lut_config(custom_data);
                break;
            }
            case id_custom_set_lut_config: {
                letmesleep_set_lut_config(custom_data);
                break;
            }
            case id_custom_save_lut_config: {
                letmesleep_save_lut_config(custom_data);
                break;
            }
            default: {
                /* Unhandled message */
                *sub_command_id = id_unhandled;
                break;
            }
        }
        return;
    }

    /* Return the unhandled state */
	*sub_command_id = id_unhandled;

	/* DO NOT call raw_hid_send(data,length) here, let caller do this */
}

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id = &(data[0]);

    // Vial uses an older version of via.c
    // whidh does not have "via_custom_value_command_kb"
    // use "id_unhandled" to invoke "letmesleep_custom_command_kb"
    if (*command_id == id_unhandled) {
        letmesleep_custom_command_kb(&data[1], length - 1);
    }
}

# else

enum letmesleep_key_config {
	id_key_mode = 1,
	id_key_actuation_point,
	id_key_deadzone,
	id_key_down,
	id_key_up,
};

void via_config_set_value(uint8_t *data) {
    /* data = [ value_id, value_data ] */
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    // loop through rows
    for (uint8_t row = 0; row < MATRIX_ROWS; row++){
        // only run if row isn't DKS
        if (
# ifdef SPLIT_KEYBOARD
            (row != MATRIX_ROWS   - 1) && // last row on right
            (row != ROWS_PER_HAND - 1) // last row on left
# else
            (row != MATRIX_ROWS   - 1) && // last row
            (row != MATRIX_ROWS   - 2) // second last row
# endif
        )
        {   
            // loop through columns
            for (uint8_t col = 0; col < MATRIX_COLS; col++){
                switch (*value_id) {
                    case id_key_mode:
                        analog_key[row][col].mode = *value_data;    
                        analog_config[row][col].mode = *value_data;
                        break;
                    case id_key_actuation_point:
                        analog_config[row][col].lower = *value_data;
                        break;
                    case id_key_deadzone:
                        analog_config[row][col].upper = *value_data;
                        break;
                    case id_key_down:
                        analog_config[row][col].down = *value_data;
                        break;
                    case id_key_up:
                        analog_config[row][col].up = *value_data;
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void via_config_get_value(uint8_t *data) {
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_key_mode:
            *value_data = analog_config[0][0].mode;
            break;
        case id_key_actuation_point:
            *value_data = analog_config[0][0].lower;
            break;
        case id_key_deadzone:
            *value_data = analog_config[0][0].upper;
            break;
		case id_key_down:
            *value_data = analog_config[0][0].down;
            break;
		case id_key_up:
            *value_data = analog_config[0][0].up;
            break;
		default:
			break;
	}
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length){
    /* data = [ command_id, channel_id, value_id, value_data ] */
    uint8_t *command_id        = &(data[0]);
    uint8_t *channel_id        = &(data[1]);
    uint8_t *value_id_and_data = &(data[2]);

	if (*channel_id == id_custom_channel) {
        switch (*command_id) {
            case id_custom_set_value: {
                via_config_set_value(value_id_and_data);
                break;
            }
            case id_custom_get_value: {
                via_config_get_value(value_id_and_data);
                break;
            }
            case id_custom_save: {
				eeconfig_update_user_datablock(&analog_config);
                break;
            }
            default: {
                /* Unhandled message */
                *command_id = id_unhandled;
                break;
            }
        }
        return;
    }

	/* Return the unhandled state */
	*command_id = id_unhandled;

	/* DO NOT call raw_hid_send(data,length) here, let caller do this */
}

# endif

#endif