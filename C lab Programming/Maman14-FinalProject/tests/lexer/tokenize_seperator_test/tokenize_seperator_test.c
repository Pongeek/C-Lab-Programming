#include "../../../headers/lexer.h"
#include "../../../headers/char_util.h"

int main() {
    Lexer lexer;
    char* test_input = "*,:##**,,\n#comment\n:*,#another comment\n,,**:";

    printf("Test input: %s\n", test_input);
    lexer_initialize_from_cstr(&lexer, test_input);

    while (!lexer_is_end_of_input(&lexer)) {
        lexer_tokenize_separator(&lexer);
    }

    printf("Lexer output:\n");
    lexer_print_token_list(&lexer);
    error_handler_report_errors(&lexer.error_handler);
    lexer_free(&lexer);
    return 0;
}

