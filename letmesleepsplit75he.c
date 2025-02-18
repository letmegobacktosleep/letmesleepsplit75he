/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "transactions.h"
#include "pointing_device.h"

#include "config.h"
#include "custom_matrix.h"
#include "custom_analog.h"
#include "custom_scanning.h"
#include "eeconfig_set_defaults.h"
#include "letmesleepsplit75he.h"

// External definitions
extern ADCManager adcManager;
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];
extern analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS];
extern calibration_parameters_t calibration_parameters;
// External joystick definitions
#ifdef ANALOG_KEY_VIRTUAL_AXES
extern uint8_t virtual_axes_from_self[4][4];
extern uint8_t virtual_axes_from_slave[4][4];
# ifdef JOYSTICK_COORDINATES_LEFT
extern const uint8_t joystick_coordinates_left[4][2];
# endif
# ifdef JOYSTICK_COORDINATES_RIGHT
extern const uint8_t joystick_coordinates_right[4][2];
# endif
# ifdef MOUSE_COORDINATES_LEFT
extern const uint8_t mouse_coordinates_left[4][2];
# endif
# ifdef MOUSE_COORDINATES_RIGHT
extern const uint8_t mouse_coordinates_right[4][2];
# endif
# ifdef SCROLL_COORDINATES_LEFT
extern const uint8_t scroll_coordinates_left[4][2];
# endif
# ifdef SCROLL_COORDINATES_RIGHT
extern const uint8_t scroll_coordinates_right[4][2];
# endif
#endif

// bit 1 = joystick toggle
// bit 2 = left mouse toggle
// bit 3 = right mouse toggle
uint8_t virtual_axes_toggle = 0;

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
#    define EEPROM_USER_PARTIAL_UPDATE(__struct) eeprom_update_block(&(__struct), (void *)(EECONFIG_USER_DATABLOCK), sizeof(__struct))
#    define EEPROM_USER_PARTIAL_READ(__struct) eeprom_read_block(&(__struct), (void *)(EECONFIG_USER_DATABLOCK), sizeof(__struct))
#endif
// https://discord.com/channels/440868230475677696/440868230475677698/1334525203044106241

#ifdef SPLIT_KEYBOARD
void kb_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    // copy virtual_axes_from_self to the outbound buffer
    memcpy(out_data, virtual_axes_from_self, sizeof(virtual_axes_from_self));
}

void user_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    // Cast data to correct type
    const uint8_t *m2s = (const uint8_t*) in_data;

    // Set config
    uint8_t row = m2s[0];
    uint8_t col = m2s[1];
    analog_config[row][col].mode  = m2s[2];
    analog_config[row][col].lower = m2s[3];
    analog_config[row][col].upper = m2s[4];
    analog_config[row][col].down  = m2s[5];
    analog_config[row][col].up    = m2s[6];
    
    // Update mode in analog_key
    analog_key[row][col].mode = analog_config[row][col].mode;

    // Save to eeprom
    EEPROM_USER_PARTIAL_UPDATE(analog_config[row][col]);
}
#endif

