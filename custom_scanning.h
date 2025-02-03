#include <stdint.h>

void multiplexer_init(void);

bool select_multiplexer_channel(uint8_t channel);

bool actuation(struct key_settings *key, uint8_t current, uint8_t max_key_displacement);