#include "../../../headers/lexer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/parser.h"
#include "../../../headers/semantic_analyzer.h"
#include "../../../headers/code_generator.h"
#include <stdio.h>


int main() {
    Lexer lexer_pre_processor;
    Lexer lexer_post_processor;
    Preprocessor pre_processor;
    SemanticAnalyzer analyzer;
    TranslationUnit translation_unit;
    CodeGenerator generator;


    char *file_path = "test1";

    printf("Attempting to open file: %s\n", file_path);


    lexer_initialize_from_file(&lexer_pre_processor, file_path);
    lexer_analyze(&lexer_pre_processor);
    error_handler_report_errors(&lexer_pre_processor.error_handler);

    /* Postprocess lexer */
    preprocessor_initialize(&pre_processor, lexer_pre_processor, file_path);
    preprocessor_process(&pre_processor, lexer_pre_processor.source_code);
    error_handler_report_errors(&pre_processor.error_handler);

    /* postprocess lexer  */
    lexer_initialize_from_string(&lexer_post_processor, pre_processor.error_handler.file_path,pre_processor.processed_source);
    lexer_analyze(&lexer_post_processor);
    error_handler_report_errors(&lexer_post_processor.error_handler);

    /* Parser*/
    parser_initialize_translation_unit(&translation_unit, lexer_post_processor);
    parse_translation_unit_content(&translation_unit);
    error_handler_report_errors(&translation_unit.error_handler);

    /* semantic analyzer  */
    semantic_analyzer_initialize(&analyzer, &translation_unit, lexer_post_processor);
    semantic_analyzer_analyze_translation_unit(&analyzer,&translation_unit);
    error_handler_report_errors(&analyzer.error_handler);

    code_generator_initialize(&generator,lexer_post_processor);
    code_generator_update_labels(&generator,&translation_unit);
    generate_entry_file_string(&generator,&analyzer,&translation_unit);
    output_generate(&generator,&analyzer,&translation_unit,file_path);
    error_handler_report_errors(&generator.error_handler);

    string_debug_info(generator.entry_file);
    string_debug_info(generator.external_file);
    string_debug_info(generator.object_file);

    code_generator_free(&generator);
    semantic_analyzer_free(&analyzer);
    parser_free_translation_unit(&translation_unit);
    lexer_free(&lexer_post_processor);
    lexer_free(&lexer_pre_processor);
    preprocessor_free(&pre_processor);

    return 0;
}
