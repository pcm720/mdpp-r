/*
Console control functions
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

#include "control.h"
#include <util/atomic.h>

void console_set_region(uint8_t region, uint8_t reset) {
    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        if (reset && !(PINB_RESET & (1 << PB7))) { // Assert WRES if reset is required and WRES is not pulled by user
            #ifdef RESET_ACTIVE_LOW
                DDRB |= (1 << PB7);
            #else
                PORTB |= (1 << PB7);
                DDRB |= (1 << PB7);
            #endif
            PORTB |= (1 << PB2);   // Turn on PAD_DBG_LED
        }
        if (region == NXT) {       // If next region is selected
            switch (PORTC & 0x3) { // Read current region from PORTC by clearing everything except PC0 and PC1
                case USA:          // Set region variable to next region
                    region = EUR;
                    break;
                case JAP:
                    region = USA;
                    break;
                case EUR:
                    region = JAP;
                    break;
                default:
                    region = DEFAULT_REGION;
                    break;
            }
        }
        PORTC = (PORTC & ~(0x3)) | region; // Set region by reading current value of PORTC, clearing PC0 and PC1 and adding region variable
        PORTB = (PORTB & ~(0x3)) | region; // Same, but with LEDs
        if (reset) {                       
            _delay_ms(350);                // Release WRES after 350 ms
            PORTB &= ~(1 << PB2);          // Turn off PAD_DBG_LED
            #ifdef RESET_ACTIVE_LOW
                DDRB &= ~(1 << PB7);
            #else
                DDRB &= ~(1 << PB7);
                PORTB &= ~(1 << PB7);
            #endif
        }
    }
}

void console_reset() {
    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        #ifdef RESET_ACTIVE_LOW
            DDRB |= (1 << PB7);   // Pull WRES down
        #else
            PORTB |= (1 << PB7);  // Pull WRES up
            DDRB |= (1 << PB7);   // Change direction to output
        #endif
        PORTB |= (1 << PB2);      // Turn on PAD_DBG_LED
        _delay_ms(350);
        PORTB &= ~(1 << PB2);     // Turn off PAD_DBG_LED
        #ifdef RESET_ACTIVE_LOW
            DDRB &= ~(1 << PB7);  // Release WRES
        #else
            DDRB &= ~(1 << PB7);  // Change direction to input
            PORTB &= ~(1 << PB7); // Release WRES
        #endif
    }
}