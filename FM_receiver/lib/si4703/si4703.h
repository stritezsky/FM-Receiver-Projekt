/**
 * @file si4703.h
 * @defgroup si4703_driver Si4703 Radio Driver <si4703.h>
 * @code #include <si4703.h> @endcode
 *
 * @brief Driver library for the Si4703 FM Radio Tuner.
 *
 * This library handles I2C communication with the Si4703 chip,
 * allowing for frequency setting, volume control, seeking, and RDS reading.
 *
 * @author Simon Monk
 * @{
 */
#ifndef SI4703_H
#define SI4703_H

#include <avr/io.h>
#include <util/delay.h>
#include "twi.h"

/** @brief I2C address of the Si4703 module. */
#define SI4703_ADDR  0x10


/**
 * @name Si4703 Registers
 * @brief Register definitions for the Si4703 chip.
 * @{
 */
#define POWERCFG     0x02
#define CHANNEL      0x03
#define SYSCONFIG1   0x04
#define SYSCONFIG2   0x05
#define SYSCONFIG3   0x06 // Defined SYSCONFIG3
#define STATUSRSSI   0x0A
#define READCHAN     0x0B
#define RDSB         0x0D
#define RDSD         0x0F
/** @} */

/**
 * @name Register Bits
 * @brief Important bit masks for control and status registers.
 * @{
 */
#define TUNE         15
#define STC          14
#define RDSR         15
#define SEEKUP        9
#define SEEK          8
#define SKMODE       10
#define SFBL         13
/** @} */

/**
 * @brief Initialize the Si4703 chip.
 * * Performs the hardware reset sequence and configures initial registers
 * (oscillator, stereo mode, volume, etc.).
 */
void si4703_init(void);

/**
 * @brief Turn on the radio (wrapper for init).
 */
void si4703_power_on(void);

/**
 * @brief Tune to a specific frequency.
 * * @param freq Frequency in 100kHz steps (e.g., 1055 for 105.5 MHz).
 */
void si4703_set_channel(uint16_t freq);
/**
 * @brief Seek the next valid station upwards.
 * * @return uint16_t The frequency of the station found.
 */
uint16_t si4703_seek_up(void);
/**
 * @brief Seek the next valid station downwards.
 * * @return uint16_t The frequency of the station found.
 */
uint16_t si4703_seek_down(void);

/**
 * @brief Set the volume level.
 * * @param vol Volume level (0 to 15).
 */
void si4703_set_volume(uint8_t vol);

/**
 * @brief Get the currently tuned frequency.
 * * @return uint16_t Frequency in 100kHz steps.
 */
uint16_t si4703_get_channel(void);

/**
 * @brief Read RDS (Radio Data System) data.
 * * Attempts to read the Station Name (PS) from the RDS data stream.
 * This function blocks for up to `timeout_ms` while waiting for valid data.
 * * @param buffer Pointer to a char array (min size 9) to store the station name.
 * @param timeout_ms Maximum time to wait for RDS data in milliseconds.
 */
void si4703_read_rds(char *buffer, uint16_t timeout_ms);


/**
 * @brief Configure seek sensitivity thresholds.
 * * @param snr Signal-to-Noise Ratio threshold (0-15).
 * @param rssi Received Signal Strength Indicator threshold (0-127).
 */
void si4703_set_seek_threshold(uint8_t snr, uint8_t rssi); 
/** @} */ // End of si4703_driver group
#endif