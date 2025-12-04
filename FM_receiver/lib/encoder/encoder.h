#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

/**
 * @brief Initialize the rotary encoder and Timer0 for polling.
 */
void encoder_init(void);

/**
 * @brief Read the number of steps the encoder has rotated since the last check.
 * * @return int8_t  Positive value for CW, negative for CCW, 0 for no movement.
 */
int8_t encoder_read(void);

uint32_t millis_homebrew(void);

#endif