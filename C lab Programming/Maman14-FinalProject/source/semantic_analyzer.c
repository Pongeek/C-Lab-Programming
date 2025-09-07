#include "../headers/safe_allocations.h"
#include "../headers/semantic_analyzer.h"
#include "../headers/string_util.h"
#include <stdio.h>

/* Maximum positive value for a 15-bit signed integer */
#define MAX_15BIT_SIGNED_INT  (signed int)(((2 << (14-1))) -1)
/* Minimum negative value for a 15-bit signed integer */
#define MIN_15BIT_SIGNED_INT (signed int)(-(2 << (14-1)))
/* Maximum positive value for a 12-bit signed integer */
#define MAX_12BIT_SIGNED_INT  (signed int)(((2 << (11-1))) -1)
/* Minimum negative value for a 12-bit signed integer */
#define MIN_12BIT_SIGNED_INT (signed int)(-(2 << (11-1)))

static void report_error(SemanticAnalyzer *analyzer, const char *message, Token *token);
static int get_expected_operand_count(TokenType operation_type);
static AddressingMode validate_and_determine_addressing_mode(SemanticAnalyzer *analyzer, Token *operand_token, bool is_dereferenced);
static void validate_identifier(SemanticAnalyzer *analyzer, Token *token);
static void validate_entry_declarations(SemanticAnalyzer *analyzer, EntryNodeList *entry_node_list);
static void validate_external_declarations(SemanticAnalyzer *analyzer, ExternalNodeList *external_node_list);
static void validate_guidance_labels(SemanticAnalyzer *analyzer, LabelNodeList *guidance_label_list);
static void validate_instruction_labels(SemanticAnalyzer *analyzer, LabelNodeList *instruction_label_list);
static void validate_label_list(SemanticAnalyzer *analyzer, LabelNodeList *label_list);

unsigned long compute_string_hash(String str) {
    unsigned long h = 5381;
    unsigned char *us = (unsigned char *) str.data;

    while (*us) {
        h = ((h << 5) + h) + *us++;
    }

    return h;
}

void semantic_analyzer_initialize(SemanticAnalyzer *analyzer, TranslationUnit *unit, Lexer lexer) {
    ExternalNodeList *externalNodeList;
    LabelNodeList *instructionLabelList;
    LabelNodeList *guidanceLabelList;
    LabelNodeList *list;
    ExternalNodeList *extList;

    analyzer->size = 0;

    instructionLabelList = unit->instruction_label_list;
    guidanceLabelList = unit->guidance_label_list;
    externalNodeList = unit->external_list;

    /* Count total identifiers */
    for (list = instructionLabelList; list != NULL; list = list->next) {
        analyzer->size++;
    }
    for (list = guidanceLabelList; list != NULL; list = list->next) {
        if (list->label.label != NULL) {
            analyzer->size++;
        }
    }
    for (extList = externalNodeList; extList != NULL; extList = extList->next) {
        analyzer->size++;
    }

    /* Allocate hash table with a load factor of 0.75 */
    analyzer->size = (unsigned int) (analyzer->size / 0.75) + 1;
    analyzer->hash = safe_calloc(analyzer->size, sizeof(IdentifierCell));

    error_handler_initialize(&analyzer->error_handler, lexer.source_code, lexer.file_path);
}

void semantic_analyzer_free(SemanticAnalyzer *analyzer) {
    if (analyzer == NULL) {
        return; /* Nothing to free if the pointer is NULL */
    }

    /* Free the hash table */
    free(analyzer->hash);
    analyzer->hash = NULL; /* Set to NULL to prevent use after free */

    /* Free resources held by the error handler */
    error_handler_free(&analyzer->error_handler);

    /* Reset size to 0 */
    analyzer->size = 0;

    /* Note: We don't free Semantic Analyzer itself as it might not have been dynamically allocated */
}

