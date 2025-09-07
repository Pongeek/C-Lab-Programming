; file unknown_char

.entry STR
.extern  

.testThis

MAIN: add r3, r3
     - add r3, LIST  "
      jsr fn1   []
      mov r1, #$
      lea @r2, LABEL
      sub r4, %5
      cmp &r6, *r7
      not ^r0
      clr ~r5
      inc |r3
      dec \r2
      jmp `LOOP
      bne ?END
      red !r1
      prn #1+2
      rts <r4>
      stop {r7}
