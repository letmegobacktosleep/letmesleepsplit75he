/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>
#include <math.h>
#include "util.h"

#include "config.h"
#include "custom_matrix.h"
#include "custom_calibration.h"

uint8_t analog_to_distance(uint16_t input, lookup_table_t *lut_params) {

    double intermediate = (log((input - lut_params->lut_d) / lut_params->lut_a) - lut_params->lut_c) / lut_params->lut_b;

    return (uint8_t) MAX(0, MIN(intermediate, lut_params->max_output));
}

uint16_t distance_to_analog(uint8_t input, lookup_table_t *lut_params) {

    double intermediate = lut_params->lut_a * exp(lut_params->lut_b * input + lut_params->lut_c) + lut_params->lut_d;

    return (uint16_t) MAX(0, MIN(intermediate, lut_params->max_input));
}

uint16_t rest_to_absolute_change(uint16_t input, lookup_table_t *lut_params) {

    double intermediate = lut_params->lut_a * exp(lut_params->lut_b * input + lut_params->lut_c) + lut_params->lut_d;

    return (uint16_t) MAX(0, MIN(intermediate, lut_params->max_output));
}

uint16_t scale_raw_value(uint16_t raw, uint8_t rest, uint16_t *lut_multiplier){

    // Limit to be less than ANALOG_CAL_MAX_VALUE
    return (uint16_t) MIN(ANALOG_CAL_MAX_VALUE, 
        // Scale raw value between 0 and ANALOG_CAL_MAX_VALUE (cast to uint32_t to prevent overflows)
        (uint32_t) ((raw < rest) ? 0 : (raw - rest)) * ANALOG_CAL_MAX_VALUE / lut_multiplier[rest]
    );
}

// analog filter variables
static uint8_t counter = 0;
static uint16_t buffer[ROWS_PER_HAND][MATRIX_COLS][SMA_FILTER_SIZE] = { 0 };

uint16_t sma_filter_set(uint16_t value, uint8_t row, uint8_t col){

    buffer[row][col][counter] = value;

    return sma_filter_get(row, col);

}

uint16_t sma_filter_get(uint8_t row, uint8_t col){

    uint32_t sum = 0;

    for (uint8_t i = 0; i < SMA_FILTER_SIZE; i++){
        sum += buffer[row][col][i];
    }

    return (uint16_t) MIN(ANALOG_CAL_MAX_VALUE, (sum / SMA_FILTER_SIZE));
}

void sma_filter_pointer(void){
    
    // increments by one
    counter = (counter + 1) % SMA_FILTER_SIZE;

}