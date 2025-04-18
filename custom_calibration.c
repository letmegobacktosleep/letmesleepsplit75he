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
        (uint32_t) MAX(0, raw - rest) * ANALOG_CAL_MAX_VALUE / lut_multiplier[rest]
    );
}