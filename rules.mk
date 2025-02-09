SRC += custom_matrix.c custom_analog.c custom_calibration.c custom_scanning.c eeconfig_set_defaults.c

MCU_LDSCRIPT = STM32F303xB
WEAR_LEVELING_DRIVER = embedded_flash

DEBOUNCE_TYPE = sym_eager_pk
# instant change followed by 5 ms of no change
# to prevent people from triggering way too fast

JOYSTICK_ENABLE = yes
# enable joystick

# POINTING_DEVICE_ENABLE = yes
# enable pointing device