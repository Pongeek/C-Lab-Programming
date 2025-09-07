#include "../headers/error_handler.h"
#include "../headers/lexer.h"
#include "../headers/safe_allocations.h"
#include "../headers/char_util.h"
#include "../headers/string_util.h"
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define RED_COLOR   "\x1B[1;91m"
#define RESET_COLOR "\x1B[0m"

static char* safe_strdup(const char* str);
static void add_token(Lexer * lexer, Token token);
static bool is_valid_macro_char(char ch);


void lexer_initialize_from_cstr(Lexer * lexer, char * source_string){
    lexer->source_code = string_create_from_cstr(source_string);
    lexer->index = 0;
    lexer->column = 0;
    lexer->current_char = string_char_at(lexer->source_code, lexer->index);
    lexer->line_number = 1;

    lexer->file_path = safe_strdup("from_string.as");
    lexer->token_list = NULL;

    error_handler_initialize(&lexer->error_handler, lexer->source_code, lexer->file_path);

    /* Append EOF to ensure consistent end-of-input handling */
    string_append_char(&lexer->source_code, EOF);
}

bool lexer_initialize_from_file(Lexer *lexer, char * file_path){
    FILE *file;
    int ch;
    char *full_path;

    lexer->source_code = string_create();
    full_path = safe_malloc(strlen(file_path) + 4); /* +4 for ".as" and null terminator*/
    sprintf(full_path, "%s.as", file_path);
    lexer->file_path = full_path;

    lexer->token_list = NULL;

    error_handler_initialize(&lexer->error_handler, lexer->source_code, lexer->file_path);

    file = fopen(lexer->file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "%sLexer Error:%s Couldn't open \"%s\".\n", RED_COLOR, RESET_COLOR, lexer->file_path);
        return false;
    }

    while ((ch = fgetc(file)) != EOF) {
        string_append_char(&lexer->source_code, (char)ch);
    }
    string_append_char(&lexer->source_code, EOF);

    fclose(file);

    lexer->index = 0;
    lexer->column = 0;
    lexer->current_char = string_char_at(lexer->source_code, lexer->index);
    lexer->line_number = 1;

    return true;
}

void lexer_initialize_from_string(Lexer *lexer, const char * file_path, String source_code){
    lexer->source_code = string_create();
    string_append(&lexer->source_code, source_code);

    lexer->file_path = safe_strdup(file_path);
    lexer->token_list = NULL;

    error_handler_initialize(&lexer->error_handler, lexer->source_code, lexer->file_path);

    lexer->index = 0;
    lexer->column = 0;
    lexer->current_char = string_char_at(lexer->source_code, lexer->index);
    lexer->line_number = 1;
}

void lexer_free(Lexer *lexer) {
    TokenNode *current = lexer->token_list;
    TokenNode *temp;

    while (current != NULL) {
        string_free(current->token.string);
        temp = current;
        current = current->next;
        free(temp);
    }

    string_free(lexer->source_code);
    free(lexer->file_path);
}

void lexer_print_token_list(Lexer * lexer){
    const TokenNode *tokens = lexer->token_list;

    while (tokens != NULL) {
        switch (tokens->token.type) {
            case TOKEN_COMMENT:
                printf("Comment: %s\n", tokens->token.string.data);
                break;
            case TOKEN_EOL:
                printf("End of line: \\n\n");
                break;
            case TOKEN_EOFT:
                printf("End of file: (-1)\n");
                break;
            case TOKEN_COMMA:
                printf("Comma: ','\n");
                break;
            case TOKEN_COLON:
                printf("Colon: ':'\n");
                break;
            case TOKEN_HASHTAG:
                printf("Hashtag: '#'\n");
                break;
            case TOKEN_STAR:
                printf("Star: '*'\n");
                break;
            case TOKEN_NUMBER:
                printf("Number: %d\n", atoi(tokens->token.string.data));
                break;
            case TOKEN_STRING:
                printf("String: %s\n", tokens->token.string.data);
                break;
            case TOKEN_DATA_INS:
            case TOKEN_STRING_INS:
            case TOKEN_ENTRY_INS:
            case TOKEN_EXTERN_INS:
                printf("Non-operative instruction: %s\n", tokens->token.string.data);
                break;
            case TOKEN_ERROR:
                printf("Error token: %s\n", tokens->token.string.data);
                break;
            case TOKEN_REGISTER:
                printf("Register: %s\n", tokens->token.string.data);
                break;
            case TOKEN_MACR:
                printf("Macro start token: %s\n", tokens->token.string.data);
                break;
            case TOKEN_ENDMACR:
                printf("Macro end token: %s\n", tokens->token.string.data);
                break;
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
                printf("Operative instruction: %s\n", tokens->token.string.data);
                break;
            case TOKEN_IDENTIFIER:
                printf("Identifier: %s\n", tokens->token.string.data);
                break;
            default:
                printf("Unknown token type\n");
                break;
        }
        tokens = tokens->next;
    }
}



