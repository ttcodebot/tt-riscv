/* Wait-until-cycle stub, position independent. It is copied to 0x80000000 + (n << 16) so that
   while the CPU executes it, PC[18:16] = n, which the SoC drives on uo_out[7:5] (soc.v: led = PC[16+:3]).
   a0 = cycle counter value to wait for (CYCLE_CNT at 0x10000018). */
.section .text.stub, "ax"
.global stub_begin, stub_end
.balign 4
stub_begin:
    lui  t0, 0x10000
1:  lw   t1, 0x18(t0)
    sub  t1, t1, a0
    blt  t1, zero, 1b
    ret
stub_end:
