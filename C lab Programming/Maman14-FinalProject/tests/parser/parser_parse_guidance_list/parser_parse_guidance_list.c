#include "stdint.h"
#include "../../../headers/lexer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/parser.h"
#include "../../../headers/string_util.h"

int main(){
    Lexer lexer_preprocess;
    Lexer lexer_postprocess;
    Preprocessor preprocessor;
    TranslationUnit unit;
    GuidanceNodeList * guidance_list;
    GuidanceNodeList * c;
    TokenReferenceNode * list = NULL;

    char* file_path = "test1";

    /* preprocess lexer */
    lexer_initialize_from_file(&lexer_preprocess, file_path);
    lexer_analyze(&lexer_preprocess);
    error_handler_report_errors(&lexer_preprocess.error_handler);

    /* preprocesser */
    preprocessor_initialize(&preprocessor, lexer_preprocess,file_path);
    preprocessor_process(&preprocessor, lexer_preprocess.source_code);
    error_handler_report_errors(&preprocessor.error_handler);

    /* postprocess lexer */
    lexer_initialize_from_string(&lexer_postprocess, preprocessor.error_handler.file_path, preprocessor.processed_source);
    lexer_analyze(&lexer_postprocess);
    error_handler_report_errors(&lexer_postprocess.error_handler);

    /* parser */
    parser_initialize_translation_unit(&unit, lexer_postprocess);
    guidance_list = parser_parse_guidance_list(&unit);
    c = guidance_list;
    error_handler_report_errors(&unit.error_handler);

    while (guidance_list != NULL){
        if (guidance_list->type == DATA_NODE){
            printf(".data ");

            list = guidance_list->node.dataNode.data_numbers;

            while (list != NULL){
                printf("%d, ", atoi(list->token->string.data));
                list = list->next;
            }
            printf("\n");
        } else if (guidance_list->type == STRING_NODE){
            if (guidance_list->node.stringNode.string_label != NULL)
                printf(".string %s\n", guidance_list->node.stringNode.string_label->string.data);
        }

        guidance_list = guidance_list->next;
    }
    printf("\n");


    lexer_free(&lexer_preprocess);
    lexer_free(&lexer_postprocess);
    preprocessor_free(&preprocessor);
    parser_free_guidance_list(c);
    parser_free_translation_unit(&unit);

    return 0;
}
