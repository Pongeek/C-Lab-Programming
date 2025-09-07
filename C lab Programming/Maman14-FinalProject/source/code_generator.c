#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "../headers/safe_allocations.h"
#include "../headers/code_generator.h"
#include "../headers/semantic_analyzer.h"
#include "../headers/string_util.h"


#define RED_COLOR   "\x1B[1;91m"
#define RESET_COLOR "\x1B[0m"

#define STARTING_POSITION 100
#define MAX_POSITION 9999

#define TokenTypeToInstrCode(type) (InstructionCode)(type - TOKEN_MOV)
/* 0x7FFF is a mask for 15 bit */
#define IntTo2Complement(value) ((value >= 0)? (value & 0x7FFF) : (((~(-value) & 0x7FFF) + 1) & 0x7FFF))
#define InstrMemToBinary(inst) ( \
((inst).ARE & 0x7) |                  \
((inst).dst & 0xF) << 3 |             \
((inst).src & 0xF) << 7 |             \
((inst).code & 0xF) << 11)
#define InstrOperandMemToBinary(inst) ( \
((inst).ARE & 0x7) |                         \
((inst).other.operand_value & 0xFFF) << 3)


static void generate_instruction_memory(CodeGenerator *generator, SemanticAnalyzer *analyzer,
                                        InstructionNode node, int *position);

static void write_to_object_file(CodeGenerator *generator, int *position, unsigned int toWrite);
static void handle_direct_mode(SemanticAnalyzer *analyzer, CodeGenerator *generator, Token *operand, InstructionOperandMemory *operandMemory, int *position);
static void handle_register_mode(Token *operand, InstructionOperandMemory *operandMemory, bool isDst);
static void handle_operand(SemanticAnalyzer *analyzer, CodeGenerator *generator, Token *operand, AddressingMode mode, InstructionOperandMemory *operandMemory, int *position, bool isDst);
static void generate_instruction(CodeGenerator *generator, int *position, InstructionMemory instrucitionMemory);
static void generate_operand_instruction(CodeGenerator *generator, int *position, InstructionOperandMemory operandMemory);
static AddressingMode determine_addressing_mode(Token *operand_token, bool isDerefrenced);
static unsigned int calculate_label_memory_size(LabelNode label);

void code_generator_initialize(CodeGenerator *generator, Lexer lexer) {
    if (generator == NULL) {
        fprintf(stderr, "Error: Invalid parameters passed to code_generator_initialize\n");
        return;
    }
    generator->entry_file = string_create();
    generator->external_file = string_create();
    generator->object_file = string_create();

    /* Assuming error_handler_initialize doesn't return a value */
    error_handler_initialize(&generator->error_handler, lexer.source_code, lexer.file_path);
}

void code_generator_free(CodeGenerator *generator) {
    if (generator == NULL) {
        return;
    }

    if (generator->entry_file.data != NULL) {
        string_free(generator->entry_file);
    }

    if (generator->external_file.data != NULL) {
        string_free(generator->external_file);
    }

    if (generator->object_file.data != NULL) {
        string_free(generator->object_file);
    }

    error_handler_free(&generator->error_handler);

    /* Set pointers to NULL after freeing */
    generator->entry_file.data = NULL;
    generator->external_file.data = NULL;
    generator->object_file.data = NULL;
}

void code_generator_update_labels(CodeGenerator *generator, TranslationUnit *unit) {
    LabelNodeList *currentLabel;
    unsigned int currentPosition = STARTING_POSITION;

    if (generator == NULL || unit == NULL) {
        fprintf(stderr, "Error: Invalid parameters passed to code_generator_update_labels\n");
        return;
    }

    /* Update instruction labels */
    currentLabel = unit->instruction_label_list;
    while (currentLabel != NULL) {
        currentLabel->label.size = calculate_label_memory_size(currentLabel->label);
        currentLabel->label.position = currentPosition;
        currentPosition += currentLabel->label.size;

        if (currentPosition > MAX_POSITION) {
            TokenError error;
            error.token = *(currentLabel->label.label); /* Assuming label is a Token* */
            error.message = string_create_from_cstr("Memory overflow: Program exceeds maximum allowed size");
            error_handler_add_token_error(&generator->error_handler, OUTPUT_GENERATOR_ERROR_TYPE, error);
            return;
        }

        currentLabel = currentLabel->next;
    }

    /* Update guidance labels */
    currentLabel = unit->guidance_label_list;
    while (currentLabel != NULL) {
        currentLabel->label.size = calculate_label_memory_size(currentLabel->label);
        currentLabel->label.position = currentPosition;
        currentPosition += currentLabel->label.size;

        if (currentPosition > MAX_POSITION) {
            TokenError error;
            error.token = *(currentLabel->label.label); /* Assuming label is a Token* */
            error.message = string_create_from_cstr("Memory overflow: Program exceeds maximum allowed size");
            error_handler_add_token_error(&generator->error_handler, OUTPUT_GENERATOR_ERROR_TYPE, error);
            return;
        }

        currentLabel = currentLabel->next;
    }
}

