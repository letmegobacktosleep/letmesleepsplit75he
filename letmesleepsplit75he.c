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
extern static_config_t static_config;

// External joystick definitions
#ifdef ANALOG_KEY_VIRTUAL_AXES

extern uint8_t virtual_axes_toggle = 0;
extern uint8_t virtual_axes_from_self[4][4];
extern uint8_t virtual_axes_from_slave[4][4];

joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT] = {
    JOYSTICK_AXIS_VIRTUAL, // x
    JOYSTICK_AXIS_VIRTUAL, // y
    JOYSTICK_AXIS_VIRTUAL, // Rx
    JOYSTICK_AXIS_VIRTUAL  // Ry
};

# ifdef JOYSTICK_COORDINATES
extern const uint8_t joystick_coordinates[8][2];
# endif
# ifdef MOUSE_COORDINATES
extern const uint8_t mouse_coordinates[8][2];
# endif
# ifdef MOUSE_COORDINATES_RIGHT
extern const uint8_t mouse_coordinates_right[8][2];
# endif
#endif

#ifdef RGB_MATRIX_ENABLE
static const pin_t rgb_enable_pin = CUSTOM_RGB_ENABLE_PIN;
#endif

#ifdef DEBUG_LAST_PRESSED
extern uint8_t last_pressed_row;
extern uint8_t last_pressed_col;
extern uint16_t last_pressed_value;
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
    set_default_virtual_axes();
    // write it to eeprom
    eeconfig_update_kb_datablock(&static_config);
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
    eeconfig_read_kb_datablock(&static_config);
    generate_lookup_tables();
#endif
#ifdef SPLIT_KEYBOARD
    transaction_register_rpc(KEYBOARD_SYNC_A, kb_sync_a_slave_handler);
    transaction_register_rpc(USER_SYNC_A, user_sync_a_slave_handler);
#endif
    // Set default state - ignore
    BIT_SET(virtual_axes_toggle, va_ignore_keypresses);
    
    // Call user version of this function
    keyboard_post_init_user();
}



void handle_virtual_mouse_layer(uint8_t virtual_axes_toggle){
    if (
        BIT_GET(virtual_axes_toggle, va_mouse)
    )
    {
        layer_on(MOUSE_LAYER); // turn on mouse layer
    }
    else {
        layer_off(MOUSE_LAYER); // turn off mouse layer
    }
    return;
}

#if (defined(JOYSTICK_LEFT) || defined(JOYSTICK_RIGHT) || defined(MOUSE_MOVEMENT) || defined(MOUSE_SCROLL))
void handle_virtual_axes_keys(virtual_axes_coordinate_t* coordinates, bool should_ignore){
    if (BIT_GET(virtual_axes_toggle, va_ignore_keypresses)){
        for (uint8_t i = 0; i < 4; i++){
            uint8_t row = coordinates->row[i];
            uint8_t col = coordinates->col[i];
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

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

# ifdef DEBUG_SAVE_REST_DOWN
    uint8_t offset = 0;
    char str_buf[8] = { 0 };
    const matrix_row_t mask[MATRIX_ROWS] = CUSTOM_MATRIX_MASK;
    if (!is_keyboard_left()){
        offset = ROWS_PER_HAND;
    }
# endif
# ifdef DEBUG_LAST_PRESSED
    last_pressed_row = record->event.key.row;
    last_pressed_col = record->event.key.col;
# endif

    switch (keycode){
        case KC_ESC:
            // press caps if caps is on
            if (host_keyboard_led_state().caps_lock && record->event.pressed){
                tap_code(KC_CAPS);
            }
            return false;

# ifdef ANALOG_KEY_VIRTUAL_AXES
#    if (defined(JOYSTICK_LEFT) || defined(JOYSTICK_RIGHT))
        // check for keycodes which toggle joystick or mouse
        case J_TG:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, va_joystick);
            }
            // always set mode of joystick keys to 255
#        if defined(JOYSTICK_LEFT)
            handle_virtual_axes_keys(
                &static_config.joystick_left, 
                (bool) BIT_GET(virtual_axes_toggle, va_joystick)
            );
#        endif
#        if defined(JOYSTICK_RIGHT)
            handle_virtual_axes_keys(
                &static_config.joystick_right, 
                (bool) BIT_GET(virtual_axes_toggle, va_joystick)
            );
#        endif
            return false;
#    endif
#    if (defined(MOUSE_MOVEMENT) || defined(MOUSE_SCROLL))
        case M_TG:
            if (record->event.pressed){ // only change state on key press
                BIT_FLP(virtual_axes_toggle, va_mouse);
            }
            // switch mouse layer
            handle_virtual_mouse_layer(virtual_axes_toggle);
            // set mode of mouse keys to 255
#        if defined(MOUSE_MOVEMENT)
            handle_virtual_axes_keys(
                &static_config.mouse_movement, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse)
            );
#        endif
#        if defined(MOUSE_SCROLL)
            handle_virtual_axes_keys(
                &static_config.mouse_scroll, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse)
            );
