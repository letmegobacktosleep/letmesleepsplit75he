/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

extern uint8_t virtual_axes_toggle;

#define BIT_SET(byte, nbit) ((byte) |=  (1 << (nbit)))
#define BIT_CLR(byte, nbit) ((byte) &= ~(1 << (nbit)))
#define BIT_FLP(byte, nbit) ((byte) ^=  (1 << (nbit)))
#define BIT_GET(byte, nbit) ((byte) &   (1 << (nbit)))

// Function prototypes
void user_write_new_config(uint8_t row, uint8_t col);

// Virtual axes toggle bits
enum virtual_axes_bits {
    va_ignore_keypresses = 0,
    va_joystick,
    va_mouse,
    va_mouse_right,
}

// Custom keycodes
enum custom_keycodes {
    J_TG = QK_KB_0,
    M_TG,
    M_MO,
    M_TG1,
    M_MO1,
    M_TG2,
    M_MO2,
    PRINT_REST_DOWN
};
/* This goes in the vial.json
"customKeycodes": [
    {
        "name": "Analog Joystick Toggle",
        "title": "Toggle use of WASD & PL;' as a joystick",
        "shortName": "KC_JS_TG"
    },
    {
        "name": "Analog Mouse Toggle All",
        "title": "Toggle use of WASD & Arrow Keys to control your mouse",
        "shortName": "KC_MS_TG"
    },
    {
        "name": "Analog Mouse Momentary All",
        "title": "Momentarily use WASD & Arrow Keys to control your mouse",
        "shortName": "KC_MS_MO"
    },
    {
        "name": "Analog Mouse Toggle Left",
        "title": "Toggle use of WASD to control your mouse",
        "shortName": "KC_MS_TG1"
    },
    {
        "name": "Analog Mouse Momentary Left",
        "title": "Momentarily use WASD Keys to control your mouse",
        "shortName": "KC_MS_MO1"
    },
    {
        "name": "Analog Mouse Toggle Right",
        "title": "Toggle use of Arrow Keys to control your mouse",
        "shortName": "KC_MS_TG2"
    },
    {
        "name": "Analog Mouse Momentary Right",
        "title": "Momentarily use Arrow Keys to control your mouse",
        "shortName": "KC_MS_MO2"
    }
],
*/