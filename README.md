# ALi SDK
Running custom programs on ALi CPUs found in Set-Top Boxes
## Building

1. Create toolchain from .config file (crosstool-NG)
2. `mkdir build && cd build`
3. `cmake .. && make`

## Flashing
Direct SPI Flash:

`flashrom -p ch341a_spi -w build/ali_sdk_padded.bin`

## Features
### M3801
- [X] UART
- [X] Working newlib
## TODO
- [ ] ISR
- [ ] UART on interrupt
- [ ] GPIO Driver
- [ ] Bootloader
