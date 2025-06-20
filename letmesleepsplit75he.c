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
#include "via_vial_communication.h"
#include "letmesleepsplit75he.h"

// External definitions
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];
extern analog_config_t analog_config[MATRIX_ROWS][MATRIX_COLS];
extern static_config_t static_config;

// External joystick definitions
#ifdef ANALOG_KEY_VIRTUAL_AXES

extern uint8_t virtual_axes_toggle;
extern uint8_t virtual_axes_from_self[4][4];
extern uint8_t virtual_axes_from_slave[4][4];

joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT] = {
    JOYSTICK_AXIS_VIRTUAL, // x
    JOYSTICK_AXIS_VIRTUAL, // y
    JOYSTICK_AXIS_VIRTUAL, // Rx
    JOYSTICK_AXIS_VIRTUAL  // Ry
};

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
    transaction_register_rpc(KEYBOARD_SYNC_CONFIG, kb_sync_a_slave_handler);
    transaction_register_rpc(USER_SYNC_JOYSTICK, user_sync_a_slave_handler);
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
        if (is_keyboard_master()){
            if (
                BIT_GET(virtual_axes_toggle, va_joystick) || // joystick
                BIT_GET(virtual_axes_toggle, va_mouse)       // mouse
            )
            {
                // send the virtual axes toggle to the slave, receive its virtual axes
                if (
                    transaction_rpc_exec(
                        USER_SYNC_JOYSTICK, 
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
