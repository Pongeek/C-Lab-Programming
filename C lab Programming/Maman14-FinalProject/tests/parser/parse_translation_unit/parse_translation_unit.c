#include "stdint.h"
#include "../../../headers/lexer.h"
#include "../../../headers/preprocessor.h"
#include "../../../headers/parser.h"
#include "../../../headers/string_util.h"

static void print_label(LabelNode label) {

    InstructionNodeList *current;
    TokenReferenceNode *data;
	
    if (label.instruction_list != NULL) {
        current = label.instruction_list;
        while (current != NULL) {
            printf("  Instruction: %s\n", current->node.operation->string.data);
            if (current->node.first_operand != NULL) {
                printf("    Operand1: %s\n", current->node.first_operand->string.data);
                if (current->node.is_first_operand_derefrenced) {
                    printf("    (Dereferenced)\n");
                }
            }
            if (current->node.second_operand != NULL) {
                printf("    Operand2: %s\n", current->node.second_operand->string.data);
                if (current->node.is_second_operand_derefrenced) {
                    printf("    (Dereferenced)\n");
                }
            }
            current = current->next;
        }
    }

    if (label.guidance_list != NULL) {
        GuidanceNodeList *current = label.guidance_list;
        while (current != NULL) {
            if (current->type == DATA_NODE) {
                printf("  Data: ");
                data = current->node.dataNode.data_numbers;
                while (data != NULL) {
                    printf("%s ", data->token->string.data);
                    data = data->next;
                }
                printf("\n");
            } else if (current->type == STRING_NODE) {
                printf("  String: %s\n", current->node.stringNode.string_label->string.data);
            }
            current = current->next;
        }
    }
}

void process_file(char* file_path) {
    Lexer lexer_preprocess;
    Lexer lexer_postprocess;
    Preprocessor preprocessor;
    TranslationUnit unit;
    ExternalNodeList *external_node_list = NULL;
    EntryNodeList *entry_node_list = NULL;
    LabelNodeList *instruction_label_list = NULL;
    LabelNodeList *guidance_label_list = NULL;

    printf("Processing file: %s\n", file_path);

    lexer_initialize_from_file(&lexer_preprocess, file_path);
    lexer_analyze(&lexer_preprocess);
    error_handler_report_errors(&lexer_preprocess.error_handler);

    preprocessor_initialize(&preprocessor, lexer_preprocess, file_path);
    preprocessor_process(&preprocessor, lexer_preprocess.source_code);
    error_handler_report_errors(&preprocessor.error_handler);

    lexer_initialize_from_string(&lexer_postprocess, preprocessor.error_handler.file_path, preprocessor.processed_source);
    lexer_analyze(&lexer_postprocess);
    error_handler_report_errors(&lexer_postprocess.error_handler);

    parser_initialize_translation_unit(&unit, lexer_postprocess);
    parse_translation_unit_content(&unit);
    error_handler_report_errors(&unit.error_handler);

    external_node_list = unit.external_list;
    entry_node_list = unit.entry_list;
    instruction_label_list = unit.instruction_label_list;
    guidance_label_list = unit.guidance_label_list;

    printf("Extern declarations:\n");
    while (external_node_list != NULL) {
        printf(".extern %s\n", external_node_list->external_node.external_label->string.data);
        external_node_list = external_node_list->next;
    }

    printf("\nEntry declarations:\n");
    while (entry_node_list != NULL) {
        printf(".entry %s\n", entry_node_list->entry_node.entry_label->string.data);
        entry_node_list = entry_node_list->next;
    }

    printf("\nInstruction labels:\n");
    while (instruction_label_list != NULL) {
        print_label(instruction_label_list->label);
        instruction_label_list = instruction_label_list->next;
    }

    printf("\nGuidance labels:\n");
    while (guidance_label_list != NULL) {
        print_label(guidance_label_list->label);
        guidance_label_list = guidance_label_list->next;
    }

    lexer_free(&lexer_preprocess);
    lexer_free(&lexer_postprocess);
    preprocessor_free(&preprocessor);
    parser_free_translation_unit(&unit);

    printf("\n");
}

int main() {
    char* test_file1 = "test1";
    char* test_file2 = "test2";

    process_file(test_file1);
    process_file(test_file2);

    return 0;
}