IdentifierCell *semantic_analyzer_find_identifier(SemanticAnalyzer *analyzer, String key) {
    unsigned long index;
    unsigned long start_index;

    if (analyzer == NULL || analyzer->hash == NULL || analyzer->size == 0) {
        return NULL; /* Return NULL if the validator or hash table is invalid */
    }

    index = compute_string_hash(key) % analyzer->size;
    start_index = index;

    do {
        IdentifierCell *cell = &analyzer->hash[index];

        if (cell->key == NULL) {
            return NULL; /* Empty cell, key not found */
        }

        if (string_equals(key, *cell->key)) {
            return cell; /* Key found */
        }

        index = (index + 1) % analyzer->size; /* Move to next cell, wrap around if necessary */
    } while (index != start_index);

    return NULL; /* Key not found after searching entire table */
}

bool semantic_analyzer_insert_identifier(SemanticAnalyzer *analyzer, IdentifierCell cell) {
    unsigned long hashValue;
    unsigned long index;
    unsigned long startIndex;

    if (analyzer == NULL || analyzer->hash == NULL || analyzer->size == 0) {
        return false; /* Invalid validator or hash table */
    }

    hashValue = compute_string_hash(*cell.key);
    index = hashValue % analyzer->size;
    startIndex = index;

    cell.has_entry = false; /* Ensure this is not set during validation */

    do {
        if (analyzer->hash[index].key == NULL) {
            /* Found an empty slot, insert the new cell */
            analyzer->hash[index] = cell;
            return true;
        }

        /* Check for duplicate key */
        if (string_equals(*cell.key, *analyzer->hash[index].key)) {
            return false; /* Duplicate key found, insertion fails */
        }

        index = (index + 1) % analyzer->size; /* Move to next slot, wrap around if necessary */
    } while (index != startIndex);

    return false; /* Hash table is full */
}

void semantic_analyzer_analyze_directive_guidance(SemanticAnalyzer *analyzer, DataNode node) {
    TokenReferenceNode *current;
    int value;

    if (analyzer == NULL) {
        fprintf(stderr, "Error: Null Semantic Analyzer passed to semantic_analyzer_data_node\n");
        return;
    }

    current = node.data_numbers;
    while (current != NULL) {
        if (current->token == NULL) {
            fprintf(stderr, "Error: Null token in data node\n");
            continue;
        }

        value = atoi(current->token->string.data);
        if (value > MAX_15BIT_SIGNED_INT || value < MIN_15BIT_SIGNED_INT) {
            report_error(analyzer, "Integer value is out of the allowed range", current->token);
        }

        current = current->next;
    }
}

