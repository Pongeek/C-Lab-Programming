MAIN: mov r0, #1
      add r1, r2
      sub r3, #5
      jmp L1

L1:   prn #-50
      add r1, r2
      sub r3, #5
      jsr SUB1

SUB1: add r1, r2
      ret

      .entry MAIN
      .extern L2
      jmp L2
