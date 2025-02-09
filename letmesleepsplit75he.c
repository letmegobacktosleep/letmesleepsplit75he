/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "custom_matrix.h"
#include "custom_analog.h"

// External definitions
extern ADCManager adcManager;
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];
extern analog_key_t analog_config[MATRIX_ROWS][MATRIX_COLS];
extern calibration_parameters_t calibration_parameters;
// External joystick definitions
#ifdef ANALOG_KEY_VIRTUAL_AXES
extern int8_t virtual_axes_from_self[6][4];
extern int8_t virtual_axes_from_slave[6][4];
# ifdef JOYSTICK_COORDINATES_ONE
extern const uint8_t joystick_coordinates_one[4][2];
# endif
# ifdef JOYSTICK_COORDINATES_TWO
extern const uint8_t joystick_coordinates_two[4][2];
# endif
# ifdef MOUSE_COORDINATES_ONE
extern const uint8_t mouse_coordinates_two[4][2];
# endif
# ifdef MOUSE_COORDINATES_TWO
extern const uint8_t mouse_coordinates_two[4][2];
# endif
# ifdef SCROLL_COORDINATES_ONE
extern const uint8_t scroll_coordinates_one[4][2];
# endif
# ifdef SCROLL_COORDINATES_TWO
extern const uint8_t scroll_coordinates_two[4][2];
# endif
#endif

static bool virtual_joystick_toggle = false;
static bool virtual_mouse_toggle    = false;

enum custom_keycodes {
    KC_JS_TG = QK_KB_0,
    KC_MS_TG,
    KC_MS_MO,
}
/* This goes in the vial.json
"customKeycodes": [
    {"name": "Analog Joystick Toggle",
    "title": "Toggle use of WASD & PL;' as a joystick",
    "shortName": "KC_JS_TG"
    },
    {"name": "Analog Mouse Toggle",
    "title": "Toggle use of WASD & Arrow Keys to control your mouse",
    "shortName": "KC_MS_TG"
    },
    {"name": "Analog Mouse Momentary",
    "title": "Momentarily use WASD & Arrow Keys to control your mouse",
    "shortName": "KC_MS_MO"
    }
],
*/

#ifdef RGB_MATRIX_ENABLE
static const pin_t rgb_enable_pin = CUSTOM_RGB_ENABLE_PIN;
#endif



// field in struct
// EEPROM_KB_PARTIAL_UPDATE(calibration_parameters, displacement);
#if (EECONFIG_KB_DATA_SIZE) > 0
#    define EEPROM_KB_PARTIAL_READ(__struct, __field) eeprom_read_block(&(__struct.__field), (void *)((void *)(EECONFIG_KB_DATABLOCK) + offsetof(typeof(__struct), __field)), sizeof(__struct.__field))
#    define EEPROM_KB_PARTIAL_UPDATE(__struct, __field) eeprom_update_block(&(__struct.__field), (void *)((void *)(EECONFIG_KB_DATABLOCK) + offsetof(typeof(__struct), __field)), sizeof(__struct.__field))
#endif
// struct in array of struct
// EEPROM_USER_PARTIAL_UPDATE(analog_config[row][col]);
#if (EECONFIG_USER_DATA_SIZE) > 0
#    define EEPROM_USER_PARTIAL_UPDATE(__struct) eeprom_update_block(&(__struct), (void *)((void *)(EECONFIG_USER_DATABLOCK) + offsetof(typeof(__struct), __struct)), sizeof(__struct))
#    define EEPROM_USER_PARTIAL_READ(__struct) eeprom_read_block(&(__struct), (void *)((void *)(EECONFIG_USER_DATABLOCK) + offsetof(typeof(__struct), __struct)), sizeof(__struct))
#endif
// https://discord.com/channels/440868230475677696/440868230475677698/1334525203044106241

#ifdef SPLIT_KEYBOARD
void kb_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    // copy virtual_axes_from_self to the outbound buffer
    memcpy(out_data, virtual_axes_from_self, sizeof(virtual_axes_from_self));
}
void user_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    // Cast data to correct type
    const uint8_t *m2s[7] = (const uint8_t*) in_data;

    // Set config
    uint8_t row = *m2s[0];
    uint8_t col = *m2s[1];
    analog_config[row][col].mode  = *m2s[2];
    analog_config[row][col].lower = *m2s[3];
    analog_config[row][col].upper = *m2s[4];
    analog_config[row][col].down  = *m2s[5];
    analog_config[row][col].up    = *m2s[6];
    
    // Update mode in analog_key
    analog_key[row][col].mode = analog_config[row][col].mode;

    // Save to eeprom
    EEPROM_USER_PARTIAL_UPDATE(analog_config[row][col]);
}
#endif