void semantic_analyzer_analyze_instruction(SemanticAnalyzer *analyzer, InstructionNode node) {
    Token *source;
    bool isSourceDereferenced;
    Token *destination;
    bool isDestinationDereferenced;
    AddressingMode sourceAM;
    AddressingMode destinationAM;
    Token *operation;
    int expectedOperandCount;
    int actualOperandCount;

    if (analyzer == NULL || node.operation == NULL) {
        fprintf(stderr, "Error: Invalid parameters passed to semantic_analyzer_analyze_instruction\n");
        return;
    }

    source = node.first_operand;
    isSourceDereferenced = node.is_first_operand_derefrenced;
    destination = node.second_operand;
    isDestinationDereferenced = node.is_second_operand_derefrenced;
    sourceAM = ADDRESSING_MODE_IMMEDIATE;
    destinationAM = ADDRESSING_MODE_IMMEDIATE;
    operation = node.operation;

    /* Validate operand count */
    expectedOperandCount = get_expected_operand_count(operation->type);
    actualOperandCount = (source != NULL) + (destination != NULL);

    if (actualOperandCount != expectedOperandCount) {
        report_error(analyzer, "Invalid number of operands", operation);
        return;
    }

    /* Validate addressing modes and identifiers */
    if (source != NULL) {
        sourceAM = validate_and_determine_addressing_mode(analyzer, source, isSourceDereferenced);
        if (source->type == TOKEN_IDENTIFIER) {
            validate_identifier(analyzer, source);
        }
    }
    if (destination != NULL) {
        destinationAM = validate_and_determine_addressing_mode(analyzer, destination, isDestinationDereferenced);
        if (destination->type == TOKEN_IDENTIFIER) {
            validate_identifier(analyzer, destination);
        }
    }

    /* Validate operation-specific requirements */
    switch (operation->type) {
        case TOKEN_MOV:
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_CMP:
            if (sourceAM == ADDRESSING_MODE_IMMEDIATE && destinationAM == ADDRESSING_MODE_IMMEDIATE) {
                report_error(analyzer, "Both operands cannot be immediate", operation);
            }
            break;
        case TOKEN_NOT:
        case TOKEN_CLR:
        case TOKEN_INC:
        case TOKEN_DEC:
            if (sourceAM == ADDRESSING_MODE_IMMEDIATE) {
                report_error(analyzer, "Operand cannot be immediate for this instruction", source);
            }
            break;
        case TOKEN_LEA:
            if (sourceAM != ADDRESSING_MODE_DIRECT) {
                report_error(analyzer, "LEA source must be a label", source);
            }
            if (destinationAM != ADDRESSING_MODE_DIRECT_REGISTER && destinationAM != ADDRESSING_MODE_INDIRECT_REGISTER) {
                report_error(analyzer, "LEA destination must be a register", destination);
            }
            break;
        case TOKEN_JMP:
        case TOKEN_BNE:
        case TOKEN_JSR:
            if (sourceAM != ADDRESSING_MODE_DIRECT && sourceAM != ADDRESSING_MODE_INDIRECT_REGISTER) {
                report_error(analyzer, "Invalid addressing mode for jump instruction", source);
            }
            break;
        case TOKEN_RED:
            if (sourceAM == ADDRESSING_MODE_IMMEDIATE) {
                report_error(analyzer, "RED operand cannot be immediate", source);
            }
            break;
        case TOKEN_PRN:
            /* PRN accepts all addressing modes, so no additional checks needed */
            break;
        case TOKEN_RTS:
        case TOKEN_STOP:
            /* No operands, so no additional checks needed */
            break;
        default:
            report_error(analyzer, "Unknown operation type", operation);
            break;
    }
}

void semantic_analyzer_analyze_label(SemanticAnalyzer *analyzer, LabelNode node) {
    InstructionNodeList *currentInstruction;
    GuidanceNodeList *currentGuidance;

    if (analyzer == NULL) {
        fprintf(stderr, "Error: Null Analyzer passed to semantic_analyzer_analyze_label\n");
        return;
    }

    /* Check if a label with instructions has a label identifier */
    if (node.instruction_list != NULL && node.label == NULL) {
        report_error(analyzer, "A label with instructions should have a label identifier", node.instruction_list->node.first_operand);
    }

    /* Validate all instruction nodes */
    currentInstruction = node.instruction_list;
    while (currentInstruction != NULL) {
        semantic_analyzer_analyze_instruction(analyzer, currentInstruction->node);
        currentInstruction = currentInstruction->next;
    }

    /* Validate all guidance nodes */
    currentGuidance = node.guidance_list;
    while (currentGuidance != NULL) {
        switch (currentGuidance->type) {
            case DATA_NODE:
                semantic_analyzer_analyze_directive_guidance(analyzer, currentGuidance->node.dataNode);
            break;
            case STRING_NODE:
                /* Add string validation if needed - Need to check this up. */
                    break;
            /* Add cases for other guidance node types if needed */
            default:
                /* Using label as a fallback token */
                    report_error(analyzer, "Unknown guidance node type", node.label);
            break;
        }
        currentGuidance = currentGuidance->next;
    }
}

void semantic_analyzer_analyze_duplicate_identifiers(SemanticAnalyzer *analyzer, TranslationUnit *unit) {
    if (analyzer == NULL || unit == NULL) {
        fprintf(stderr, "Error: Null pointer passed to semantic_analyzer_analyze_duplicate_identifiers\n");
        return;
    }

    validate_instruction_labels(analyzer, unit->instruction_label_list);
    validate_guidance_labels(analyzer, unit->guidance_label_list);
    validate_external_declarations(analyzer, unit->external_list);
    validate_entry_declarations(analyzer, unit->entry_list);
}

