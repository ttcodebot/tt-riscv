# SPDX-License-Identifier: Apache-2.0
# Test program for the KianV RV32E bare-metal SoC. Toolchain: Debian/Ubuntu gcc-riscv64-unknown-elf.
#   make                     # 25 MHz, 115200 baud -> kianv-rv32e-hello-25000000.bin (flash at offset 1 MiB)
#   make FREQ=10000000       # FREQ must match the clock the SoC runs at
CROSS ?= riscv64-unknown-elf-
FREQ  ?= 25000000
BAUD  ?= 115200
NAME  ?= kianv-rv32e-hello-$(FREQ)
CFLAGS = -march=rv32e -mabi=ilp32e -Os -ffreestanding -fno-pic -fno-builtin -nostdlib -Wall \
         -DFREQ=$(FREQ) -DBAUD=$(BAUD) $(EXTRA_CFLAGS)

all: $(NAME).bin
$(NAME).elf: crt0.S stub.S main.c kianv.ld
	$(CROSS)gcc $(CFLAGS) -T kianv.ld -Wl,--no-relax -o $@ crt0.S stub.S main.c -lgcc
$(NAME).bin: $(NAME).elf
	$(CROSS)objcopy -O binary $< $@
	$(CROSS)size $<
clean:
	rm -f *.elf *.bin
