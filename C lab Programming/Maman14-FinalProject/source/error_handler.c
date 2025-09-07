#include "../headers/error_handler.h"  /* Include the error handler header file */
#include "../headers/safe_allocations.h"  /* Include the safe allocations header file */

#define RED_COLOR   "\x1B[1;91m"  /* Define the red color for terminal output */
#define RESET_COLOR "\x1B[0m"  /* Define the reset color for terminal output */

/* Function prototype to count digits in an integer */
static int count_digits(int value);
/* Function prototype to get the line number from a string */
static unsigned int string_get_line_number(String str, unsigned int index);
/* Function prototype to find the start of a line in a string */
static unsigned int string_find_line_start(String str, unsigned int index);
/* Function prototype to print an error pointer */
static void print_error_pointer(unsigned int lineNumber, unsigned int errorIndex, unsigned int errorLength);
/* Function prototype to print an error line */
static void print_error_line(String sourceString, unsigned int startIndex, unsigned int errorIndex, unsigned int errorLength);
/* Function prototype to print the error location */
static void print_error_location(const char *filePath, int line, int column);

void error_handler_initialize(ErrorHandler * handler, String source_string, char * filePath){
    handler->string = source_string;  /* Initialize the source string in the error handler */
    handler->file_path = filePath;  /* Initialize the file path in the error handler */
    handler->error_list = NULL;  /* Initialize the error list to NULL */
}

void error_handler_add_token_error(ErrorHandler * handler, ErrorSource source, TokenError error){
    ErrorNode *newError = safe_malloc(sizeof(ErrorNode));  /* Allocate memory for a new error node */
    newError->error.tokenError = error;  /* Set the token error in the new error node */
    newError->type = TOKEN_ERROR_TYPE;  /* Set the error type to token error */
    newError->source = source;  /* Set the error source */
    newError->next = NULL;  /* Set the next pointer to NULL */

    if (handler->error_list == NULL) {  /* If the error list is empty */
        handler->error_list = newError;  /* Set the new error as the first error */
    } else {  /* If the error list is not empty */
        ErrorNode *current = handler->error_list;  /* Get the current error node */
        while (current->next != NULL) {  /* Traverse to the end of the error list */
            current = current->next;  /* Move to the next error node */
        }
        current->next = newError;  /* Add the new error to the end of the list */
    }
}

void error_handler_add_char_error(ErrorHandler * handler, ErrorSource source, CharError error){
    ErrorNode *newError = safe_malloc(sizeof(ErrorNode));  /* Allocate memory for a new error node */
    newError->error.charError = error;  /* Set the char error in the new error node */
    newError->type = CHAR_ERROR_TYPE;  /* Set the error type to char error */
    newError->source = source;  /* Set the error source */
    newError->next = NULL;  /* Set the next pointer to NULL */

    if (handler->error_list == NULL) {  /* If the error list is empty */
        handler->error_list = newError;  /* Set the new error as the first error */
    } else {  /* If the error list is not empty */
        ErrorNode *current = handler->error_list;  /* Get the current error node */
        while (current->next != NULL) {  /* Traverse to the end of the error list */
            current = current->next;  /* Move to the next error node */
        }
        current->next = newError;  /* Add the new error to the end of the list */
    }
}