void generate_entry_file_string(CodeGenerator *generator, SemanticAnalyzer *analyzer, TranslationUnit *unit) {
    EntryNodeList *entryNodeList;
    IdentifierCell *identifierCell;
    char positionBuffer[16]; /* Buffer for converting position to string */

    if (generator == NULL || analyzer == NULL || unit == NULL) {
        fprintf(stderr, "Error: Invalid parameters passed to generate_entry_file_string\n");
        return;
    }

    /* Free the existing entry file string and reinitialize it */
    string_free(generator->entry_file);
    generator->entry_file = string_create();

    for (entryNodeList = unit->entry_list; entryNodeList != NULL; entryNodeList = entryNodeList->next) {
        if (entryNodeList->entry_node.entry_label == NULL || entryNodeList->entry_node.entry_label->string.data ==
            NULL) {
            fprintf(stderr, "Error: Invalid entry node token\n");
            continue;
        }

        identifierCell = semantic_analyzer_find_identifier(analyzer, entryNodeList->entry_node.entry_label->string);

        if (identifierCell != NULL && !identifierCell->has_entry) {
            /* Add the name of entry */
            string_append(&generator->entry_file, entryNodeList->entry_node.entry_label->string);

            /* Convert position to string and add to entry file */
            sprintf(positionBuffer, " %04u\n", identifierCell->value.label->position + STARTING_POSITION);


            string_append_cstr(&generator->entry_file, positionBuffer);

            identifierCell->has_entry = true;
        } else if (identifierCell == NULL) {
            /* Entry not found, report error */
            TokenError error;
            error.token = *(entryNodeList->entry_node.entry_label);
            error.message = string_create_from_cstr("Entry point not defined");
            error_handler_add_token_error(&generator->error_handler, OUTPUT_GENERATOR_ERROR_TYPE, error);
        }
    }
}

