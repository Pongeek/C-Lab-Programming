#include "../../../headers/lexer.h"
#include "../../../headers/char_util.h"
#include <stdio.h>

void run_test(const char* test_input) {
    Lexer lexer;

    printf("Test input: %s\n", test_input);
    lexer_initialize_from_cstr(&lexer, (char*)test_input);

    while (!lexer_is_end_of_input(&lexer)) {
        if (is_digit(lexer.current_char) || char_exists_in_string(lexer.current_char, "+-")) {
            lexer_tokenize_number(&lexer);
        } else if (lexer.current_char == '\n') {
            lexer_tokenize_newline(&lexer);
        } else {
            lexer_advance_character(&lexer);
        }
    }

    printf("Lexer output:\n");
    error_handler_report_errors(&lexer.error_handler);
    lexer_print_token_list(&lexer);
    lexer_free(&lexer);
}

int main() {
    const char* test_input1 = "+\n-8856\n772\n+1246\n-";
    const char* test_input2 = "-7656\n8852\n+3546";

    printf("Running test 1 with errors:\n");
    run_test(test_input1);

    printf("\nRunning test 2:\n");
    run_test(test_input2);

    return 0;
}
