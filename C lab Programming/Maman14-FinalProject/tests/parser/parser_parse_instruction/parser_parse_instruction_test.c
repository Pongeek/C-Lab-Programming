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
    InstructionNode instruction_node;

    /* preprocess lexer  */
    lexer_initialize_from_file(&lexer_preprocess, file_path);
    lexer_analyze(&lexer_preprocess);
    error_handler_report_errors(&lexer_preprocess.error_handler);

    /* preprocesser  */
    preprocessor_initialize(&preprocessor, lexer_preprocess, file_path);
    preprocessor_process(&preprocessor, lexer_preprocess.source_code);
    error_handler_report_errors(&preprocessor.error_handler);

    /* postprocess lexer  */
    lexer_initialize_from_string(&lexer_postprocess, preprocessor.error_handler.file_path, preprocessor.processed_source);
    lexer_analyze(&lexer_postprocess);
    error_handler_report_errors(&lexer_postprocess.error_handler);

    /* parser  */
    parser_initialize_translation_unit(&unit, lexer_postprocess);
    instruction_node = parser_parse_instruction(&unit);
    error_handler_report_errors(&unit.error_handler);

    if (instruction_node.first_operand != NULL)
        printf("first operand: %s\n", instruction_node.first_operand->string.data);

    if (instruction_node.second_operand != NULL)
        printf("second operand: %s\n", instruction_node.second_operand->string.data);

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


    printf("Running test 1:\n");
    run_test(test_file1);

    printf("\nRunning test 2:\n");
    run_test(test_file2);

    printf("\nRunning test 3:\n");
    run_test(test_file3);

    printf("\nRunning test 4:\n");
    run_test(test_file4);


    return 0;
}
