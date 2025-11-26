#include <avr/io.h>
#include <util/delay.h>
#include <si4703.h>
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <uart.h>    
#include <stdlib.h>
#include <avr/interrupt.h>        // Peter Fleury's UART library
#include <oled.h>

int main(void) {
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    twi_init();
    si4703_power_on();
    uint16_t freq = 0;
    uint8_t vol = 0;
    char rds[9];
    sei();
    char uart_msg[8];
    oled_init(OLED_DISP_ON);
    

    oled_charMode(DOUBLESIZE);

    uart_puts("FM radio ready\r\n");
    
    while (1) {
        unsigned int uart_data = uart_getc();

        if ((uart_data & 0xFF00) == 0) {
            
            char c = (char)(uart_data & 0x00FF);

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

            uart_puts("Freq: ");
            itoa(freq, uart_msg, 10); 
            uart_puts(uart_msg);
            uart_puts(" MHz | Vol: ");
            itoa(vol, uart_msg, 10); 
            uart_puts(uart_msg);
            uart_puts("\r\n");
            oled_clrscr();
            oled_puts(uart_msg);
            oled_display();
        }
    }
}