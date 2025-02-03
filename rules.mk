SRC += matrix.c custom_analog.c custom_calibration.c custom_scanning.c eeconfig_set_defaults.c

MCU_LDSCRIPT = STM32F303xB
WEAR_LEVELING_DRIVER = embedded_flash

CUSTOM_MATRIX = lite
# https://docs.qmk.fm/reference_info_json#matrix
# OH MY FUCKING GOD YOU CAN SET LITE MATRIX IN KEYBOARD.JSON OMG

DEBOUNCE_TYPE = sym_eager_pk
# instant change followed by 5 ms of no change (5ms is the default)
# to prevent people from triggering way too fast

DEFERRED_EXEC_ENABLE = yes
# enable deferred exec for recalibration

JOYSTICK_ENABLE = yes
# enable joystick

POINTING_DEVICE_ENABLE = yes
# enable pointing device