/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

enum custom_keycodes {
    KC_JS_TG = QK_KB_0,
    KC_MS_TG,
    KC_MS_MO,
};
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