// Call this when a new value is set - do not call on slave...
void user_write_new_config(row, col){
#ifdef SPLIT_KEYBOARD
    if (is_keyboard_master()){
        uint8_t literally_zero = 0;
        transaction_rpc_exec(
            USER_SYNC_A,
            sizeof(analog_config_t),
            &analog_config[row][col], // NEED TO SET ROW AND COL
            sizeof(literally_zero),
            &literally_zero
        );
    }
#endif
    EEPROM_USER_PARTIAL_UPDATE(analog_config[row][col]);
}

#if (JOYSTICK_AXIS_COUNT == 4)
joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT] = {
    JOYSTICK_AXIS_VIRTUAL, // x
    JOYSTICK_AXIS_VIRTUAL, // y
    JOYSTICK_AXIS_VIRTUAL, // Rx
    JOYSTICK_AXIS_VIRTUAL  // Ry
};
#endif

void eeconfig_init_user(void) {
    set_default_analog_config(); // set default values
    eeconfig_update_user_datablock(&analog_config); // write it to eeprom
}

void eeconfig_init_kb(void) {
    set_default_calibration_parameters(); // set default values
    eeconfig_update_kb_datablock(&calibration_parameters); // write it to eeprom
}

void keyboard_post_init_user(void) {
#if (EECONFIG_USER_DATA_SIZE) > 0
    eeconfig_read_user_datablock(&analog_config);
#endif
#ifdef RGB_MATRIX_ENABLE
    gpio_set_pin_output(rgb_enable_pin);
    gpio_write_pin_low(rgb_enable_pin);
#endif
}

void keyboard_post_init_kb(void) {
#if (EECONFIG_KB_DATA_SIZE) > 0
    eeconfig_read_kb_datablock(&calibration_parameters);
#endif
#ifdef SPLIT_KEYBOARD
    transaction_register_rpc(KEYBOARD_SYNC_A, kb_sync_a_slave_handler);
    transaction_register_rpc(USER_SYNC_A, user_sync_a_slave_handler);
#endif
}

// Tasks that will be run repeatedly

