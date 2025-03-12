/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "quantum.h"
#include "transactions.h"
#include "pointing_device.h"

#include "config.h"
#include "custom_matrix.h"
#include "custom_analog.h"
#include "custom_scanning.h"
#include "custom_transactions.h"
#include "eeconfig_set_defaults.h"
#include "letmesleepsplit75he.h"

// External definitions
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];
extern analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS];
extern calibration_parameters_t calibration_parameters;

// External joystick definitions
#ifdef ANALOG_KEY_VIRTUAL_AXES

uint8_t virtual_axes_toggle = 0;
extern uint8_t virtual_axes_from_self[2][8];
extern uint8_t virtual_axes_from_slave[2][8];

joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT] = {
    JOYSTICK_AXIS_VIRTUAL, // x
    JOYSTICK_AXIS_VIRTUAL, // y
    JOYSTICK_AXIS_VIRTUAL, // Rx
    JOYSTICK_AXIS_VIRTUAL  // Ry
};

# ifdef JOYSTICK_COORDINATES
extern const uint8_t joystick_coordinates[2][8];
# endif
# ifdef MOUSE_COORDINATES
extern const uint8_t mouse_coordinates[2][8];
# endif
# ifdef MOUSE_COORDINATES_RIGHT
extern const uint8_t mouse_coordinates_right[2][8];
# endif
#endif

#ifdef RGB_MATRIX_ENABLE
static const pin_t rgb_enable_pin = CUSTOM_RGB_ENABLE_PIN;
#endif

void eeconfig_init_user(void) {
    // set default values
    set_default_analog_config();
    // write it to eeprom
    eeconfig_update_user_datablock(&analog_config);
}

void eeconfig_init_kb(void) {
    // set default values
    set_default_calibration_parameters();
    // write it to eeprom
    eeconfig_update_kb_datablock(&calibration_parameters);
    // call user
    eeconfig_init_user();
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
    keyboard_post_init_user();
}



#ifdef BOOTMAGIC_ENABLE
void bootmagic_scan(void) {

    uint16_t bootmagic_key_value = 0;

# if (defined(BOOTMAGIC_ROW) && defined(BOOTMAGIC_COLUMN))
    if (is_keyboard_left()){
        select_multiplexer_channel(BOOTMAGIC_COLUMN);
        adcStartAllConversions(BOOTMAGIC_COLUMN);
        adcWaitForConversions();
        bootmagic_key_value = getADCSample(BOOTMAGIC_ROW);
    }
# endif
# if (defined(BOOTMAGIC_ROW_RIGHT) && defined(BOOTMAGIC_COLUMN_RIGHT))
    if (!is_keyboard_left()){
        select_multiplexer_channel(BOOTMAGIC_COLUMN_RIGHT);
        adcStartAllConversions(BOOTMAGIC_COLUMN_RIGHT);
        adcWaitForConversions();
        bootmagic_key_value = getADCSample(BOOTMAGIC_ROW_RIGHT);
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



bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    switch (keycode){
        case KC_ESC:
            // press caps if caps is on
            if (host_keyboard_led_state().caps_lock && record->event.pressed){
                tap_code(KC_CAPS);
            }
            return false;

#    ifdef ANALOG_KEY_VIRTUAL_AXES
        // check for keycodes which toggle joystick or mouse
        case KC_JS_TG:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, va_joystick);
            }
            // always set mode of joystick keys to 255
            handle_virtual_axes_keys(
                joystick_coordinates,
                (bool) BIT_GET(virtual_axes_toggle, va_joystick)
            );
            return false;

        case KC_MS_TG:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, va_mouse);
                BIT_FLP(virtual_axes_toggle, va_mouse_right);
            }
            // switch mouse layer
            handle_virtual_mouse_layer(virtual_axes_toggle);
            // set mode of mouse keys to 255
            handle_virtual_axes_keys(
                mouse_coordinates, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse)
            );
            handle_virtual_axes_keys(
                mouse_coordinates_right, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse_right)
            );
            return false;
            
        case KC_MS_MO:
            if (record->event.pressed){
                BIT_SET(virtual_axes_toggle, va_mouse);
                BIT_SET(virtual_axes_toggle, va_mouse_right);
            }
            else {
                BIT_CLR(virtual_axes_toggle, va_mouse);
                BIT_CLR(virtual_axes_toggle, va_mouse_right);
            }
            // switch mouse layer
            handle_virtual_mouse_layer(virtual_axes_toggle);
            // set mode of mouse keys to 255
            handle_virtual_axes_keys(
                mouse_coordinates, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse)
            );
            handle_virtual_axes_keys(
                mouse_coordinates_right, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse_right)
            );
            return false;

        case KC_MS_TG1:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, va_mouse);
            }
            // switch mouse layer
            handle_virtual_mouse_layer(virtual_axes_toggle);
            // set mode of mouse keys to 255
            handle_virtual_axes_keys(
                mouse_coordinates, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse)
            );
            return false;
            
        case KC_MS_MO1:
            if (record->event.pressed){
                BIT_SET(virtual_axes_toggle, va_mouse);
            }
            else {
                BIT_CLR(virtual_axes_toggle, va_mouse);
            }
            // switch mouse layer
            handle_virtual_mouse_layer(virtual_axes_toggle);
            // set mode of mouse keys to 255
            handle_virtual_axes_keys(
                mouse_coordinates, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse)
            );
            return false;

        case KC_MS_TG2:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, va_mouse_right);
            }
            // switch mouse layer
            handle_virtual_mouse_layer(virtual_axes_toggle);
            // set mode of mouse keys to 255
            handle_virtual_axes_keys(
                mouse_coordinates_right, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse_right)
            );
            return false;
            
        case KC_MS_MO2:
            if (record->event.pressed){
                BIT_SET(virtual_axes_toggle, va_mouse_right);
            }
            else {
                BIT_CLR(virtual_axes_toggle, va_mouse_right);
            }
            // switch mouse layer
            handle_virtual_mouse_layer(virtual_axes_toggle);
            // set mode of mouse keys to 255
            handle_virtual_axes_keys(
                mouse_coordinates_right, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse_right)
            );
            return false;