void semantic_analyzer_analyze_translation_unit(SemanticAnalyzer *analyzer, TranslationUnit *unit) {
    if (analyzer == NULL || unit == NULL) {
        fprintf(stderr, "Error: Null pointer passed to semantic_analyzer_analyze_translation_unit\n");
        return;
    }

    /* Check for identifier uniqueness across the entire translation unit */
    semantic_analyzer_analyze_duplicate_identifiers(analyzer, unit);

    /* Validate instruction labels */
    validate_label_list(analyzer, unit->instruction_label_list);

    /* Validate guidance labels */
    validate_label_list(analyzer, unit->guidance_label_list);
}

/**
 * Reports an error to the Semantic Analyzer's error handler.
 *
 * @param analyzer Pointer to the Analyzer structure.
 * @param message The error message string.
 * @param token The token associated with the error.
 */
static void report_error(SemanticAnalyzer *analyzer, const char *message, Token *token) {
    TokenError error;

    if (token == NULL) {
        fprintf(stderr, "Error: NULL token passed to report_error for message: %s\n", message);
        return;
    }

    error.message = string_create_from_cstr(message);
    error.token = *token;

    error_handler_add_token_error(&analyzer->error_handler, SEMANTIC_ANALYZER_ERROR_TYPE, error);
}

/**
 * Determines the addressing mode of an operand and validates its correctness.
 *
 * This function analyzes the given operand token and its dereference status
 * to determine the appropriate addressing mode. It also performs validation
 * checks, reporting errors for invalid combinations or out-of-range values.
 *
 * @param analyzer Pointer to the Analyzer structure.
 * @param operand_token Pointer to the token representing the operand.
 * @param is_dereferenced Boolean indicating whether the operand is dereferenced.
 * @return The determined AddressingMode of the operand.
 */
static AddressingMode validate_and_determine_addressing_mode(SemanticAnalyzer *analyzer, Token *operand_token, bool is_dereferenced) {
    AddressingMode mode;
    int value;

    switch (operand_token->type) {
        case TOKEN_NUMBER:
            if (is_dereferenced) {
                report_error(analyzer, "A number cannot be dereferenced", operand_token);
            }

        value = atoi(operand_token->string.data);
        if (value > MAX_12BIT_SIGNED_INT || value < MIN_12BIT_SIGNED_INT) {
            report_error(analyzer, "Integer value is out of the allowed range", operand_token);
        }

        mode = ADDRESSING_MODE_IMMEDIATE;
        break;

        case TOKEN_IDENTIFIER:
            if (is_dereferenced) {
                report_error(analyzer, "A label cannot be dereferenced", operand_token);
            }
        mode = ADDRESSING_MODE_DIRECT;
        break;

        case TOKEN_REGISTER:
            mode = is_dereferenced ? ADDRESSING_MODE_INDIRECT_REGISTER : ADDRESSING_MODE_DIRECT_REGISTER;
        break;

        default:
            report_error(analyzer, "Invalid operand syntax", operand_token);
        mode = ADDRESSING_MODE_IMMEDIATE; /* Default to absolute as a fallback */
        break;
    }

    return mode;
}

/**
 * Gets the expected number of operands for a given operation type.
 *
 * @param operation_type The type of the operation.
 * @return The expected number of operands.
 */
static int get_expected_operand_count(TokenType operation_type) {
    switch (operation_type) {
        case TOKEN_MOV:
        case TOKEN_CMP:
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_LEA:
            return 2;
        case TOKEN_CLR:
        case TOKEN_NOT:
        case TOKEN_INC:
        case TOKEN_DEC:
        case TOKEN_JMP:
        case TOKEN_BNE:
        case TOKEN_JSR:
        case TOKEN_RED:
        case TOKEN_PRN:
            return 1;
        case TOKEN_RTS:
        case TOKEN_STOP:
            return 0;
        default:
            return -1; /* Invalid operation type */
    }
}

