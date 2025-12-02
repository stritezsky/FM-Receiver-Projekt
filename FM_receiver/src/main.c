#include <avr/io.h>
#include <util/delay.h>
#include <si4703.h>
#include <twi.h>
#include <uart.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <oled.h>
#include <gpio.h>

// --- PIN DEFINITIONS ---
#define pCLK     PD3
#define pDT      PD4
#define BTN_UP   PD5  // Seek Up
#define BTN_DOWN PD6  // Seek Down (New Pin)

// Helper function to handle the OLED layout
void update_display(uint16_t freq, uint8_t vol, char* rds_text) {
    char buf[10];

    // Clear buffer to prevent overlapping text
    oled_clrscr();

    // --- Line 1: RDS Text ---
    oled_gotoxy(0, 0); // Top left
    oled_puts(rds_text);

    // --- Line 2: Frequency & Volume ---
    oled_gotoxy(0, 2); 

    // Frequency (e.g., "105.5")
    itoa(freq / 10, buf, 10); // Integer part (105)
    oled_puts(buf);
    oled_puts(".");
    itoa(freq % 10, buf, 10); // Decimal part (5)
    oled_puts(buf);
    oled_puts("MHz");

    // Volume (e.g., " V:15")
    oled_puts(" V:");
    itoa(vol, buf, 10);
    oled_puts(buf);

    // Push data to display
    oled_display();
}

int main(void) {
    // 1. Initialize Libraries
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    twi_init();
    si4703_power_on();
    oled_init(OLED_DISP_ON);
    
    // 2. Configure Inputs (Enable Internal Pull-ups)
    gpio_mode_input_pullup(&DDRD, pCLK);
    gpio_mode_input_pullup(&DDRD, pDT);
    gpio_mode_input_pullup(&DDRD, BTN_UP);
    gpio_mode_input_pullup(&DDRD, BTN_DOWN); // Enable pull-up for new button

    sei(); // Enable Interrupts

    // 3. Variables
    uint16_t freq = 875; // Start at 87.5 MHz
    uint8_t vol = 9;
    char rds[9] = "No RDS  ";
    char uart_msg[10];

    // Encoder State
    uint8_t stateB = gpio_read(&PIND, pCLK);
    uint8_t stateCLK = 0;

    oled_charMode(DOUBLESIZE);
    
    // Initial Display Update
    si4703_set_volume(vol);
    si4703_set_channel(freq);
    update_display(freq, vol, rds);
    uart_puts("FM radio ready\r\n");
    
    while (1) {
        uint8_t changed = 0; // Flag to check if we need to update display

        // --- BUTTON UP: Seek Up ---
        if (gpio_read(&PIND, BTN_UP) == 0) {
            _delay_ms(20); // Debounce
            if (gpio_read(&PIND, BTN_UP) == 0) {
                uart_puts("Seeking Up...\r\n");
                oled_clrscr();
                oled_puts("Seeking Up..");
                oled_display();
                
                freq = si4703_seek_up();
                changed = 1;
                
                while(gpio_read(&PIND, BTN_UP) == 0); // Wait for release
            }
        }

        // --- BUTTON DOWN: Seek Down ---
        if (gpio_read(&PIND, BTN_DOWN) == 0) {
            _delay_ms(20); // Debounce
            if (gpio_read(&PIND, BTN_DOWN) == 0) {
                uart_puts("Seeking Down..\r\n");
                oled_clrscr();
                oled_puts("Seeking Dn..");
                oled_display();
                
                freq = si4703_seek_down();
                changed = 1;
                
                while(gpio_read(&PIND, BTN_DOWN) == 0); // Wait for release
            }
        }

        // --- ENCODER: Manual Tune ---
        stateCLK = gpio_read(&PIND, pCLK);
        if (stateCLK != stateB) {
            _delay_ms(2); 
            if (gpio_read(&PIND, pCLK) == stateCLK) {
                if (gpio_read(&PIND, pDT) != stateCLK) {
                    freq++; // Right/CW
                } else {
                    freq--; // Left/CCW
                }
                
                if (freq > 1080) freq = 1080;
                if (freq < 875) freq = 875;

                si4703_set_channel(freq);
                changed = 1;
            }
        }
        stateB = stateCLK;

        // --- UART Commands ---
        unsigned int uart_data = uart_getc();
        if ((uart_data & 0xFF00) == 0) {
            char c = (char)(uart_data & 0x00FF);

            if (c == 'n') {
                freq = si4703_seek_up();
                changed = 1;
            } else if (c == 'd') {
                freq = si4703_seek_down();
                changed = 1;
            } else if (c == '+') {
                if (vol < 15) vol++;
                si4703_set_volume(vol);
                changed = 1;
            } else if (c == '-') {
                if (vol > 0) vol--;
                si4703_set_volume(vol);
                changed = 1;
            } else if (c == 'r') {
                uart_puts("Reading RDS...\r\n");
                oled_clrscr();
                oled_puts("Reading RDS...");
                oled_display();
                
                si4703_read_rds(rds, 15000);
                
                uart_puts("RDS: ");
                uart_puts(rds);
                uart_puts("\r\n");
                changed = 1;
            }
        }

        // --- Update Screen if Changed ---
        if (changed) {
            update_display(freq, vol, rds);
            
            // Also update UART
            uart_puts("Freq: ");
            itoa(freq, uart_msg, 10);
            uart_puts(uart_msg);
            uart_puts(" | Vol: ");
            itoa(vol, uart_msg, 10);
            uart_puts(uart_msg);
            uart_puts("\r\n");
        }
    }
    return 0;
}