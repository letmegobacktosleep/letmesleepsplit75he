I need somebody to fork VIAL and add menus which can modify hall effect settings, as well as any other communication protocols required to be able to change the values in `analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS]` & `calibration_parameters_t calibration_parameters`

Vague instructions on how to configure this code for a new keyboard
- set up new row, col & direct pins in config.h
- re-configure custom_analog.c (adcCompleteCallback & initADCGroups & adcStartConversions & getADCSample)
- configure the correct bootmagic pins in keyboard.json
- reconfigure RGB in keyboard.json, and change indexes of joystick/mouse keys in the main source file (rgb_matrix_indicators_advanced_user)
- replace the layout in keyboard.json with your new keyboard
- replace the keymaps with your new keyboard

Actuation modes (its kinda messed up)

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