void housekeeping_task_kb(void) {
    // Sync virtual axes, if enabled https://docs.qmk.fm/features/split_keyboard#custom-data-sync
#ifdef ANALOG_KEY_VIRTUAL_AXES
    static uint32_t last_sync = 0;
    if (timer_elapsed32(last_sync) > 10){ // Only update every 10ms
#    ifdef SPLIT_KEYBOARD
        if (is_keyboard_master()) {
            if ((virtual_joystick_toggle || virtual_mouse_toggle)) {
                uint8_t literally_zero = 0; // Master doesn't have anything useful to send to slave
                if (transaction_rpc_exec(KEYBOARD_SYNC_A, sizeof(literally_zero), &literally_zero, sizeof(virtual_axes_from_slave), &virtual_axes_from_slave)) {
                    last_sync = timer_read32();
                }
            }
        }
#    endif
        static int8_t virtual_axes_combined[6][2];
        for (uint8_t i = 0; i < 6; i++){
            virtual_axes_combined[i][0] = MAX(-127, MIN(127, virtual_axes_from_self[i][1] + virtual_axes_from_slave[i][1] - virtual_axes_from_self[i][0] - virtual_axes_from_slave[i][0]))
            virtual_axes_combined[i][1] = MAX(-127, MIN(127, virtual_axes_from_self[i][3] + virtual_axes_from_slave[i][3] - virtual_axes_from_self[i][2] - virtual_axes_from_slave[i][2]))
        }
        
#    if (defined(JOYSTICK_COORDINATES_ONE) || defined(JOYSTICK_COORDINATES_TWO))
        // Only run joystick if toggled on
        // https://docs.qmk.fm/features/joystick#virtual-axes
        if (virtual_joystick_toggle){
#        ifdef JOYSTICK_COORDINATES_ONE
            // Left joystick
            joystick_set_axis(0, virtual_axes_combined[0][0]);
            joystick_set_axis(1, virtual_axes_combined[0][1]);
#        endif
#        ifdef JOYSTICK_COORDINATES_TWO
            // Right joystick
            joystick_set_axis(2, virtual_axes_combined[1][0]);
            joystick_set_axis(3, virtual_axes_combined[1][1]);
#        endif
            // Send joystick report
            joystick_flush();
        }
#    endif
#    if (defined(MOUSE_COORDINATES_ONE) || defined(MOUSE_COORDINATES_TWO) || defined(SCROLL_COORDINATES_ONE) || defined(SCROLL_COORDINATES_TWO))
        // Only run mouse if toggled on
        // https://docs.qmk.fm/features/pointing_device#manipulating-mouse-reports
        static int8_t mouse_v = 0;
        static int8_t mouse_h = 0;
        static uint8_t next_scroll[2] = { 0 };
        if (virtual_mouse_toggle){
            // Get current report
            report_mouse_t currentReport = pointing_device_get_report()
#        if (defined(MOUSE_COORDINATES_ONE) || defined(MOUSE_COORDINATES_ONE))
            // Set mouse movement
            mouse_x = MAX(-127, MIN(127, virtual_axes_combined[2][0] + virtual_axes_combined[3][0]));
            mouse_y = MAX(-127, MIN(127, virtual_axes_combined[2][1] + virtual_axes_combined[3][1]));
            currentReport.x = mouse_x
            currentReport.y = mouse_y
#        endif
#        if (defined(SCROLL_COORDINATES_ONE) || defined(SCROLL_COORDINATES_TWO))
            // Set scroll - time until next scroll
            mouse_v = MAX(-127, MIN(127, virtual_axes_combined[4][0] + virtual_axes_combined[5][0]));
            mouse_h = MAX(-127, MIN(127, virtual_axes_combined[4][1] + virtual_axes_combined[5][1])); 
            if (last_scroll[0] == 0){
                if (mouse_v > 2){
                    currentReport.v = 1;
                    next_scroll[0] = 32 - (abs(mouse_v) / 4);
                }
                else if (mouse_v < -2){
                    currentReport.v = -1;
                    next_scroll[0] = 32 - (abs(mouse_v) / 4);
                }
            }
            else {
                next_scroll[0]--;
            }
            if (last_scroll[1] == 0){
                if (mouse_h > 2){
                    currentReport.h = 1;
                    next_scroll[1] = 32 - (abs(mouse_h) / 4);
                }
                else if (mouse_h < -2){
                    currentReport.h = -1;
                    next_scroll[1] = 32 - (abs(mouse_h) / 4);
                }
            }
            else {
                next_scroll[1]--;
            }
#        endif
            // Override current report & send
            pointing_device_set_report(report_mouse_t currentReport);
            pointing_device_send();
        }
#    endif
    }
#endif
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    switch (keycode){
#    ifdef ANALOG_KEY_VIRTUAL_AXES
        // check for keycodes which toggle joystick or mouse
        case KC_JS_TG:
            if (record->event.pressed){ // only change state on key press
                virtual_joystick_toggle = !virtual_joystick_toggle;
            }
            return false;

        case KC_MS_TG:
            if (record->event.pressed){ // only change state on key press
                virtual_mouse_toggle = !virtual_mouse_toggle;
            }
            if (virtual_mouse_toggle){
                SEND_STRING(SS_DOWN(MO(MOUSE_LAYER))); // turn on mouse layer
            }
            else {
                SEND_STRING(SS_UP(MO(MOUSE_LAYER))); // turn off mouse layer
            }
            return false;
            
        case KC_MS_MO:
            if (record->event.pressed){
                virtual_mouse_toggle = true;
                SEND_STRING(SS_DOWN(MO(MOUSE_LAYER))); // turn on mouse layer
            }
            else {
                virtual_mouse_toggle = false;
                SEND_STRING(SS_UP(MO(MOUSE_LAYER))); // turn off mouse layer
            }
            return false;
#    endif
        default: 
            // press caps instead of esc if caps is on
            if (host_keyboard_led_state().caps_lock && record->event.pressed && record->event.key.row == 0 && record->event.key.col == 4){
                SEND_STRING(SS_TAP(KC_CAPS));
                return false;
            }
            // ignore keys with virtual axes
#        ifdef ANALOG_KEY_VIRTUAL_AXES
            for (uint8_t k = 0; k < 4; k++){ // ignore keys if joystick or mouse is toggled
                if (virtual_joystick_toggle){
#                ifdef JOYSTICK_COORDINATES_ONE     
                    if (record->event.key.row == joystick_coordinates_one[k][0] && record->event.key.col == joystick_coordinates_one[k][1]){
                        return false;
                    }
#                endif
#                ifdef JOYSTICK_COORDINATES_TWO
                    if (record->event.key.row == joystick_coordinates_two[k][0] && record->event.key.col == joystick_coordinates_two[k][1]){
                        return false;
                    }
#                endif
                }
                if (virtual_mouse_toggle){
#                ifdef MOUSE_COORDINATES_ONE
                    if (record->event.key.row == mouse_coordinates_one[k][0] && record->event.key.col == mouse_coordinates_one[k][1]){
                        return false;
                    }
#                endif
#                ifdef MOUSE_COORDINATES_TWO
                    if (record->event.key.row == mouse_coordinates_two[k][0] && record->event.key.col == mouse_coordinates_two[k][1]){
                        return false;
                    }
#                endif
#                ifdef SCROLL_COORDINATES_ONE
                    if (record->event.key.row == scroll_coordinates_one[k][0] && record->event.key.col == scroll_coordinates_one[k][1]){
                        return false;
                    }
#                endif
#                ifdef SCROLL_COORDINATES_TWO
                    if (record->event.key.row == scroll_coordinates_two[k][0] && record->event.key.col == scroll_coordinates_two[k][1]){
                        return false;
                    }
#                endif
                }
            }
#        endif
            return true;
    }
}

