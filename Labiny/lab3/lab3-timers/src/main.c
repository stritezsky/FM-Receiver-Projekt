// -- Includes -----------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#define LED PB5

int main(void)
{
    // set led pin to output
    gpio_mode_output(&DDRB,LED);
    gpio_write_low(&PORTB,LED);
    gpio_mode_output(&DDRB,PB0);
    gpio_write_high(&PORTB,PB0);
    // set overflow time to approx 4 times per sec
    tim1_ovf_1sec();
    tim1_ovf_enable();
    tim0_16ms();
    tim0_ovf_enable();
    //tim0_ovf_enable();
    // Enable overflow interrupt
    
    // Enables interrupts by setting the global interrupt mask
    sei();
    //forever loop
    while(1) {}
    return 0;
}

// Interrupt service routines
ISR(TIMER1_OVF_vect)
{
 gpio_toggle(&PORTB,LED); 
}
//ISR(TIMER0_OVF_vect)
//{
// gpio_toggle(&PORTB,PB0); 
//}

ISR(TIMER0_OVF_vect)
{
    static uint8_t n_ovfs = 0;
    
    n_ovfs++;
    if (n_ovfs >= 6)
    {
        // Do this every 6 x 16 ms = 100 ms
        n_ovfs = 0;
         gpio_toggle(&PORTB,PB0); 
    }
    // Else do nothing and exit the ISR
}