#include "../headers/safe_allocations.h"
#include "../headers/nodes.h"
#include "../headers/parser.h"

static InstructionOperand parse_operand(TranslationUnit *unit, bool *has_error);

static void report_error(TranslationUnit *unit, const char *message, Token *token);

static bool is_instruction_token(TokenType type);

static void append_token_to_list(TokenReferenceNode **list, Token *token);


void parser_initialize_translation_unit(TranslationUnit *unit, Lexer lexer) {
    if (unit == NULL) {
        fprintf(stderr, "Error: Null translation unit pointer\n");
        return;
    }

    /* Initialize all pointers to NULL */
    unit->external_list = NULL;
    unit->entry_list = NULL;
    unit->instruction_label_list = NULL;
    unit->guidance_label_list = NULL;

    /* Set the tokens from the lexer */
    unit->tokens = lexer.token_list;

    /* Initialize the error handler */
    error_handler_initialize(&unit->error_handler, lexer.source_code, lexer.file_path);

    /* Optional: Print debug information */
#ifdef DEBUG
    printf("Translation unit initialized with %d tokens\n", count_tokens(lexer.tokens));
#endif
}

void parser_free_translation_unit(TranslationUnit *unit) {
    void *temp;
    if (unit == NULL)
        return;

    /* Free externalNodes */
    while (unit->external_list) {
        temp = unit->external_list->next;
        free(unit->external_list);
        unit->external_list = temp;
    }

    /* Free entryNodes */
    while (unit->entry_list) {
        temp = unit->entry_list->next;
        free(unit->entry_list);
        unit->entry_list = temp;
    }

    /* Free instructionLabels */
    while (unit->instruction_label_list) {
        parser_free_instruction_list(unit->instruction_label_list->label.instruction_list);
        temp = unit->instruction_label_list->next;
        free(unit->instruction_label_list);
        unit->instruction_label_list = temp;
    }

    /* Free guidanceLabels */
    while (unit->guidance_label_list) {
        parser_free_guidance_list(unit->guidance_label_list->label.guidance_list);
        temp = unit->guidance_label_list->next;
        free(unit->guidance_label_list);
        unit->guidance_label_list = temp;
    }

    /* Free the error handler */
    error_handler_free(&unit->error_handler);

    /* Set all pointers to NULL after freeing */
    unit->external_list = NULL;
    unit->entry_list = NULL;
    unit->instruction_label_list = NULL;
    unit->guidance_label_list = NULL;
}

void parser_move_to_end_of_line(TranslationUnit *unit) {
    if (unit == NULL || unit->tokens == NULL) {
        return; /* Handle null pointer or empty token list */
    }

    while (unit->tokens && unit->tokens->token.type != TOKEN_EOL &&
           unit->tokens->token.type != TOKEN_EOFT) {
        unit->tokens = unit->tokens->next;
    }

    /* Move past the EOL token if it's not the end of file */
    if (unit->tokens && unit->tokens->token.type == TOKEN_EOL) {
        unit->tokens = unit->tokens->next;
    }
}