void generate_object_and_external_files(CodeGenerator *generator,SemanticAnalyzer *analyzer,TranslationUnit *unit,
int *instruction_lines,int *guidance_lines) {
    /* Pointers to the label lists for instructions and guidance within the translation unit */
    LabelNodeList *instructionLabelList = unit->instruction_label_list;
    LabelNodeList *guidanceLabelList = unit->guidance_label_list;

    /* Initialize node lists for the current label's instructions and guidance */
    InstructionNodeList *instructionNodeList = (instructionLabelList != NULL)
                                                   ? instructionLabelList->label.instruction_list
                                                   : NULL;
    GuidanceNodeList *guidanceNodeList = (guidanceLabelList != NULL)
                                         ? guidanceLabelList->label.guidance_list
                                         : NULL;

    /* Initialize variables for generating code */
    TokenReferenceNode *currentNumber = NULL;  /* Pointer to iterate through numbers in .data directives */
    int position = 100;  /* Memory position starts at 100 */
    int temp;  /* Temporary variable to store integer conversions */
    unsigned int toWrite = 0;  /* Variable to store binary data to write */
    char *buffer = NULL;  /* Temporary string buffer for formatting */
    int index;  /* Index variable for loops */

    /* Process each instruction label in the list */
    while (instructionLabelList != NULL) {
        instructionNodeList = instructionLabelList->label.instruction_list;

        /* Process each instruction node within the current label */
        while (instructionNodeList != NULL) {
            /* Generate and write the binary instruction data, updating the position */
            generate_instruction_memory(generator, analyzer,
                                        instructionNodeList->node,
                                        &position);

            /* Move to the next instruction node in the list */
            instructionNodeList = instructionNodeList->next;
        }

        /* Move to the next instruction label in the list */
        instructionLabelList = instructionLabelList->next;
    }

    /* Calculate the number of instruction lines generated */
    *instruction_lines = position - 100;

    /* Process each guidance label in the list */
    while (guidanceLabelList != NULL) {
        guidanceNodeList = guidanceLabelList->label.guidance_list;

        /* Process each guidance node within the current label */
        while (guidanceNodeList != NULL) {
            /* Handle .data directives */
            if (guidanceNodeList->type == DATA_NODE) {
                currentNumber = guidanceNodeList->node.dataNode.data_numbers;

                /* Write each number in the .data directive to the object file */
                while (currentNumber != NULL) {
                    temp = atoi(currentNumber->token->string.data);  /* Convert string to integer */
                    toWrite = IntTo2Complement(temp);  /* Convert integer to 2's complement */
                    buffer = safe_calloc(10, sizeof(char));  /* Allocate buffer for formatting */

                    /* Format and write the current memory position */
                    sprintf(buffer, "%04d", position);
                    string_append_cstr(&generator->object_file, buffer);  /* Add position to object file */

                    memset(buffer, 0, sizeof(char) * 10);  /* Reset buffer */
                    /* Format and write the binary data as an octal number */
                    sprintf(buffer, " %05o\n", toWrite & 0x7FFF);
                    string_append_cstr(&generator->object_file, buffer);  /* Add memory as an octal number */

                    /* Free the allocated buffer memory */
                    free(buffer);

                    /* Increment the memory position */
                    position++;
                    /* Move to the next number in the .data directive */
                    currentNumber = currentNumber->next;
                }
            }
            /* Handle .string directives */
            else /*if (guidanceNodeList->type == StringNodeKind)*/ {
                /* Write each character of the string (excluding quotes) to the object file */
                for (index = 1;  /* Start after the opening quote */
                     index < string_length(guidanceNodeList->node.stringNode.string_label->string) - 1;
                     index++, position++) {  /* End before the closing quote */

                    temp = (int) string_char_at(guidanceNodeList->node.stringNode.string_label->string, index);
                    toWrite = IntTo2Complement(temp);  /* Convert character to 2's complement */
                    buffer = safe_calloc(10, sizeof(char));  /* Allocate buffer for formatting */

                    /* Format and write the current memory position */
                    sprintf(buffer, "%04d", position);
                    string_append_cstr(&generator->object_file, buffer);  /* Add position to object file */

                    memset(buffer, 0, sizeof(char) * 10);  /* Reset buffer */
                    /* Format and write the binary data as an octal number */
                    sprintf(buffer, " %05o\n", toWrite & 0x7FFF);
                    string_append_cstr(&generator->object_file, buffer);  /* Add memory as an octal number */

                    /* Free the allocated buffer memory */
                    free(buffer);
                }

                /* Add the null terminator (\0) to the string in the object file */
                buffer = safe_calloc(10, sizeof(char));  /* Allocate buffer for formatting */
                sprintf(buffer, "%04d", position);
                string_append_cstr(&generator->object_file, buffer);  /* Add the position */
                memset(buffer, 0, sizeof(char) * 10);  /* Reset buffer */
                sprintf(buffer, " %05o\n", 0);  /* Write the null terminator as an octal number */
                string_append_cstr(&generator->object_file, buffer);  /* Add memory as an octal number */
                free(buffer);

                position++;  /* Increment position for the null terminator */
            }

            /* Move to the next guidance node in the list */
            guidanceNodeList = guidanceNodeList->next;
        }

        /* Move to the next guidance label in the list */
        guidanceLabelList = guidanceLabelList->next;
    }

    /* Calculate the number of guidance lines generated */
    *guidance_lines = position - *instruction_lines - 100;
}

