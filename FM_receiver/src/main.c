#include <avr/io.h>
#include <util/delay.h>
#include <si4703.h>
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <uart.h>    
#include <avr/interrupt.h>        // Peter Fleury's UART library
int main(void) {
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    twi_init();
    si4703_power_on();
    //twi_init();
    //twi_start();
    uint16_t freq = 0;
    uint8_t vol = 0;
    char rds[9];
    sei();
    char uart_msg[8];
    uart_puts("FM radio ready\r\n");
    
    while (1) {
        // 1. Get the 16-bit value from the library
        unsigned int uart_data = uart_getc();

        // 2. Check if the data is valid
        // The upper byte is 0 if data is valid. It is non-zero if error or no data.
        if ((uart_data & 0xFF00) == 0) {
            
            // 3. Safe to cast to char now
            char c = (char)(uart_data & 0x00FF);

            // 4. Process commands
            if (c == 'n') {
                freq = si4703_seek_up();
            } else if (c == 'd') {
                freq = si4703_seek_down();
            } else if (c == '+') {
                if (vol < 15) vol++;
                si4703_set_volume(vol);
            } else if (c == '-') {
                if (vol > 0) vol--;
                si4703_set_volume(vol);
            } else if (c == 'a') {
                freq = 910;
                si4703_set_channel(freq);
            } else if (c == 'b') {
                freq = 1055;
                si4703_set_channel(freq);
            } else if (c == 'r') {
                uart_puts("Ctu RDS...\r\n");
                si4703_read_rds(rds, 15000);
                uart_puts("RDS: ");
                uart_puts(rds);
                uart_puts("\r\n");
            }

            // 5. Update status ONLY when a command was processed
            uart_puts("Freq: ");
            itoa(freq, uart_msg, 10); // Use base 10 for readability (not 8)
            uart_puts(uart_msg);
            
            uart_puts(" MHz | Vol: ");
            itoa(vol, uart_msg, 10); // Use base 10
            uart_puts(uart_msg);
            uart_puts("\r\n");
        }
    }
