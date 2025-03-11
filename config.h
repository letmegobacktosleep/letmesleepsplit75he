/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// enable matrix scan rate debugging
#define DEBUG_MATRIX_SCAN_RATE
#define DEBUG_SAVE_REST_DOWN

// enable processing of mouse and joystick
#define ANALOG_KEY_VIRTUAL_AXES
// enable processing of DKS
#define DKS_ENABLE

// number of multiplexer channels (must be 8 or 16 or 32)
#define MATRIX_COLS 16
// number of ADC channels (whichever has more * 2)
/* any changes to this definition also requires modifying custom_analog.c */
#define MATRIX_ROWS 8
// number of direct pins (can span multiple ADCs) // MATRIX_DIRECT_RIGHT
/* any changes to this definition also requires modifying custom_analog.c */
#define MATRIX_DIRECT 4
// max number of multiplexers per ADC
/* any changes to this definition also requires modifying custom_analog.c */
#define MAX_MUXES_PER_ADC 1
// number of ADCs used on each side
#define N_ADCS_SCANNED       3
#define N_ADCS_SCANNED_RIGHT 3



// Channel select pins
#define MATRIX_COL_PINS { \
    A8,     B15,    B14,    B13,    \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN, \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN, \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN  \
}
/* same as left hand
#define MATRIX_COL_PINS_RIGHT { \
    A8,     B15,    B14,    B13,    \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN, \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN, \
    NO_PIN, NO_PIN, NO_PIN, NO_PIN  \
} */

// ADC pins
/* LEFT
row 0 = mux 1 = ADC4_IN3
row 1 = mux 2 = ADC1_IN4
row 2 = DIRECT_ROW
row 3 = DKS, left hand */
#define MATRIX_ROW_PINS { \
    B12,    \
    A3,     \
    NO_PIN, \
    NO_PIN  \
}
/* RIGHT
row 4 = mux 3 = ADC4_IN3
row 5 = mux 4 = ADC3_IN1
row 6 = mux 5 = ADC1_IN4
row 7 = DKS, right hand */
#define MATRIX_ROW_PINS_RIGHT { \
    B12,    \
    B1,     \
    A3,     \
    NO_PIN  \
}
// ADC pins for direct pins
/* LEFT
ADC2_IN3 = W
ADC2_IN4 = A
ADC2_IN2 = S
ADC2_IN1 = D */
#ifdef MATRIX_DIRECT
# define DIRECT_PINS { \
    A6,     \
    A7,     \
    A5,     \
    A4      \
}
#endif
/* RIGHT - not used
#ifdef MATRIX_DIRECT_RIGHT
# define DIRECT_PINS_RIGHT { \
    NO_PIN, \
    NO_PIN, \
    NO_PIN, \
    NO_PIN  \
}
#endif
*/


// bit array of whether key is valid
// DIRECTION IS FLIPPED, 1ST BIT IS THE LAST COLUMN
#define CUSTOM_MATRIX_MASK { \
    0b1111111111111111, \
    0b1111111111111111, \
    0b0000000000001111, \
    0b0000000000000000, \
    0b1111111111111111, \
    0b1111111111111111, \
    0b1111111111111111, \
    0b0000000000000000  \
}



// Definitions for virtual axes
#ifdef ANALOG_KEY_VIRTUAL_AXES
// the layer containing mouse keys
# define MOUSE_LAYER 3
// deadzone of around 0.5mm
# define MOUSE_DEADZONE 15
// qmk requires these
# define JOYSTICK_BUTTON_COUNT 0
# define JOYSTICK_AXIS_COUNT 4
# define JOYSTICK_AXIS_RESOLUTION 8
// rows and cols which make up the virtual axes
// order: left right up down, left right up down
# define JOYSTICK_COORDINATES { \
    {2, 1},     \
    {2, 3},     \
    {2, 0},     \
    {2, 2},     \
    {4, 9},     \
    {4, 15},    \
    {4, 7},     \
    {4, 8}      \
}
// order: left right up down, left right up down
# define MOUSE_COORDINATES { \
    {2, 1},     \
    {2, 3},     \
    {2, 0},     \
    {2, 2},     \
    {0, 0},     \
    {1, 9},     \
    {1, 10},    \
    {1, 8}      \
}
// order: left right up down, left right up down
# define MOUSE_COORDINATES_RIGHT { \
    {5, 10},    \
    {5, 13},    \
    {5, 11},    \
    {5, 12},    \
    {255, 255}, \
    {255, 255}, \
    {6, 15},    \
    {6, 14}     \

}
#endif



// Disable debounce
#define DEBOUNCE 0

// Set ADC resolution and sampling time
#define ADC_RESOLUTION      ADC_CFGR_RES_12BITS
#define ADC_SAMPLING_TIME   ADC_SMPR_SMP_2P5
#define ADC_RESOLUTION_MAX  1 << 12
// Max value of raw
#define ANALOG_RAW_MAX_VALUE 2047
// Max value of calibrated
#define ANALOG_CAL_MAX_VALUE 1023
// Max value of rest - value at around 2mm into keypress
#define ANALOG_MULTIPLIER_LUT_SIZE 512



// Set USART pins and driver
#define SERIAL_USART_FULL_DUPLEX
#define SERIAL_USART_DRIVER SD2
#define SERIAL_USART_TX_PIN B3
#define SERIAL_USART_RX_PIN B4
#define SERIAL_USART_TX_PAL_MODE 7
#define SERIAL_USART_RX_PAL_MODE 7



// Change emulated flash size
// https://docs.qmk.fm/drivers/eeprom#wear_leveling-efl-driver-configuration
#define WEAR_LEVELING_EFL_OMIT_LAST_SECTOR_COUNT 0
#define WEAR_LEVELING_LOGICAL_SIZE 4096
#define WEAR_LEVELING_BACKING_SIZE 8192
// Set size of EECONFIG for analog_config (per key)
#define EECONFIG_USER_DATA_SIZE (5 * MATRIX_ROWS * MATRIX_COLS)
// Set size of EECONFIG for calibration (global)
#define EECONFIG_KB_DATA_SIZE (36 * 3)



// Custom sync
#ifdef SPLIT_KEYBOARD
// Master to slave:
# define RPC_M2S_BUFFER_SIZE 16
// Slave to master:
# define RPC_S2M_BUFFER_SIZE 16
// Keyboard level data sync:
# define SPLIT_TRANSACTION_IDS_KB KEYBOARD_SYNC_A
# define SPLIT_TRANSACTION_IDS_USER USER_SYNC_A
#endif



// RGB config
#ifdef RGB_MATRIX_ENABLE
// Set to open-drain, with external pullup to 5v
# define WS2812_EXTERNAL_PULLUP
// Set custom enable pin
# define CUSTOM_RGB_ENABLE_PIN A9
// Set PWM driver, channel, mode
# define WS2812_PWM_DRIVER PWMD1
# define WS2812_PWM_CHANNEL 3
# define WS2812_PWM_PAL_MODE 6
// Set DMA stream, channel
# define WS2812_DMA_STREAM STM32_DMA1_STREAM5
# define WS2812_PWM_DMA_CHANNEL 5
#endif