#        endif
            return false;

        case M_MO:
            if (record->event.pressed){
                BIT_SET(virtual_axes_toggle, va_mouse);
            }
            else {
                BIT_CLR(virtual_axes_toggle, va_mouse);
            }
            // switch mouse layer
            handle_virtual_mouse_layer(virtual_axes_toggle);
            // set mode of mouse keys to 255
#        if defined(MOUSE_MOVEMENT)
            handle_virtual_axes_keys(
                &static_config.mouse_movement, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse)
            );
#        endif
#        if defined(MOUSE_SCROLL)
            handle_virtual_axes_keys(
                &static_config.mouse_scroll, 
                (bool) BIT_GET(virtual_axes_toggle, va_mouse)
            );
#        endif
            return false;
#    endif
# endif
# ifdef DEBUG_SAVE_REST_DOWN
        case DEBUG_REST_DOWN:
            SEND_STRING("row,col,rest,down\n");
            for (uint8_t row = offset; row < ROWS_PER_HAND + offset; row++){
                for (uint8_t col = 0; col < MATRIX_COLS; col++){
                    if (
                        BIT_GET(mask[row], col)
                    )
                    {
                        sprintf(str_buf, "%d", row);
                        SEND_STRING(str_buf);
                        SEND_STRING(",");
                        sprintf(str_buf, "%d", col);
                        SEND_STRING(str_buf);
                        SEND_STRING(",");
                        sprintf(str_buf, "%d", analog_key[row][col].rest);
                        SEND_STRING(str_buf);
                        SEND_STRING(",");
                        sprintf(str_buf, "%d", analog_key[row][col].down);
                        SEND_STRING(str_buf);
                        SEND_STRING("\n");
                    }
                }
            }
            return false;
# endif
        default:
            return true;
    }
}



