
#include "si4703.h"

static uint16_t regs[16];

static void read_registers(void) {
  twi_start();
  twi_write((SI4703_ADDR << 1) | 1);
  for (uint8_t i = 0; i < 32; i += 2) {
    uint8_t high = twi_read(TWI_ACK);
    uint8_t low = (i < 30) ? twi_read(TWI_ACK) : twi_read(TWI_NACK);
    regs[(0x0A + i / 2) & 0x0F] = (high << 8) | low;
  }
  twi_stop();
}

static void write_registers(void) {
  twi_start();
  twi_write((SI4703_ADDR << 1) | 0);
  for (uint8_t i = 0x02; i <= 0x07; i++) {
    twi_write(regs[i] >> 8);
    twi_write(regs[i] & 0xFF);
  }
  twi_stop();
}

void si4703_init(void) {
    // 1. DEFINE PINS
    // SDIO (SDA) is on PC4 (Analog 4 on Uno)
    // RESET is on PD2 (Digital 2 on Uno)

    // 2. SET SDIO (SDA) LOW to select 2-Wire (I2C) mode
    DDRC |= (1 << PC4);    // Set PC4 (SDA) as Output
    PORTC &= ~(1 << PC4);  // Set PC4 (SDA) LOW

    // 3. RESET SEQUENCE
    DDRD |= (1 << PD2);    // Set RESET pin as Output
    PORTD &= ~(1 << PD2);  // Set RESET LOW
    _delay_ms(1);          
    
    PORTD |= (1 << PD2);   // Set RESET HIGH (Chip wakes up now)
    _delay_ms(1);

    // 4. Release SDIO (SDA) so TWI hardware can take over
    // Setting it to input allows the TWI library to control it
    DDRC &= ~(1 << PC4);   // Set PC4 back to Input 
    
    // 5. Initialize TWI Hardware
    twi_init();

    // The rest of your initialization code remains the same...
    read_registers();
    regs[0x07] = 0x8100; // Enable Oscillator
    write_registers();
    _delay_ms(500);

    read_registers();
    regs[POWERCFG] = 0x4001; // Enable IC
    regs[SYSCONFIG1] |= (1 << 12); // RDS
    regs[SYSCONFIG2] |= (1 << 4);  // 100 kHz spacing
    regs[SYSCONFIG2] = (regs[SYSCONFIG2] & 0xFFF0) | 0x0001; // Min Volume
    write_registers();
    _delay_ms(110);
}

void si4703_power_on(void) { si4703_init(); }

void si4703_set_channel(uint16_t freq) {
  uint16_t chan = (freq - 875);
  read_registers();
  regs[CHANNEL] = (chan & 0x03FF) | (1 << TUNE);
  write_registers();

  do {
    read_registers();
  } while (!(regs[STATUSRSSI] & (1 << STC)));

  regs[CHANNEL] &= ~(1 << TUNE);
  write_registers();

  do {
    read_registers();
  } while (regs[STATUSRSSI] & (1 << STC));
}

uint16_t si4703_seek(uint8_t dir) {
  read_registers();
  regs[POWERCFG] |= (1 << SKMODE);
  if (dir)
    regs[POWERCFG] |= (1 << SEEKUP);
  else
    regs[POWERCFG] &= ~(1 << SEEKUP);
  regs[POWERCFG] |= (1 << SEEK);
  write_registers();

  do {
    read_registers();
  } while (!(regs[STATUSRSSI] & (1 << STC)));

  uint8_t failed = (regs[STATUSRSSI] & (1 << SFBL)) ? 1 : 0;
  regs[POWERCFG] &= ~(1 << SEEK);
  write_registers();

  do {
    read_registers();
  } while (regs[STATUSRSSI] & (1 << STC));

  if (failed)
    return 0;
  return si4703_get_channel();
}

uint16_t si4703_seek_up(void) { return si4703_seek(1); }
uint16_t si4703_seek_down(void) { return si4703_seek(0); }

void si4703_set_volume(uint8_t vol) {
  if (vol > 15)
    vol = 15;
  read_registers();
  regs[SYSCONFIG2] = (regs[SYSCONFIG2] & 0xFFF0) | vol;
  write_registers();
}

uint16_t si4703_get_channel(void) {
  read_registers();
  uint16_t chan = regs[READCHAN] & 0x03FF;
  return chan + 875;
}

void si4703_read_rds(char *buffer, uint16_t timeout_ms) {
  uint16_t t = 0;
  uint8_t completed_mask = 0; // Bitmask to track received blocks (0-3)

  for (uint8_t i = 0; i < 8; i++)
    buffer[i] = '-';
  buffer[8] = '\0';

  while (t < timeout_ms) {
    read_registers();
    if (regs[STATUSRSSI] & (1 << RDSR)) {
      uint16_t b = regs[RDSB];
      uint8_t idx = b & 0x03;
      
      // FIX 4: Only update buffer if no errors in block B (< 500 is a rough check from C++)
      if (b < 500) { 
          buffer[idx * 2] = (regs[RDSD] >> 8) & 0xFF;
          buffer[idx * 2 + 1] = regs[RDSD] & 0xFF;
          
          // Track which blocks we have received
          completed_mask |= (1 << idx);
      }
    }
    
    // FIX 5: Exit early if we have all 4 blocks (mask binary 1111 = 15)
    if (completed_mask == 15) break;

    _delay_ms(40);
    t += 40;
  }
}
