/* 
 * Stopwatch on Liquid Crystal Display (LCD).
 * (c) 2017-2025 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and Atmel AVR platform.
 * Tested on Arduino Uno board and ATmega328P, 16 MHz.
 */

// -- Includes ---------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <lcd.h>            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for number conversions


// -- Global variables -------------------------------------
volatile uint8_t flag_update_lcd = 0;

// Stopwatch values
// Declaration of "stopwatch" variable with structure "Stopwatch_structure"
struct Stopwatch_structure
{
    uint8_t tenths;  // Tenths of a second
    uint8_t secs;    // Seconds
    uint8_t mins;    // Minutes
} stopwatch;


// -- Function definitions ---------------------------------
/*
 * Function: Main function where the program execution begins
 * Purpose:  Update stopwatch value on LCD screen when 8-bit Timer/Counter2
 *           overflows.
 * Returns:  none
 */
int main(void)
{
    char lcd_msg[5];
    char lcd_msg2[5];  // String for converted numbers by itoa()

    // Initialize display
    lcd_init(LCD_DISP_ON_CURSOR_BLINK);
    //lcd_init(LCD_DISP_ON);
    //lcd_init(LCD_DISP_ON_CURSOR );
    // Put string(s) on LCD screen
   

    // Set backlight
    gpio_mode_output(&DDRB,PB2);
    gpio_write_high(&PORTB,PB2);

    // Custom character(s)
    // WRITE YOUR CODE HERE
   
    
   

    lcd_home();
    lcd_puts(" 00:00.0");
    // Configuration of 8-bit Timer/Counter2 for Stopwatch update
    // Set the overflow prescaler to 16 ms and enable interrupt
    // WRITE YOUR CODE HERE
    tim2_ovf_16ms();
    tim2_ovf_enable();

    // Enable global interrupts
    sei();

    // Infinite loop
    while (1)
    {
        if (flag_update_lcd == 1)
        {
            // Display "00:00.tenths"
            itoa(stopwatch.tenths, lcd_msg, 10);
            itoa(stopwatch.secs, lcd_msg2, 10);
            if (stopwatch.secs < 10)
            {lcd_gotoxy(5, 0);
            lcd_puts(lcd_msg2);}
            else {lcd_gotoxy(4, 0);
            lcd_puts(lcd_msg2);}
            lcd_gotoxy(7, 0);
            lcd_puts(lcd_msg);

            flag_update_lcd = 0;
        }
    }

    // Will never reach this
    return 0;
}


// -- Interrupt service routines ---------------------------
/*
 * Function: Timer/Counter2 overflow interrupt
 * Purpose:  Update the stopwatch on LCD screen every sixth overflow,
 *           ie approximately every 100 ms (6 x 16 ms = 100 ms).
 */
ISR(TIMER2_OVF_vect)
{
    
    static uint8_t n_ovfs = 0;
    static uint8_t n_ovfs2 = 0;
    static uint8_t n_blocks = 0;
    n_ovfs++;
    n_ovfs2++;
    
    if (n_ovfs2 >= 60)
    {
        n_ovfs2 = 0;
        stopwatch.secs++;
        // Count tenth of seconds 0, 1, ..., 9, 0, 1, ...
        // WRITE YOUR CODE HERE
          if(stopwatch.secs > 60)
            stopwatch.secs = 0;

        flag_update_lcd = 1;
    }

    // Do this every 6 x 16 ms = 100 ms
    if (n_ovfs >= 6)
    {
        n_ovfs = 0;
        stopwatch.tenths++;
        // Count tenth of seconds 0, 1, ..., 9, 0, 1, ...
        // WRITE YOUR CODE HERE
          if(stopwatch.tenths > 9)
            stopwatch.tenths = 0;
        n_blocks++;
        if (n_blocks> 9) {
            n_blocks = 0;
            lcd_gotoxy(0, 1);
            lcd_puts("          ");
        }

        flag_update_lcd = 1;
    }
        uint8_t new_char1[8] = {0x0,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f};
        lcd_custom_char(0, new_char1);
        lcd_gotoxy(n_blocks, 1);
        lcd_putc(0);

    // Else do nothing and exit the ISR
}