DataNode parse_data_directive_guidance(TranslationUnit *unit) {
    DataNode data_node = {NULL};
    TokenError error;
    Token default_token;

    /* Initialize default_token */
    memset(&default_token, 0, sizeof(Token));

    if (unit == NULL || unit->tokens == NULL) {
        error.message = string_create_from_cstr("Expected .data directive, but got null pointer");
        error.token = default_token; /* Initialize with a default token */
        if (unit != NULL) {
            error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        } else {
            /* Handle the case where translationUnit is NULL */
            fprintf(stderr, "Error: Null translation unit in parse_data_directive\n");
        }
        data_node.has_parser_error = true;
        return data_node;
    }

    if (unit->tokens->token.type != TOKEN_DATA_INS) {
        error.message = string_create_from_cstr("Expected .data directive");
        error.token = unit->tokens->token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        return data_node;
    }

    unit->tokens = unit->tokens->next;

    while (unit->tokens && unit->tokens->token.type != TOKEN_EOFT) {
        if (unit->tokens->token.type == TOKEN_NUMBER) {
            append_token_to_list(&data_node.data_numbers, &unit->tokens->token);
            unit->tokens = unit->tokens->next;

            if (unit->tokens == NULL) break;

            if (unit->tokens->token.type == TOKEN_COMMA) {
                unit->tokens = unit->tokens->next;
            } else if (unit->tokens->token.type == TOKEN_EOL) {
                unit->tokens = unit->tokens->next;
                break;
            } else if (unit->tokens->token.type != TOKEN_EOFT) {
                error.message = string_create_from_cstr(
                    "Expected comma or end of line after number in .data directive");
                error.token = unit->tokens->token;
                error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
                parser_move_to_end_of_line(unit);
                data_node.has_parser_error = true;
                return data_node;
            }
        } else {
            error.message = string_create_from_cstr("Expected number in .data directive");
            error.token = unit->tokens->token;
            error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
            parser_move_to_end_of_line(unit);
            data_node.has_parser_error = true;
            return data_node;
        }
    }

    if (data_node.data_numbers == NULL) {
        error.message = string_create_from_cstr("No numbers found in .data directive");
        error.token = unit->tokens ? unit->tokens->token : default_token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        data_node.has_parser_error = true;
    }

    return data_node;
}


void parser_free_directive_guidance(DataNode data_node) {
    TokenReferenceNode *head = data_node.data_numbers;

    while (head != NULL) {
        TokenReferenceNode *temp = head;
        head = head->next;
        free(temp);
    }
}

StringNode parse_string_directive_guidance(TranslationUnit *unit) {
    StringNode string_node = {NULL};
    TokenError error;
    Token default_token;

    /* Initialize default_token */
    memset(&default_token, 0, sizeof(Token));

    if (unit == NULL || unit->tokens == NULL) {
        error.message = string_create_from_cstr("Expected .string directive, but got null pointer");
        error.token = default_token; /* Initialize with a default token */
        if (unit != NULL) {
            error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        } else {
            /* Handle the case where translationUnit is NULL */
            fprintf(stderr, "Error: Null translation unit in parse_string_directive\n");
        }
        return string_node;
    }

    if (unit->tokens->token.type != TOKEN_STRING_INS) {
        error.message = string_create_from_cstr("Expected .string directive");
        error.token = unit->tokens->token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        string_node.has_parser_error = true;
        return string_node;
    }

    unit->tokens = unit->tokens->next; /* Move past .string token */

    if (unit->tokens == NULL || unit->tokens->token.type != TOKEN_STRING) {
        error.message = string_create_from_cstr("Expected string after .string directive");
        error.token = unit->tokens ? unit->tokens->token : default_token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        string_node.has_parser_error = true;
        return string_node;
    }

    string_node.string_label = &unit->tokens->token;
    unit->tokens = unit->tokens->next; /* Move past string token */

    /* Check for end of line */
    if (unit->tokens != NULL &&
        unit->tokens->token.type != TOKEN_EOL &&
        unit->tokens->token.type != TOKEN_EOFT) {
        error.message = string_create_from_cstr("Unexpected tokens after string in .string directive");
        error.token = unit->tokens->token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        string_node.has_parser_error = true;
    } else if (unit->tokens != NULL && unit->tokens->token.type == TOKEN_EOL) {
        unit->tokens = unit->tokens->next; /* Move past EOL token */
    }

    return string_node;
}

