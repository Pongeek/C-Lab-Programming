
#include "stdint.h"
#include "../../../headers/lexer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/parser.h"
#include "../../../headers/semantic_analyzer.h"
#include "../../../headers/code_generator.h"
#include "../../../headers/string_util.h"

int main(){
    Lexer lexer_preprocess;
    Lexer lexer_postprocess;
    Preprocessor preprocessor;
    TranslationUnit unit;
    SemanticAnalyzer semantic_analyzer;
    CodeGenerator code_generator;

    char *file_path = "test1";

    /* preprocess lexer pass */
    lexer_initialize_from_file(&lexer_preprocess, file_path);
    lexer_analyze(&lexer_preprocess);
    error_handler_report_errors(&lexer_preprocess.error_handler);

    /* preprocesser pass */
    preprocessor_initialize(&preprocessor, lexer_preprocess, file_path);
    preprocessor_process(&preprocessor, lexer_preprocess.source_code);
    error_handler_report_errors(&preprocessor.error_handler);

    /* postprocess lexer pass */
    lexer_initialize_from_string(&lexer_postprocess, preprocessor.error_handler.file_path, preprocessor.processed_source);
    lexer_analyze(&lexer_postprocess);
    error_handler_report_errors(&lexer_postprocess.error_handler);

    /* parser pass */
    parser_initialize_translation_unit(&unit, lexer_postprocess);
    parse_translation_unit_content(&unit);
    error_handler_report_errors(&unit.error_handler);

    /* semantic analyzer pass */
    semantic_analyzer_initialize(&semantic_analyzer, &unit, lexer_postprocess);
    semantic_analyzer_analyze_translation_unit(&semantic_analyzer, &unit);
    error_handler_report_errors(&semantic_analyzer.error_handler);

    /* emitter chekc pass*/
    code_generator_initialize(&code_generator, lexer_postprocess);
    code_generator_update_labels(&code_generator, &unit);
    generate_entry_file_string(&code_generator, &semantic_analyzer, &unit);
    output_generate(&code_generator, &semantic_analyzer, &unit, file_path);
    error_handler_report_errors(&code_generator.error_handler);

    string_debug_info(code_generator.entry_file);
    string_debug_info(code_generator.external_file);
    string_debug_info(code_generator.object_file);

    code_generator_free(&code_generator);
    semantic_analyzer_free(&semantic_analyzer);
    parser_free_translation_unit(&unit);
    preprocessor_free(&preprocessor);
    lexer_free(&lexer_postprocess);
    lexer_free(&lexer_preprocess);

    return 0;
}

