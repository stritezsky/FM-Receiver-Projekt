#include "encoder.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "gpio.h"

// Define Encoder Pins (Must match your wiring in main.c)
#define ENC_PORT PIND
#define ENC_DDR  DDRD
#define ENC_CLK  PD3
#define ENC_DT   PD4

// Volatile variables are used because they are modified inside an ISR
static volatile int8_t encoder_diff = 0;
static volatile uint8_t last_clk_state = 0;
static volatile uint32_t timer_millis = 0; // Millisecond counter

void encoder_init(void) {
    // 1. Configure pins as input with pull-up resistors
    gpio_mode_input_pullup(&ENC_DDR, ENC_CLK);
    gpio_mode_input_pullup(&ENC_DDR, ENC_DT);

    // 2. Read initial state
    last_clk_state = gpio_read(&ENC_PORT, ENC_CLK);

    // 3. Configure Timer0 for ~1ms overflow interrupts
    //    Using the macro from your timer.h library
    tim0_1ms();
    tim0_ovf_enable();
}

int8_t encoder_read(void) {
    int8_t val;
    
    // Disable interrupts briefly to ensure atomic read/write of volatile variable
    cli();
    val = encoder_diff;
    encoder_diff = 0; // Reset the counter after reading
    sei();
    
    return val;
}

uint32_t millis_homebrew(void) {
    uint32_t ms;
    cli(); // Disable interrupts to read 32-bit value safely
    ms = timer_millis;
    sei();
    return ms;
}

// Timer0 Overflow Interrupt - Polls the encoder every 1ms
ISR(TIMER0_OVF_vect) {
    timer_millis++; // Increment millisecond counter
    // Read current state of CLK pin
    uint8_t current_clk_state = gpio_read(&ENC_PORT, ENC_CLK);
    // Check for change in CLK state (Edge detection)
    if (current_clk_state != last_clk_state) {
        
        // If we are on the falling edge (or rising, depending on preference),
        // check the DT pin to determine direction.
        // We check if current state is 1 to count only one step per detent.
        if (current_clk_state == 1) { 
            if (gpio_read(&ENC_PORT, ENC_DT) != current_clk_state) {
                encoder_diff++; // Clockwise
            } else {
                encoder_diff--; // Counter-Clockwise
            }
        }
    }
    // Update last state
    last_clk_state = current_clk_state;
}