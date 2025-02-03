#include <custom_scanning.h>

void multiplexer_init(void){
    multiplexer_number_of_bits = 0;
    for (uint8_t i = 0; i < MATRIX_COLS; i++){
        if (col_pins[i] != NO_PIN){
            setPinOutput(pin)
            multiplexer_number_of_bits += 1;
        }
    }
}

bool select_multiplexer_channel(uint8_t channel){
    if (channel > MATRIX_COLS){
        return 0;
    }
    for (uint8_t i = 0; i < multiplexer_number_of_bits; i++){
        writePin(col_pins[i], channel & (1 << i));
    }
    return 1
}

bool actuation(struct analog_config_t *config, struct analog_key_t *key, uint8_t current, uint8_t max_key_displacement){
    switch (key->mode){
        case 0: // normal, not pressed
            if (current > config->lower){
                key->mode = 1;
                return 1;
            }
            else {
                return 0;
            }

        case 1: // normal, pressed
            if (current < config->upper){
                key->mode = 0;
                return 0;
            }
            else {
                return 1;
            }

        case 2: // rapid trigger, at top
            if (current > config->lower){
                key->mode = 3;
                return 1;
            }
            else {
                return 0;
            }

        case 3: // rapid trigger, pressed
            if (current < config->upper){ // top deadzone
                key->mode = 2;
                return 0;
            }
            else if (current > key->old){ // update lowest position
                key->old = current;
                return 1;
            }
            else if (current < key->old - config->up){ // rapid untrigger
                key->old = current;
                key->mode = 4;
                return 0;
            }
            else {
                return 1;
            }

        case 4: // rapid trigger, not pressed
            if (current < config->upper){ // top deadzone
                key->mode = 2;
                return 0;
            }
            else if (current < key->old){ // update highest position
                key->old = current;
            }
            else if ((current > key->old + config->down) || (current > max_key_displacement - config->upper)){ // rapid trigger or bottom deadzone
                key->old = current;
                key->mode = 3;
                return 1;
            }
            else {
                return 0;
            }

        default: // invalid mode
        return 0;
    }
}