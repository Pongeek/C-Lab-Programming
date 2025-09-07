#include "../../../headers/lexer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/string_util.h"

void run_test(char* file_path) {
    Lexer lexer;
    Preprocessor pre_processor;

    lexer_initialize_from_file(&lexer, file_path);
    lexer_analyze(&lexer);
    error_handler_report_errors(&lexer.error_handler);

    preprocessor_initialize(&pre_processor, lexer, file_path);
    preprocessor_process(&pre_processor, lexer.source_code);
    error_handler_report_errors(&pre_processor.error_handler);

    lexer_free(&lexer);
    preprocessor_free(&pre_processor);
}

int main() {
    char* test_file1 = "test1";
    char* test_file2 = "test2";

    printf("Running test 1:\n");
    run_test(test_file1);

    printf("\nRunning test 2:\n");
    run_test(test_file2);

    return 0;
}
