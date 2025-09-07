MACR GREET
    prn "Greetings!"
ENDMACR

MACR ADD
    add r1, r2
ENDMACR

MACR INVALID_MACRO
    mov r1, r2
    ; Missing ENDMACR

.data 123
.string "Test String"
GREET
ADD
INVALID_MACRO
stop