void lexer_advance_character(Lexer * lexer){
    if (lexer->current_char == '\n') {
        lexer->line_number++;
        lexer->column = 0;
    } else {
        lexer->column++;
    }
    lexer->index++;
    lexer->current_char = string_char_at(lexer->source_code, lexer->index);
}

void lexer_tokenize_comment(Lexer * lexer){
    Token token;
    token.type = TOKEN_COMMENT;
    token.index = lexer->index;
    token.index_in_line = lexer->column;
    token.line = lexer->line_number;
    token.string = string_create();

    while (lexer->current_char != '\0' && lexer->current_char != EOF && lexer->current_char != '\n') {
        string_append_char(&token.string, lexer->current_char);
        lexer_advance_character(lexer);
    }

    /* Comments are typically ignored, so we don't add them to the token list */
    string_free(token.string);
}

void lexer_tokenize_newline(Lexer * lexer){

    Token token;
    token.type = TOKEN_EOL;
    token.index = lexer->index;
    token.index_in_line = lexer->column;
    token.line = lexer->line_number;
    token.string = string_create_from_cstr("\n");


    lexer_advance_character(lexer);
    add_token(lexer, token);
}

void lexer_tokenize_separator(Lexer * lexer){
    Token token;
    token.index = lexer->index;
    token.index_in_line = lexer->column;
    token.line = lexer->line_number;
    token.string = string_create();


    switch (lexer->current_char) {
        case ',':
            token.type = TOKEN_COMMA;
        break;
        case ':':
            token.type = TOKEN_COLON;
        break;
        case '#':
            token.type = TOKEN_HASHTAG;
        break;
        case '*':
            token.type = TOKEN_STAR;
        break;
        default:
            token.type = TOKEN_ERROR;
        break;
    }

    string_append_char(&token.string, lexer->current_char);
    lexer_advance_character(lexer);
    add_token(lexer, token);
}

void lexer_tokenize_number(Lexer * lexer){
    Token token;
    token.type = TOKEN_NUMBER;
    token.index = lexer->index;
    token.index_in_line = lexer->column;
    token.line = lexer->line_number;
    token.string = string_create();


    bool first_char = true;
    while (lexer->current_char != '\0' && lexer->current_char != EOF &&
           (isdigit(lexer->current_char) || (first_char && (lexer->current_char == '+' || lexer->current_char == '-')))) {
        string_append_char(&token.string, lexer->current_char);
        lexer_advance_character(lexer);
        first_char = false;
           }

    if (string_length(token.string) == 1 && (string_char_at(token.string, 0) == '+' || string_char_at(token.string, 0) == '-')) {
        CharError error;
        error.character = string_char_at(token.string, 0);
        error.fileIndex = token.index;
        error.lineIndex = token.index_in_line;
        error.lineNumber = token.line;
        error.message = string_create_from_cstr("it seems that you have a ' - ' or ' + ' without any numerical chars after it");

        error_handler_add_char_error(&lexer->error_handler, LEXER_ERROR_TYPE, error);
        token.type = TOKEN_ERROR;
    }

    add_token(lexer, token);
}

