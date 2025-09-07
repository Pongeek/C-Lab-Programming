#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "string_util.h"
#include "token.h"
#include "error_handler.h"
#include "lexer.h"

/**
 * Represents a macro in the source code.
 * A macro consists of an identifier and its content, along with the start and end indices in the source file.
 */
typedef struct Macro {
    Token identifier;  /* The identifier token of the macro */
    String content;    /* The content of the macro expansion */
    int start_index;   /* The starting index of the macro in the source file */
    int end_index;     /* The ending index of the macro in the source file */
} Macro;

/**
 * Represents a node in the linked list of macros.
 * Each node contains a macro and a pointer to the next node in the list.
 */
typedef struct MacroList {
    Macro macro;               /* The current macro */
    struct MacroList *next;    /* Pointer to the next macro in the list */
} MacroList;

/**
 * Represents the preprocessor, which handles macro processing and expansion.
 * The preprocessor maintains a list of macros, processes the source code, and handles errors.
 */
typedef struct Preprocessor {
    String processed_source;   /* The source file content after preprocessing */
    MacroList *macro_list;     /* List of macros found in the source */
    ErrorHandler error_handler; /* Error handler for preprocessing errors */
    TokenNode *tokens;         /* Token list reference from the lexer */
} Preprocessor;

/**
 * Initializes the preprocessor with data from the lexer.
 *
 * @param preprocessor Pointer to the Preprocessor to initialize.
 * @param lexer The Lexer containing tokenized source.
 * @param file_path The path of the source file (without extension).
 */
void preprocessor_initialize(Preprocessor * preprocessor, Lexer lexer, char * file_path);

/**
 * Frees all memory allocated for the preprocessor.
 *
 * @param preprocessor Pointer to the Preprocessor to free.
 */
void preprocessor_free(Preprocessor * preprocessor);

/**
 * Creates a macro from the current token in the preprocessor.
 * This function assumes the current token is a MACR token and processes the macro definition.
 *
 * @param preprocessor Pointer to the Preprocessor.
 * @param source The original source code as a String.
 */
void preprocessor_create_macro(Preprocessor * preprocessor, String source);

/**
 * Creates a list of all macros in the source code.
 * This function iterates through the tokens and identifies macro definitions, adding them to the macro list.
 *
 * @param preprocessor Pointer to the Preprocessor.
 * @param source The original source code as a String.
 */
void preprocessor_create_macro_list(Preprocessor * preprocessor, String source);

/**
 * Adds a macro to the end of the preprocessor's macro list.
 * This function appends a new macro to the linked list of macros maintained by the preprocessor.
 *
 * @param preprocessor Pointer to the Preprocessor.
 * @param macro The Macro to be added.
 */
void preprocessor_append_macro(Preprocessor * preprocessor, Macro macro);

/**
 * Prints the preprocessor's macro list in a formatted way.
 * This function iterates through the macro list and prints each macro's identifier and content.
 *
 * @param preprocessor Pointer to the Preprocessor.
 */
void preprocessor_display_macro_list(Preprocessor * preprocessor);

/**
 * Generates the preprocessed source code by expanding macros.
 * This function replaces macro identifiers in the source code with their corresponding content.
 *
 * @param preprocessor Pointer to the Preprocessor.
 * @param source The original source code as a String.
 */
void preprocessor_expand_macros(Preprocessor * preprocessor, String source);

/**
 * Performs preprocessing on the source string.
 * This function creates the macro list, expands macros, and writes the processed source to a file.
 *
 * @param preprocessor The preprocessor.
 * @param source The source file as a string.
 */
void preprocessor_process(Preprocessor * preprocessor, String source);

#endif /* PREPROCESS_H */