GuidanceNodeList *parser_parse_guidance_list(TranslationUnit *unit) {
    GuidanceNodeList *guidance_node_list = NULL;
    GuidanceNodeList **guidanceListLast = &guidance_node_list; /* Pointer to the last node in the guidanceList */

    while (unit->tokens != NULL && unit->tokens->token.type != TOKEN_EOFT) {
        if (unit->tokens->token.type == TOKEN_EOL) {
            unit->tokens = unit->tokens->next; /* Skip empty lines */
        } else if (unit->tokens->token.type == TOKEN_DATA_INS ||
                   unit->tokens->token.type == TOKEN_STRING_INS) {
            /* Parse .data or .string directive */
            GuidanceNodeList *newNode;

            if (guidance_node_list == NULL) {
                /* First guidance directive encountered */
                newNode = guidance_node_list = malloc(sizeof(GuidanceNodeList));
            } else {
                /* Append to existing list */
                newNode = (*guidanceListLast)->next = malloc(sizeof(GuidanceNodeList));
                guidanceListLast = &((*guidanceListLast)->next);
            }

            if (newNode == NULL) {
                /* Handle memory allocation failure */
                fprintf(stderr, "Memory allocation failed in parse_guidance_directives\n");
                /* Consider adding proper error handling and cleanup here */
                return guidance_node_list;
            }

            newNode->next = NULL;

            /* Parse specific directive type */
            if (unit->tokens->token.type == TOKEN_DATA_INS) {
                newNode->type = DATA_NODE;
                newNode->node.dataNode = parse_data_directive_guidance(unit);
            } else {
                newNode->type = STRING_NODE;
                newNode->node.stringNode = parse_string_directive_guidance(unit);
            }

            guidanceListLast = &newNode;
        } else {
            /* Non-guidance token encountered, end of guidance section */
            break;
        }
    }

    return guidance_node_list;
}

void parser_free_guidance_list(GuidanceNodeList *guidance_list) {
    GuidanceNodeList *current;
    if (guidance_list == NULL) {
        return; /* Nothing to free if the list is already NULL */
    }

    while (guidance_list != NULL) {
        current = guidance_list;
        guidance_list = guidance_list->next;

        /* Free the specific guidance node based on its type */
        if (current->type == DATA_NODE) {
            parser_free_directive_guidance(current->node.dataNode);
        }

        /* Free the current GuidanceNodeList node */
        free(current);
    }
}

void parser_free_sentences(AssemblyStatementList *statement_list) {
    AssemblyStatementList *copy = statement_list;

    while (statement_list != NULL) {
        copy = statement_list->next;

        if (statement_list->node.type == DATA_NODE)
            parser_free_directive_guidance(statement_list->node.node.data_node);

        free(statement_list);
        statement_list = copy;
    }
}


InstructionNode parser_parse_instruction(TranslationUnit *unit) {
    InstructionNode instruction = {0}; /* Initialize all fields to 0/NULL */
    TokenError error;
    bool error_in_operand = false;
    InstructionOperand first_operand;
    InstructionOperand second_operand;
    Token default_token;

    /* Initialize default_token */
    memset(&default_token, 0, sizeof(Token));

    /* Check if the current token is a valid instruction */
    if (unit->tokens == NULL || !is_instruction_token(unit->tokens->token.type)) {
        error.message = string_create_from_cstr("Expected an instruction");
        error.token = unit->tokens ? unit->tokens->token : default_token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        instruction.has_parser_error = true;
        return instruction;
    }

    instruction.operation = &unit->tokens->token;
    unit->tokens = unit->tokens->next; /* Move past the instruction token */

    /* Check if it's a zero-operand instruction */
    if (unit->tokens == NULL || unit->tokens->token.type == TOKEN_EOL ||
        unit->tokens->token.type == TOKEN_EOFT) {
        return instruction;
    }

    /* Parse first operand */
    first_operand = parse_operand(unit, &error_in_operand);
    if (error_in_operand) {
        instruction.has_parser_error = true;
        return instruction;
    }

    instruction.first_operand = first_operand.operand;
    instruction.is_first_operand_derefrenced = first_operand.is_dereferenced;

    /* Check if it's a one-operand instruction */
    if (unit->tokens == NULL || unit->tokens->token.type == TOKEN_EOL ||
        unit->tokens->token.type == TOKEN_EOFT) {
        instruction.has_parser_error |= error_in_operand;
        return instruction;
    }

    /* Check for comma separator between operands */
    if (unit->tokens->token.type != TOKEN_COMMA) {
        error.message = string_create_from_cstr("Expected comma between operands");
        error.token = unit->tokens->token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        instruction.has_parser_error = true;
        return instruction;
    }
    unit->tokens = unit->tokens->next; /* Move past the comma */

    /* Parse second operand */
    second_operand = parse_operand(unit, &error_in_operand);
    if (error_in_operand) {
        instruction.has_parser_error = true;
        return instruction;
    }

    instruction.second_operand = second_operand.operand;
    instruction.is_second_operand_derefrenced = second_operand.is_dereferenced;

    /* Check for end of line */
    if (unit->tokens && unit->tokens->token.type != TOKEN_EOL &&
        unit->tokens->token.type != TOKEN_EOFT) {
        error.message = string_create_from_cstr("Expected end of line after instruction");
        error.token = unit->tokens->token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        instruction.has_parser_error = true;
    }

    instruction.has_parser_error |= error_in_operand;
    return instruction;
}

