
#include "../../../headers/lexer.h"
#include "../../../headers/semantic_analyzer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/parser.h"

int main(){
    Lexer lexer_preprocess;
    Lexer lexer_postprocess;
    Preprocessor preprocessor;
    TranslationUnit unit;
    SemanticAnalyzer analyzer;
    DataNode node;

    /* preprocess lexer */
    char* file_path = "test1";
    lexer_initialize_from_file(&lexer_preprocess, file_path);
    lexer_analyze(&lexer_preprocess);
    error_handler_report_errors(&lexer_preprocess.error_handler);

    /* preprocesser  */
    preprocessor_initialize(&preprocessor, lexer_preprocess, "test1");
    preprocessor_process(&preprocessor, lexer_preprocess.source_code);
    error_handler_report_errors(&preprocessor.error_handler);

    /* postprocess lexer */
    lexer_initialize_from_string(&lexer_postprocess, preprocessor.error_handler.file_path, preprocessor.processed_source);
    lexer_analyze(&lexer_postprocess);
    error_handler_report_errors(&lexer_postprocess.error_handler);

    /* parser */
    parser_initialize_translation_unit(&unit, lexer_postprocess);
    node = parse_data_directive_guidance(&unit);
    error_handler_report_errors(&unit.error_handler);

    semantic_analyzer_initialize(&analyzer, &unit, lexer_postprocess);
    semantic_analyzer_analyze_directive_guidance(&analyzer, node);
    error_handler_report_errors(&analyzer.error_handler);

    parser_free_directive_guidance(node);

    lexer_free(&lexer_preprocess);
    lexer_free(&lexer_postprocess);
    preprocessor_free(&preprocessor);
    parser_free_translation_unit(&unit);
    semantic_analyzer_free(&analyzer);

    return 0;
}
