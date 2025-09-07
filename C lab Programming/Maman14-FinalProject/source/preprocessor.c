#include "../headers/safe_allocations.h"
#include "../headers/preprocessor.h"

#define RED_COLOR   "\x1B[1;91m"
#define RESET_COLOR "\x1B[0m"

static void write_string_to_file(FILE *file, String data);

void preprocessor_initialize(Preprocessor *preprocessor, Lexer lexer, char *file_path) {
    /* Allocate memory for the output file path (.am extension) */
    char *curated_file_path = safe_calloc(strlen(file_path) + 4, sizeof(char));
    sprintf(curated_file_path, "%s.am", file_path);

    /* Initialize preprocessor fields */
    preprocessor->processed_source = string_create();
    preprocessor->macro_list = NULL;
    preprocessor->tokens = lexer.token_list;

    /* Initialize the error handler */
    error_handler_initialize(&preprocessor->error_handler, lexer.source_code, curated_file_path);
}

void preprocessor_free(Preprocessor *preprocessor) {
    MacroList *current = preprocessor->macro_list;
    MacroList *temp;

    /* Free the processed source string */
    string_free(preprocessor->processed_source);
    /* Free the error handler */
    error_handler_free(&preprocessor->error_handler);

    /* Free all macros in the list */
    while (current != NULL) {
        string_free(current->macro.content);
        temp = current;
        current = current->next;
        free(temp);
    }

    /* Free the file path */
    free(preprocessor->error_handler.file_path);
}

void preprocessor_create_macro(Preprocessor * preprocessor, String source){
    Macro macro;
    TokenError error;
    int content_start, content_end;
    bool valid_end_macro = false;
    TokenNode *current_token;
    MacroList *existing;

    current_token = preprocessor->tokens;
    /*printf("Debug: Attempting to create a macro...\n");*/

    /* Check if the current token is a MACR token */
    if (current_token->token.type != TOKEN_MACR) {
        error.message = string_create_from_cstr("Expected MACR token");
        error.token = current_token->token;
        error_handler_add_token_error(&preprocessor->error_handler, PREPROCCESSOR_ERROR_TYPE, error);
        return;
    }
    /*printf("Debug: Found MACR token at index %d\n", current_token->token.index);*/

    /* Store the start index of the macro */
    macro.start_index = current_token->token.index;
    current_token = current_token->next;

    /* Check for and store the macro identifier */
    if (current_token->token.type != TOKEN_IDENTIFIER) {
        error.message = string_create_from_cstr("Expected identifier after MACR");
        error.token = current_token->token;
        error_handler_add_token_error(&preprocessor->error_handler, PREPROCCESSOR_ERROR_TYPE, error);
        /*printf("Debug: Macro identifier found: %s\n", current_token->token.string.data);*/
        return;
    }

    macro.identifier = current_token->token;

    /* Check for duplicate macro names */
    existing = preprocessor->macro_list;
    while (existing != NULL) {
        if (string_equals(macro.identifier.string, existing->macro.identifier.string)) {
            error.message = string_create_from_cstr("Duplicate macro identifier");
            error.token = macro.identifier;
            error_handler_add_token_error(&preprocessor->error_handler, PREPROCCESSOR_ERROR_TYPE, error);
            return;
        }
        existing = existing->next;
    }

    current_token = current_token->next;

    /* Ensure newline after macro identifier */
    if (current_token->token.type != TOKEN_EOL) {
        error.message = string_create_from_cstr("Expected newline after macro identifier");
        error.token = current_token->token;
        error_handler_add_token_error(&preprocessor->error_handler, PREPROCCESSOR_ERROR_TYPE, error);
        return;
    }

    /* Mark the start of macro content */
    content_start = current_token->token.index + 1;
    current_token = current_token->next;

    /* Find the end of the macro */
    while (current_token != NULL && current_token->next != NULL && current_token->next->next != NULL) {
        if (current_token->token.type == TOKEN_EOL &&
            current_token->next->token.type == TOKEN_ENDMACR &&
            (current_token->next->next->token.type == TOKEN_EOL || current_token->next->next->token.type == TOKEN_EOFT)) {
            content_end = current_token->token.index;
            macro.end_index = current_token->next->next->token.index;
            valid_end_macro = true;
            break;
        }
        current_token = current_token->next;
    }

    /* Check if the macro end was found */
    if (!valid_end_macro) {
        error.message = string_create_from_cstr("Invalid or missing ENDMACR");
        error.token = preprocessor->tokens->token;
        error_handler_add_token_error(&preprocessor->error_handler, PREPROCCESSOR_ERROR_TYPE, error);
        /*printf("Debug: ENDMACR token was not found\n");*/
        return;
    }
    /*printf("Debug: Macro '%s' created successfully with content:\n%s\n",
       macro.identifier.string.data, macro.content.data);*/

    /* Extract macro content */
    macro.content = string_substring(source, content_start, content_end);
    preprocessor_append_macro(preprocessor, macro);
}

