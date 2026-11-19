/* Startup for the KianV RV32E bare-metal SoC (tt07 #588, ttihp0p2 #10, ttsky25b #838).
   Based on test/firmware/crt0.S in splinedrive/RISCV-KianV-BareMetalStyle (MIT).
   Reset vector: flash offset 1 MiB = 0x20100000 (src/defines_soc.vh RESET_ADDR). */
.section .text.start
.global _start
#define IO_BASE      0x10000000
#define REG_DIV      (IO_BASE + 0x0010)            /* UART divider: cycles per bit (low 16 bits) */
#define SPI_REG_DIV  (IO_BASE + 0x500000 + 0x0010) /* generic SPI clock divider */
_start:
    li   t0, REG_DIV
    li   t1, (FREQ / BAUD)
    sw   t1, 0(t0)
    li   t0, SPI_REG_DIV
    li   t1, (FREQ / 100000)
    sw   t1, 0(t0)
    la   sp, __stacktop
    la   a0, _sbss
    la   a1, _ebss
    bge  a0, a1, 2f
1:  sw   zero, 0(a0)
    addi a0, a0, 4
    blt  a0, a1, 1b
2:  la   a0, _sidata
    la   a1, _sdata
    la   a2, _edata
    bge  a1, a2, 4f
3:  lw   a3, 0(a0)
    sw   a3, 0(a1)
    addi a0, a0, 4
    addi a1, a1, 4
    blt  a1, a2, 3b
4:  call main
5:  j    5b