void housekeeping_task_kb(void) {
# ifdef DEBUG_LAST_PRESSED
    // Print analog value of the last pressed key
    static uint32_t last_print;
    if (timer_elapsed32(last_print) > 100){ // Only print every 100ms
        dprintf("row: %2u, col: %2u, val: %4u\n", last_pressed_row, last_pressed_col, last_pressed_value);
        last_print = timer_read32();
    }
# endif
    
#ifdef ANALOG_KEY_VIRTUAL_AXES
    // Sync virtual axes, if enabled https://docs.qmk.fm/features/split_keyboard#custom-data-sync
    static uint32_t last_sync = 0;
    if (timer_elapsed32(last_sync) > 10){ // Only update every 10ms
#    ifdef SPLIT_KEYBOARD
        if (is_keyboard_master()) {
            if (
                BIT_GET(virtual_axes_toggle, va_joystick) || // joystick
                BIT_GET(virtual_axes_toggle, va_mouse)       // mouse
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
        static int8_t virtual_axes_combined[4][2];
        for (uint8_t i = 0; i < 4; i++){
            for (uint8_t j = 0; j < 4; j += 2){
                virtual_axes_combined[i][j/2] = MAX(-127, MIN(127,
                    virtual_axes_from_self [i][j+1] + // right or down
                    virtual_axes_from_slave[i][j+1] - // right or down
                    virtual_axes_from_self [i][j] -   // left or up
                    virtual_axes_from_slave[i][j]     // left or up
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
            joystick_set_axis(2, virtual_axes_combined[1][0]);
            joystick_set_axis(3, virtual_axes_combined[1][1]);

            // Send joystick report
            joystick_flush();
        }
#    endif
#    if (defined(MOUSE_COORDINATES) || defined(MOUSE_COORDINATES_RIGHT))      
        // Only run mouse if toggled on
        // https://docs.qmk.fm/features/pointing_device#manipulating-mouse-reports
        if (
            BIT_GET(virtual_axes_toggle, va_mouse)
        )
        {
            // Get current report
            report_mouse_t currentReport = pointing_device_get_report();

            // Mouse movement
            currentReport.x = virtual_axes_combined[2][0];
            currentReport.y = virtual_axes_combined[2][1];

            // Scroll speed
            int8_t mouse_h = virtual_axes_combined[3][0];
            int8_t mouse_v = virtual_axes_combined[3][1];

            // How much time until next scroll report
            static uint8_t next_scroll[2] = { 0 };

            // Horizontal scroll
            if (next_scroll[0] == 0){
                if (mouse_h > SCROLL_DEADZONE){
                    currentReport.h = 1;
                    next_scroll[0] = 32 - (abs(mouse_h) / 4);
                }
                else if (mouse_h < -SCROLL_DEADZONE){
                    currentReport.h = -1;
                    next_scroll[0] = 32 - (abs(mouse_h) / 4);
                }
            }
            else {
                next_scroll[0]--;
            }

            // Vertical scroll
            if (next_scroll[1] == 0){
                if (mouse_v > SCROLL_DEADZONE){
                    currentReport.v = 1;
                    next_scroll[1] = 32 - (abs(mouse_v) / 4);
                }
                else if (mouse_v < -SCROLL_DEADZONE){
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
void set_virtual_axes_rgb_white(virtual_axes_coordinate_t* virtual_axes, uint8_t index, uint8_t brightness){

    // fetch the LED index
    uint8_t row = virtual_axes->row[index]
    uint8_t col = virtual_axes->col[index]
    uint8_t led_index = g_led_config.matrix_co[row][col];

    // if there is an LED on that key
    if (
        led_index >= led_min && 
        led_index < led_max && 
        led_index != NO_LED
    )
    {
        rgb_matrix_set_color(led_index, brightness, brightness, brightness);
    }

    return;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    static bool value_was_zero = true;
    static uint8_t brightness = 64;
    static uint8_t caps_brightness = 128;

    // Get current value
    brightness = rgb_matrix_get_val();

    // Light up Esc if CapsLock
    if (host_keyboard_led_state().caps_lock){
        // Always make brightness brighter than current brightness
        caps_brightness = MIN(255, brightness + 64);
        RGB_MATRIX_INDICATOR_SET_COLOR(0, caps_brightness, caps_brightness, caps_brightness);
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
    for (uint8_t i = 0; i < 4; i++){
# if (defined(JOYSTICK_LEFT) || defined(JOYSTICK_RIGHT))
        if (
            BIT_GET(virtual_axes_toggle, va_joystick)
        )
        {
#    ifdef JOYSTICK_LEFT
            set_virtual_axes_rgb_white(&static_config.joystick_left, i, brightness);
#    endif
#    ifdef JOYSTICK_RIGHT
            set_virtual_axes_rgb_white(&static_config.joystick_right, i, brightness);
#    endif
        }
# endif
# if (defined(JOYSTICK_LEFT) || defined(JOYSTICK_RIGHT))
        if (
            BIT_GET(virtual_axes_toggle, va_mouse)
        )
        {
#    ifdef MOUSE_MOVEMENT
            set_virtual_axes_rgb_white(&static_config.mouse_movement, i, brightness);
#    endif
#    ifdef MOUSE_SCROLL
            set_virtual_axes_rgb_white(&static_config.mouse_scroll, i, brightness);
#    endif
        }
# endif
    }
#endif

    // Always return false
    return false;
}
#endif