void output_generate(CodeGenerator *generator,
                     SemanticAnalyzer *analyzer,
                     TranslationUnit *unit,
                     char *file_path) {
    FILE *file;  /* File pointer for writing output files */
    char *filePathCurated;  /* String to hold the full file path for output files */
    int instructionLines = 0;  /* Variable to hold the number of instruction lines */
    int guidanceLines = 0;  /* Variable to hold the number of guidance lines */
    char *temp;  /* Temporary buffer for formatting */

    /* Generate the object and external files' content, and count instruction and guidance lines */
    generate_object_and_external_files(generator, analyzer, unit,
                                       &instructionLines, &guidanceLines);

    /* Check if there were no errors and if the external file has content */
    if (generator->error_handler.error_list == NULL && string_length(generator->external_file) != 0) {
        /* Allocate memory for the external file path and create it */
        filePathCurated = safe_calloc((strlen(file_path) + 4) + 1, sizeof(char)); /* ".ext" adds 4 chars */
        strcpy(filePathCurated, file_path);
        strcat(filePathCurated, ".ext");  /* Append the ".ext" extension to the file path */

        file = fopen(filePathCurated, "w");  /* Open the external file for writing */

        if (file == NULL) {
            /* Error handling if the external file could not be created */
            printf("%sOutput Error:%s couldn't create the \"%s.ext\" file.\n",
                   RED_COLOR, RESET_COLOR, file_path);
        } else {
            /* Write the content of the external file buffer to the file */
            fprintf(file, "%s", generator->external_file.data);
            fclose(file);  /* Close the external file after writing */
        }

        /* Free the allocated memory for the file path */
        free(filePathCurated);
    }

    /* Check if there were no errors and if the entry file has content */
    if (generator->error_handler.error_list == NULL && string_length(generator->entry_file) != 0) {
        /* Allocate memory for the entry file path and create it */
        filePathCurated = safe_calloc((strlen(file_path) + 4) + 1, sizeof(char)); /* ".ent" adds 4 chars */
        strcpy(filePathCurated, file_path);
        strcat(filePathCurated, ".ent");  /* Append the ".ent" extension to the file path */

        file = fopen(filePathCurated, "w");  /* Open the entry file for writing */

        if (file == NULL) {
            /* Error handling if the entry file could not be created */
            printf("%sOutput Error:%s couldn't create the \"%s.ent\" file.",
                   RED_COLOR, RESET_COLOR, file_path);
        } else {
            /* Write the content of the entry file buffer to the file */
            fprintf(file, "%s", generator->entry_file.data);
            fclose(file);  /* Close the entry file after writing */
        }

        /* Free the allocated memory for the file path */
        free(filePathCurated);
    }

    /* Check if there were no errors before creating the object file */
    if (generator->error_handler.error_list == NULL) {
        /* Allocate memory for the object file path and create it */
        filePathCurated = safe_calloc((strlen(file_path) + 3) + 1, sizeof(char)); /* ".ob" adds 3 chars */
        strcpy(filePathCurated, file_path);
        strcat(filePathCurated, ".ob");  /* Append the ".ob" extension to the file path */

        file = fopen(filePathCurated, "w");  /* Open the object file for writing */

        if (file == NULL) {
            /* Error handling if the object file could not be created */
            printf("%sOutput Error:%s couldn't create the \"%s.ob\" file.",
                   RED_COLOR, RESET_COLOR, file_path);
        } else {
            /* Allocate memory for a temporary buffer to format the header */
            temp = safe_calloc(20, sizeof(char));
            /* Format the number of instruction and guidance lines as the header of the object file */
            sprintf(temp, " %d %d\n", instructionLines, guidanceLines);
            fprintf(file, "%s", temp);  /* Write the header to the object file */
            fprintf(file, "%s", generator->object_file.data);  /* Write the content of the object file */
            fclose(file);  /* Close the object file after writing */
            free(temp);  /* Free the temporary buffer */
        }

        /* Free the allocated memory for the file path */
        free(filePathCurated);
    }
}

/*  ------------------------- Helper Functions -------------------------- */

/**
 * write_to_object_file
 *
 * This function writes the given binary instruction or operand data to the
 * object file. It formats the data as an octal number and appends it to the
 * generator's object file, along with the current memory position.
 *
 * @param generator A pointer to the CodeGenerator struct, which manages the output files.
 * @param position A pointer to an integer representing the current memory position.
 * @param toWrite The binary data to write to the object file.
 */
static void write_to_object_file(CodeGenerator *generator, int *position, unsigned int toWrite) {
    /* Allocate memory for a temporary string buffer (10 characters) */
    char *tempAtoiS = safe_calloc(10, sizeof(char));

    /* Format the current position as a 4-digit number and append it to the object file */
    sprintf(tempAtoiS, "%04d ", *position);
    string_append_cstr(&generator->object_file, tempAtoiS); /* Add the position to the object file */

    /* Clear the temporary buffer for reuse */
    memset(tempAtoiS, 0, sizeof(char) * 10);

    /* Format the binary data as a 5-digit octal number and append it to the object file */
    sprintf(tempAtoiS, "%05o\n", toWrite & 0x7FFF);
    string_append_cstr(&generator->object_file, tempAtoiS); /* Add the memory as an octal number to the object file */

    /* Free the allocated memory for the temporary buffer */
    free(tempAtoiS);

    /* Increment the memory position counter */
    (*position)++;
}

