/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// Function prototypes
void set_default_analog_config(void);
void set_default_analog_key(void);
void set_default_calibration_parameters(void);
void set_default_virtual_axes(void);

// EEPROM_KB_PARTIAL_UPDATE(static_config, displacement);
#if (EECONFIG_KB_DATA_SIZE) > 0
# define EEPROM_KB_PARTIAL_UPDATE(__struct, __field) eeprom_update_block(               \
    &(__struct.__field),                                                                \
    (void *)((void *)(EECONFIG_KB_DATABLOCK) + offsetof(typeof(__struct), __field)),    \
    sizeof(__struct.__field)                                                            \
)
# define EEPROM_KB_PARTIAL_READ(__struct, __field) eeprom_read_block(                   \
    &(__struct.__field),                                                                \
    (void *)((void *)(EECONFIG_KB_DATABLOCK) + offsetof(typeof(__struct), __field)),    \
    sizeof(__struct.__field)                                                            \
)
#endif
// EEPROM_USER_PARTIAL_UPDATE(analog_config, row, col);
#if (EECONFIG_USER_DATA_SIZE) > 0
# define EEPROM_USER_PARTIAL_UPDATE(__array, __row, __col) eeprom_update_block(                             \
    &(__array[__row][__col]),                                                                               \
    (void *)((void *)(EECONFIG_USER_DATABLOCK) + sizeof(__array[0][0]) * (__row * MATRIX_COLS + __col)),    \
    sizeof(__array)                                                                                         \
)
# define EEPROM_USER_PARTIAL_READ(__array, __row, __col) eeprom_read_block(                                 \
    &(__array[__row][__col]),                                                                               \
    (void *)((void *)(EECONFIG_USER_DATABLOCK) + sizeof(__array[0][0]) * (__row * MATRIX_COLS + __col)),    \
    sizeof(__array)                                                                                         \
)
#endif
// https://discord.com/channels/440868230475677696/440868230475677698/1334525203044106241
// KB version does not seem to be working