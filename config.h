#pragma once

#include <stdbool.h>
#include <stdint.h>

// number of multiplexer channels
#define MATRIX_COLS 16
// number of ADC channels (whichever has more * 2)
#define MATRIX_ROWS 8

// max number of multiplexers per ADC (cannot be greater than 5) 
/* any changes to this definition also requires modifying custom_analog.c */
#define MAX_MUXES_PER_ADC 1

// number of direct pins (cannot be greater than 16) // MATRIX_DIRECT_RIGHT
/* any changes to this definition also requires modifying custom_analog.c */
#define MATRIX_DIRECT 4
// the row which contains the direct pins // MATRIX_DIRECT_ROW_RIGHT
/* any changes to this definition also requires modifying custom_analog.c */
#define MATRIX_DIRECT_ROW 2

// max value of raw
#define ANALOG_RAW_MAX_VALUE 2047
// max value of calibrated
#define ANALOG_CAL_MAX_VALUE 1023

// channel select pins
#define MATRIX_COL_PINS { \
    PA8,    PB15,   PB14,   PB13,   \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN, \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN, \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN  \
}
#define MATRIX_COL_PINS_RIGHT { \
    PA8,    PB15,   PB14,   PB13,   \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN, \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN, \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN  \
}
// ADC pins
#define MATRIX_ROW_PINS { \
    PB12,   \
    PA3,    \
    NO_PIN, \
    NO_PIN  \
}
#define MATRIX_ROW_PINS_RIGHT { \
    PB12,   \
    PA7,    \
    PA3,    \
    NO_PIN  \
}
// ADC pins for direct pins // DIRECT_PINS_RIGHT
#define DIRECT_PINS { \
    PA6   \
    PA7,  \
    PA5,  \
    PA4   \
}
/* LEFT
row 0 = mux 1 = ADC4_IN3
row 1 = mux 2 = ADC1_IN4
row 2 = WASD  = ADC2_IN3, ADC2_IN4, ADC2_IN2, ADC2_IN1
row 3 = DKS, left hand
// RIGHT
row 4 = mux 3 = ADC4_IN3
row 5 = mux 4 = ADC3_IN1
row 6 = mux 5 = ADC1_IN4
row 7 = DKS, right hand
*/

// bit array of whether key is valid
#define CUSTOM_MATRIX_MASK { \
    {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1}, \
    {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1}, \
    {1,1,1,1, 0,0,0,0, 0,0,0,0, 0,0,0,0}, \
    {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}, \
    {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1}, \
    {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1}, \
    {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1}, \
    {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}  \
}

// joystick shit
#define ANALOG_KEY_VIRTUAL_AXES




// Change emulated flash size
// https://docs.qmk.fm/drivers/eeprom#wear_leveling-efl-driver-configuration
#define WEAR_LEVELING_EFL_OMIT_LAST_SECTOR_COUNT 0
#define WEAR_LEVELING_LOGICAL_SIZE 4096
#define WEAR_LEVELING_BACKING_SIZE 8192
// Set size of EECONFIG for analog_config (per key)
#define EECONFIG_USER_DATA_SIZE (5 * MATRIX_ROWS * MATRIX_COLS)
// Set size of EECONFIG for calibration (global)
#define EECONFIG_KB_DATA_SIZE (35 * 3)

// Set ADC resolution and sampling time
#define ADC_RESOLUTION      ADC_CFGR_RES_12BITS
#define ADC_SAMPLING_TIME   ADC_SMPR_SMP_2P5
#define ADC_RESOLUTION_MAX  1 << 12




// only define these other things if ANALOG_KEY_VIRTUAL_AXES was defined (make it easier to disable)
#ifdef ANALOG_KEY_VIRTUAL_AXES
# define MOUSE_LAYER 3
# define JOYSTICK_COORDINATES_ONE { \
    {2, 1},  \
    {2, 3},  \
    {2, 0},  \
    {2, 2}   \
}
# define JOYSTICK_COORDINATES_TWO {
    {4, 9},  \
    {4, 15}, \
    {4, 7},  \
    {4, 8}   \
}
# define MOUSE_COORDINATES_ONE {
    {2, 1},  \
    {2, 3},  \
    {2, 0},  \
    {2, 2}   \
}
# define MOUSE_COORDINATES_TWO {
    {5, 10}, \
    {5, 13}, \
    {5, 11}, \
    {5, 12}  \
}
# define SCROLL_COORDINATES_ONE {
    {1, 10}, \
    {1, 8}   \
}
# define SCROLL_COORDINATES_TWO {
    {6, 15}, \
    {6, 14}  \
}
#endif

// Master to slave:
#define RPC_M2S_BUFFER_SIZE 32
// Slave to master:
#define RPC_S2M_BUFFER_SIZE 32
// Keyboard level data sync:
#define SPLIT_TRANSACTION_IDS_KB KEYBOARD_SYNC_A









#define WS2812_EXTERNAL_PULLUP
// fuck, deal with it later
// https://docs.qmk.fm/features/rgb_matrix#common-configuration

#ifdef RGB_MATRIX_ENABLE
#define WS2812_PWM_DRIVER PWMD2
#define WS2812_PWM_CHANNEL 4
#define WS2812_PWM_PAL_MODE 10
// NOT SURE IF THE NEXT TWO ARE CORRECT
#define WS2812_DMA_STREAM STM32_DMA1_STREAM2 
#define WS2812_PWM_DMA_CHANNEL 2
#endif

/*
// set PA9 as output
pin_t rgb_enable_pin = PA9;
setPinOutput(rgb_enable_pin);

if (rgb_matrix_is_enabled()){
    // drive PA9 high
}
else {
    // drive PA9 low
}

this should go in the main source file keyboardname.c
*/