/**
 * handle_register_mode
 *
 * This function processes an operand that is a register. It sets the appropriate
 * fields in the InstructionOperandMemory structure based on whether the register
 * is being used as a source or destination. The ARE (Absolute/Relative/External)
 * field is set to indicate that this is a direct or indirect register addressing mode.
 *
 * @param operand A pointer to the Token struct representing the operand.
 * @param operandMemory A pointer to the InstructionOperandMemory struct to be populated.
 * @param isDst A boolean indicating if the operand is a destination (true) or source (false).
 */
static void handle_register_mode(Token *operand, InstructionOperandMemory *operandMemory, bool isDst) {
    int regNum;
    /* Set ARE to 4 (binary 0b100) to indicate a register direct/indirect addressing mode */
    operandMemory->ARE = 4;

    /* Extract the register number from the operand (e.g., 'r1' -> 1) */
    regNum = atoi(operand->string.data + 1); /* Skip the 'r' character */

    /* Assign the register number to the appropriate field in operandMemory */
    if (isDst) {
        operandMemory->other.reg.rdst = regNum; /* Set destination register */
        operandMemory->other.reg.rsrc = 0; /* Ensure source register is cleared */
    } else {
        operandMemory->other.reg.rsrc = regNum; /* Set source register */
        operandMemory->other.reg.rdst = 0; /* Ensure destination register is cleared */
    }
}

/**
 * handle_direct_mode
 *
 * This function processes an operand that is a direct address or an external reference.
 * It determines the correct value and ARE (Absolute/Relative/External) bits for the operand
 * based on whether it references a label (direct) or an external symbol.
 *
 * @param analyzer A pointer to the SemanticAnalyzer struct for symbol resolution.
 * @param generator A pointer to the CodeGenerator struct, which manages the output files.
 * @param operand A pointer to the Token struct representing the operand.
 * @param operandMemory A pointer to the InstructionOperandMemory struct to be populated.
 * @param position A pointer to an integer representing the current memory position.
 */
static void handle_direct_mode(SemanticAnalyzer *analyzer, CodeGenerator *generator, Token *operand, InstructionOperandMemory *operandMemory, int *position) {
    /* Look up the operand in the semantic analyzer to determine if it's a label or external symbol */
    IdentifierCell *tempCellP = semantic_analyzer_find_identifier(analyzer, operand->string);
    char *tempAtoiS;

    if (tempCellP != NULL) {
        if (tempCellP->type == IDENTIFIER_CELL_LABEL) {
            /* The operand is a direct label; set ARE to 2 (binary 0b010) and store the label's position */
            operandMemory->ARE = 2;
            operandMemory->other.operand_value = tempCellP->value.label->position;
        } else if (tempCellP->type == IDENTIFIER_CELL_EXTERNAL) {
            /* The operand is an external symbol; set ARE to 1 (binary 0b001) and mark the position for external reference */
            operandMemory->ARE = 1;
            operandMemory->other.operand_value = 0; /* External references have no specific offset */

            /* Record the external reference in the external file with the current position */
            tempAtoiS = safe_calloc(10, sizeof(char));
            string_append(&generator->external_file, operand->string);
            sprintf(tempAtoiS, " %04d\n", *position + 1);
            string_append_cstr(&generator->external_file, tempAtoiS);
            free(tempAtoiS);
        }
    }
}

/**
 * handle_operand
 *
 * This function processes an operand based on its addressing mode and updates the
 * InstructionOperandMemory structure accordingly. It handles immediate values,
 * direct addressing, and register modes, setting the appropriate ARE bits and
 * storing the operand's value in the operandMemory structure.
 *
 * @param analyzer A pointer to the SemanticAnalyzer struct for symbol resolution.
 * @param generator A pointer to the CodeGenerator struct, which manages the output files.
 * @param operand A pointer to the Token struct representing the operand.
 * @param mode The addressing mode of the operand.
 * @param operandMemory A pointer to the InstructionOperandMemory struct to be populated.
 * @param position A pointer to an integer representing the current memory position.
 * @param isDst A boolean indicating if the operand is a destination (true) or source (false).
 */
