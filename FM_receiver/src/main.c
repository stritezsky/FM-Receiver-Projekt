#include <avr/io.h>
#include <util/delay.h>
#include <si4703.h>
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <uart.h>    
#include <stdlib.h>
#include <avr/interrupt.h>        // Peter Fleury's UART library
#include <oled.h>
#include <gpio.h> 


#define pCLK PD3 //odpovídá pinům 345
#define pDT PD4
#define BTN PD5

int main(void) {
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    twi_init();
    si4703_power_on();
    uint16_t freq = 875;  //test zmenil jsem na 875, otestovat
    uint8_t vol = 9;
    char rds[9];
    sei();
    char uart_msg[8];
    oled_init(OLED_DISP_ON);

    //gpio_mode_output(&DDRB, LED_BUILTIN);
    uint8_t stateB = gpio_read(&PIND, pCLK); //before
    uint8_t stateCLK = 0; //CLK
    uint8_t posENK = 0; //position enkoder
    //uint8_t stateB = 0; //before

    oled_charMode(DOUBLESIZE);

    uart_puts("FM radio ready\r\n");
    
    while (1) {

        if (gpio_read(&PIND, BTN) == 0)
        {
            _delay_ms(150); // Delay in milliseconds
            freq = si4703_seek_up();
        }
        stateCLK = gpio_read(&PIND, pCLK);
        if (stateCLK != stateB) {
            _delay_ms(50); //test
            // pokud stav pinu DT neodpovídá stavu pinu CLK,
            // byl pin CLK změněn jako první a rotace byla
            // po směru hodin, tedy vpravo
        if (gpio_read(&PIND, pDT) != stateCLK) {
          _delay_ms(50); //test
          // vytištění zprávy o směru rotace a přičtení
          // hodnoty 1 u počítadla pozice enkodéru
          //Serial.print("Rotace vpravo => | ");
          //poziceEnkod ++;
          freq = freq + 1; //mozna + 10?
          si4703_set_channel(freq);
        }
        else {
            _delay_ms(50); //test
            // vytištění zprávy o směru rotace a odečtení
            // hodnoty 1 u počítadla pozice enkodéru
            //Serial.print("Rotace vlevo  <= | ");
            //poziceEnkod--;
            freq = freq - 1; //mozna + 10?
            si4703_set_channel(freq);
        }
        // vytištění aktuální hodnoty pozice enkodéru
        //Serial.print("Pozice enkoderu: ");
        //Serial.println(poziceEnkod);
        }
        stateB = stateCLK;


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
                oled_puts(rds);
                oled_display();
            }

            uart_puts("Freq: ");
            itoa(freq, uart_msg, 10); 
            uart_puts(uart_msg);
            uart_puts(" MHz | Vol: ");
            itoa(vol, uart_msg, 10); 
            uart_puts(uart_msg);
            uart_puts("\r\n");
            
        }
    }
}
