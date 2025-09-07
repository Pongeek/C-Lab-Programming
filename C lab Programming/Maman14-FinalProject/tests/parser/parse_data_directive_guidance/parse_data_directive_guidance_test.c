#include "stdint.h"
#include "../../../headers/lexer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/parser.h"
#include "../../../headers/string_util.h"

void run_test(char* file_path) {
    Lexer lexer_preprocess;
    Lexer lexer_postprocess;
    Preprocessor preprocessor;
    TranslationUnit unit;
    DataNode data_node;
    TokenReferenceNode *list = NULL;

    /* preprocess lexer  */
    lexer_initialize_from_file(&lexer_preprocess, file_path);
    lexer_analyze(&lexer_preprocess);
    error_handler_report_errors(&lexer_preprocess.error_handler);

    /* preprocesser */
    preprocessor_initialize(&preprocessor, lexer_preprocess, file_path);
    preprocessor_process(&preprocessor, lexer_preprocess.source_code);
    error_handler_report_errors(&preprocessor.error_handler);

    /* postprocess lexer */
    lexer_initialize_from_string(&lexer_postprocess, preprocessor.error_handler.file_path, preprocessor.processed_source);
    lexer_analyze(&lexer_postprocess);
    error_handler_report_errors(&lexer_postprocess.error_handler);

    /* parser pass */
    parser_initialize_translation_unit(&unit, lexer_postprocess);
    data_node = parse_data_directive_guidance(&unit);
    list = data_node.data_numbers;
    error_handler_report_errors(&unit.error_handler);

    while (list != NULL) {
        printf("%d, ", atoi(list->token->string.data));
        list = list->next;
    }
    printf("\n");

    parser_free_directive_guidance(data_node);
    lexer_free(&lexer_preprocess);
    lexer_free(&lexer_postprocess);
    preprocessor_free(&preprocessor);
    parser_free_translation_unit(&unit);
}

int main() {
    char* test_file1 = "test1";
    char* test_file2 = "test2";
    char* test_file3 = "test3";
    char* test_file4 = "test4";
    char* test_file5 = "test5";

    printf("Running test 1:\n");
    run_test(test_file1);

    printf("\nRunning test 2:\n");
    run_test(test_file2);

    printf("\nRunning test 3:\n");
    run_test(test_file3);

    printf("\nRunning test 4:\n");
    run_test(test_file4);

    printf("\nRunning test 5:\n");
    run_test(test_file5);


    return 0;
}
