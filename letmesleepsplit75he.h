/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "config.h"

#ifdef ANALOG_KEY_VIRTUAL_AXES
extern uint8_t virtual_axes_toggle;
#endif

// Custom keycodes
enum custom_keycodes {
    J_TG = QK_KB_0,
    M_TG,
    M_MO,
    M_TG_L,
    M_MO_L,
    M_TG_R,
    M_MO_R,
    DEBUG_REST_DOWN,
};
/* This goes in the vial.json
"customKeycodes": [
    {
        "name": "Analog Joystick Toggle",
        "title": "Toggle use of WASD & PL;' as a joystick",
        "shortName": "J_TG"
    },
    {
        "name": "Analog Mouse Toggle All",
        "title": "Toggle use of WASD & Arrow Keys to control your mouse",
        "shortName": "M_TG"
    },
    {
        "name": "Analog Mouse Momentary All",
        "title": "Momentarily use WASD & Arrow Keys to control your mouse",
        "shortName": "M_MO"
    },
    {
        "name": "Analog Mouse Toggle Left",
        "title": "Toggle use of WASD to control your mouse",
        "shortName": "M_TG_L"
    },
    {
        "name": "Analog Mouse Momentary Left",
        "title": "Momentarily use WASD Keys to control your mouse",
        "shortName": "M_MO_L"
    },
    {
        "name": "Analog Mouse Toggle Right",
        "title": "Toggle use of Arrow Keys to control your mouse",
        "shortName": "M_TG_R"
    },
    {
        "name": "Analog Mouse Momentary Right",
        "title": "Momentarily use Arrow Keys to control your mouse",
        "shortName": "M_MO_R"
    }
],
*/