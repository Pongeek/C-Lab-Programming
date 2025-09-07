#include "../../../headers/lexer.h"

void run_test(const char* test_input) {
    Lexer lexer;
    lexer_initialize_from_cstr(&lexer, (char*)test_input);

    while (!lexer_is_end_of_input(&lexer)) {
        lexer_tokenize_non_op_instruction(&lexer);
        lexer_tokenize_newline(&lexer);
    }

    lexer_print_token_list(&lexer);
    lexer_free(&lexer);
}

int main() {
    const char* test_input1 = ".invalidDirective\n.data 123\n.entry\n.string \"Hello\n";
    const char* test_input2 = ".string.data.entry.string";

    printf("Running test 1 with error:\n");
    run_test(test_input1);

    printf("\nRunning test 2:\n");
    run_test(test_input2);

    return 0;
}
