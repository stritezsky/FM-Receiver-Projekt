#ifndef SI4703_H
#define SI4703_H

#include <avr/io.h>
#include <util/delay.h>
#include "twi.h"

// I2C adresa modulu
#define SI4703_ADDR  0x10

// registry čipu
#define POWERCFG     0x02
#define CHANNEL      0x03
#define SYSCONFIG1   0x04
#define SYSCONFIG2   0x05
#define STATUSRSSI   0x0A
#define READCHAN     0x0B
#define RDSB         0x0D
#define RDSD         0x0F

// bity
#define TUNE         15
#define STC          14
#define RDSR         15
#define SEEKUP        9
#define SEEK          8
#define SKMODE       10
#define SFBL         13

void si4703_init(void);
void si4703_power_on(void);
void si4703_set_channel(uint16_t freq);
uint16_t si4703_seek_up(void);
uint16_t si4703_seek_down(void);
void si4703_set_volume(uint8_t vol);
uint16_t si4703_get_channel(void);
void si4703_read_rds(char *buffer, uint16_t timeout_ms);

#endif