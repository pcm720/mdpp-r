/*
Main definitions file for MegaDrive++ R
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

#ifndef COMMON_H_
#define COMMON_H_

#include <avr/io.h>
#include <util/delay.h>

// Undefine for Mega Drives with active high reset (VA5 and lower (?))
#define RESET_ACTIVE_LOW

// Amount of (seconds - 1) to monitor the combo for before switching the region or resetting the console
#define LENGTH_COMBO 1
// Amount of (seconds - 1) to monitor reset button for
#define LENGTH_RESET 2

// Pad button status map: S A C B R L D U, button is 0 if pressed
/*
#define COMBO_RESET       0x0F // Start + A + B + C          / ~(11110000)
// Saves current region to EEPROM
#define COMBO_SAVE        0x0B // Left + Start + A + B + C   / ~(11110100)
#define COMBO_USA         0x3B // Left + Start + A           / ~(11000100)
#define COMBO_JAP         0x5B // Left + Start + C           / ~(10100100)
#define COMBO_EUR         0x6B // Left + Start + B           / ~(10010100)
#define COMBO_USA_RESET   0x37 // Right + Start + A          / ~(11001000)
#define COMBO_JAP_RESET   0x57 // Right + Start + C          / ~(10101000)
#define COMBO_EUR_RESET   0x67 // Right + Start + B          / ~(10011000)
#define COMBO_OFF         0x00 // To use when no combo is currently active */
#define COMBO_RESET       0x8F // A + B + C  / ~(01110000)
#define COMBO_SAVE        0x8B // Left + Start + A + B + C   / ~(01110100)
#define COMBO_USA         0xBB // Left + A   / ~(01000100)
#define COMBO_JAP         0xDB // Left + C   / ~(00100100)
#define COMBO_EUR         0xEB // Left + B   / ~(00010100)
#define COMBO_USA_RESET   0xB7 // Right + A  / ~(01001000)
#define COMBO_JAP_RESET   0xD7 // Right + C  / ~(00101000)
#define COMBO_EUR_RESET   0xE7 // Right + B  / ~(00011000)
#define COMBO_OFF         0x00

// Pins:    LANGUAGE    VIDEOMODE
// USA:     HIGH        HIGH
// EUR:     HIGH        LOW
// JAP:     LOW         HIGH
#define JAP 0x1 // 01
#define EUR 0x2 // 10
#define USA 0x3 // 11
#define NXT 0x4 // Next region
#define DEFAULT_REGION USA // Default region to use if EEPROM read fails or if this is first boot

// End of configurable area

#ifdef RESET_ACTIVE_LOW
    #define PINB_RESET ~PINB
#else
    #define PINB_RESET PINB
#endif

#endif

/* 
 * Pin assignments:
 * PORT B:
 * PORTB Map: 76XXX210
 * NTSC_LED — 0 (OUT)
 * JAP_LED — 1 (OUT)
 * PAD_DBG_LED — 2 (OUT)
 * MD_RESET_IN — 6 (IN) [UNUSED]
 * MD_RESET_OUT — 7 (IN/OUT)
 * 
 * PORT C:
 * PORTC Map: XXXXXX10
 * NTSC — 0 (OUT)
 * JAP — 1 (OUT)
 * 
 * PORT D:
 * PORTD Map: X6543210 / All inputs
 * MD_PAD_UP — 0 (Pad Pin 1 / UP)
 * MD_PAD_DOWN — 1 (Pad Pin 2 / DOWN)
 * MD_PAD_LEFT — 2 (Pad Pin 3 / LEFT)
 * MD_PAD_RIGHT — 3 (Pad Pin 4 / RIGHT)
 * MD_PAD_TL — 4 (Pad Pin 6 / A/B)
 * MD_PAD_TR — 5 (Pad Pin 9 / Start/C)
 * MD_PAD_TH — 6 (Pad Pin 7 / SELECT)
 * 
 * Some technical notes on Mega Drive gamepads:
 *
 * 3-button pad:
 * State   Select(TH)   Up    Down    Left    Right    TL    TR
 * 0       HIGH         Up    Down    Left    Right    B     C
 * 1       LOW          Up    Down    LOW     LOW      A     Start
 * 
 * 6-button pad:
 * State   Select(TH)   Up    Down    Left    Right    TL    TR
 * 0       HIGH         Up    Down    Left    Right    B     C
 * 1       LOW          Up    Down    LOW     LOW      A     Start
 * 2       HIGH         Up    Down    Left    Right    B     C
 * 3       LOW          Up    Down    LOW     LOW      A     Start
 * 4       HIGH         Up    Down    Left    Right    B     C
 * 5       LOW          LOW   LOW     LOW     LOW      A     Start
 * 6       HIGH         Z     X       Y       Mode     B     C
 * 7       LOW          -     -       -       -        A     Start
 * 
 * Notes:
 * 1. Select idles in HIGH state in official games
 *    Unlicensed stuff like Everdrive and chinese multicarts actually read pads incorrectly, leaving Select pulled down after reading.
 *    We need to consider every option.
 * 2. For 3-button mode, Select pulse length is 6-6.5 us;
 *    For 6-button mode, Select period (Tsel) is non-uniform and depends on the game.
 *      For Street Fighter 2, select cycle looks like this:
 *          Select is active high
 *          first select pluse (to LOW state) is 25 us
 *          second (HIGH) is 4.7-5 us
 *          third (LOW) is 8-9 us
 *          fourth (HIGH) is 4.7-5 us
 *          fifth (LOW) is 9-11 us
 *          sixth (HIGH) is 4.7-5 us
 *          seventh (LOW) is 25 us
 *      For Streets of Rage 3:  
 *          Select is active high
 *          first select pluse (to LOW state) is 10 us
 *          second (HIGH) is 6.2 us
 *          third (LOW) is 7.3 us
 *          fourth (HIGH) is 6.25 us
 *          fifth (LOW) is 7.3 us
 *          sixth (HIGH) is 6.25 us
 *          seventh (LOW) is 7 us
 * 3. 6-button Select cycle length is around 50-100 us (50 for Streets of Rage 3, 84 for Street Fighter 2)
 * 4. Pad internal reset (Tres) is 2 ms (according to SEGA specifications on 6-button pad)
 * 5. Some 3-button pad-only games pulse Select line multiple times in 2 ms interval, triggering 6-button pad to output more than two states.
 *    For example, Ms. Pac-Man pulses Select line thrice in 100 us interval. This behaviour is considered incorrect.
 *    We can skip these pulses by delaying for more than 100 us after the first level change.
 * 6. It takes ~200-600 ns for 6-button pad to update its output after Select level change
 * 7. Frame period is 16.67 ms for NTSC and 20 ms for PAL
 * 8. Since we don't really need to read all buttons of 6-button pad, we can pretend that it doesn't exist
 */