static void handle_operand(SemanticAnalyzer *analyzer, CodeGenerator *generator, Token *operand, AddressingMode mode, InstructionOperandMemory *operandMemory, int *position, bool isDst) {
    int temp;

    switch (mode) {
        case ADDRESSING_MODE_IMMEDIATE:
            /* Handle immediate mode; set ARE to 4 (binary 0b100) and convert the value to 2's complement */
            operandMemory->ARE = 4;
            temp = atoi(operand->string.data);
            operandMemory->other.operand_value = IntTo2Complement(temp);
            break;

        case ADDRESSING_MODE_DIRECT:
            /* Handle direct addressing mode using the handle_direct_mode function */
            handle_direct_mode(analyzer, generator, operand, operandMemory, position);
            break;

        case ADDRESSING_MODE_DIRECT_REGISTER:
        case ADDRESSING_MODE_INDIRECT_REGISTER:
            /* Handle register modes using the handle_register_mode function */
            handle_register_mode(operand, operandMemory, isDst);
            break;

        default:
            /* Handle unexpected or unsupported addressing modes safely by setting ARE to 0 */
            operandMemory->ARE = 0;
            operandMemory->other.operand_value = 0;
            break;
    }
}

/**
 * generate_instruction
 *
 * This function converts the InstructionMemory structure into its binary
 * representation and writes it to the object file. It increments the position
 * counter after writing the data.
 *
 * @param generator A pointer to the CodeGenerator struct, which manages the output files.
 * @param position A pointer to an integer representing the current memory position.
 * @param instrucitionMemory The InstructionMemory struct containing the encoded instruction.
 */
static void generate_instruction(CodeGenerator *generator, int *position, InstructionMemory instrucitionMemory) {
    /* Convert the instruction memory structure to a binary format */
    unsigned int toWrite = InstrMemToBinary(instrucitionMemory);

    /* Write the binary data to the object file */
    write_to_object_file(generator, position, toWrite);
}

/**
 * generate_operand_instruction
 *
 * This function converts the InstructionOperandMemory structure into its binary
 * representation and writes it to the object file. It increments the position
 * counter after writing the data.
 *
 * @param generator A pointer to the CodeGenerator struct, which manages the output files.
 * @param position A pointer to an integer representing the current memory position.
 * @param operandMemory The InstructionOperandMemory struct containing the encoded operand.
 */
static void generate_operand_instruction(CodeGenerator *generator, int *position, InstructionOperandMemory operandMemory) {
    /* Convert the operand memory structure to a binary format */
    unsigned int toWrite = InstrOperandMemToBinary(operandMemory);

    /* Write the binary data to the object file */
    write_to_object_file(generator, position, toWrite);
}

/**
 * generate_instruction_memory
 *
 * This function generates the binary representation of a machine instruction
 * based on the provided operation and its operands. It handles the encoding
 * of the instruction into memory, determines the addressing modes of the operands,
 * and generates the corresponding memory for both the instruction and its operands.
 *
 * The function supports three cases:
 * 1. Instructions with no operands (e.g., "stop").
 * 2. Instructions with one operand (e.g., "clr r3").
 * 3. Instructions with two operands (e.g., "add r1, r2").
 *
 * The function updates the position counter as it writes the binary representation
 * of the instruction and its operands to the object file. Special handling is
 * provided for instructions where both operands are registers.
 *
 * @param generator A pointer to the CodeGenerator struct, which manages the output files.
 * @param analyzer A pointer to the SemanticAnalyzer struct, used for symbol resolution.
 * @param node The InstructionNode struct containing the operation and operands.
 * @param position A pointer to an integer representing the current memory position.
 */
