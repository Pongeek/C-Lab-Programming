#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H
#include "string.h"
#include "token.h"

typedef enum {
    TOKEN_ERROR_TYPE,
    CHAR_ERROR_TYPE
} ErrorType;

typedef enum {
    LEXER_ERROR_TYPE,
    PREPROCCESSOR_ERROR_TYPE,
    PARSER_ERROR_TYPE,
    SEMANTIC_ANALYZER_ERROR_TYPE,
    OUTPUT_GENERATOR_ERROR_TYPE
} ErrorSource;

/**
 * Represents an error associated with a token.
 */
typedef struct {
    Token token;    /* The token associated with the error */
    String message; /* The error message */
} TokenError;


typedef struct {
    char character;           /* The character associated with the error */
    unsigned int fileIndex;   /* The index of the character in the source file */
    unsigned int lineIndex;   /* The index of the character in its line */
    unsigned int lineNumber;  /* The line number where the error occurred */
    String message;           /* The error message */
} CharError;

typedef struct ErrorNode {
    union {
        TokenError tokenError;
        CharError charError;
    } error;

    ErrorType type;       /* The type of error (token or char) */
    ErrorSource source;   /* The source of the error (lexer, parser, etc.) */
    struct ErrorNode *next;
} ErrorNode;

/**
 * Manages error handling and reporting.
 */
typedef struct ErrorHandler {
    String string;        /* The source code being processed */
    char *file_path;      /* The path to the source file */
    ErrorNode *error_list; /* Linked list of errors */
} ErrorHandler;

/**
 * Initializes the error handler.
 *
 * @param handler Pointer to the ErrorHandler to initialize
 * @param source_string The source code being processed
 * @param filePath The path to the source file
 */
void error_handler_initialize(ErrorHandler * handler, String source_string, char * filePath);

/**
 * Adds a token error to the error list.
 *
 * @param handler Pointer to the ErrorHandler
 * @param source The source of the error
 * @param error The TokenError to add
 */
void error_handler_add_token_error(ErrorHandler * handler, ErrorSource source, TokenError error);

/**
 * Adds a character error to the error list.
 *
 * @param handler Pointer to the ErrorHandler
 * @param source The source of the error
 * @param error The CharError to add
 */
void error_handler_add_char_error(ErrorHandler * handler, ErrorSource source, CharError error);

/**
 * Outputs all errors in the error list to the user.
 *
 * @param handler Pointer to the ErrorHandler
 */
void error_handler_report_errors(ErrorHandler * handler);

/**
 * Frees all memory associated with the error list.
 *
 * @param handler Pointer to the ErrorHandler
 */
void error_handler_free(ErrorHandler * handler);

#endif /* ERROR_HANDLER_H */