#    endif
#    ifdef DEBUG_REST_DOWN
        case PRINT_REST_DOWN:
            uint8_t offset = 0;
            char str_buf = [8];
            const matrix_row_t mask[MATRIX_ROWS] = CUSTOM_MATRIX_MASK;

            if (!is_keyboard_left()){
                offset = ROWS_PER_HAND
            }
            SEND_STRING("row,col,rest,down\n");
            for (row = offset; row < ROWS_PER_HAND + offset; row++){
                for (col = 0; col < MATRIX_COLS; col++){
                    if (
                        BIT_GET(mask[row], col)
                    )
                    {
                        sprintf(str_buf, "%d", row);
                        SEND_STRING(str_buf);
                        SEND_STRING(",")
                        sprintf(str_buf, "%d", col);
                        SEND_STRING(str_buf);
                        SEND_STRING(",")
                        sprintf(str_buf, "%d", analog_key[row][col].rest);
                        SEND_STRING(str_buf);
                        SEND_STRING(",")
                        sprintf(str_buf, "%d", analog_key[row][col].down);
                        SEND_STRING(str_buf);
                        SEND_STRING("\n")
                    }
                }
            }
#    endif
        default:
            return true;
    }
}



void handle_virtual_mouse_layer(uint8_t virtual_axes_toggle){
    if (
        BIT_GET(virtual_axes_toggle, va_mouse) || 
        BIT_GET(virtual_axes_toggle, va_mouse_right)
    )
    {
        layer_on(MOUSE_LAYER); // turn on mouse layer
    }
    else {
        layer_off(MOUSE_LAYER); // turn off mouse layer
    }
    return;
}

void handle_virtual_axes_keys(uint8_t coordinates[8][2], bool should_ignore){
    if (BIT_GET(virtual_axes_toggle, va_ignore_keypresses)){
        for (uint8_t i = 0; i < 8, i++){
            row = coordinates[i][0];
            col = coordinates[i][1];
            if (row != 255 && col != 255){
                if (should_ignore){
                    analog_key[row][col].mode = 255;
                }
                else {
                    analog_key[row][col].mode = analog_config[row][col].mode;
                }
            }
        }
    }
}