void lexer_tokenize_string(Lexer * lexer){
    bool closer_string_found = false;
    int index = lexer->index;
    int line = lexer->line_number;
    int index_in_line = lexer->line_number;
    int i;

    CharError error;

    Token token;
    token.type = TOKEN_STRING;
    token.index = index;
    token.index_in_line = index_in_line;
    token.line = line;
    token.string = string_create();


    for (i = 0; !(chars_are_equal(lexer->current_char, EOF) || chars_are_equal(lexer->current_char, '\0')); i++){
        if (i != 0 && lexer->current_char == '\"') {
            closer_string_found = true;
        }

        string_append_char(&token.string, lexer->current_char);
        lexer_advance_character(lexer);

        if (closer_string_found == true)
            break;
    }

    if (closer_string_found == false){
        /* there isn't a known non operative instruction that match the one that we know, so we raise an error */
        token.type = TOKEN_ERROR;

        error.character = '\"';
        error.fileIndex = index;
        error.lineIndex = index_in_line;
        error.lineNumber = line;
        error.message = string_create_from_cstr("There is no string after \"");

        error_handler_add_char_error(&lexer->error_handler, LEXER_ERROR_TYPE, error);
    }

    add_token(lexer, token);
}

void lexer_tokenize_non_op_instruction(Lexer * lexer){
    Token token;
    TokenError error;
    token.index = lexer->index;
    token.index_in_line = lexer->column;
    token.line = lexer->line_number;
    token.string = string_create();
    


    while (lexer->current_char != '\0' && lexer->current_char != EOF &&
           (token.string.length == 0 || is_valid_identifier_start(lexer->current_char))) {
        string_append_char(&token.string, lexer->current_char);
        lexer_advance_character(lexer);
           }

    if (string_equals_cstr(token.string, ".data"))
        token.type = TOKEN_DATA_INS;
    else if (string_equals_cstr(token.string, ".string"))
        token.type = TOKEN_STRING_INS;
    else if (string_equals_cstr(token.string, ".entry"))
        token.type = TOKEN_ENTRY_INS;
    else if (string_equals_cstr(token.string, ".extern"))
        token.type = TOKEN_EXTERN_INS;
    else {
        token.type = TOKEN_ERROR;
        error.token = token;
        error.message = string_create_from_cstr("Unknown non-operative instruction");

        error_handler_add_token_error(&lexer->error_handler, LEXER_ERROR_TYPE, error);
    }

    add_token(lexer, token);
}


void lexer_tokenize_identifier(Lexer * lexer){
    Token token;
    token.index = lexer->index;
    token.index_in_line = lexer->column;
    token.line = lexer->line_number;
    token.string = string_create();


    bool is_macro = false;
    if (lexer->current_char == '_') {
        is_macro = true;
    }

    while (lexer->current_char != '\0' && lexer->current_char != EOF &&
           (is_macro ? is_valid_macro_char(lexer->current_char) : is_valid_identifier_char(lexer->current_char))) {
        string_append_char(&token.string, lexer->current_char);
        lexer_advance_character(lexer);
           }

    /* Classify identifiers */
    if (string_equals_cstr(token.string, "r0") || string_equals_cstr(token.string, "r1") ||
        string_equals_cstr(token.string, "r2") || string_equals_cstr(token.string, "r3") ||
        string_equals_cstr(token.string, "r4") || string_equals_cstr(token.string, "r5") ||
        string_equals_cstr(token.string, "r6") || string_equals_cstr(token.string, "r7"))
        token.type = TOKEN_REGISTER;
    else if (string_equals_cstr(token.string, "macr"))
        token.type = TOKEN_MACR;
    else if (string_equals_cstr(token.string, "endmacr"))
        token.type = TOKEN_ENDMACR;
    else if (string_equals_cstr(token.string, "mov"))
        token.type = TOKEN_MOV;
    else if (string_equals_cstr(token.string, "cmp"))
        token.type = TOKEN_CMP;
    else if (string_equals_cstr(token.string, "add"))
        token.type = TOKEN_ADD;
    else if (string_equals_cstr(token.string, "sub"))
        token.type = TOKEN_SUB;
    else if (string_equals_cstr(token.string, "lea"))
        token.type = TOKEN_LEA;
    else if (string_equals_cstr(token.string, "clr"))
        token.type = TOKEN_CLR;
    else if (string_equals_cstr(token.string, "not"))
        token.type = TOKEN_NOT;
    else if (string_equals_cstr(token.string, "inc"))
        token.type = TOKEN_INC;
    else if (string_equals_cstr(token.string, "dec"))
        token.type = TOKEN_DEC;
    else if (string_equals_cstr(token.string, "jmp"))
        token.type = TOKEN_JMP;
    else if (string_equals_cstr(token.string, "bne"))
        token.type = TOKEN_BNE;
    else if (string_equals_cstr(token.string, "red"))
        token.type = TOKEN_RED;
    else if (string_equals_cstr(token.string, "prn"))
        token.type = TOKEN_PRN;
    else if (string_equals_cstr(token.string, "jsr"))
        token.type = TOKEN_JSR;
    else if (string_equals_cstr(token.string, "rts"))
        token.type = TOKEN_RTS;
    else if (string_equals_cstr(token.string, "stop"))
        token.type = TOKEN_STOP;
    else
        token.type = TOKEN_IDENTIFIER;

    add_token(lexer, token);
}

