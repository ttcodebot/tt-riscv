/* KianV RV32E bare metal: boot from flash at 0x20100000 (flash offset 1 MiB).
   Code + data are copied by crt0 to the top 8 KiB of PSRAM (0x807FE000), as in the upstream
   test/firmware/kianv.ld. PSRAM is 8 MiB at 0x80000000 (src/defines_soc.vh). */
MEMORY {
  FLASH (rx)  : ORIGIN = 0x20100000, LENGTH = 0x00F00000
  RAM   (rwx) : ORIGIN = 0x807FE000, LENGTH = 0x00002000
}
ENTRY(_start)
__stacktop = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
  .start : { KEEP(*(.text.start)) . = ALIGN(4); } > FLASH
  _sidata = LOADADDR(.data);
  .data : {
    . = ALIGN(4); _sdata = .;
    *(.text .text.*) *(.rodata .rodata.* .srodata .srodata.*) *(.data .data.* .sdata .sdata.*)
    . = ALIGN(4); _edata = .;
  } > RAM AT > FLASH
  .bss (NOLOAD) : { . = ALIGN(4); _sbss = .; *(.bss .bss.* .sbss .sbss.* COMMON) . = ALIGN(4); _ebss = .; } > RAM
  /DISCARD/ : { *(.comment) *(.riscv.attributes) *(.eh_frame*) }
}
