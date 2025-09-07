; Test file: comprehensive_test.as

; Entry and extern declarations
.entry MAIN
.entry FACTORIAL
.extern PRINT

; Main function
MAIN:   mov #5, r0     ; Number to calculate factorial of (5!)
        jsr FACTORIAL
        jsr PRINT      ; Print the result
        stop

; Factorial function
FACTORIAL:  mov r0, r1  ; Copy input to r1
            mov #1, r2  ; Initialize result in r2

LOOP:       cmp #1, r1
            beq END
            mul r1, r2  ; r2 = r2 * r1
            dec r1
            jmp LOOP

END:        mov r2, r0  ; Move result to r0
            rts

; Data section
DATA_SECTION: .data 100, -50, 250
STRING1:     .string "Hello, World!"
NUMBERS:     .data 2047, -2048  ; Max and min values

; Test various addressing modes
TEST_ADDR:   mov #5, r3
             add r3, *r4
             sub NUMBERS, r5
             lea STRING1, r6

; Test all instructions
             mov r1, r2
             cmp r1, #10
             add #20, r3
             sub r4, r5
             lea DATA_SECTION, r6
             clr r7
             not r0
             inc *r1
             dec r2
             jmp MAIN
             bne LOOP
             jsr FACTORIAL
             rts
             prn #-5
             red r3

; Edge cases
             .entry NONEXISTENT
             .data 2048    ; Just over the limit
             .data -2049   ; Just under the limit
             mov r1, #5    ; Invalid addressing mode
             unknown_op r1, r2  ; Unknown operation
