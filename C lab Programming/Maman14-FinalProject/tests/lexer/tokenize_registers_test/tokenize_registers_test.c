#include "../../../headers/lexer.h"
#include <stdio.h>

int main(){
    Lexer lexer;
    lexer_initialize_from_cstr(&lexer, "r0\nr1\nr2\nr3\nr4\nr5\nr6\nr7\n");

    while (!lexer_is_end_of_input(&lexer)) {
        lexer_tokenize_identifier(&lexer);
        lexer_tokenize_newline(&lexer);
    }

    printf("Printing tokens...\n"); /* Debugging*/
    lexer_print_token_list(&lexer);

    lexer_free(&lexer);


    return 0;
}
