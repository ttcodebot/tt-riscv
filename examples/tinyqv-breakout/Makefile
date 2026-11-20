# Breakout for TinyQV "Asteroids" (ttsky25a #495), built with Mike Bell's tinyQV toolchain and SDK.
# scripts/build-tinyqv-breakout.sh fetches both (pinned in pins.env) and runs this Makefile.
# Options: make EXTRA_CFLAGS="-DAUDIO_UIO7" (sound on uio[7] only, see hal.h), -DSHOW_LOAD (CPU load bar),
#          -DTEXTURE=n (brick textures, see breakout.c), -DSEED=n (attract-mode random seed).
RISCV_TOOLCHAIN ?= /opt/tinyQV
TINYQV_SDK ?= ../../build/tinyQV-sdk

CC = $(RISCV_TOOLCHAIN)/bin/riscv32-unknown-elf-gcc
LD = $(RISCV_TOOLCHAIN)/bin/riscv32-unknown-elf-ld
OBJCOPY = $(RISCV_TOOLCHAIN)/bin/riscv32-unknown-elf-objcopy
CFLAGS = -O2 -I$(TINYQV_SDK) -march=rv32ec_zicsr_zcb_zicond_zilsd -mabi=ilp32e -mno-strict-align -nostdlib \
         -nostartfiles -ffreestanding -ffunction-sections -fdata-sections -Wall -Werror $(EXTRA_CFLAGS)

all: breakout.bin

breakout.o: breakout.c hal.h
	$(CC) $(CFLAGS) -c $< -o $@

breakout.elf: breakout.o
	$(LD) $^ $(TINYQV_SDK)/start.o $(TINYQV_SDK)/tinyQV.a $(TINYQV_SDK)/tinyQV-asteroids.a \
	  $(RISCV_TOOLCHAIN)/riscv32-unknown-elf/lib/libc.a $(RISCV_TOOLCHAIN)/lib/gcc/riscv32-unknown-elf/*/libgcc.a \
	  -T $(TINYQV_SDK)/memmap --gc-sections -o $@

breakout.bin: breakout.elf
	$(OBJCOPY) $< -O binary $@

clean:
	rm -f *.o *.elf *.bin

.PHONY: all clean
