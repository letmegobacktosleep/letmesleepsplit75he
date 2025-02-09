/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>
#include <math.h>
#include "util.h"

#include "config.h"
#include "custom_matrix.h"
#include "custom_calibration.h"

uint16_t distance_to_analog(uint8_t distance, lookup_table_t *lut_params) {
    double intermediate = lut_params->lut_a * exp(lut_params->lut_b * distance + lut_params->lut_c) + lut_params->lut_d;
    uint16_t adc = (uint16_t) MAX(0, MIN(intermediate, lut_params->max_input));
    return adc;
}

uint8_t analog_to_distance(uint16_t adc, lookup_table_t *lut_params) {
    double intermediate = (log((adc - lut_params->lut_d) / lut_params->lut_a) - lut_params->lut_c) / lut_params->lut_b;
    uint8_t distance = (uint8_t) MAX(0, MIN(intermediate, lut_params->max_output));
    return distance;
}

uint16_t rest_to_absolute_change(uint16_t adc, lookup_table_t *lut_params) {
    double intermediate = lut_params->lut_a * exp(lut_params->lut_b * distance + lut_params->lut_c) + lut_params->lut_d;
    uint16_t adc = (uint16_t) MAX(0, MIN(intermediate, lut_params->max_input));
    return adc;
}

uint16_t scale_raw_value(uint16_t raw, uint8_t rest, uint16_t *lut_multiplier){

    // Find the multiplier
    double intermediate = (double) ANALOG_CAL_MAX_VALUE / lut_multiplier[rest];
    
    // Comment this out when lut_multiplier values have been added
    // intermediate = 1.1983185203883775;

    // Multiply by difference between raw and rest
    intermediate *= MAX(0, raw - rest);

    // Cast to an unsigned integer and limit value
    uint16_t calibrated = (uint16_t) MIN(raw * multiplier, ANALOG_CAL_MAX_VALUE);
    
    return calibrated;
}