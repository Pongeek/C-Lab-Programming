macr m1
    add r1, r2
    sub r3, #5
    .string "macro test"
endmacr

macr m2
    .data 100, 200, 300
    mov *r1, r4
    m1
endmacr

MAIN: m1
      jmp L1
      m2

L1:   prn #-50
      m2
      stop

macr complex_macro
    .entry MAIN
    .extern EXTERNAL_FUNC
    cmp r0, #1024
    bne END
    jsr EXTERNAL_FUNC
    .string "Complex macro test"
    .data 2047, -2048
endmacr

complexmacro

END:  stop
