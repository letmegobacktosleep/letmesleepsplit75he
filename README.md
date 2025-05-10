# Vague instructions on how to configure this code for a new keyboard

1. (keyboard.json) keep or remove the split configuration
2. (keyboard.json) generate a new keyboard layout
3. (keyboard.json) set the correct bootmagic row/col
4. (keymap.c) generate a new keymap matching the keyboard layout
5. (config.h) set the MATRIX_COL_PINS - gpio which tells the multiplexer which channel to output
6. (config.h) set the MATRIX_ROW_PINS - adc pins which are connected to the output of the multiplexer
7. (config.h) set the DIRECT_PINS - adc pins connected directly to a key
8. (config.h) set the CUSTOM_MATRIX_MASK to match your keyboard wiring
    - for a split keyboard the last row on each side is used for DKS
    - otherwise, the last two rows are used for DKS
    - make sure all row/col combinations without a sensor connected are masked off (the corresponding bit is zero)
9. (config.h) set MATRIX_COLS, MATRIX_ROWS, MATRIX_DIRECT, MAX_MUXES_PER_ADC, N_ADCS_SCANNED, N_ADCS_SCANNED_RIGHT, and choose whether to enable ANALOG_KEY_VIRTUAL_AXES and DKS_ENABLE
    - if using ANALOG_KEY_VIRTUAL_AXES, make sure to also set the correct row/col for each axes in `JOYSTICK_COORDINATES`, `MOUSE_COORDINATES`, `MOUSE_COORDINATES_RIGHT`
10. (custom_analog.c) scroll down to `initADCGroups` and look for `adcStart`
    - make sure to start all the ADCs which are going to be used
11. (custom_analog.c) scroll down to `adcStartAllConversions` and look for `adcStartConversionI`
    - make sure it starts the correct `adcConversionGroup` to read the correct channel (e.g. ADC1_IN3 would be channel 3)
    - if you are using direct pins, use `switch (current_col)` and set it to read the channel corresponding to each direct pin's column
    - make sure each adc or "row" has it's own unique sample buffer, you will have to add/remove buffers from the `ADCManager` struct in `custom_analog.h`
12. (custom_analog.c) scroll down to `getADCSample` and look for `switch (current_row)`
    - modify each `case` to return the correct value from the buffer for each row
    - ensure rows with DKS return `ANALOG_RAW_MAX_VALUE`, which when processed becomes zero
13. (custom_analog.c) if you have more than one row connected to at least one adc, you must create your own conversion groups. i can't help you with that.

14. (keyboard.json) & (mcuconf.h) & (config.h) set up RGB - refer to QMK docs for this
15. (letmesleepsplit75he.c) if using ANALOG_KEY_VIRTUAL_AXES scroll down to `rgb_matrix_indicators_advanced_user` 
    - change the indexes to match the keys which are used for joystick and mouse

# Actuation modes (its kinda messed up)

normal = 0
- actuation point   (functions like normal mechanical switches)
- deadzone          (between press and release)

rapid trigger = 2
- actuation point   (first press only)
- down sensitivity  (how far down before it is pressed)
- up sensitivity    (how far up before it is released)
- deadzone          (top and bottom)

inverted = 5
- actuation point   (must go past this before it can trigger on upstroke)
- up sensitivity    (how far up before it is released)
- deadzone          (top)

inverted rt = 8
- down sensitivity  (how far down before it is released)
- up sensitivity    (how far up before it is pressed)
- deadzone          (top and bottom)

DKS 1 = 10
DKS 2 = 11
DKS 3 = 12
DKS 4 = 13
DKS 5 = 14
DKS 6 = 15
DKS 7 = 16
DKS 8 = 17