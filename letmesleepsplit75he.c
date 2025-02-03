#include <stdint.h>
#include <stdbool.h>

#include <config.h>
#include <matrix.h>

// https://discord.com/channels/440868230475677696/440868230475677698/1334525203044106241
// when you modify a struct, 
// you should try to save it using these functions
// it only updates a part of it at a time
// EEPROM_USER_PARTIAL_READ(analog_config[row][col], name_of_element);
// EEPROM_USER_PARTIAL_UPDATE(analog_config[row][col], name_of_element);

#if (EECONFIG_KB_DATA_SIZE) > 0
#    define EEPROM_KB_PARTIAL_READ(__struct, __field) eeprom_read_block(&(__struct.__field), (void *)((void *)(EECONFIG_KB_DATABLOCK) + offsetof(typeof(__struct), __field)), sizeof(__struct.__field))
#    define EEPROM_KB_PARTIAL_UPDATE(__struct, __field) eeprom_update_block(&(__struct.__field), (void *)((void *)(EECONFIG_KB_DATABLOCK) + offsetof(typeof(__struct), __field)), sizeof(__struct.__field))
#endif
#if (EECONFIG_USER_DATA_SIZE) > 0
#    define EEPROM_USER_PARTIAL_READ(__struct, __field) eeprom_read_block(&(__struct.__field), (void *)((void *)(EECONFIG_USER_DATABLOCK) + offsetof(typeof(__struct), __field)), sizeof(__struct.__field))
#    define EEPROM_USER_PARTIAL_UPDATE(__struct, __field) eeprom_update_block(&(__struct.__field), (void *)((void *)(EECONFIG_USER_DATABLOCK) + offsetof(typeof(__struct), __field)), sizeof(__struct.__field))
#endif

bool virtual_joystick_toggle = false;
bool virtual_mouse_toggle    = false;

enum custom_keycodes {
    KC_JOYSTICK_TOGGLE = QK_KB_0,
    KC_MOUSE_TOGGLE,
    KC_MOUSE_MOMENTARY,
}
/* This goes in the vial.json
"customKeycodes": [
	{"name": "JoystickTG",
	 "title": "Toggle use of WASD & PL;' as a joystick",
	 "shortName": "KC_JOYSTICK_TOGGLE"
	},
	{"name": "MouseTG",
	 "title": "Toggle use of WASD & Arrow Keys to control your mouse",
	 "shortName": "KC_MOUSE_TOGGLE"
	},
	{"name": "MouseMO",
	 "title": "Momentarily use WASD & Arrow Keys to control your mouse",
	 "shortName": "KC_MOUSE_MOMENTARY"
	}
],
*/

#ifdef SPLIT_KEYBOARD
void kb_sync_a_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    // copy virtual_axes_from_self to the outbound buffer
    memcpy(out_data, virtual_axes_from_self, sizeof(virtual_axes_from_self));
}
#endif
/*
// https://docs.qmk.fm/features/joystick#configuration
#define JOYSTICK_BUTTON_COUNT 0
#define JOYSTICK_AXIS_COUNT 4
#define JOYSTICK_AXIS_RESOLUTION 8
*/
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
    eeconfig_update_user_datablock(&analog_config) // write it to eeprom
}

void eeconfig_init_kb(void) {
    set_default_calibration_parameters(); // set default values
    eeconfig_update_kb_datablock(&calibration_parameters); // write it to eeprom
}

void keyboard_post_init_user(void) {
#if (EECONFIG_USER_DATA_SIZE) > 0
    eeconfig_read_user_datablock(&analog_config);
#endif
}

void keyboard_post_init_kb(void) {
#if (EECONFIG_KB_DATA_SIZE) > 0
    eeconfig_read_kb_datablock(&calibration_parameters);
#endif
#ifdef SPLIT_KEYBOARD
    transaction_register_rpc(KEYBOARD_SYNC_A, kb_sync_a_slave_handler);
#endif
}

// Tasks that will be run repeatedly

void housekeeping_task_user(void) {
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
            virtual_axes_combined[i][0] = MAX(-127, MIN(127, virtual_axes_from_self[i][0] + virtual_axes_from_slave[i][0] - virtual_axes_from_self[i][1] - virtual_axes_from_slave[i][1]))
            virtual_axes_combined[i][1] = MAX(-127, MIN(127, virtual_axes_from_self[i][2] + virtual_axes_from_slave[i][2] - virtual_axes_from_self[i][3] - virtual_axes_from_slave[i][3]))
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
            // joystick_flush(); // Shouldn't have to manually call it...
        }
#    endif
#    if (defined(MOUSE_COORDINATES_ONE) || defined(MOUSE_COORDINATES_TWO) || defined(SCROLL_COORDINATES_ONE) || defined(SCROLL_COORDINATES_TWO))
        // Only run mouse if toggled on
        // https://docs.qmk.fm/features/pointing_device#manipulating-mouse-reports
        if (virtual_mouse_toggle){
            // Get current report
            report_mouse_t currentReport = pointing_device_get_report()
#        if (defined(MOUSE_COORDINATES_ONE) || defined(MOUSE_COORDINATES_ONE))
            // Set mouse movement
            currentReport.x = MAX(-127, MIN(127, virtual_axes_combined[2][0] + virtual_axes_combined[3][0])); // x
            currentReport.y = MAX(-127, MIN(127, virtual_axes_combined[2][1] + virtual_axes_combined[3][1])); // y
#        endif
#        if (defined(SCROLL_COORDINATES_ONE) || defined(SCROLL_COORDINATES_TWO))
            // Set scroll
            currentReport.v = MAX(-127, MIN(127, virtual_axes_combined[4][0] + virtual_axes_combined[5][0])); // vertical scroll
            currentReport.h = MAX(-127, MIN(127, virtual_axes_combined[4][1] + virtual_axes_combined[5][1])); // horizontal scroll
#        endif
            // Override current report & send
            pointing_device_set_report(report_mouse_t currentReport)
            pointing_device_send()
        }
#    endif
    }
#endif
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    switch (keycode){
#    ifdef ANALOG_KEY_VIRTUAL_AXES
        // check for keycodes which toggle joystick or mouse
        case KC_JOYSTICK_TOGGLE:
            if (record->event.pressed){ // only change state on key press
                virtual_joystick_toggle = !virtual_joystick_toggle;
            }
            return false;

        case KC_MOUSE_TOGGLE:
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
            
        case KC_MOUSE_MOMENTARY:
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
        default: // ignore keypresses on keys used for virtual axes
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