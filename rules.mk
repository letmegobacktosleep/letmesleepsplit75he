SRC += custom_matrix.c custom_analog.c custom_calibration.c custom_scanning.c eeconfig_set_defaults.c dummy_pointing_device.c

MCU_LDSCRIPT = STM32F303xB_tinyuf2
WEAR_LEVELING_DRIVER = embedded_flash

# DEBOUNCE_TYPE = sym_defer_g
# if debounce is required, uncomment this and
# set debounce to 5 milliseconds in config.h

CUSTOM_MATRIX = lite
# custom lite matrix
# keyboard.json method doesn't seem to work

JOYSTICK_ENABLE = yes
# enable joystick

POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = custom
# enable pointing device