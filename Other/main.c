#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "si4703.h"

int main(void) {
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    si4703_power_on();

    uint16_t freq = 0;
    uint8_t vol = 0;
    char rds[9];
    uart_puts("FM radio ready\r\n");

    while (1) {
        if (uart_available()) {
            char c = uart_getc();

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
            uart_putint(freq);
            uart_puts(" MHz | Vol: ");
            uart_putint(vol);
            uart_puts("\r\n");
        }
    }
}
