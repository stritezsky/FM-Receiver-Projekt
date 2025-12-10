/**
 * @file main.c
 * @defgroup main_app Main Application
 * @brief Main control logic for the FM Receiver.
 *
 * Contains the infinite loop, state management, and interaction 
 * between the Encoder, Buttons, Radio, and Display.
 *
 * @author Střítežský Jiří
 * @author Svoboda Patrik
 * @author Javůrek Kryštof
 * @{
 */
#include <avr/io.h>
#include <util/delay.h>
#include <si4703.h>
#include <twi.h>
#include <uart.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <oled.h>
#include <gpio.h>
#include <encoder.h>
#include "timer.h"

// --- PIN DEFINITIONS ---
/** @brief Pin definition for the Seek Up button (integrated in Encoder). */
#define BTN_UP   PD5  // Seek Up

/**
 * @brief Updates the OLED display with frequency and RDS data.
 * * This function clears the screen and redraws the current station
 * information, including the frequency formatted in MHz and any
 * received Radio Data System (RDS) text.
 *
 * @param freq The current frequency (e.g., 1055 for 105.5 MHz).
 * @param rds_text A pointer to the string containing the RDS message/station name.
 * @return none
 */
void update_display(uint16_t freq, char* rds_text) {
    char buf[10];
    oled_clrscr();
    
    // Line 1: RDS
    oled_gotoxy(0, 0); 
    oled_puts(rds_text);
    
    // Line 2: Frequency
    oled_gotoxy(0, 2); 
    itoa(freq / 10, buf, 10); 
    oled_puts(buf);
    oled_puts(".");
    itoa(freq % 10, buf, 10); 
    oled_puts(buf);
    oled_puts(" MHz");
    
    oled_display();
}
/**
 * @brief Main entry point of the program.
 * * Initializes all libraries (UART, I2C, Si4703, OLED, GPIO, Encoder)
 * and enters the main event loop. The loop monitors the encoder for
 * frequency changes and the button for seek operations. It also manages
 * the timing for reading RDS data to ensure the station is settled
 * before attempting a read.
 * * @return int Returns 0 upon termination (never reached in embedded context).
 */
int main(void) {
    // Initialize Libraries
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    twi_init();
    si4703_power_on();
    oled_init(OLED_DISP_ON);
    gpio_mode_input_pullup(&DDRD, BTN_UP);
    encoder_init();
    sei(); // Enable Interrupts

    // Variables
    uint16_t freq = 951; // Start at 87.5 MHz
    uint8_t vol = 5;
    char rds_buffer[9] = "No RDS  ";
    uint32_t last_tune_time = 0;
    uint8_t rds_update_needed = 0;
    oled_charMode(DOUBLESIZE);
    
    // Initial Display Update
    si4703_set_volume(vol);
    si4703_set_channel(freq);
    update_display(freq, rds_buffer);
    uart_puts("FM radio ready\r\n");
    
    while (1) {
        int8_t enc_diff = encoder_read();
        if (enc_diff != 0) {
            freq += enc_diff;
            if (freq > 1080) freq = 875;
            if (freq < 875) freq = 1080;

            si4703_set_channel(freq);
            
            // Clear RDS text immediately on tune
            
            update_display(freq, "Tuning.."); 

            // Reset the "Settle Timer"
            last_tune_time = millis_homebrew();
            rds_update_needed = 1; 
        }

        // Button (Seek)
        if (gpio_read(&PIND, BTN_UP) == 0) {
            _delay_ms(20); 
            if (gpio_read(&PIND, BTN_UP) == 0) {
                update_display(freq, "Seeking..");
                freq = si4703_seek_up();
                si4703_set_channel(freq);
                
                // Reset settle timer after seek too
                last_tune_time = millis_homebrew();
                rds_update_needed = 1;
                
                while(gpio_read(&PIND, BTN_UP) == 0); 
            }
        }

        if (rds_update_needed && (millis_homebrew() - last_tune_time > 2000)) {
            
            uart_puts("Station Settled. Reading RDS...\r\n");
            
            // Read RDS 
            si4703_read_rds(rds_buffer, 2000); 
            
            // Update screen with final data
            update_display(freq, rds_buffer);
            
            // Clear flag so we don't read again until next tune
            rds_update_needed = 0; 
        }
    }
    return 0;
}
/** @} */ // End of main_app group