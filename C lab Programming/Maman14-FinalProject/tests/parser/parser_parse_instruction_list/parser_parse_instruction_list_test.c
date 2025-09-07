#include "../../../headers//lexer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/parser.h"
#include "../../../headers/string_util.h"


int main(){
    Lexer lexer_preprocess;
    Lexer lexer_postprocess;
    Preprocessor preprocessor;
    TranslationUnit unit;
    InstructionNodeList * instruction_list;
    InstructionNodeList * temp;

    char *file_path = "test";

    /* preprocess lexer  */
    lexer_initialize_from_file(&lexer_preprocess, file_path);
    lexer_analyze(&lexer_preprocess);
    error_handler_report_errors(&lexer_preprocess.error_handler);

    /* preprocesser  */
    preprocessor_initialize(&preprocessor, lexer_preprocess,file_path);
    preprocessor_process(&preprocessor, lexer_preprocess.source_code);
    error_handler_report_errors(&preprocessor.error_handler);

    /* postprocess lexer  */
    lexer_initialize_from_string(&lexer_postprocess, preprocessor.error_handler.file_path, preprocessor.processed_source);
    lexer_analyze(&lexer_postprocess);
    error_handler_report_errors(&lexer_postprocess.error_handler);

    /* parser  */
    parser_initialize_translation_unit(&unit, lexer_postprocess);
    instruction_list = parser_parse_instruction_list(&unit);
    temp = instruction_list;
    error_handler_report_errors(&unit.error_handler);

    while (instruction_list){
        printf("operation: %s\n", instruction_list->node.operation->string.data);

        if (instruction_list->node.first_operand != NULL)
            printf("first operand: %s\n", instruction_list->node.first_operand->string.data);

        if (instruction_list->node.second_operand != NULL)
            printf("second operand: %s\n", instruction_list->node.second_operand->string.data);

        instruction_list = instruction_list->next;
    }

    lexer_free(&lexer_preprocess);
    lexer_free(&lexer_postprocess);
    preprocessor_free(&preprocessor);
    parser_free_instruction_list(temp);
    parser_free_translation_unit(&unit);

    return 0;
}