void preprocessor_generate_macro_list(Preprocessor *preprocessor, String source) {
    TokenNode *current = preprocessor->tokens;
    while (current != NULL && current->token.type != TOKEN_EOFT) {
        if (current->token.type == TOKEN_MACR) {
            preprocessor->tokens = current;  /* Set the current token for macro generation */
            preprocessor_create_macro(preprocessor, source);
        }
        current = current->next;
    }
    preprocessor->tokens = current;  /* Reset to the original token list */
}

void preprocessor_append_macro(Preprocessor * preprocessor, Macro macro){
    /* Create a new macro node */
    MacroList *new_macro = safe_malloc(sizeof(MacroList));
    new_macro->macro = macro;
    new_macro->next = NULL;

    /* Append the new macro to the list */
    if (preprocessor->macro_list == NULL) {
        preprocessor->macro_list = new_macro;
    } else {
        MacroList *last = preprocessor->macro_list;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = new_macro;
    }
}

void preprocessor_display_macro_list(Preprocessor * preprocessor){
    MacroList *current;

    current = preprocessor->macro_list;
    /* Iterate through the macro list and print each macro */
    while (current != NULL) {
        printf("Macro identifier: %s\n", current->macro.identifier.string.data);
        printf("Macro content:\n%s\n", current->macro.content.data);
        printf("End of macro\n\n");
        current = current->next;
    }
}

void preprocessor_expand_macros(Preprocessor * preprocessor, String source){
    TokenNode *current_token;
    unsigned int i;
    MacroList *macro;
    bool is_macro;

    current_token = preprocessor->tokens;
    i = 0;
    /*printf("Debug: Starting macro expansion...\n");*/

    /* Process the source code character by character */
    while (i < string_length(source)) {
        if (current_token != NULL && i == current_token->token.index) {
            /* Skip the entire macro definition */
            is_macro = false;
            macro = preprocessor->macro_list;
            /* printf("Debug: Expanding or skipping token at index %d\n", i);*/
            /* printf("Debug: Processing token '%s' at index %d\n", current_token->token.string.data, i);*/


            while (macro != NULL) {
                if (i >= macro->macro.start_index && i <= macro->macro.end_index) {
                    /* Skip the entire macro definition */
                    i = macro->macro.end_index + 1;
                    while (current_token != NULL && current_token->token.index <= macro->macro.end_index) {
                        current_token = current_token->next;
                    }
                    is_macro = true;
                    /* printf("Debug: Skipping macro definition for '%s'\n", macro->macro.identifier.string.data);*/

                    break;
                }
                if (string_equals(macro->macro.identifier.string, current_token->token.string)) {
                    /* Expand the macro */
                    string_append(&preprocessor->processed_source, macro->macro.content);
                    i += string_length(current_token->token.string);
                    current_token = current_token->next;
                    /* Skip newline after macro expansion if present */
                    if (current_token != NULL && current_token->token.type == TOKEN_EOL) {
                        current_token = current_token->next;
                        i++;
                    }
                    is_macro = true;
                    /* printf("Debug: Expanding macro '%s' at index %d\n", macro->macro.identifier.string.data, i);*/

                    break;
                }
                macro = macro->next;
            }

            /* If not a macro, append the token as is */
            if (!is_macro) {
                string_append(&preprocessor->processed_source, current_token->token.string);
                i += string_length(current_token->token.string);
                current_token = current_token->next;
            }
            /* Append non-token characters */
        } else {
            string_append_char(&preprocessor->processed_source, string_char_at(source, i));
            i++;
        }
    }
    /* printf("Debug: Macro expansion completed. Processed source:\n%s\n", preprocessor->processed_source.data);*/

}

void preprocessor_process(Preprocessor *preprocessor, String source) {
    FILE *file;
    /* Create the list of macros */
    preprocessor_generate_macro_list(preprocessor, source);

    /* If there are errors, stop processing */
    if (preprocessor->error_handler.error_list != NULL) return;

    /* Expand macros in the source code */
    preprocessor_expand_macros(preprocessor, source);

    /* Open the output file */
    file = fopen(preprocessor->error_handler.file_path, "w");
    if (file == NULL) {
        /*printf("%sPreprocessor Error:%s Couldn't open \"%s\".\n", RED_COLOR, RESET_COLOR, preprocessor->error_handler.file_path);*/
        return;
    }

    /* Write the processed source to the file */
    write_string_to_file(file, preprocessor->processed_source);
    fclose(file);

    /* Update the error handler with the processed source */
    preprocessor->error_handler.string = preprocessor->processed_source;
}

/**
 * Writes the given string data to a file.
 *
 * @param file The file to write to.
 * @param data The string data to write.
 */
static void write_string_to_file(FILE *file, String data) {
    int i;
    /* Write each character of the string to the file */
    for (i = 0; i < string_length(data); i++) {
        char c = string_char_at(data, i);
        if (c == EOF || c == '\0') break;
        fputc(c, file);
    }
}
