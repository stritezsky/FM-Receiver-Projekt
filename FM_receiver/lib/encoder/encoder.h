/**
 * @file encoder.h
 * @defgroup encoder_driver Rotary Encoder Driver <encoder.h>
 * @code #include <encoder.h> @endcode
 *
 * @brief Library for handling rotary encoder inputs.
 *
 * Uses Timer interrupts to poll the encoder state for robust debouncing.
 *
 * @author Střítežský Jiří
 * @{
 */
#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

/**
 * @brief Initialize the rotary encoder and Timer0.
 * * Configures the encoder pins as inputs with pull-up resistors and
 * starts Timer0 to poll the encoder state approximately every 1ms.
 */
void encoder_init(void);

/**
 * @brief Read the accumulated encoder steps.
 * * Returns the net movement since the last call. 
 * This function disables interrupts briefly to ensure atomic reading.
 * * @return int8_t Positive value for Clockwise rotation, 
 * Negative value for Counter-Clockwise rotation, 
 * 0 for no movement.
 */
int8_t encoder_read(void);

/**
 * @brief Custom millis() function.
 * * Returns the number of milliseconds passed since the encoder_init() 
 * was called. Relies on the same Timer0 interrupt as the encoder.
 * * @return uint32_t Milliseconds elapsed.
 */
uint32_t millis_homebrew(void);
/** @} */ // End of encoder_driver group
#endif