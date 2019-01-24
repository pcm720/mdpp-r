## MegaDrive++ R

**MegaDrive++ R** is a modchip for the **Sega Mega Drive/Genesis** heavily inspired by [MegaDrive++ project](https://github.com/SukkoPera/MegaDrivePlusPlus/).
It uses cheap ATmega88PA microcontroller, [cheap PCB](https://oshpark.com/shared_projects/NskgzQ7B) and supports following features:
- **Region switching**: allows user to switch regions with a button combination on Player 1 controller or Reset button.
  - Hold **Left + Start + A** for two seconds to change region to USA, **Left + Start + B** to change region to Europe, and **Left + Start + C** to change region to Japan.
  - Hold **Right** instead of **Left** to change region **and** reset the console.
  - Hold **Left + Start + A + B + C** to save current region to memory and use it as default every time you turn on your console.
  - Hold the **Reset** button for more than four seconds to cycle through modes. This will reset the console.
  - Button combos can be customized in common.h.
- **In-game Reset**: hold **Start + A + B + C** to reset the console.
- Dual-frequency oscillator support: PCB has a special pin for DFO control.
- Unlike [MegaDrive++](https://github.com/SukkoPera/MegaDrivePlusPlus/), it doesn't support reset level autosensing, so to use it on boards with active-high reset signal you'll need to comment out a line in common.h. Note that this is untested: I don't own any consoles with active-high reset.
- Suggested PCB layout is optimized for Model 2 VA1 board.

Visit [wiki](https://github.com/pcm720/mdpp-r/wiki) for installation diagram for Model 2 VA1 board.
