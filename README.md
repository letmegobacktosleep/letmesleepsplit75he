I need somebody to fork VIAL and add menus which can modify hall effect settings, as well as any other communication protocols required to be able to change the values in `analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS]` & `calibration_parameters_t calibration_parameters`

Vague instructions on how to configure this code for a new keyboard
- set up new row, col & direct pins in config.h
- re-configure custom_analog.c (initADCGroups & adcStartConversions & getADCSample)
- configure the correct bootmagic pins in keyboard.json
- reconfigure RGB in keyboard.json, and change indexes of joystick/mouse keys in the main source file (rgb_matrix_indicators_advanced_user)
- replace the layout in keyboard.json with your new keyboard
- replace the keymaps with your new keyboard