.entry INIT
.entry PROCESS
.extern A
.extern B

INIT:  clr r1
       lea STR, r2
       jsr A

PROCESS: cmp r3, r4
         bne END
         add r5, B
         sub r6, r7

END:    stop

STR:    .string "test"
NUMS:   .data 10, 20, 30
NEG:    .data -10, -20, -30