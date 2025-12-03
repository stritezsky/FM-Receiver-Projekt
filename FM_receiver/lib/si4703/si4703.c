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
    DDRC |= (1 << PC4);    // Set PC4 (SDA) as Output
    PORTC &= ~(1 << PC4);  // Set PC4 (SDA) LOW

    // 2. RESET SEQUENCE
    DDRD |= (1 << PD2);    // Set RESET pin as Output
    PORTD &= ~(1 << PD2);  // Set RESET LOW
    _delay_ms(1);          
    
    PORTD |= (1 << PD2);   // Set RESET HIGH (Chip wakes up now)
    _delay_ms(1);

    // 3. Release SDIO (SDA) so TWI hardware can take over
    DDRC &= ~(1 << PC4);   // Set PC4 back to Input 
    
    // 4. Initialize TWI Hardware
    twi_init();

    // 5. Enable Oscillator
    read_registers();
    regs[0x07] = 0x8100; 
    write_registers();
    _delay_ms(500);

    // 6. Basic Configuration
    read_registers();
    regs[POWERCFG] = 0x4001; // Enable IC
    regs[SYSCONFIG1] |= (1 << 12); // RDS
    regs[SYSCONFIG2] |= (1 << 4);  // 100 kHz spacing
    regs[SYSCONFIG2] = (regs[SYSCONFIG2] & 0xFFF0) | 0x0001; // Min Volume
    
    // 7. NOISE REDUCTION SETTINGS (Seek Thresholds)
    // Set SKSNR (SNR Threshold) to 4 (Bits 7:4 in SYSCONFIG3)
    // Set SKCNT (FM Impulse Detection) to 0 (Bits 3:0 in SYSCONFIG3)
    regs[SYSCONFIG3] &= 0xFF00; 
    regs[SYSCONFIG3] |= (9 << 4); // SNR = 4 (range 0-15)
    
    // Set SEEKTH (RSSI Threshold) to 25 (Bits 15:8 in SYSCONFIG2)
    regs[SYSCONFIG2] &= 0x00FF;
    regs[SYSCONFIG2] |= (20 << 8); // RSSI = 25 (range 0-127)

    write_registers();
    _delay_ms(110);
}

void si4703_set_seek_threshold(uint8_t snr, uint8_t rssi) {
    read_registers();
    
    // Set SEEKTH (RSSI Threshold) in SYSCONFIG2 (Bits 15:8)
    regs[SYSCONFIG2] &= 0x00FF; // Clear upper byte
    regs[SYSCONFIG2] |= ((uint16_t)rssi << 8);

    // Set SKSNR (SNR Threshold) in SYSCONFIG3 (Bits 7:4)
    // SKCNT (Bits 3:0) is kept as 0
    regs[SYSCONFIG3] &= 0xFF00; // Clear lower byte
    regs[SYSCONFIG3] |= ((uint16_t)snr << 4);
    
    write_registers();
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
  uint8_t completed_mask = 0; 

  for (uint8_t i = 0; i < 8; i++)
    buffer[i] = '-';
  buffer[8] = '\0';

  while (t < timeout_ms) {
    read_registers();
    if (regs[STATUSRSSI] & (1 << RDSR)) {
      uint16_t b = regs[RDSB];
      uint8_t idx = b & 0x03;
      
      if ((b & 0xF000) == 0) { 
          buffer[idx * 2] = (regs[RDSD] >> 8) & 0xFF;
          buffer[idx * 2 + 1] = regs[RDSD] & 0xFF;
          completed_mask |= (1 << idx);
      }
    }
    
    if (completed_mask == 15) break;

    _delay_ms(40);
    t += 40;
  }
}