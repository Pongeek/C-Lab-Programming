.extern funcB
.extern funcC
.extern varB

.entry mainFunc

mainFunc:
        mov r1, #0
        jsr funcB
        jsr funcC
        lea stringB, r2
        prn r2
        stop

stringB:  
        .string "Hello, World!"
        .data 100, 200, 300

