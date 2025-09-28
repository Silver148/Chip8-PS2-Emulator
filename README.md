# This is an Chip 8 Emulator for the PlayStation 2 :D

## Is very simple, but functional :)

# Input

| **Chip 8 Keypad** | **Buttons Dualshock 2**   |
|------------------|---------------------------|
| 0                | START                     |
| 1                | L1                        |
| 2                | R1                        |
| 3                | L2                        |
| 4                | TRIANGLE                  |
| 5                | UP                        |
| 6                | SQUARE                    |
| 7                | LEFT                      |
| 8                | DOWN                      |
| 9                | RIGHT                     |
| A                | SELECT                    |
| B                | L3                        |
| C                | R2                        |
| D                | CIRCLE                    |
| E                | CROSS                     |
| F                | R3                        |

## Pressing L1 + R1 can pause the emulation. Also by pressing L1 + R1 + L2 + R2 + SELECT + START (At the same time) you can exit the game to play another one :D

By the way, the emulator is divided into two executables: the GUI and the core. The GUI is the ELF file called "Chip8-Emulator-PS2.ELF," while the core is called "Chip8-CORE.ELF." The core file should be in a folder called "core," which should be in the same folder as the GUI ELF file. (The emulator folder, where the core and GUI files are located, should be called "Chip8-Emulator-PS2.")

One limitation of the program is that the emulator can only run from a flash drive (mass0:); otherwise, it won't work because it won't be able to find the core. I might improve this soon :)