InstructionNodeList *parser_parse_instruction_list(TranslationUnit *unit) {
    InstructionNodeList *instructionList = NULL;
    InstructionNodeList **instructionListLast = &instructionList; /* Pointer to the last node in the instructionList */

    while (unit->tokens != NULL && unit->tokens->token.type != TOKEN_EOFT) {
        if (unit->tokens->token.type == TOKEN_EOL) {
            unit->tokens = unit->tokens->next; /* Skip empty lines */
        } else if (is_instruction_token(unit->tokens->token.type)) {
            /* Parse instruction */
            InstructionNodeList *newNode;

            if (instructionList == NULL) {
                /* First instruction encountered */
                newNode = instructionList = malloc(sizeof(InstructionNodeList));
            } else {
                /* Append to existing list */
                newNode = (*instructionListLast)->next = malloc(sizeof(InstructionNodeList));
                instructionListLast = &((*instructionListLast)->next);
            }

            if (newNode == NULL) {
                /* Handle memory allocation failure */
                fprintf(stderr, "Memory allocation failed in parse_instructions\n");
                /* Consider adding proper error handling and cleanup here*/
                return instructionList;
            }

            newNode->next = NULL;
            newNode->node = parser_parse_instruction(unit);

            instructionListLast = &newNode;
        } else {
            /* Non-instruction token encountered, end of instruction section */
            break;
        }
    }

    return instructionList;
}

void parser_free_instruction_list(InstructionNodeList *instruction_list) {
    InstructionNodeList *current;

    if (instruction_list == NULL) {
        return; /* Nothing to free if the list is already NULL */
    }


    while (instruction_list != NULL) {
        current = instruction_list;
        instruction_list = instruction_list->next;

        /* Free any dynamically allocated memory within the instruction node if necessary */
        /* For example, if operands are dynamically allocated:
        if (current->node.firstOperand) free(current->node.firstOperand);
        if (current->node.secondOperand) free(current->node.secondOperand);
        */

        /* Free the current InstructionNodeList node */
        free(current);
    }
}