static void generate_instruction_memory(CodeGenerator *generator,
SemanticAnalyzer *analyzer,InstructionNode node,int *position) {

    /* Initialize the addressing modes for the first and second operands */
    AddressingMode first = ADDRESSING_MODE_IMMEDIATE;
    AddressingMode second = ADDRESSING_MODE_IMMEDIATE;

    /* Initialize the instruction memory and operand memory structures to zero */
    InstructionMemory instrucitionMemory = {0};
    InstructionOperandMemory instrucitionFirstOperandMemory = {0};
    InstructionOperandMemory instrucitionSecondOperandMemory = {0};

    /* Case 1: No operands (e.g., a simple operation like "stop") */
    if (node.first_operand == NULL && node.second_operand == NULL) {
        /* Set ARE to 4 (binary 0b100), indicating an absolute instruction */
        instrucitionMemory.ARE = 4;
        /* Set the instruction code based on the operation type */
        instrucitionMemory.code = TokenTypeToInstrCode(node.operation->type);
        /* Generate and write the instruction to memory */
        generate_instruction(generator, position, instrucitionMemory);
    }
    /* Case 2: One operand (e.g., an operation like "clr r3") */
    else if (node.first_operand != NULL && node.second_operand == NULL) {
        /* Determine the addressing mode of the first operand */
        first = determine_addressing_mode(node.first_operand, node.is_first_operand_derefrenced);
        /* Set ARE to 4 (absolute), and assign the destination addressing mode */
        instrucitionMemory.ARE = 4;
        instrucitionMemory.dst = first;
        /* Set the instruction code based on the operation type */
        instrucitionMemory.code = TokenTypeToInstrCode(node.operation->type);
        /* Generate and write the instruction to memory */
        generate_instruction(generator, position, instrucitionMemory);

        /* Handle the memory for the single operand */
        handle_operand(analyzer, generator, node.first_operand, first, &instrucitionFirstOperandMemory, position, true);
        /* Generate and write the operand's memory to the object file */
        generate_operand_instruction(generator, position, instrucitionFirstOperandMemory);
    }
    /* Case 3: Two operands (e.g., an operation like "add r1, r2") */
    else if (node.first_operand != NULL && node.second_operand != NULL) {
        /* Determine the addressing modes of the first and second operands */
        first = determine_addressing_mode(node.first_operand, node.is_first_operand_derefrenced);
        second = determine_addressing_mode(node.second_operand, node.is_second_operand_derefrenced);
        /* Set ARE to 4 (absolute), assign source and destination addressing modes */
        instrucitionMemory.ARE = 4;
        instrucitionMemory.dst = second;
        instrucitionMemory.src = first;
        /* Set the instruction code based on the operation type */
        instrucitionMemory.code = TokenTypeToInstrCode(node.operation->type);
        /* Generate and write the instruction to memory */
        generate_instruction(generator, position, instrucitionMemory);

        /* Special case: Both operands are registers */
        if ((first == ADDRESSING_MODE_INDIRECT_REGISTER ||
             first == ADDRESSING_MODE_DIRECT_REGISTER) &&
            (second == ADDRESSING_MODE_INDIRECT_REGISTER ||
             second == ADDRESSING_MODE_DIRECT_REGISTER)) {
            /* Handle the memory for register operands (source and destination) */
            handle_register_mode(node.first_operand, &instrucitionFirstOperandMemory, false);
            handle_register_mode(node.second_operand, &instrucitionFirstOperandMemory, true);
            /* Generate and write the combined operand's memory to the object file */
            generate_operand_instruction(generator, position, instrucitionFirstOperandMemory);
        } else {
            /* Handle the first operand */
            handle_operand(analyzer, generator, node.first_operand, first, &instrucitionFirstOperandMemory, position, false);
            generate_operand_instruction(generator, position, instrucitionFirstOperandMemory);

            /* Handle the second operand */
            handle_operand(analyzer, generator, node.second_operand, second, &instrucitionSecondOperandMemory, position, true);
            generate_operand_instruction(generator, position, instrucitionSecondOperandMemory);
        }
    }
}

/**
 * determine_addressing_mode
 *
 * This function determines the addressing mode of a given operand based on its
 * token type and whether it is dereferenced. It supports immediate values,
 * identifiers (which are treated as direct addresses), and registers. For registers,
 * the function distinguishes between direct and indirect addressing modes based on
 * the `isDereferenced` flag.
 *
 * @param operand_token A pointer to the Token struct representing the operand.
 * @param isDerefrenced A boolean indicating if the operand is dereferenced (true) or not (false).
 * @return The determined AddressingMode for the operand.
 */
