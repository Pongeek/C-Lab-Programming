
#include "stdint.h"
#include "../../../headers/lexer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/parser.h"
#include "../../../headers/semantic_analyzer.h"
#include "../../../headers/string_util.h"

static void print_label(LabelNode label){
    InstructionNodeList * copyI = label.instruction_list;
    GuidanceNodeList * copyG = label.guidance_list;
    TokenReferenceNode * list = NULL;

    printf("Label size: %d\n", label.size);
    printf("Label position: %d\n", label.position);

    if (label.label != NULL){
        printf("%s:\n", label.label->string.data);
    }

    while (copyI != NULL){
        printf("    %s", copyI->node.operation->string.data);

        if (copyI->node.first_operand != NULL)
            printf(" %s", copyI->node.first_operand->string.data);

        if (copyI->node.second_operand != NULL)
            printf(", %s", copyI->node.second_operand->string.data);

        printf("\n");
        copyI = copyI->next;
    }

    while (copyG != NULL){
        if (copyG->type == DATA_NODE){
            printf("    .data ");

            list = copyG->node.dataNode.data_numbers;

            while (list != NULL){
                printf("%d", atoi(list->token->string.data));
                if (list->next != NULL) printf(", ");
                list = list->next;
            }

            printf("\n");
        } else if (copyG->type == STRING_NODE){
            if (copyG->node.stringNode.string_label != NULL)
                printf("    .string %s\n", copyG->node.stringNode.string_label->string.data);
        }

        copyG = copyG->next;
    }
}

int main(){
    Lexer lexer_preprocess;
    Lexer lexer_postprocess;
    Preprocessor preprocessor;
    TranslationUnit unit;
    SemanticAnalyzer analyzer;
    ExternalNodeList * external_node_list = NULL;
    EntryNodeList * entryNodeList = NULL;
    LabelNodeList * instruction_label_list = NULL;
    LabelNodeList * guidance_label_list = NULL;

    char* file_path = "test";


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

    /* ast check pass */
    semantic_analyzer_initialize(&analyzer, &unit, lexer_postprocess);
    semantic_analyzer_analyze_duplicate_identifiers(&analyzer, &unit);
    error_handler_report_errors(&analyzer.error_handler);

    external_node_list = unit.external_list;
    entryNodeList = unit.entry_list;
    instruction_label_list = unit.instruction_label_list;
    guidance_label_list = unit.guidance_label_list;

    while (external_node_list != NULL){
        printf(".extern %s\n", external_node_list->external_node.external_label->string.data);
        external_node_list = external_node_list->next;
    }

    while (entryNodeList != NULL){
        printf(".entry %s\n", entryNodeList->entry_node.entry_label->string.data);
        entryNodeList = entryNodeList->next;
    }

    while (instruction_label_list != NULL){
        print_label(instruction_label_list->label);
        instruction_label_list = instruction_label_list->next;
    }

    while (guidance_label_list != NULL){
        print_label(guidance_label_list->label);
        guidance_label_list = guidance_label_list->next;
    }

    semantic_analyzer_free(&analyzer);
    parser_free_translation_unit(&unit);
    preprocessor_free(&preprocessor);
    lexer_free(&lexer_postprocess);
    lexer_free(&lexer_preprocess);

    return 0;
}
