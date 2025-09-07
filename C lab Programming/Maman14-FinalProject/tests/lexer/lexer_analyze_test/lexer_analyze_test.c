#include "../../../headers/lexer.h"



void run_test(const char* test_input);

int main() {
    const char* test_input1 = "start: .data 10, 20, 30\n"
                              "label1: .string \"Hello, World!\"\n"
                              "label2: mov r1, r2\n"
                              "label3: add r3, #5\n"
                              "label4: sub r4, r5\n"
                              "label5: jmp start\n"
                              "label6: cmp r6, r7\n"
                              "label7: lea label1, r8\n"
                              "label8: not r9\n"
                              "label9: clr r10\n"
                              "label10: inc r11\n"
                              "label11: dec r12\n"
                              "label12: red r13\n"
                              "label13: prn r14\n"
                              "label14: jsr label2\n"
                              "label15: rts\n"
                              "label16: stop\n";

    const char* test_input2 = ".data 123\n"
                              ".string \"Test String\"\n"
                              ".entry\n"
                              ".extern\n"
                              "mov r1, r2\n"
                              "add r3, #5\n"
                              "sub r4, r5\n"
                              "jmp start\n"
                              "cmp r6, r7\n"
                              "lea label1, r8\n"
                              "not r9\n"
                              "clr r10\n"
                              "inc r11\n"
                              "dec r12\n"
                              "red r13\n"
                              "prn r14\n"
                              "jsr label2\n"
                              "rts\n"
                              "stop\n";

    const char* test_input3 = "invalid@token\n"
                              ".unknownDirective\n"
                              "mov r1, r2\n"
                              "+\n"
                              "-\n"
                              "\"Unclosed string\n"
                              "label: .data 10, 20, 30\n";

    printf("Running test 1:\n");
    run_test(test_input1);

    printf("\nRunning test 2:\n");
    run_test(test_input2);

    printf("\nRunning test 3 (with errors):\n");
    run_test(test_input3);

    return 0;
}

void run_test(const char* test_input) {
    Lexer lexer;
    lexer_initialize_from_cstr(&lexer, (char*)test_input);
    lexer_analyze(&lexer);
    lexer_print_token_list(&lexer);
    error_handler_report_errors(&lexer.error_handler);
    lexer_free(&lexer);
}
