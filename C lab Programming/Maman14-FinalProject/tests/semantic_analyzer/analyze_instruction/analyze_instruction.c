
#include "stdint.h"
#include "../../../headers/lexer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/parser.h"
#include "../../../headers/semantic_analyzer.h"
#include "../../../headers/string_util.h"

int main(){
    Lexer lexer_preprocess;
    Lexer lexer_postprocess;
    Preprocessor preprocessor;
    TranslationUnit unit;
    SemanticAnalyzer analyzer;
    LabelNodeList * instruction_label_list = NULL;

    char* file_path = "test";

    /* preprocess lexer*/
    lexer_initialize_from_file(&lexer_preprocess, file_path);
    lexer_analyze(&lexer_preprocess);
    error_handler_report_errors(&lexer_preprocess.error_handler);

    /* preprocesser */
    preprocessor_initialize(&preprocessor, lexer_preprocess, file_path);
    preprocessor_process(&preprocessor, lexer_preprocess.source_code);
    error_handler_report_errors(&preprocessor.error_handler);

    /* postprocess lexer  */
    lexer_initialize_from_string(&lexer_postprocess, preprocessor.error_handler.file_path, preprocessor.processed_source);
    lexer_analyze(&lexer_postprocess);
    error_handler_report_errors(&lexer_postprocess.error_handler);

    /* parser*/
    parser_initialize_translation_unit(&unit, lexer_postprocess);
    parse_translation_unit_content(&unit);
    error_handler_report_errors(&unit.error_handler);

    /* Analyzer */
    semantic_analyzer_initialize(&analyzer, &unit, lexer_postprocess);
    semantic_analyzer_analyze_duplicate_identifiers(&analyzer, &unit);

    instruction_label_list = unit.instruction_label_list;

    while (instruction_label_list != NULL){
        while (instruction_label_list->label.instruction_list != NULL){
            semantic_analyzer_analyze_instruction(&analyzer, instruction_label_list->label.instruction_list->node);
            instruction_label_list->label.instruction_list = instruction_label_list->label.instruction_list->next;
        }

        instruction_label_list = instruction_label_list->next;
    }

    error_handler_report_errors(&analyzer.error_handler);

    semantic_analyzer_free(&analyzer);
    parser_free_translation_unit(&unit);
    preprocessor_free(&preprocessor);
    lexer_free(&lexer_postprocess);
    lexer_free(&lexer_preprocess);

    return 0;
}
