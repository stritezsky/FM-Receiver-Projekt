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
#define BTN_UP   PD5  // Seek Up

// function to handle the OLED layout
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
    uint16_t freq = 875; // Start at 87.5 MHz
    uint8_t vol = 9;
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
            if (freq > 1080) freq = 1080;
            if (freq < 875) freq = 875;

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

        if (rds_update_needed && (millis_homebrew() - last_tune_time > 1000)) {
            
            uart_puts("Station Settled. Reading RDS...\r\n");
            
            // Read RDS 
            si4703_read_rds(rds_buffer, 1500); 
            
            // Update screen with final data
            update_display(freq, rds_buffer);
            
            // Clear flag so we don't read again until next tune
            rds_update_needed = 0; 
        }
    }
    return 0;
}