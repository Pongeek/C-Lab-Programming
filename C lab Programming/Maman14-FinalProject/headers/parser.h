#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "nodes.h"

/**
 * Initializes the translation unit with the tokens from the lexer.
 * This function sets up the initial state of the translation unit,
 * including initializing all pointers to NULL and setting up the error handler.
 *
 * @param unit Pointer to the TranslationUnit to be initialized.
 * @param lexer The Lexer containing the tokens to be parsed.
 */
void parser_initialize_translation_unit(TranslationUnit * unit, Lexer lexer);

/**
 * Frees all memory associated with the translation unit.
 * This includes freeing all nodes (external, entry, instruction labels, guidance labels)
 * and cleaning up the error handler.
 *
 * @param unit Pointer to the TranslationUnit to be freed.
 */
void parser_free_translation_unit(TranslationUnit * unit);

/**
 * Advances the current token pointer to the end of the current line.
 * This is typically used for error recovery, skipping to the next line
 * after encountering an error in the current line.
 *
 * @param unit Pointer to the TranslationUnit being parsed.
 */
void parser_move_to_end_of_line(TranslationUnit * unit);

/**
 * Parses a .data directive, which defines numeric data.
 * This function expects a series of numbers separated by commas.
 * It creates a DataNode containing all the numbers found in the directive.
 *
 * @param unit Pointer to the TranslationUnit being parsed.
 * @return A DataNode containing the parsed numeric data.
 */
DataNode parse_data_directive_guidance(TranslationUnit * unit);

/**
 * Frees the memory associated with a DataNode.
 * This includes freeing the list of number tokens.
 *
 * @param data_node The DataNode to be freed.
 */
void parser_free_directive_guidance(DataNode data_node);

/**
 * Parses a .string directive, which defines a string constant.
 * This function expects a single string token after the .string directive.
 *
 * @param unit Pointer to the TranslationUnit being parsed.
 * @return A StringNode containing the parsed string data.
 */
StringNode parse_string_directive_guidance(TranslationUnit * unit);

/**
 * Parses guidance directives (.data and .string).
 * This function handles multiple consecutive guidance directives.
 *
 * @param unit Pointer to the TranslationUnit being parsed.
 * @return A linked list of GuidanceNodes, or NULL if no guidance directives are found.
 */
GuidanceNodeList * parser_parse_guidance_list(TranslationUnit * unit);

/**
 * Frees the memory associated with a list of guidance nodes.
 * This includes freeing both DataNodes and StringNodes.
 *
 * @param guidance_list Pointer to the head of the GuidanceNodeList to be freed.
 */
void parser_free_guidance_list(GuidanceNodeList * guidance_list);

/**
 * Parses a single assembly instruction.
 * This includes parsing the operation and its operands.
 *
 * @param unit Pointer to the TranslationUnit being parsed.
 * @return An InstructionNode representing the parsed instruction.
 */
InstructionNode parser_parse_instruction(TranslationUnit * unit);

/**
 * Parses multiple consecutive assembly instructions.
 *
 * @param unit Pointer to the TranslationUnit being parsed.
 * @return A linked list of InstructionNodes.
 */
InstructionNodeList * parser_parse_instruction_list(TranslationUnit * unit);

/**
 * Frees the memory associated with a list of instruction nodes.
 *
 * @param instruction_list Pointer to the head of the InstructionNodeList to be freed.
 */
void parser_free_instruction_list(InstructionNodeList * instruction_list);

/**
 * Parses an .entry directive, which declares a symbol as globally accessible.
 *
 * @param translation_unit Pointer to the TranslationUnit being parsed.
 * @return An EntryNode representing the parsed .entry directive.
 */
EntryNode parser_parse_entry(TranslationUnit * translation_unit);

/**
 * Parses an .extern directive, which declares a symbol as externally defined.
 *
 * @param unit Pointer to the TranslationUnit being parsed.
 * @return An ExternalNode representing the parsed .extern directive.
 */
ExternalNode parser_parse_external(TranslationUnit * unit);

/**
 * Parses a labeled statement, which can be either an instruction or a guidance directive.
 * This function handles the label and the associated content (instruction or guidance).
 *
 * @param unit Pointer to the TranslationUnit being parsed.
 * @return A LabelNode representing the parsed labeled statement.
 */
LabelNode parse_labeled_statement(TranslationUnit * unit);

/**
 * Parses the entire content of the translation unit.
 * This is the main parsing function that handles all elements of the assembly code,
 * including directives (.entry, .extern), labels, instructions, and guidance directives.
 * It populates the TranslationUnit structure with all parsed information.
 *
 * @param unit Pointer to the TranslationUnit to be parsed and populated.
 */
void parse_translation_unit_content(TranslationUnit * unit);

/**
 * Frees the memory associated with a list of assembly statements.
 *
 * @param statement_list Pointer to the head of the AssemblyStatementList to be freed.
 */
void parser_free_sentences(AssemblyStatementList * statement_list);

#endif /*PARSER_H */