static AddressingMode determine_addressing_mode(Token *operand_token, bool isDerefrenced) {
    /* Check if the operand is a number (immediate value) */
    if (operand_token->type == TOKEN_NUMBER) {
        return ADDRESSING_MODE_IMMEDIATE; /* Return immediate addressing mode */
    }
    /* Check if the operand is an identifier (e.g., a variable or label) */
    else if (operand_token->type == TOKEN_IDENTIFIER) {
        return ADDRESSING_MODE_DIRECT; /* Return direct addressing mode */
    }
    /* Check if the operand is a register */
    else if (operand_token->type == TOKEN_REGISTER) {
        /* If the register is dereferenced, return indirect register addressing mode */
        if (isDerefrenced) {
            return ADDRESSING_MODE_INDIRECT_REGISTER;
        }
        /* Otherwise, return direct register addressing mode */
        return ADDRESSING_MODE_DIRECT_REGISTER;
    }

    /* Default case (should not normally be reached) returns immediate addressing mode */
    return ADDRESSING_MODE_IMMEDIATE;
}

/**
 * calculate_label_memory_size
 *
 * This function calculates the total memory size required for a given label in the
 * assembly program. It processes both instruction and guidance nodes associated
 * with the label and computes the number of memory words needed to store the
 * instructions and data. The function accounts for the size of instructions, operands,
 * and any string or data guidance nodes.
 *
 * @param label A LabelNode struct representing the label for which to calculate the memory size.
 * @return The total number of memory words required for the label.
 */
static unsigned int calculate_label_memory_size(LabelNode label) {
    /* Pointers to the instruction and guidance node lists associated with the label */
    InstructionNodeList *instructionNodeList = label.instruction_list;
    GuidanceNodeList *guidanceNodeList = label.guidance_list;
    TokenReferenceNode *numbers = NULL; /* Pointer to iterate over data numbers in guidance nodes */

    /* Initialize addressing modes for operands (default to immediate mode) */
    AddressingMode first = ADDRESSING_MODE_IMMEDIATE;
    AddressingMode second = ADDRESSING_MODE_IMMEDIATE;

    /* Initialize the output size to 0 */
    int totalSize = 0;

    /* Iterate through all instructions associated with the label */
    while (instructionNodeList != NULL) {
        totalSize++; /* Add 1 for the memory instruction itself */

        /* Reset addressing modes to immediate for each instruction */
        first = ADDRESSING_MODE_IMMEDIATE;
        second = ADDRESSING_MODE_IMMEDIATE;

        /* Determine addressing mode for the first operand, if it exists */
        if (instructionNodeList->node.first_operand != NULL) {
            first = determine_addressing_mode(instructionNodeList->node.first_operand,
                                                instructionNodeList->node.is_first_operand_derefrenced);
        }

        /* Determine addressing mode for the second operand, if it exists */
        if (instructionNodeList->node.second_operand != NULL) {
            second = determine_addressing_mode(instructionNodeList->node.second_operand,
                                                 instructionNodeList->node.is_second_operand_derefrenced);
        }

        /* Special case: both operands are registers that can fit in one memory word */
        if ((first == ADDRESSING_MODE_INDIRECT_REGISTER || first == ADDRESSING_MODE_DIRECT_REGISTER) &&
            (second == ADDRESSING_MODE_INDIRECT_REGISTER || second == ADDRESSING_MODE_DIRECT_REGISTER)) {
            totalSize++; /* Add 1 for the combined register operands */
        } else {
            /* Add memory for the first operand, if it exists */
            if (instructionNodeList->node.first_operand != NULL) {
                totalSize++;
            }
            /* Add memory for the second operand, if it exists */
            if (instructionNodeList->node.second_operand != NULL) {
                totalSize++;
            }
        }

        /* Move to the next instruction in the list */
        instructionNodeList = instructionNodeList->next;
    }

    /* Iterate through all guidance nodes associated with the label */
    while (guidanceNodeList != NULL) {
        /* Handle string guidance nodes */
        if (guidanceNodeList->type == STRING_NODE) {
            /* Add the length of the string plus one for the null terminator, minus two for the quotes */
            totalSize += string_length(guidanceNodeList->node.stringNode.string_label->string) + 1 - 2;
        }

        /* Handle data guidance nodes (e.g., .data) */
        if (guidanceNodeList->type == DATA_NODE) {
            numbers = guidanceNodeList->node.dataNode.data_numbers;
            /* Iterate through all numbers in the data node and add 1 for each */
            while (numbers != NULL) {
                totalSize++; /* Each number occupies one memory word */
                numbers = numbers->next;
            }
        }

        /* Move to the next guidance node in the list */
        guidanceNodeList = guidanceNodeList->next;
    }

    /* Return the total memory size calculated for the label */
    return totalSize;
}

