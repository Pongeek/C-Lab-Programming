; file factorial.as

.entry MAIN
.entry FACTORIAL
.entry ff

; Assuming PRINT is an external function to print the values
.extern PRINT

MAIN:   mov #5, *r0
        ; Number to calculate the factorial of (5!) (*r0 just to test derefrece)
        ; Jump to the factorial function
        jsr FACTORIAL
        ; Print the value stored in the register
        jsr PRINT
        ; Stop execution
        stop

; test data also
.data 525, 985, -784, 4
.string "dfsdf"

FACTORIAL: mov r0, r1
        ; r1 = n (the number we are calculating the factorial for)
        mov #1, *r2
        ; r2 = 1 (this will hold the result of the factorial) (*r2 just to test derefrece)
        jsr FACLOOP

FACLOOP: cmp #1, r1
        ; Compare r1 with 1
        ; If r1 <= 1, exit loop
        bne ENDFAC
        ; r2 = r2 + r1 (add current result by r1) (we don't have multiplication so we just do this)
        add r1, r2
        ; r1 = r1 - 1 (decrement r1)
        dec *r1
        ; Repeat the loop (*r1 just to test derefrece)
        jmp FACLOOP

ff: .string "dfdf"
    .data 12
    .string ""
    .data 123

ENDFAC: prn r2
        ; Print the factorial result
        ; Return from the function
        rts