void housekeeping_task_kb(void) {
    // Sync virtual axes, if enabled https://docs.qmk.fm/features/split_keyboard#custom-data-sync
#ifdef ANALOG_KEY_VIRTUAL_AXES
    static uint32_t last_sync = 0;
    if (timer_elapsed32(last_sync) > 10){ // Only update every 10ms
#    ifdef SPLIT_KEYBOARD
        if (is_keyboard_master()) {
            if (
                BIT_GET(virtual_axes_toggle, va_joystick) || // joystick
                BIT_GET(virtual_axes_toggle, va_mouse)    || // left mouse
                BIT_GET(virtual_axes_toggle, va_mouse_right) // right mouse
            )
            {
                // send the virtual axes toggle to the slave, receive its virtual axes
                if (
                    transaction_rpc_exec(
                        KEYBOARD_SYNC_A, 
                        sizeof(virtual_axes_toggle),
                        &virtual_axes_toggle, 
                        sizeof(virtual_axes_from_slave),
                        &virtual_axes_from_slave
                    )
                )
                {
                    last_sync = timer_read32();
                }
            }
        }
#    endif
        static int8_t virtual_axes_combined[2][4];
        for (uint8_t i = 0; i < 2; i++){
            for (uint8_t j = 0; j < 8; j += 2){
                virtual_axes_combined[i][j/2] = MAX(-127, MIN(127, 
                    virtual_axes_from_self [i][j+1] + 
                    virtual_axes_from_slave[i][j+1] - 
                    virtual_axes_from_self [i][j] - 
                    virtual_axes_from_slave[i][j]
                ));
            }
        }
        
#    if defined(JOYSTICK_COORDINATES)
        // Only run joystick if toggled on
        // https://docs.qmk.fm/features/joystick#virtual-axes
        if (
            BIT_GET(virtual_axes_toggle, va_joystick)
        )
        {
            // Left joystick
            joystick_set_axis(0, virtual_axes_combined[0][0]);
            joystick_set_axis(1, virtual_axes_combined[0][1]);

            // Right joystick
            joystick_set_axis(2, virtual_axes_combined[0][2]);
            joystick_set_axis(3, virtual_axes_combined[0][3]);

            // Send joystick report
            joystick_flush();
        }
#    endif
#    if (defined(MOUSE_COORDINATES) || defined(MOUSE_COORDINATES_RIGHT))      
        // Only run mouse if toggled on
        // https://docs.qmk.fm/features/pointing_device#manipulating-mouse-reports
        if (
            BIT_GET(virtual_axes_toggle, va_mouse) ||
            BIT_GET(virtual_axes_toggle, va_mouse_right)
        )
        {
            // Get current report
            report_mouse_t currentReport = pointing_device_get_report();

            // Get current mouse speed
            int8_t mouse_x = virtual_axes_combined[1][0];
            int8_t mouse_y = virtual_axes_combined[1][1];
            int8_t mouse_h = virtual_axes_combined[1][2];
            int8_t mouse_v = virtual_axes_combined[1][3];

            // Horizontal mouse
            if (mouse_x > MOUSE_DEADZONE){
                currentReport.x = mouse_x - MOUSE_DEADZONE;
            }
            else if (mouse_x < - MOUSE_DEADZONE){
                currentReport.x = mouse_x + MOUSE_DEADZONE;
            }

            // Vertical mouse
            if (mouse_y > MOUSE_DEADZONE){
                currentReport.y = mouse_y - MOUSE_DEADZONE;
            }
            else if (mouse_y < - MOUSE_DEADZONE){
                currentReport.y = mouse_y + MOUSE_DEADZONE;
            }

            // How much time until next scroll report
            static uint8_t next_scroll[2] = { 0 };

            // Horizontal scroll
            if (next_scroll[0] == 0){
                if (mouse_h > MOUSE_DEADZONE){
                    currentReport.h = 1;
                    next_scroll[0] = 32 - (abs(mouse_h) / 4);
                }
                else if (mouse_h < - MOUSE_DEADZONE){
                    currentReport.h = -1;
                    next_scroll[0] = 32 - (abs(mouse_h) / 4);
                }
            }
            else {
                next_scroll[0]--;
            }

            // Vertical scroll
            if (next_scroll[1] == 0){
                if (mouse_v > MOUSE_DEADZONE){
                    currentReport.v = 1;
                    next_scroll[1] = 32 - (abs(mouse_v) / 4);
                }
                else if (mouse_v < - MOUSE_DEADZONE){
                    currentReport.v = -1;
                    next_scroll[1] = 32 - (abs(mouse_v) / 4);
                }
            }
            else {
                next_scroll[1]--;
            }

            // Override current report & send
            pointing_device_set_report(currentReport);
            pointing_device_send();
        }
#    endif
    }
#endif
}

