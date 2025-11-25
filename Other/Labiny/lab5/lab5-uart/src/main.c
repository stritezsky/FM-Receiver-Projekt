/*
 * Use USART unit and transmit data between ATmega328P and computer.
 * (c) 2018-2025 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and Atmel AVR platform.
 * Tested on Arduino Uno board and ATmega328P, 16 MHz.
 */

// -- Includes ---------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions


// -- Function definitions ---------------------------------
/*
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and transmit UART data.
 * Returns:  none
 */
int main(void)
{
   // Initialize USART to asynchronous, 8-N-1, 9600 Bd
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    tim1_ovf_1sec();
    tim2_ovf_16ms();

    //tim1_ovf_enable();
    // Configure 16-bit Timer/Counter1 to transmit UART data
    // Set prescaler to 1 sec and enable overflow interrupt

    // WRITE YOUR CODE HERE

    // Interrupts must be enabled, bacause of `uart_puts()`
    sei();

    // Put strings to ringbuffer for transmitting via UART
    //uart_puts("\r\n");  // New line only
    //uart_puts("Start \tusing Serial monitor...\r\n");
    
    uint16_t value;
    char uart_msg[8];

     uart_puts("\r\n");  // New line only
   
    while (1)
    {
        // Get received data from UART
        value = uart_getc();
        if ((value & 0xff00) == 0)  // If successfully received data from UART
        {
            if (value == '1')
            {
                itoa(TCNT1, uart_msg, 16);
                uart_puts("Timer1: 0x");
                uart_puts(uart_msg);
                uart_puts("\r\n");
            }
            else if (value == '0')
            {
                itoa(TCNT0, uart_msg, 16);
                uart_puts("Timer0: 0x");
                uart_puts(uart_msg);
                uart_puts("\r\n");
            }
            else
            {

            // Transmit the received character back via UART
            uart_putc(value);

            // Transmit the ASCII code also in hex, dec, and bin
            itoa(value, uart_msg, 16);

            // WRITE YOUR CODE HERE
            
            // New line
            uart_puts("\x1b[0m"); 
            uart_puts(":");
            uart_puts("\x1b[1;32m");  
            uart_puts("\t0x");  // 1: bold style; 32: green foreground
            uart_puts(uart_msg);
            

           itoa(value, uart_msg, 10);

           uart_puts("\t");
           uart_puts("\x1b[4;31m"); 
            uart_puts(uart_msg);
           
            
            itoa(value, uart_msg, 2);
            uart_puts("\x1b[4;33m"); 
           uart_puts("\t0b");
            uart_puts(uart_msg);
            uart_puts("\x1b[0m"); 
            uart_puts("\r\n");
            


            }
        }
    }
    return 0;
}


// -- Interrupt service routines ---------------------------
/*
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Transmit UART data.
 */
ISR(TIMER1_OVF_vect)
{
    
       // uart_puts("Paris\t ");
}
