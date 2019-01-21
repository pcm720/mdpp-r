/*
MegaDrive++ R code for ATmega88PA
    Copyright (C) 2018-2019 pcm720 <pcm720@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see 
<http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include <avr/interrupt.h>
#include "eeprom.h"
#include "control.h"

void init();
uint8_t parse_buttons(uint8_t buttons, uint8_t * action_flag);
void reset_loop();

volatile uint8_t read_status;
volatile uint8_t button_status;
volatile uint8_t timer_ticks;

int main() {
    init();

    uint8_t active_combo = COMBO_OFF;
    uint8_t parsed_combo = COMBO_OFF;
    uint8_t action_flag = 0x0; // Bits 0-1 — region (as in console_set_region), bits 4-7 — 0xF if reset is required, 0x0 if not

    while(1) {
        // Handle RESET button presses
        if (PINB_RESET & (1 << PB7)) {                 // If RESET button is pressed
            PCICR &= ~(1 << PCIE2);                    // Disable PCINT2
            TCCR1B &= ~(1 << CS12);                    // Disable timer in case it was running
            timer_ticks = 0;                           // Clear ticks
            _delay_ms(20);                             // Debounce delay
            if (PINB_RESET & (1 << PB7)) reset_loop(); // If button is still pressed, enter reset loop
            PCICR |= (1 << PCIE2);                     // Re-enable PCINT2
        }

        // Handle pad inputs
        if (read_status == 0xFF) {                     // If we received valid input during PCINT
            parsed_combo = parse_buttons(button_status, &action_flag);
            if (parsed_combo != active_combo) {        // If combo changed
                active_combo = parsed_combo;
                TCCR1B &= ~(1 << CS12);                // Disable timer
                timer_ticks = 0;                       // Clear ticks
                if (parsed_combo != COMBO_OFF) {
                    TCNT1 = 0;                         // Reset TC1
                    TCCR1B |=  (1 << CS12);            // Enable timer
                }
            } else if (timer_ticks > LENGTH_COMBO) {   // If valid combo was held for LENGTH_COMBO seconds
                TCCR1B &= ~(1 << CS12);                // Disable timer
                timer_ticks = 0;                       // Clear ticks
                if (active_combo == COMBO_RESET) console_reset();                 // Reset the console
                else if (active_combo == COMBO_SAVE) eeprom_write(PORTC & 0x3);   // Save current region to EEPROM
                else console_set_region((action_flag & 0xF), (action_flag & 0xF0));       // Set region accordingly
                active_combo = COMBO_OFF;              // Reset combo state
            }
        }
    }
}

void init() {
    // Enable pull-ups on unused pins:
    PORTB = 0x78;             // 01111000
    PORTC = 0xFC;             // 11111100
    PORTD = 0x83;             // 10000011
    // Set directions:
    DDRB = 0x7;               // 00000111 — pins PB2-0 are outputs
    DDRC = 0x3;               // 00000011 — pins PC0 and PC1 are outputs
    DDRD = 0x0;               // Set PORTD to input
    PRR = 0xE7;               // Enable power reduction

    uint8_t temp_region;
    if (eeprom_read(&temp_region)) {  // Read region from EEPROM
        temp_region = DEFAULT_REGION; // If reading failed, revert to default parameters
        eeprom_write(temp_region);    // Try to save defaults
    }
    console_set_region(temp_region,0x0);

    PCMSK2 |= (1 << PCINT22); // Enable PCINT on PD6 (Select)

    // Setup timers for reset button and combo button
    TCCR1B |= (1 << WGM12);   // Set up T/C1 to generate an interrupt every 1 s
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 31249;            // 31249 ticks ≈ 1 s

    console_reset();

    _delay_ms(20);            // Wait for the console to stabilize

    PCICR |= (1 << PCIE2);    // Enable PCINT2
    sei();
}

uint8_t parse_buttons(uint8_t buttons, uint8_t* action_flag) {
    switch (buttons) {
        case COMBO_RESET:
            *action_flag = 0x0;
            return COMBO_RESET;
        case COMBO_SAVE:
            *action_flag = 0x0;
            return COMBO_SAVE;
        case COMBO_USA:
            *action_flag = USA;
            return COMBO_USA;
        case COMBO_JAP:
            *action_flag = JAP;
            return COMBO_JAP;
        case COMBO_EUR:
            *action_flag = EUR;
            return COMBO_EUR;
        case COMBO_USA_RESET:
            *action_flag = (0xF0 | USA);
            return COMBO_USA_RESET;
        case COMBO_JAP_RESET:
            *action_flag = (0xF0 | JAP);
            return COMBO_JAP_RESET;
        case COMBO_EUR_RESET:
            *action_flag = (0xF0 | EUR);
            return COMBO_EUR_RESET;
        default:
            *action_flag = 0x0;
            return COMBO_OFF;
    }
}

void reset_loop() {
    TCNT1 = 0;              // Reset TC1
    TCCR1B |=  (1 << CS12); // Set the clock source to Clk/256 (enable timer)
    while (1) {
        _delay_ms(100);     // Wait
        // Break if RESET button was released or timer has ticked more than LENGTH_RESET times
        if (!(PINB_RESET & (1 << PB7)) || (timer_ticks > LENGTH_RESET)) break; 
    }
    TCCR1B &= ~(1 << CS12); // Disable timer
    if (timer_ticks > LENGTH_RESET) console_set_region(NXT,0x1);
    timer_ticks = 0;
}

// Interrupt vectors

// Detected level change on SELECT line:
ISR(PCINT2_vect) {
    // State 0:       * / C B R L D U (apply mask 00111111/0x3F)
    // State 1:       * / S A * * D U (apply mask 00110000/0x30 and shift to the left two times)
    // buttons_status map: A S B C R L D U
    uint8_t input = PIND;                   // Read pins on PORTD
    uint8_t int_button_status;              // To skip accessing volatile variable everytime we write here.
    // int_button_status is unintialized because GCC will generate more push instructions in the beginning of interrupt vector if variable is initialized here
    // That will cause additional delays, and we don't need that.
    
    read_status = 0x0;
    if (input & (1 << PD6)) {               // If select is high (PD6), this is state 0
        int_button_status = (input & 0x3F); // Save buttons (state 0)
        read_status = 0x0F;                 // Write 0x0F to status variable to indicate that state 0 has been read
    } else {                                // State 1
        int_button_status = ((input & 0x30) << 2); // To "initialize variable" in this branch
        if (!(input & 0xC)) {               // Check that LEFT and RIGHT are LOW before setting read status
            read_status = 0xF0;             // Write 0xF0 to status variable to indicate that state 1 has been read
        }
    }
    _delay_ms(1);                           // Wait until six-button reads are over. It's okay to delay here since we won't be doing anything during the read routine anyway
    input = PIND;                           // Read the other state
    if ((input & (1 << PD6)) && (read_status != 0x0F)) {
        int_button_status |= (input & 0x3F);
        read_status |= 0x0F;
    } else if (read_status != 0xF0) {
        if (!(input & 0xC)) {
            int_button_status |= ((input & 0x30) << 2);
            read_status |= 0xF0;
        }
    }

    if (read_status != 0xFF) read_status = 0x0; // Clear read status: the read was unsuccessful
    else button_status = int_button_status;
}

// Increase tick variable on timer-generated interrupt:
ISR(TIMER1_COMPA_vect) {
    timer_ticks++;
}