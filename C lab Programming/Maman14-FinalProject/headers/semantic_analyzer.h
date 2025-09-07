#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "parser.h"
#include "lexer.h"

/* Enum representing different addressing modes in assembly */
typedef enum AddressingMode {
    ADDRESSING_MODE_IMMEDIATE = 1,          /* 0001 -  Immediate value (e.g., #5) */
    ADDRESSING_MODE_DIRECT = 2,            /* 0010 -  Direct label reference */
    ADDRESSING_MODE_INDIRECT_REGISTER = 4,  /* 0100 - Dereferencing a register (e.g., *r3) */
    ADDRESSING_MODE_DIRECT_REGISTER = 8     /* 1000 - Direct register access */
} AddressingMode;

/* Enum for types of identifiers in the hash table */
typedef enum IdentifierCellType {
    IDENTIFIER_CELL_LABEL, /* Represents a label in the code */
    IDENTIFIER_CELL_EXTERNAL /* Represents an external symbol */
} IdentifierCellType;

/* Structure for storing identifier information in the hash table */
typedef struct IdentifierCell {
    String *key; /* The identifier name (label or external symbol) */

    union {
        LabelNode *label; /* Pointer to label node if it's a label */
        ExternalNode *external; /* Pointer to external node if it's an external symbol */
    } value; /* The data associated with the identifier */

    IdentifierCellType type; /* The type of the identifier (label or external) */
    bool has_entry; /* Flag indicating if an entry was added for this identifier in the .ent file */
} IdentifierCell;

/* Main structure for semantic analysis */
typedef struct SemanticAnalyzer {
    IdentifierCell *hash; /* Pointer to the hash table of identifiers */
    unsigned int size; /* Number of cells in the array (hashTable) */

    ErrorHandler error_handler; /* Error handler for reporting semantic errors */
} SemanticAnalyzer;

/**
 * Computes a hash value for a given string.
 *
 * This function implements the djb2 hash algorithm, which is known for its
 * simplicity and good distribution properties. The algorithm iterates through
 * each character of the string, updating the hash value at each step.
 *
 * @param str The String structure containing the string to be hashed.
 * @return An unsigned long representing the computed hash value.
 */
unsigned long compute_string_hash(String str);

/**
 * Initializes the analyzer with data from the translation unit and lexer.
 *
 * This function sets up the hash table for the analyzer, calculating its size
 * based on the number of labels and external nodes in the translation unit.
 * It also initializes the error handler with information from the lexer.
 *
 * @param analyzer Pointer to the SemanticAnalyzer to initialize.
 * @param unit Pointer to the parsed TranslationUnit.
 * @param lexer The Lexer containing source information.
 */
void semantic_analyzer_initialize(SemanticAnalyzer *analyzer, TranslationUnit *unit, Lexer lexer);

/**
 * Frees all memory associated with the anaylyzer.
 *
 * This function releases the memory allocated for the hash table and cleans up
 * any resources held by the error handler. It should be called when the
 * analyzer is no longer needed to prevent memory leaks.
 *
 * @param analyzer Pointer to the SemanticAnalyzer to free.
 */
void semantic_analyzer_free(SemanticAnalyzer *analyzer);

/**
 * Retrieves a hash cell from the analyzer's hash table based on the given key.
 *
 * This function uses linear probing to handle collisions. It computes the hash
 * of the key, then searches the hash table starting from the computed index.
 * If it reaches the end of the table, it wraps around to the beginning.
 *
 * @param analyzer Pointer to the SemanticAnalyzer.
 * @param key The identifier name to look up.
 * @return Pointer to the IdentifierHashCell if found, NULL otherwise.
 */
IdentifierCell *semantic_analyzer_find_identifier(SemanticAnalyzer *analyzer, String key);

/**
 * Inserts a new identifier hash cell into the validator's hash table.
 *
 * This function uses linear probing to handle collisions. It computes the hash
 * of the cell's key, then searches for an empty slot or a slot with the same key.
 * If a slot with the same key is found, the insertion fails to prevent duplicates.
 *
 * @param analyzer Pointer to the SemanticAnalyzer.
 * @param cell The IdentifierHashCell to insert.
 * @return true if insertion was successful, false if the identifier already exists.
 */
bool semantic_analyzer_insert_identifier(SemanticAnalyzer *analyzer, IdentifierCell cell);

/**
 * Analyzes the numeric values in a data directive guidance node.
 *
 * This function checks each number in the data node to ensure it falls within
 * the allowed range for a 15-bit signed integer. It reports an error for any
 * value that is out of range.
 *
 * @param analyzer Pointer to the SemanticAnalyzer.
 * @param node The DataNode to validate.
 */
void semantic_analyzer_analyze_directive_guidance(SemanticAnalyzer *analyzer, DataNode node);

/**
 * Analyzes an instruction node in the SemanticAnalyzer.
 *
 * This function checks the validity of an instruction, including its operands
 * and addressing modes based on the operation type.
 *
 * @param analyzer Pointer to the SemanticAnalyzer.
 * @param node The InstructionNode to validate.
 */
void semantic_analyzer_analyze_instruction(SemanticAnalyzer *analyzer, InstructionNode node);

/**
 * Validates a label node and its associated instructions or guidance nodes.
 *
 * This function checks if a label with instructions has a valid label identifier,
 * and then proceeds to validate all instructions and guidance nodes associated
 * with the label.
 *
 * @param analyzer Pointer to the SemanticAnalyzer.
 * @param node The LabelNode to validate.
 */
void semantic_analyzer_analyze_label(SemanticAnalyzer *analyzer, LabelNode node);

/**
 * Validates for duplicate identifiers in the translation unit.
 *
 * @param analyzer Pointer to the SemanticAnalyzer.
 * @param unit Pointer to the TranslationUnit to check.
 */
void semantic_analyzer_analyze_duplicate_identifiers(SemanticAnalyzer *analyzer, TranslationUnit *unit);

/**
 * Performs a comprehensive validation of the entire translation unit.
 *
 * This function serves as the main entry point for Analyzer. It checks
 * for duplicate identifiers and validates all labels, instructions, and guidance
 * nodes in the translation unit.
 *
 * @param analyzer Pointer to the SemanticAnalyzer.
 * @param unit Pointer to the TranslationUnit to analyze.
 */
void semantic_analyzer_analyze_translation_unit(SemanticAnalyzer *analyzer, TranslationUnit *unit);

#endif /* SEMANTIC_ANALYZER_H */