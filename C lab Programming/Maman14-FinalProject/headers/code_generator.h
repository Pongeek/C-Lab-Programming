#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "lexer.h"
#include "semantic_analyzer.h"
#include "nodes.h"

typedef struct InstructionMemory {
    /* Represents the layout of an instruction in memory as it appears in the object file. */
    unsigned int ARE: 3;  /* The ARE field (3 bits): Specifies if the instruction is Absolute, Relocatable, or External. */
    unsigned int dst: 4;  /* Destination Addressing Mode (4 bits): Specifies the addressing mode for the destination operand. */
    unsigned int src: 4;  /* Source Addressing Mode (4 bits): Specifies the addressing mode for the source operand. */
    unsigned int code : 4; /* Operation Code (4 bits): Represents the specific operation or instruction code. */
    /* The total size of this struct is 3 (ARE) + 4 (dst) + 4 (src) + 4 (code) = 15 bits. */
} InstructionMemory;

typedef struct InstructionOperandMemory {
    /* Represents the layout of an instruction's operand in memory as it appears in the object file. */
    unsigned int ARE: 3;  /* The ARE field (3 bits): Specifies if the operand is Absolute, Relocatable, or External. */

    union {
        unsigned int operand_value: 12;  /* Operand Value (12 bits): Used when the operand is an immediate value, label, or external reference. */

        /* Used when the operand is a register or involves multiple registers. */
        struct {
            unsigned int rdst: 3;  /* Destination Register (3 bits): Specifies the register used as the destination operand. */
            unsigned int rsrc: 3;  /* Source Register (3 bits): Specifies the register used as the source operand. */
        } reg;
    } other;
    /* The total size of this struct is 3 (ARE) + max(12 (full), 3 (rdst) + 3 (rsrc)) = 15 bits. */
} InstructionOperandMemory;


/**
 * Enum representing the instruction codes.
 */
typedef enum InstructionCode {
    MOV_CODE,
    CMP_CODE,
    ADD_CODE,
    SUB_CODE,
    LEA_CODE,
    CLR_CODE,
    NOT_CODE,
    INC_CODE,
    DEC_CODE,
    JMP_CODE,
    BNE_CODE,
    RED_CODE,
    PRN_CODE,
    JSR_CODE,
    RTS_CODE,
    STOP_CODE
}InstructionCode;

/**
 * Structure representing the code generator.
 */
typedef struct CodeGenerator {
    String entry_file; /* the .ent file as string */
    String external_file; /* the .ext file as string */
    String object_file; /* the .ob file as string */

    ErrorHandler error_handler; /* the error handler of the translation unit */
}CodeGenerator;

/**
 * Initialize the code generator.
 *
 * This function initializes the CodeGenerator structure, setting up the entry file,
 * external file, and object file strings. It also initializes the error handler
 * using the provided lexer information.
 *
 * @param generator Pointer to the CodeGenerator structure to be initialized.
 * @param lexer Pointer to the Lexer containing input and file path information.
 * @return int Returns 0 on success, -1 on failure (e.g., if generator or lexer is NULL).
 */
void code_generator_initialize(CodeGenerator * generator, Lexer lexer);

/**
 * Free the resources used by the CodeGenerator.
 *
 * This function deallocates all memory used by the CodeGenerator structure,
 * including the entry file, external file, and object file strings. It also
 * frees any resources used by the error handler.
 *
 * @param generator Pointer to the CodeGenerator structure to be freed.
 */
void code_generator_free(CodeGenerator * generator);

/**
 * Update the size and position of labels in the translation unit.
 *
 * This function calculates and updates the size and position of each label
 * in the instruction and guidance label lists of the translation unit.
 * It also performs error checking and reports any issues encountered.
 *
 * @param generator Pointer to the CodeGenerator structure.
 * @param unit Pointer to the TranslationUnit structure.
 */
void code_generator_update_labels(CodeGenerator * generator, TranslationUnit * unit);

/**
 * Generate the entry file string for the code generator.
 *
 * This function creates a string representation of all entry points in the program,
 * including their memory positions. It validates each entry against the semantic analyzer
 * to ensure it exists and is properly defined. The generated content is stored in
 * the CodeGenerator's entryFile string.
 *
 * @param generator Pointer to the CodeGenerator structure.
 * @param analyzer Pointer to the AstValidator structure.
 * @param unit Pointer to the TranslationUnit structure.
 */
void generate_entry_file_string(CodeGenerator * generator, SemanticAnalyzer * analyzer, TranslationUnit * unit);


/**
 * generate_object_and_external_files_string
 *
 * This function generates the object and external files by processing the instruction
 * and guidance labels within a translation unit. It iterates through the instructions
 * and guidance nodes, writing the corresponding binary data to the object file.
 * The function also updates the instruction and guidance line counters.
 *
 * @param generator A pointer to the CodeGenerator struct, which manages the output files.
 * @param analyzer A pointer to the SemanticAnalyzer struct, used for symbol resolution.
 * @param unit A pointer to the TranslationUnit struct, representing the parsed assembly code.
 * @param instruction_lines A pointer to an integer that will store the number of instruction lines generated.
 * @param guidance_lines A pointer to an integer that will store the number of guidance lines generated.
 */
void generate_object_and_external_files(CodeGenerator * generator, SemanticAnalyzer * analyzer, TranslationUnit * unit, int * instruction_lines, int * guidance_lines);

/**
 * output_generate
 *
 * This function generates the necessary output files for the assembly program,
 * including the object file (.ob), external file (.ext), and entry file (.ent).
 * It first generates the object and external file contents using the
 * `generate_object_and_external_files` function, then checks for errors and
 * writes the corresponding data to the output files.
 *
 * @param generator A pointer to the CodeGenerator struct, which manages the output files.
 * @param analyzer A pointer to the SemanticAnalyzer struct, used for symbol resolution.
 * @param unit A pointer to the TranslationUnit struct, representing the parsed assembly code.
 * @param file_path A string containing the base file path for the output files.
 */
void output_generate(CodeGenerator * generator, SemanticAnalyzer * analyzer, TranslationUnit * unit, char * file_path);

#endif /*CODE_GENERATOR_H*/