// Call this when a new value is set - do not call on slave...
void user_write_new_config(uint8_t row, uint8_t col){
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

void handle_virtual_mouse_layer(uint8_t virtual_axes_toggle){
    if (
        BIT_GET(virtual_axes_toggle, 1) || 
        BIT_GET(virtual_axes_toggle, 2)
    )
    {
        layer_on(MOUSE_LAYER); // turn on mouse layer
    }
    else {
        layer_off(MOUSE_LAYER); // turn off mouse layer
    }
    return;
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
    palSetLineMode(rgb_enable_pin, PAL_MODE_OUTPUT_PUSHPULL); // gpio_set_pin_output(rgb_enable_pin);
    palClearLine(rgb_enable_pin); // gpio_write_pin_low(rgb_enable_pin);
#endif
}

void keyboard_post_init_kb(void) {
#if (EECONFIG_KB_DATA_SIZE) > 0
    eeconfig_read_kb_datablock(&calibration_parameters);
    generate_lookup_tables();
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
            if (
                BIT_GET(virtual_axes_toggle, 0) || // joystick
                BIT_GET(virtual_axes_toggle, 1) || // left mouse
                BIT_GET(virtual_axes_toggle, 2)    // right mouse
            )
            {
                // send the virtual axes toggle to the slave, receive its virtual axes
                if (transaction_rpc_exec(KEYBOARD_SYNC_A, sizeof(virtual_axes_toggle), &virtual_axes_toggle, sizeof(virtual_axes_from_slave), &virtual_axes_from_slave)) {
                    last_sync = timer_read32();
                }
            }
        }
#    endif
        static int8_t virtual_axes_combined[6][2];
        for (uint8_t i = 0; i < 4; i++){
            virtual_axes_combined[i][0] = MAX(-127, MIN(127, virtual_axes_from_self[i][1] + virtual_axes_from_slave[i][1] - virtual_axes_from_self[i][0] - virtual_axes_from_slave[i][0]));
            virtual_axes_combined[i][1] = MAX(-127, MIN(127, virtual_axes_from_self[i][3] + virtual_axes_from_slave[i][3] - virtual_axes_from_self[i][2] - virtual_axes_from_slave[i][2]));
        }
        
#    if (defined(JOYSTICK_COORDINATES_LEFT) || defined(JOYSTICK_COORDINATES_RIGHT))
        // Only run joystick if toggled on
        // https://docs.qmk.fm/features/joystick#virtual-axes
        if (
            BIT_GET(virtual_axes_toggle, 0)
        )
        {
#        ifdef JOYSTICK_COORDINATES_LEFT
            // Left joystick
            joystick_set_axis(0, virtual_axes_combined[0][0]);
            joystick_set_axis(1, virtual_axes_combined[0][1]);
#        endif
#        ifdef JOYSTICK_COORDINATES_RIGHT
            // Right joystick
            joystick_set_axis(2, virtual_axes_combined[1][0]);
            joystick_set_axis(3, virtual_axes_combined[1][1]);
#        endif
            // Send joystick report
            joystick_flush();
        }
#    endif
#    if (defined(MOUSE_COORDINATES_LEFT) || defined(MOUSE_COORDINATES_RIGHT) || defined(SCROLL_COORDINATES_LEFT) || defined(SCROLL_COORDINATES_RIGHT))
        // How much time until next scroll report
        static uint8_t next_scroll[2] = { 0 };        
        // Only run mouse if toggled on
        if (
            BIT_GET(virtual_axes_toggle, 1) ||
            BIT_GET(virtual_axes_toggle, 2)
        )
        {
            // Get current report
            report_mouse_t currentReport = pointing_device_get_report();
#        if (defined(MOUSE_COORDINATES_LEFT) || defined(MOUSE_COORDINATES_LEFT))
            // Set mouse movement
            currentReport.x = virtual_axes_combined[2][0];
            currentReport.y = virtual_axes_combined[2][1];
#        endif
#        if (defined(SCROLL_COORDINATES_LEFT) || defined(SCROLL_COORDINATES_RIGHT))
            int8_t mouse_v = virtual_axes_combined[3][0];
            int8_t mouse_h = virtual_axes_combined[3][1]; 
            // Vertical scroll
            if (next_scroll[0] == 0){
                if (mouse_v > MOUSE_DEADZONE){
                    currentReport.v = 1;
                    next_scroll[0] = 32 - (abs(mouse_v) / 4);
                }
                else if (mouse_v < -MOUSE_DEADZONE){
                    currentReport.v = -1;
                    next_scroll[0] = 32 - (abs(mouse_v) / 4);
                }
            }
            else {
                next_scroll[0]--;
            }
            // Horizontal scroll
            if (next_scroll[1] == 0){
                if (mouse_h > MOUSE_DEADZONE){
                    currentReport.h = 1;
                    next_scroll[1] = 32 - (abs(mouse_h) / 4);
                }
                else if (mouse_h < -MOUSE_DEADZONE){
                    currentReport.h = -1;
                    next_scroll[1] = 32 - (abs(mouse_h) / 4);
                }
            }
            else {
                next_scroll[1]--;
            }
#        endif
            // Override current report & send
            // https://docs.qmk.fm/features/pointing_device#manipulating-mouse-reports
            pointing_device_set_report(currentReport);
            pointing_device_send();
        }
#    endif
    }
#endif
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    switch (keycode){
        case KC_ESC:
            // press caps instead of esc if caps is on
            if (host_keyboard_led_state().caps_lock && record->event.pressed){
                tap_code(KC_CAPS);
                return false;
            }

#    ifdef ANALOG_KEY_VIRTUAL_AXES
        // check for keycodes which toggle joystick or mouse
        case KC_JS_TG:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, 0);
            }
            return false;

        case KC_MS_TG:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, 1);
                BIT_FLP(virtual_axes_toggle, 2);
            }
            handle_virtual_mouse_layer(virtual_axes_toggle);
            return false;
            
        case KC_MS_MO:
            if (record->event.pressed){
                BIT_SET(virtual_axes_toggle, 1);
                BIT_SET(virtual_axes_toggle, 2);
            }
            else {
                BIT_CLR(virtual_axes_toggle, 1);
                BIT_CLR(virtual_axes_toggle, 2);
            }
            handle_virtual_mouse_layer(virtual_axes_toggle);
            return false;

        case KC_MS_TG1:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, 1);
            }
            handle_virtual_mouse_layer(virtual_axes_toggle);
            return false;
            
        case KC_MS_MO1:
            if (record->event.pressed){
                BIT_SET(virtual_axes_toggle, 1);
            }
            else {
                BIT_CLR(virtual_axes_toggle, 1);
            }
            handle_virtual_mouse_layer(virtual_axes_toggle);
            return false;

        case KC_MS_TG2:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, 2);
            }
            handle_virtual_mouse_layer(virtual_axes_toggle);
            return false;
            
        case KC_MS_MO2:
            if (record->event.pressed){
                BIT_SET(virtual_axes_toggle, 2);
            }
            else {
                BIT_CLR(virtual_axes_toggle, 2);
            }
            handle_virtual_mouse_layer(virtual_axes_toggle);
            return false;