#ifdef RGB_MATRIX_ENABLE
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    static bool value_was_zero = true;
    static uint8_t current_val = 64;

    // Get current value
    current_val = rgb_matrix_get_val();

    // Light up Esc if CapsLock
    if (host_keyboard_led_state().caps_lock){
        // Always make brightness brighter than current brightness
        uint8_t brightness = MIN(255, current_val + 128);
        rgb_matrix_set_color(0, brightness, brightness, brightness);
    }

# ifdef ANALOG_KEY_VIRTUAL_AXES
    // Highlight joystick buttons
    if (virtual_joystick_toggle){
        uint8_t brightness = MIN(255, current_val + 64);
#    ifdef JOYSTICK_COORDINATES_ONE
        rgb_matrix_set_color(15, brightness, brightness, brightness);
        rgb_matrix_set_color(20, brightness, brightness, brightness);
        rgb_matrix_set_color(21, brightness, brightness, brightness);
        rgb_matrix_set_color(22, brightness, brightness, brightness);
#    endif
#    ifdef JOYSTICK_COORDINATES_TWO
        rgb_matrix_set_color(56, brightness, brightness, brightness);
        rgb_matrix_set_color(64, brightness, brightness, brightness);
        rgb_matrix_set_color(65, brightness, brightness, brightness);
        rgb_matrix_set_color(66, brightness, brightness, brightness);
#    endif
    }
    // Highlight mouse buttons
    if (virtual_mouse_toggle){
        uint8_t brightness = MIN(255, current_val + 64);
#    ifdef MOUSE_COORDINATES_ONE
        rgb_matrix_set_color(15, brightness, brightness, brightness);
        rgb_matrix_set_color(20, brightness, brightness, brightness);
        rgb_matrix_set_color(21, brightness, brightness, brightness);
        rgb_matrix_set_color(22, brightness, brightness, brightness);
#    endif
#    ifdef MOUSE_COORDINATES_TWO
        rgb_matrix_set_color(75, brightness, brightness, brightness);
        rgb_matrix_set_color(81, brightness, brightness, brightness);
        rgb_matrix_set_color(82, brightness, brightness, brightness);
        rgb_matrix_set_color(83, brightness, brightness, brightness);
#    endif
    }
# endif

    // Cut power to most LEDs if brightness is zero
    if (!value_was_zero && current_val == 0){
        gpio_write_pin_low(rgb_enable_pin);
        value_was_zero = true;
    }
    else if (value_was_zero && current_val != 0){
        gpio_write_pin_high(rgb_enable_pin);
        value_was_zero = false;
    }

    // Always return false
    return false;
}
#endif

#ifdef BOOTMAGIC_ENABLE
void bootmagic_scan(void) {

    uint16_t bootmagic_key_value = 0;

# if (defined(BOOTMAGIC_ROW) && defined(BOOTMAGIC_COLUMN))
    if (keyboard_is_left()){
        uint8_t current_direct_pin = 0;
#    ifdef MATRIX_DIRECT
        if (BOOTMAGIC_ROW == MATRIX_DIRECT_ROW && BOOTMAGIC_COLUMN < MATRIX_DIRECT){
            current_direct_pin = BOOTMAGIC_COLUMN;
        }
#    endif
        select_multiplexer_channel(BOOTMAGIC_COLUMN);
        adcStartAllConversions(adcManager, current_direct_pin);
        bootmagic_key_value = getADCSample(BOOTMAGIC_ROW);
    }
# endif
# if (defined(BOOTMAGIC_ROW_RIGHT) && defined(BOOTMAGIC_COLUMN_RIGHT))
    if (!keyboard_is_left()){
#    ifdef MATRIX_DIRECT_RIGHT
        if (BOOTMAGIC_ROW_RIGHT == MATRIX_DIRECT_ROW_RIGHT && BOOTMAGIC_COLUMN_RIGHT < MATRIX_DIRECT_RIGHT){
            current_direct_pin = BOOTMAGIC_COLUMN_RIGHT;
        }
#    endif
        select_multiplexer_channel(BOOTMAGIC_COLUMN_RIGHT);
        adcStartAllConversions(adcManager, current_direct_pin);
        bootmagic_key_value = getADCSample(BOOTMAGIC_ROW_RIGHT);
    }
#endif

    if (bootmagic_key_value < 2048){
        bootmagic_key_value = 2047 - bootmagic_key_value ;
    }
    else { // bootmagic_key_value > 2047
        bootmagic_key_value = bootmagic_key_value - 2048;
    }

    // arbitrary value
    if (bootmagic_key_value > 500) {
        bootloader_jump();
    }
}
#endif