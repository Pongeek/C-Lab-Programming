#ifndef TOKEN_H
#define TOKEN_H

#include "string_util.h"

typedef enum {
    TOKEN_COMMENT, /* ; ... */
    TOKEN_REGISTER, /* r0, r1, r2, r3, r4, r5, r6, r7 */
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER, /* 25 -25 +25 */
    TOKEN_STRING, /* "..." */

    TOKEN_MOV, /* mov - 0 */
    TOKEN_CMP, /* cmp - 1 */
    TOKEN_ADD, /* add - 2 */
    TOKEN_SUB, /* sub - 3 */
    TOKEN_LEA, /* lea - 4 */
    TOKEN_CLR, /* clr - 5 */
    TOKEN_NOT, /* not - 6 */
    TOKEN_INC, /* inc - 7 */
    TOKEN_DEC, /* dec - 8 */
    TOKEN_JMP, /* jmp - 9 */
    TOKEN_BNE, /* bne - 10 */
    TOKEN_RED, /* red - 11 */
    TOKEN_PRN, /* prn - 12 */
    TOKEN_JSR, /* jsr - 13 */
    TOKEN_RTS, /* rts - 14 */
    TOKEN_STOP, /* stop - 15 */
    TOKEN_MACR, /* macr */
    TOKEN_ENDMACR, /* endmacr */
    TOKEN_DATA_INS, /* .data */
    TOKEN_STRING_INS, /* .string */
    TOKEN_ENTRY_INS, /* .entry */
    TOKEN_EXTERN_INS, /* .extern */

    TOKEN_COMMA, /* , */
    TOKEN_COLON, /* : */
    TOKEN_HASHTAG, /* # */
    TOKEN_STAR, /* * */

    TOKEN_ERROR, /* an error has raised in this token */

    TOKEN_EOL, /* end of line */
    TOKEN_EOFT /* end of file token (end of token stream) \0 */
} TokenType;

typedef struct Token {
    TokenType type;  /* Token type */
    unsigned int index;  /* Index of the starting char of the token (in the file) */
    unsigned int index_in_line; /* Index of the starting char of the token (in the token line) */
    unsigned int line;  /* Index of a token is inside */
    String string; /* String data */
} Token;

typedef struct TokenNode {
    Token token;  /* the current token */
    struct TokenNode *next;  /* the next token */
} TokenNode;

typedef struct TokenReferenceNode {
    Token *token; /* Current token */
    struct TokenReferenceNode *next; /* Next token */
} TokenReferenceNode;

#endif /* TOKEN_H */