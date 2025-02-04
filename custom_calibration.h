#include <stdint.h>
#include <matrix.h>

uint16_t distance_to_analog(uint8_t distance, lookup_table_t *lut_params);

uint8_t analog_to_distance(uint16_t adc, lookup_table_t *lut_params);

uint8_t rest_to_absolute_change(uint16_t adc, lookup_table_t *lut_params);

uint16_t scale_raw_value(uint16_t raw, uint8_t rest, lookup_table_t *multiplier);