/**
 * Validates that an identifier exists in the symbol table.
 *
 * @param analyzer Pointer to the Analyzer structure.
 * @param token The token containing the identifier to validate.
 */
static void validate_identifier(SemanticAnalyzer *analyzer, Token *token) {
    if (semantic_analyzer_find_identifier(analyzer, token->string) == NULL) {
        report_error(analyzer, "Unknown identifier", token);
    }
}

/**
 * Validates instruction labels for duplicates.
 */
static void validate_instruction_labels(SemanticAnalyzer *analyzer, LabelNodeList *instruction_label_list) {
    IdentifierCell cell;

    while (instruction_label_list != NULL) {
        cell.key = &instruction_label_list->label.label->string;
        cell.type = IDENTIFIER_CELL_LABEL;
        cell.value.label = &instruction_label_list->label;

        if (!semantic_analyzer_insert_identifier(analyzer, cell)) {
            report_error(analyzer, "Duplicate label declaration", instruction_label_list->label.label);
        }

        instruction_label_list = instruction_label_list->next;
    }
}

/**
 * Validates guidance labels for uniqueness.
 */
static void validate_guidance_labels(SemanticAnalyzer *analyzer, LabelNodeList *guidance_label_list) {
    IdentifierCell cell;
    while (guidance_label_list != NULL) {
        if (guidance_label_list->label.label != NULL) {
            cell.key = &guidance_label_list->label.label->string;
            cell.type = IDENTIFIER_CELL_LABEL;
            cell.value.label = &guidance_label_list->label;


            if (!semantic_analyzer_insert_identifier(analyzer, cell)) {
                report_error(analyzer,"Duplicate label declaration",guidance_label_list->label.label);
            }
        }

        guidance_label_list = guidance_label_list->next;
    }
}

/**
 * Validates external declarations for uniqueness and conflicts with labels.
 */
static void validate_external_declarations(SemanticAnalyzer *analyzer, ExternalNodeList *external_node_list) {
    IdentifierCell newCell;

    while (external_node_list != NULL) {
        IdentifierCell *existingCell = semantic_analyzer_find_identifier(analyzer, external_node_list->external_node.external_label->string);

        if (existingCell == NULL) {
            newCell.key = &external_node_list->external_node.external_label->string;
            newCell.type = IDENTIFIER_CELL_EXTERNAL;
            newCell.value.external = &external_node_list->external_node;
            semantic_analyzer_insert_identifier(analyzer, newCell);
        } else {
            if (existingCell->type == IDENTIFIER_CELL_LABEL) {
                report_error(analyzer, "Identifier declared as both label and external", external_node_list->external_node.external_label);
            } else {
                report_error(analyzer, "Duplicate external declaration", external_node_list->external_node.external_label);
            }
        }

        external_node_list = external_node_list->next;
    }
}

/**
 * Validates entry declarations against existing identifiers.
 */
static void validate_entry_declarations(SemanticAnalyzer *analyzer, EntryNodeList *entry_node_list) {
    while (entry_node_list != NULL) {
        IdentifierCell *existingCell = semantic_analyzer_find_identifier(analyzer, entry_node_list->entry_node.entry_label->string);

        if (existingCell == NULL) {
            report_error(analyzer,"Entry point has no corresponding label declaration", entry_node_list->entry_node.entry_label);
        } else if (existingCell->type == IDENTIFIER_CELL_EXTERNAL) {
            report_error(analyzer,"Entry point cannot be an external declaration", entry_node_list->entry_node.entry_label);
        }

        entry_node_list = entry_node_list->next;
    }
}

/**
 * Validates a list of labels, including their associated instructions and guidance nodes.
 *
 * @param analyzer Pointer to the Analyzer structure.
 * @param label_list Pointer to the LabelNodeList to be validated.
 */
static void validate_label_list(SemanticAnalyzer *analyzer, LabelNodeList *label_list) {
    while (label_list != NULL) {
        semantic_analyzer_analyze_label(analyzer, label_list->label);
        label_list = label_list->next;
    }
}