#    endif

        default:

#        ifdef ANALOG_KEY_VIRTUAL_AXES
            // ignore keys with virtual axes
            for (uint8_t k = 0; k < 4; k++){ // ignore keys if joystick or mouse is toggled
                if (BIT_GET(virtual_axes_toggle, 0)){
#                ifdef JOYSTICK_COORDINATES_LEFT     
                    if (record->event.key.row == joystick_coordinates_left[k][0] && record->event.key.col == joystick_coordinates_left[k][1]){
                        return false;
                    }
#                endif
#                ifdef JOYSTICK_COORDINATES_RIGHT
                    if (record->event.key.row == joystick_coordinates_right[k][0] && record->event.key.col == joystick_coordinates_right[k][1]){
                        return false;
                    }
#                endif
                }
                if (BIT_GET(virtual_axes_toggle, 1)){
#                ifdef MOUSE_COORDINATES_LEFT
                    if (record->event.key.row == mouse_coordinates_left[k][0] && record->event.key.col == mouse_coordinates_left[k][1]){
                        return false;
                    }
#                endif
#                ifdef SCROLL_COORDINATES_LEFT
                    if (record->event.key.row == scroll_coordinates_left[k][0] && record->event.key.col == scroll_coordinates_left[k][1]){
                        return false;
                    }
#                endif
                }
                if (BIT_GET(virtual_axes_toggle, 2)){
#                ifdef MOUSE_COORDINATES_RIGHT
                    if (record->event.key.row == mouse_coordinates_right[k][0] && record->event.key.col == mouse_coordinates_right[k][1]){
                        return false;
                    }
#                endif
#                ifdef SCROLL_COORDINATES_RIGHT
                    if (record->event.key.row == scroll_coordinates_right[k][0] && record->event.key.col == scroll_coordinates_right[k][1]){
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
    static uint8_t base_brightness = 64;

    // Get current value
    base_brightness = rgb_matrix_get_val();

    // Light up Esc if CapsLock
    if (host_keyboard_led_state().caps_lock){
        // Always make brightness brighter than current brightness
        uint8_t brightness = MIN(255, base_brightness + 128);
        rgb_matrix_set_color(0, brightness, brightness, brightness);
    }

# ifdef ANALOG_KEY_VIRTUAL_AXES
    // Highlight joystick buttons
    if (BIT_GET(virtual_axes_toggle, 0)){
        uint8_t brightness = MIN(255, base_brightness + 64);
#    ifdef JOYSTICK_COORDINATES_LEFT
        rgb_matrix_set_color(15, brightness, brightness, brightness);
        rgb_matrix_set_color(20, brightness, brightness, brightness);
        rgb_matrix_set_color(21, brightness, brightness, brightness);
        rgb_matrix_set_color(22, brightness, brightness, brightness);
#    endif
#    ifdef JOYSTICK_COORDINATES_RIGHT
        rgb_matrix_set_color(56, brightness, brightness, brightness);
        rgb_matrix_set_color(64, brightness, brightness, brightness);
        rgb_matrix_set_color(65, brightness, brightness, brightness);
        rgb_matrix_set_color(66, brightness, brightness, brightness);
#    endif
    }
    // Highlight left mouse buttons
    if (BIT_GET(virtual_axes_toggle, 1)){
        uint8_t brightness = MIN(255, base_brightness + 64);
#    ifdef MOUSE_COORDINATES_LEFT
        rgb_matrix_set_color(15, brightness, brightness, brightness);
        rgb_matrix_set_color(20, brightness, brightness, brightness);
        rgb_matrix_set_color(22, brightness, brightness, brightness);
        rgb_matrix_set_color(23, brightness, brightness, brightness);
#    endif
#    ifdef SCROLL_COORDINATES_LEFT
        rgb_matrix_set_color(14, brightness, brightness, brightness);
        rgb_matrix_set_color(16, brightness, brightness, brightness);
        rgb_matrix_set_color(17, brightness, brightness, brightness);
        rgb_matrix_set_color(21, brightness, brightness, brightness);
#    endif
    }
    // Highlight right mouse buttons
    if (BIT_GET(virtual_axes_toggle, 2)){
        uint8_t brightness = MIN(255, base_brightness + 64);
#    ifdef MOUSE_COORDINATES_RIGHT
        rgb_matrix_set_color(74, brightness, brightness, brightness);
        rgb_matrix_set_color(76, brightness, brightness, brightness);
        rgb_matrix_set_color(77, brightness, brightness, brightness);
        rgb_matrix_set_color(78, brightness, brightness, brightness);
#    endif
#    ifdef SCROLL_COORDINATES_RIGHT
        rgb_matrix_set_color(61, brightness, brightness, brightness);
        rgb_matrix_set_color(76, brightness, brightness, brightness);
#    endif
    }
# endif

    // Cut power to most LEDs if brightness is zero
    if (!value_was_zero && base_brightness == 0){
        palClearLine(rgb_enable_pin); // gpio_write_pin_low(rgb_enable_pin);
        value_was_zero = true;
    }
    else if (value_was_zero && base_brightness != 0){
        palSetLine(rgb_enable_pin); // gpio_write_pin_high(rgb_enable_pin);
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
    if (is_keyboard_left()){
        select_multiplexer_channel(BOOTMAGIC_COLUMN);
        adcStartAllConversions(&adcManager, BOOTMAGIC_COLUMN);
        adcWaitForConversions(&adcManager);
        bootmagic_key_value = getADCSample(&adcManager, BOOTMAGIC_ROW);
    }
# endif
# if (defined(BOOTMAGIC_ROW_RIGHT) && defined(BOOTMAGIC_COLUMN_RIGHT))
    if (!is_keyboard_left()){
        select_multiplexer_channel(BOOTMAGIC_COLUMN_RIGHT);
        adcStartAllConversions(&adcManager, BOOTMAGIC_COLUMN_RIGHT);
        adcWaitForConversions(&adcManager);
        bootmagic_key_value = getADCSample(&adcManager, BOOTMAGIC_ROW_RIGHT);
    }
#endif

    if (bootmagic_key_value <= ANALOG_RAW_MAX_VALUE){
        bootmagic_key_value = ANALOG_RAW_MAX_VALUE - bootmagic_key_value ;
    }
    else { // bootmagic_key_value > 2047
        bootmagic_key_value = bootmagic_key_value - ANALOG_RAW_MAX_VALUE - 1;
    }

    // greater than the max rest value
    if (bootmagic_key_value > ANALOG_MULTIPLIER_LUT_SIZE) {
        bootloader_jump();
    }
}
#endif

#if defined(VIA_ENABLE) && !defined(VIAL_ENABLE)

#define ITERATE_ALL_PHYSICAL_KEYS(__user_code)              \
for (uint8_t row = 0; row < MATRIX_ROWS; row++){	        \
    if (                                                    \
        (row != ROWS_PER_HAND - 1) &&                       \
        (row != MATRIX_ROWS - 1)                            \
    )                                                       \
    {                                                       \
        for (uint8_t col = 0; col < MATRIX_COLS; col++){    \
            __user_code  								    \
        }                                                   \
	}												        \
}

enum via_hall_effect {
	id_mode = 1,
	id_actuation_point,
	id_deadzone,
	id_down,
	id_up,
};

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

void via_config_set_value(uint8_t *data) {
    /* data = [ value_id, value_data ] */
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_mode:
			ITERATE_ALL_PHYSICAL_KEYS(analog_config[row][col].mode = *value_data; analog_key[row][col].mode = *value_data;)
            break;
        case id_actuation_point:
			ITERATE_ALL_PHYSICAL_KEYS(analog_config[row][col].lower = *value_data;)
            break;
        case id_deadzone:
			ITERATE_ALL_PHYSICAL_KEYS(analog_config[row][col].upper = *value_data;)
            break;
        case id_down:
			ITERATE_ALL_PHYSICAL_KEYS(analog_config[row][col].down = *value_data;)
            break;
        case id_up:
			ITERATE_ALL_PHYSICAL_KEYS(analog_config[row][col].up = *value_data;)
            break;
        default:
            break;
    }
}

void via_config_get_value(uint8_t *data) {
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_mode:
            *value_data = analog_config[0][0].mode;
            break;
        case id_actuation_point:
            *value_data = analog_config[0][0].lower;
            break;
        case id_deadzone:
            *value_data = analog_config[0][0].upper;
            break;
		case id_down:
            *value_data = analog_config[0][0].down;
            break;
		case id_up:
            *value_data = analog_config[0][0].up;
            break;
		default:
			break;
	}
}
#endif