EntryNode parser_parse_entry(TranslationUnit *translation_unit) {
    EntryNode node = {NULL};
    TokenError error;

    /* Check for .entry directive */
    if (translation_unit->tokens == NULL || translation_unit->tokens->token.type != TOKEN_ENTRY_INS) {
        error.message = string_create_from_cstr("Expected .entry directive");
        error.token = translation_unit->tokens->token;
        error_handler_add_token_error(&translation_unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(translation_unit);
        node.has_parser_error = true;
        return node;
    }

    translation_unit->tokens = translation_unit->tokens->next; /* Move past .entry token */

    /* Check for identifier after .entry */
    if (translation_unit->tokens == NULL || translation_unit->tokens->token.type != TOKEN_IDENTIFIER) {
        error.message = string_create_from_cstr("Expected identifier after .entry directive");
        error.token = translation_unit->tokens->token;
        error_handler_add_token_error(&translation_unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(translation_unit);
        node.has_parser_error = true;
        return node;
    }

    node.entry_label = &translation_unit->tokens->token;
    translation_unit->tokens = translation_unit->tokens->next; /* Move past identifier token */

    /* Check for end of line */
    if (translation_unit->tokens && translation_unit->tokens->token.type != TOKEN_EOL &&
        translation_unit->tokens->token.type != TOKEN_EOFT) {
        error.message = string_create_from_cstr("Unexpected tokens after .entry identifier");
        error.token = translation_unit->tokens->token;
        error_handler_add_token_error(&translation_unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(translation_unit);
        node.has_parser_error = true;
    } else if (translation_unit->tokens && translation_unit->tokens->token.type == TOKEN_EOL) {
        translation_unit->tokens = translation_unit->tokens->next; /* Move past EOL token */
    }

    return node;
}

ExternalNode parser_parse_external(TranslationUnit *unit) {
    ExternalNode node = {NULL};
    TokenError error;
    Token default_token;

    /* Initialize default_token */
    memset(&default_token, 0, sizeof(Token));

    /* Check for .extern directive */
    if (unit->tokens == NULL || unit->tokens->token.type != TOKEN_EXTERN_INS) {
        error.message = string_create_from_cstr("Expected .extern directive");
        error.token = unit->tokens ? unit->tokens->token : default_token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        node.has_parser_error = true;
        return node;
    }

    unit->tokens = unit->tokens->next; /*  past .extern token */

    /* Check for identifier after .extern */
    if (unit->tokens == NULL || unit->tokens->token.type != TOKEN_IDENTIFIER) {
        error.message = string_create_from_cstr("Expected identifier after .extern directive");
        error.token = unit->tokens ? unit->tokens->token : default_token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        node.has_parser_error = true;
        return node;
    }

    node.external_label = &unit->tokens->token;
    unit->tokens = unit->tokens->next; /* Move past identifier token */

    /* Check for end of line */
    if (unit->tokens && unit->tokens->token.type != TOKEN_EOL &&
        unit->tokens->token.type != TOKEN_EOFT) {
        error.message = string_create_from_cstr("Unexpected tokens after .extern identifier");
        error.token = unit->tokens->token;
        error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
        parser_move_to_end_of_line(unit);
        node.has_parser_error = true;
    } else if (unit->tokens && unit->tokens->token.type == TOKEN_EOL) {
        unit->tokens = unit->tokens->next; /* Move past EOL token */
    }

    return node;
}


LabelNode parse_labeled_statement(TranslationUnit *unit) {
    LabelNode label = {0}; /* Initialize all fields to 0/NULL */
    TokenError error;
    bool label_identifier_found = false;
    Token default_token;

    /* Initialize default_token */
    memset(&default_token, 0, sizeof(Token));

    /* Check for label identifier */
    if (unit->tokens != NULL && unit->tokens->token.type == TOKEN_IDENTIFIER) {
        label.label = &unit->tokens->token;
        unit->tokens = unit->tokens->next; /* Move over the label identifier */

        /* Check for colon after label identifier */
        if (unit->tokens == NULL || unit->tokens->token.type != TOKEN_COLON) {
            error.message = string_create_from_cstr("No colon found after label identifier");
            error.token = unit->tokens ? unit->tokens->token : default_token;
            error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
            parser_move_to_end_of_line(unit);
            return label;
        }

        /* Check if colon is immediately after label identifier */
        if (label.label->index + string_length(label.label->string) != unit->tokens->token.index) {
            error.message = string_create_from_cstr("The colon should be immediately after the label identifier");
            error.token = unit->tokens->token;
            error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
            parser_move_to_end_of_line(unit);
            return label;
        }

        unit->tokens = unit->tokens->next; /* Move over the colon token */

        /* Skip EOL tokens after the colon */
        while (unit->tokens != NULL && unit->tokens->token.type != TOKEN_EOFT) {
            if (unit->tokens->token.type == TOKEN_EOL)
                unit->tokens = unit->tokens->next;
            else
                break;
        }

        label_identifier_found = true;
    }

    /* Parse instruction or guidance after label */
    if (unit->tokens != NULL) {
        if (is_instruction_token(unit->tokens->token.type)) {
            if (!label_identifier_found) {
                error.message = string_create_from_cstr("An instruction was found here but no label identifier, "
                    "please add a label identifier");
                error.token = unit->tokens->token;
                error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
                parser_move_to_end_of_line(unit);
                return label;
            }
            label.instruction_list = parser_parse_instruction_list(unit);
        } else if (unit->tokens->token.type == TOKEN_STRING_INS ||
                   unit->tokens->token.type == TOKEN_DATA_INS) {
            label.guidance_list = parser_parse_guidance_list(unit);
        } else {
            error.message = string_create_from_cstr("No instruction/guidance was found here");
            error.token = unit->tokens->token;
            error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
            parser_move_to_end_of_line(unit);
        }
    }

    return label;
}

void parse_translation_unit_content(TranslationUnit *unit) {
    ExternalNodeList *external_node_list = NULL;
    ExternalNodeList **external_node_list_last = &external_node_list;
    EntryNodeList *entry_node_list = NULL;
    EntryNodeList **entry_node_list_last = &entry_node_list;
    LabelNodeList *instruction_label_list = NULL;
    LabelNodeList **instruction_label_list_last = &instruction_label_list;
    LabelNodeList *guidance_label_list = NULL;
    LabelNodeList **guidance_label_list_last = &guidance_label_list;
    TokenError error;
    LabelNode label = {0}; /* Initialize all fields to 0/NULL */
    bool was_label_found = false;
    /*printf("Debug: Starting parsing of translation unit...\n");*/


    while (unit->tokens != NULL && unit->tokens->token.type != TOKEN_EOFT) {
        /*printf("Debug: Processing token of type %d at index %d\n", unit->tokens->token.type, unit->tokens->token.index);*/

        if (unit->tokens->token.type == TOKEN_EOL) {
            unit->tokens = unit->tokens->next; /* Skip empty lines */
        } else if (unit->tokens->token.type == TOKEN_EXTERN_INS) {
            /* Parse external directive */
            ExternalNodeList *new_node = malloc(sizeof(ExternalNodeList));
            if (new_node == NULL) {
                fprintf(stderr, "Memory allocation failed in parse_translation_unit\n");
                /*Consider adding proper error handling and cleanup here*/
                return;
            }
            new_node->next = NULL;
            new_node->external_node = parser_parse_external(unit);
            *external_node_list_last = new_node;
            external_node_list_last = &new_node->next;
        } else if (unit->tokens->token.type == TOKEN_ENTRY_INS) {
            /* Parse entry directive */
            EntryNodeList *newNode = malloc(sizeof(EntryNodeList));
            if (newNode == NULL) {
                fprintf(stderr, "Memory allocation failed in parse_translation_unit\n");
                /* Consider adding proper error handling and cleanup here */
                return;
            }
            newNode->next = NULL;
            newNode->entry_node = parser_parse_entry(unit);
            *entry_node_list_last = newNode;
            entry_node_list_last = &newNode->next;
        } else if (unit->tokens->token.type == TOKEN_IDENTIFIER ||
                   unit->tokens->token.type == TOKEN_DATA_INS ||
                   unit->tokens->token.type == TOKEN_STRING_INS) {
            /* Parse labeled statement (instruction or guidance) */
            label = parse_labeled_statement(unit);
            was_label_found = true;
        } else if (unit->tokens->token.type == TOKEN_MACR ||
                   unit->tokens->token.type == TOKEN_ENDMACR) {
            /*printf("Debug: Encountered macro-related token '%s'. Skipping...\n", unit->tokens->token.string.data);*/
            /* Skip macro-related tokens if they somehow made it through preprocessing */
            /* unit->tokens = unit->tokens->next;*/
            parser_move_to_end_of_line(unit);
        } else {
            /*printf("Debug: Unexpected token encountered: type %d at index %d\n",
                   unit->tokens->token.type, unit->tokens->token.index);*/
            /* Unexpected token */
            error.message = string_create_from_cstr("Unexpected token: expected label, .extern, or .entry");
            error.token = unit->tokens->token;
            error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
            parser_move_to_end_of_line(unit);
        }

        /* Add the label to the corresponding list */
        if (was_label_found) {
            LabelNodeList *newNode = malloc(sizeof(LabelNodeList));
            if (newNode == NULL) {
                fprintf(stderr, "Memory allocation failed in parse_translation_unit\n");
                /* Consider adding proper error handling and cleanup here */
                return;
            }
            newNode->next = NULL;
            newNode->label = label;

            if (label.instruction_list != NULL) {
                *instruction_label_list_last = newNode;
                instruction_label_list_last = &newNode->next;
            } else if (label.guidance_list != NULL) {
                *guidance_label_list_last = newNode;
                guidance_label_list_last = &newNode->next;
            }

            was_label_found = false;
            memset(&label, 0, sizeof(LabelNode)); /* Reset label for next iteration */
        }
    }

    unit->external_list = external_node_list;
    unit->entry_list = entry_node_list;
    unit->instruction_label_list = instruction_label_list;
    unit->guidance_label_list = guidance_label_list;
    /*printf("Debug: Finished parsing translation unit.\n");*/
}


/**
 * Add a token reference to the end of the token reference list
 *
 * @param list the reference to the token reference list.
 * @param token the token reference.
*/
static void append_token_to_list(TokenReferenceNode **list, Token *token) {
    TokenReferenceNode *new_node;
    new_node = safe_malloc(sizeof(TokenReferenceNode));

    if (list == NULL || token == NULL)
        return;


    new_node->token = token;
    new_node->next = NULL;

    if (*list == NULL) {
        *list = new_node;
    } else {
        TokenReferenceNode *current = *list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

/**
 * Parse instruction operand
 *
 * @param unit the translation unit.
 * @param error_occurred pointer to a boolean flag indicating if an error occurred.
 * @return the instruction operand
 */

static InstructionOperand parse_operand(TranslationUnit *unit, bool *error_occurred) {
    InstructionOperand operand = {NULL, false};
    TokenNode *current;
    TokenType type;

    *error_occurred = true;

    if (unit->tokens == NULL)
        return operand;

    current = unit->tokens;
    type = current->token.type;

    if (type == TOKEN_HASHTAG) {
        /* Handle immediate operand */
        if (current->next == NULL || current->next->token.type != TOKEN_NUMBER) {
            report_error(unit, "Expected number after '#'", &current->token);
            return operand;
        }
        operand.operand = &current->next->token;
        unit->tokens = current->next->next;
        *error_occurred = false;
    } else {
        /* Handle register or identifier operand */
        if (type == TOKEN_STAR) {
            operand.is_dereferenced = true;
            current = current->next;
            if (current == NULL) {
                report_error(unit, "Expected operand after '*'", &unit->tokens->token);
                return operand;
            }
        }

        if (current->token.type == TOKEN_REGISTER || current->token.type == TOKEN_IDENTIFIER) {
            operand.operand = &current->token;
            unit->tokens = current->next;
            *error_occurred = false;
        } else {
            report_error(unit, "Expected register or identifier", &current->token);
        }
    }
    return operand;
}

static bool is_instruction_token(TokenType type) {
    switch (type) {
        case TOKEN_MOV:
        case TOKEN_CMP:
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_LEA:
        case TOKEN_CLR:
        case TOKEN_NOT:
        case TOKEN_INC:
        case TOKEN_DEC:
        case TOKEN_JMP:
        case TOKEN_BNE:
        case TOKEN_RED:
        case TOKEN_PRN:
        case TOKEN_JSR:
        case TOKEN_RTS:
        case TOKEN_STOP:
            return true;
        default:
            return false;
    }
}

static void report_error(TranslationUnit *unit, const char *message, Token *token) {
    TokenError error;
    error.message = string_create_from_cstr(message);
    error.token = *token;
    error_handler_add_token_error(&unit->error_handler, PARSER_ERROR_TYPE, error);
    parser_move_to_end_of_line(unit);
}

