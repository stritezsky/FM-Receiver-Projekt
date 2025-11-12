/*
 * The I2C (TWI) bus scanner tests all addresses and detects devices
 * that are connected to the SDA and SCL signals.
 * (c) 2023-2025 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and Atmel AVR platform.
 * Tested on Arduino Uno board and ATmega328P, 16 MHz.
 */

// -- Includes ---------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <uart.h>           // Peter Fleury's UART library
#include <stdio.h>          // C library. Needed for `sprintf`
#include "timer.h"
#include <oled.h>

#define RTC_ADR 0x68
volatile uint8_t flag_update_uart = 0;
volatile uint8_t rtc_values[2];

// -- Function definitions ---------------------------------
/*
 * Function: Main function where the program execution begins
 * Purpose:  Call function to test all I2C (TWI) combinations and send
 *           detected devices to UART.
 * Returns:  none
 */
int main(void)
{
    char uart_msg[10];
    oled_init(OLED_DISP_ON);
    oled_clrscr();

    oled_charMode(DOUBLESIZE);
    oled_puts("JIRKA DISP.");

    oled_charMode(NORMALSIZE);

    // oled_gotoxy(x, y)
    oled_gotoxy(0, 2);
    oled_puts("gg");

    // oled_drawLine(x1, y1, x2, y2, color)
    oled_drawLine(0, 25, 120, 25, WHITE);

    oled_drawLine(60, 25, 0, 50, WHITE);
    
    oled_drawLine(60, 25, 120, 50, WHITE);
    
    oled_drawLine(0, 50, 120, 50, WHITE);
    // Copy buffer to display RAM
    oled_display();
    twi_init();
    twi_start();
    twi_write((RTC_ADR<<1)|(TWI_WRITE));
    twi_write(0x00);
    twi_write(0x00);
    twi_write(0x35);
    twi_stop();
    // Initialize USART to asynchronous, 8-N-1, 115200 Bd
    // NOTE: Add `monitor_speed = 115200` to `platformio.ini`
    uart_init(UART_BAUD_SELECT(115200, F_CPU));

    sei();  // Needed for UART

    uart_puts("Scanning I2C... ");
    for (uint8_t sla = 8; sla < 120; sla++)
    {
        if (twi_test_address(sla) == 0)  // If ACK from Slave
        {
            sprintf(uart_msg, "\r\n0x%x", sla);
            
            uart_puts(uart_msg);
        }
    }
    uart_puts("\r\nDone\r\n");


    tim1_ovf_1sec();
    tim1_ovf_enable();
    // Infinite empty loop
    while (1)
    {
      if (flag_update_uart == 1)
        {
            // Display temperature

            sprintf(uart_msg, "Time: %x:%x \r", rtc_values[1], rtc_values[0]);
            uart_puts(uart_msg);
             oled_gotoxy(0, 4);
             oled_puts(uart_msg);
             oled_display();
            // Do not print it again and wait for the new data
            flag_update_uart = 0;
        }
    }

    // Will never reach this
    return 0;
}

ISR(TIMER1_OVF_vect)
{
    twi_readfrom_mem_into(RTC_ADR, 0, rtc_values, 2);
    flag_update_uart = 1;
}