void error_handler_report_errors(ErrorHandler * handler){
    ErrorNode *current = handler->error_list;  /* Get the current error node */
    const char *error_type_strings[] = {
        "Lexer Error",
        "Preprocessor Error",
        "Parser Error",
        "AST Validator Error",
        "Output Generator Error"
    };

    while (current != NULL) {  /* Traverse the error list */
        const char *error_type = (current->source < sizeof(error_type_strings) / sizeof(error_type_strings[0]))
        ? error_type_strings[current->source]: "Unknown Error";  /* Get the error type string */

        switch (current->type) {  /* Switch based on the error type */
            case TOKEN_ERROR_TYPE: {  /* If the error type is token error */
                TokenError *error = &current->error.tokenError;  /* Get the token error */
                unsigned int startIndex = string_find_line_start(handler->string, error->token.index);  /* Find the start of the error line */
                printf("Debug: Token Error - Line: %d, IndexInLine: %d, Index: %d\n", error->token.line, error->token.index_in_line, error->token.index);  /* Print debug information */
                print_error_location(handler->file_path, error->token.line, error->token.index_in_line + 1);  /* Print the error location */
                printf("%s%s%s: %s\n", RED_COLOR, error_type, RESET_COLOR, error->message.data);  /* Print the error message */
                print_error_line(handler->string, startIndex, error->token.index, string_length(error->token.string));  /* Print the error line */
                print_error_pointer(error->token.line, error->token.index_in_line, string_length(error->token.string));  /* Print the error pointer */
                break;
            }
            case CHAR_ERROR_TYPE: {  /* If the error type is char error */
                CharError *error = &current->error.charError;  /* Get the char error */
                unsigned int startIndex = string_find_line_start(handler->string, error->fileIndex);  /* Find the start of the error line */
                printf("Debug: Char Error - Line: %d, LineIndex: %d, Index: %d\n", error->lineNumber, error->lineIndex, error->fileIndex);  /* Print debug information */
                print_error_location(handler->file_path, error->lineNumber, error->lineIndex + 1);  /* Print the error location */
                printf("%s%s%s: %s\n", RED_COLOR, error_type, RESET_COLOR, error->message.data);  /* Print the error message */
                print_error_line(handler->string, startIndex, error->fileIndex, 1);  /* Print the error line */
                print_error_pointer(error->lineNumber, error->lineIndex, 1);  /* Print the error pointer */
                break;
            }
            default:
                printf("Unknown error type\n");  /* Print unknown error type */
                break;
        }
        current = current->next;  /* Move to the next error node */
    }
}

void error_handler_free(ErrorHandler * handler){
    ErrorNode * temp;  /* Temporary pointer for freeing memory */
    ErrorNode * current = handler->error_list;  /* Get the current error node */

    while (current != NULL) {  /* Traverse the error list */
        switch (current->type) {  /* Switch based on the error type */
        case TOKEN_ERROR_TYPE:
            string_free(current->error.tokenError.message);  /* Free the token error message */
            break;
        case CHAR_ERROR_TYPE:
            string_free(current->error.charError.message);  /* Free the char error message */
            break;
        default:
            break;
        }

        temp = current;  /* Store the current node in temp */
        current = current->next;  /* Move to the next node */
        free(temp);  /* Free the current node */
    }
}

/* ----------------------- Helper Functions -------------------------- */

/* Helper function to count digits in an integer */
static int count_digits(int value) {
    int count = 0;  /* Initialize the digit count */
    while (value != 0) {  /* Loop until the value is 0 */
        value /= 10;  /* Divide the value by 10 */
        count++;  /* Increment the digit count */
    }
    return count;  /* Return the digit count */
}

static void print_error_location(const char *filePath, int line, int column) {
    printf("%s:%d:%d: ", filePath, line, column);  /* Print the error location */
}

static void print_error_line(String sourceString, unsigned int startIndex, unsigned int errorIndex, unsigned int errorLength) {
    unsigned int i;
    printf("    %d | ", string_get_line_number(sourceString, startIndex));  /* Print the line number */
    for (i = startIndex; string_char_at(sourceString, i) != '\0' && string_char_at(sourceString, i) != '\n'; i++) {  /* Loop through the line */
        if (i == errorIndex) printf("%s", RED_COLOR);  /* Print the error in red */
        putchar(string_char_at(sourceString, i));  /* Print the character */
        if (i == errorIndex + errorLength - 1) printf("%s", RESET_COLOR);  /* Reset the color */
    }
    printf("\n");  /* Print a newline */
}

static void print_error_pointer(unsigned int lineNumber, unsigned int errorIndex, unsigned int errorLength) {
    unsigned int i;
    printf("    %*s | ", count_digits(lineNumber), "");  /* Print the line number */
    printf("%*s", errorIndex, "");  /* Align the pointer with the error character */
    printf("%s", RED_COLOR);  /* Set the color to red */
    for (i = 0; i < errorLength; i++) {  /* Loop through the error length */
        printf(i == 0 ? "^" : "~");  /* Print the error pointer */
    }
    printf("%s\n", RESET_COLOR);  /* Reset the color */
}

static unsigned int string_find_line_start(String str, unsigned int index) {
    while (index > 0 && string_char_at(str, index - 1) != '\n') {  /* Loop until the start of the line */
        index--;  /* Decrement the index */
    }
    return index;  /* Return the start index */
}

static unsigned int string_get_line_number(String str, unsigned int index) {
    unsigned int i;
    unsigned int line = 1;  /* Initialize the line number */
    for (i = 0; i < index; i++) {  /* Loop through the string */
        if (string_char_at(str, i) == '\n') {  /* If a newline is found */
            line++;  /* Increment the line number */
        }
    }
    return line;  /* Return the line number */
}
