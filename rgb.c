#include "config.h"
#include "custom_matrix.h"

#ifdef RGB_MATRIX_ENABLE

extern uint8_t virtual_axes_toggle;

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
        RGB_MATRIX_INDICATOR_SET_COLOR(led_index, brightness, brightness, brightness);
    }

    return;
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    // Declare variables as static
    static uint8_t brightness = 64;

    // Get current value
    brightness = rgb_matrix_get_val();

#ifdef ANALOG_KEY_VIRTUAL_AXES
    for (uint8_t i = 0; i < 4; i++)
    {
#if (defined(JOYSTICK_LEFT) || defined(JOYSTICK_RIGHT))
        if (
            BIT_GET(virtual_axes_toggle, va_joystick))
        {
#ifdef JOYSTICK_LEFT
            set_virtual_axes_rgb_white(&static_config.joystick_left, i, brightness);
#endif
#ifdef JOYSTICK_RIGHT
            set_virtual_axes_rgb_white(&static_config.joystick_right, i, brightness);
#endif
        }
#endif
#if (defined(JOYSTICK_LEFT) || defined(JOYSTICK_RIGHT))
        if (
            BIT_GET(virtual_axes_toggle, va_mouse))
        {
#ifdef MOUSE_MOVEMENT
            set_virtual_axes_rgb_white(&static_config.mouse_movement, i, brightness);
#endif
#ifdef MOUSE_SCROLL
            set_virtual_axes_rgb_white(&static_config.mouse_scroll, i, brightness);
#endif
        }
#endif
    }
#endif

    return rgb_matrix_indicators_advanced_user(led_min, led_max);
}
#endif