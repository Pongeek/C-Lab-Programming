#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "error_handler.h"
#include "char_util.h"
#include "string_util.h"

/**
 * Represents the state and functionality of the lexical analyzer.
 */
typedef struct Lexer {
    String source_code; /* The input source code as a String */
    unsigned int index; /* Current position in the source code */
    unsigned int column; /* Current column in the current line */
    char current_char; /* Current character being examined */
    unsigned int line_number; /* Current line number (starting from 1) */

    char *file_path; /* Relative path to the source file */

    ErrorHandler error_handler; /* Error handler for reporting lexer errors */
    TokenNode *token_list; /* List of tokens produced by the lexer */
} Lexer;

/**
 * Initializes the lexer with a C-style string.
 * @param lexer Pointer to the Lexer to initialize.
 * @param source_string The source code as a null-terminated string.
 */
void lexer_initialize_from_cstr(Lexer *lexer, char *source_string);

/**
 * Initializes the lexer with a file.
 * @param lexer Pointer to the Lexer to initialize.
 * @param file_path Path to the source file.
 * @return true if initialization was successful, false otherwise.
 */
bool lexer_initialize_from_file(Lexer *lexer, char *file_path);

/**
 * Initializes the lexer with a String object.
 * @param lexer Pointer to the Lexer to initialize.
 * @param file_path Path to associate with the source code.
 * @param source_code The source code as a String object.
 */
void lexer_initialize_from_string(Lexer *lexer, const char *file_path, String source_code);

/**
 * Frees resources used by the lexer.
 * @param lexer Pointer to the Lexer to free.
 */
void lexer_free(Lexer *lexer);

/**
 * Prints the token list in a formatted way.
 * @param lexer Pointer to the Lexer containing the token list.
 */
void lexer_print_token_list(Lexer *lexer);

/**
 * Advance the lexer char pointer to the next char.
 * Update the lexer internal variables due to that advancement.
 */
void lexer_advance_character(Lexer *lexer);

/**
 * Processes and creates a token for a comment.
 * @param lexer Pointer to the Lexer.
 */
void lexer_tokenize_comment(Lexer *lexer);

/**
 * Processes and creates a token for a newline.
 * @param lexer Pointer to the Lexer.
 */
void lexer_tokenize_newline(Lexer *lexer);

/**
 * Processes and creates a token for a separator.
 * @param lexer Pointer to the Lexer.
 */
void lexer_tokenize_separator(Lexer *lexer);

/**
 * Processes and creates a token for a number.
 * @param lexer Pointer to the Lexer.
 */
void lexer_tokenize_number(Lexer *lexer);

/**
 * Processes and creates a token for a string literal.
 * @param lexer Pointer to the Lexer.
 */
void lexer_tokenize_string(Lexer *lexer);

/**
 * Processes and creates a token for a non-operative instruction.
 * @param lexer Pointer to the Lexer.
 */
void lexer_tokenize_non_op_instruction(Lexer *lexer);

/**
 * Processes and creates a token for an identifier or keyword.
 * @param lexer Pointer to the Lexer.
 */
void lexer_tokenize_identifier(Lexer *lexer);

/**
 * Creates an end-of-file token.
 * @param lexer Pointer to the Lexer.
 */
void lexer_tokenize_eof(Lexer *lexer);

/**
 * Performs a full lexical analysis on the source code.
 * @param lexer Pointer to the Lexer to perform analysis with.
 */
void lexer_analyze(Lexer *lexer);

int lexer_is_end_of_input(Lexer *lexer);

#endif /* LEXER_H */