#ifdef RGB_MATRIX_ENABLE
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    static bool value_was_zero = true;
    static uint8_t brightness = 64;
    static uint8_t caps_brightness = 192;

    // Get current value
    brightness = rgb_matrix_get_val();

    // Light up Esc if CapsLock
    if (host_keyboard_led_state().caps_lock){
        // Always make brightness brighter than current brightness
        caps_brightness = MIN(255, brightness + 128);
        rgb_matrix_set_color(0, caps_brightness, caps_brightness, caps_brightness);
    }

    // Cut power to all other LEDs if brightness is zero
    if (!value_was_zero && brightness == 0){
        palClearLine(rgb_enable_pin); // gpio_write_pin_low(rgb_enable_pin);
        value_was_zero = true;
    }
    else if (value_was_zero && brightness != 0){
        palSetLine(rgb_enable_pin); // gpio_write_pin_high(rgb_enable_pin);
        value_was_zero = false;
    }

#ifdef ANALOG_KEY_VIRTUAL_AXES
# ifdef JOYSTICK_COORDINATES
    // Highlight joystick buttons
    if (BIT_GET(virtual_axes_toggle, va_joystick)){
        // Left joystick
        rgb_matrix_set_color(15, brightness, brightness, brightness);
        rgb_matrix_set_color(21, brightness, brightness, brightness);
        rgb_matrix_set_color(22, brightness, brightness, brightness);
        rgb_matrix_set_color(23, brightness, brightness, brightness);
        // Right joystick
        rgb_matrix_set_color(56, brightness, brightness, brightness);
        rgb_matrix_set_color(63, brightness, brightness, brightness);
        rgb_matrix_set_color(64, brightness, brightness, brightness);
        rgb_matrix_set_color(65, brightness, brightness, brightness);
    }
# endif
# ifdef MOUSE_COORDINATES
    // Highlight left mouse buttons
    if (BIT_GET(virtual_axes_toggle, va_mouse)){
        rgb_matrix_set_color(15, brightness, brightness, brightness);
        rgb_matrix_set_color(21, brightness, brightness, brightness);
        rgb_matrix_set_color(22, brightness, brightness, brightness);
        rgb_matrix_set_color(23, brightness, brightness, brightness);
        // Left scroll
        rgb_matrix_set_color(14, brightness, brightness, brightness);
        rgb_matrix_set_color(16, brightness, brightness, brightness);
        rgb_matrix_set_color(17, brightness, brightness, brightness);
        rgb_matrix_set_color(20, brightness, brightness, brightness);
    }
# endif
# ifdef MOUSE_COORDINATES_RIGHT
    // Highlight right mouse buttons
    if (BIT_GET(virtual_axes_toggle, va_mouse_right)){
        // Right mouse
        rgb_matrix_set_color(75, brightness, brightness, brightness);
        rgb_matrix_set_color(77, brightness, brightness, brightness);
        rgb_matrix_set_color(78, brightness, brightness, brightness);
        rgb_matrix_set_color(79, brightness, brightness, brightness);
        // Right scroll
        rgb_matrix_set_color(61, brightness, brightness, brightness);
        rgb_matrix_set_color(76, brightness, brightness, brightness);
    }
# endif
#endif

    // Always return false
    return false;
}
#endif