void lexer_tokenize_eof(Lexer *lexer){

    Token token;
    token.type = TOKEN_EOFT;
    token.index = lexer->index;
    token.index_in_line = lexer->column;
    token.line = lexer->line_number;
    token.string = string_create_from_cstr("\0");

    lexer_advance_character(lexer);
    add_token(lexer, token);
}

/*For each character, it checks its type and calls the appropriate function to handle it */
void lexer_analyze(Lexer * lexer){
    while (!chars_are_equal(lexer->current_char, '\0')) {
        if (is_whitespace(lexer->current_char)) {
            lexer_advance_character(lexer); /* we simply move over whitespaces */
        } else if (chars_are_equal(lexer->current_char, ';')) {
            lexer_tokenize_comment(lexer);
        } else if (chars_are_equal(lexer->current_char, '\n')) {
            lexer_tokenize_newline(lexer);
        } else if (chars_are_equal(lexer->current_char, EOF)) {
            lexer_tokenize_eof(lexer);
        } else if (char_exists_in_string(lexer->current_char, ",:#*")) {
            lexer_tokenize_separator(lexer);
        } else if (is_digit(lexer->current_char) || char_exists_in_string(lexer->current_char, "+-")) {
            lexer_tokenize_number(lexer);
        } else if (chars_are_equal(lexer->current_char, '\"')) {
            lexer_tokenize_string(lexer);
        } else if (chars_are_equal(lexer->current_char, '.')) {
            lexer_tokenize_non_op_instruction(lexer);
        } else if (is_valid_identifier_start(lexer->current_char) || lexer->current_char == '_' ){
            lexer_tokenize_identifier(lexer);
        } else {
            CharError error;

            error.character = lexer->current_char;
            error.fileIndex = lexer->index;
            error.lineIndex = lexer->column;
            error.lineNumber = lexer->line_number;
            error.message = string_create_from_cstr("unknown char (in the current context)");

            error_handler_add_char_error(&lexer->error_handler, LEXER_ERROR_TYPE, error);

            lexer_advance_character(lexer);
        }
    }
}

int lexer_is_end_of_input(Lexer *lexer) {
    return lexer->index >= lexer->source_code.length - 1; /* Use index instead of size*/
}


/* ---------------------- FUNCTION HELPER -----------------------------*/
static char* safe_strdup(const char* str) {
    char* new_str;
    size_t len;

    if (str == NULL) {
        fprintf(stderr, "Error: Attempt to duplicate NULL string\n");
        exit(EXIT_FAILURE);
    }

    len = strlen(str) + 1;  /* +1 for the null terminator */
    new_str = (char*)malloc(len);  /* Allocate memory for the new string */

    if (new_str == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in safe_strdup\n");
        exit(EXIT_FAILURE);
    }

    memcpy(new_str, str, len);  /* Copy the string */

    return new_str;
}

static void add_token(Lexer * lexer, Token token){
    TokenNode * new_node = safe_malloc(sizeof(TokenNode));
    new_node->token = token;
    new_node->next = NULL;

    if (lexer->token_list == NULL){
        lexer->token_list = new_node;
    }else{
        TokenNode * last = lexer->token_list;
        while (last->next != NULL){
            last = last->next;
        }
        last->next = new_node;
    }
}


static bool is_valid_macro_char(char ch){
    return isalnum(ch) || ch == '_';
}
