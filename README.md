# RaspberryPi baremetal OS/firmware
- Everything is in kernel mode
- Linux-like scheduler
- Kernel heap, page allocation
- More to come

## With help from the following resources
- [Scheduler/MMU/ASM](https://github.com/s-matyukevich/raspberry-pi-os)
- [Display/mboxes](https://github.com/bztsrc/raspi3-tutorial)
- [Font/glyphs](https://github.com/isometimes/rpi4-osdev)

## To build
- aarch64 gcc toolchain of some sort (I used the x86 crosscompiler)
  - Ex `gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc`
- QEMU for emulation
