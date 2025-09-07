#ifndef NODE_H
#define NODE_H

#include "error_handler.h"
#include "token.h"

typedef enum NodeType {
    ENTRY_NODE,
    EXTERNAL_NODE,
    DATA_NODE,
    STRING_NODE,
    INSTRUCTION_NODE
} NodeType;

typedef struct EntryNode {
    Token *entry_label;  /* The token (label) that is referenced as entry */
    bool has_parser_error; /* True if a parser error occurred, false otherwise */
} EntryNode;

typedef struct EntryNodeList {
    EntryNode entry_node;  /* The current entry node */
    struct EntryNodeList *next; /* Pointer to the next entry node */
} EntryNodeList;

typedef struct ExternalNode {
    Token *external_label;  /* The token (label) that is referenced as external */
    bool has_parser_error; /* True if a parser error occurred, false otherwise */
} ExternalNode;

typedef struct ExternalNodeList {
    ExternalNode external_node;  /* The current external node */
    struct ExternalNodeList *next; /* Pointer to the next external node */
} ExternalNodeList;

typedef struct DataNode {
    TokenReferenceNode *data_numbers; /* List of numbers in the data node */
    bool has_parser_error; /* True if a parser error occurred, false otherwise */
} DataNode;

typedef struct StringNode {
    Token *string_label; /* The token (label) representing the string */
    bool has_parser_error; /* True if a parser error occurred, false otherwise */
} StringNode;

typedef struct InstructionOperand {
    Token *operand; /* The operand token */
    bool is_dereferenced; /* True if the operand is dereferenced, false otherwise */
} InstructionOperand;

typedef struct InstructionNode {
    Token *operation; /* The operation token */
    Token *first_operand; /* The first operand token */
    Token *second_operand; /* The second operand token */
    bool is_first_operand_derefrenced; /* True if the first operand is derefrenced else false */
    bool is_second_operand_derefrenced; /* True if the second operand is derefrenced else false */
    bool has_parser_error; /* True if got a parser error, else, false */
} InstructionNode;

typedef struct InstructionNodeList {
    InstructionNode node; /* Current instruction node */
    struct InstructionNodeList *next; /* Next node */
} InstructionNodeList;

typedef struct GuidanceNodeList {
    union {
        DataNode dataNode;
        StringNode stringNode;
    } node; /* The current guidance node (either data or string) */
    NodeType type; /* The type of the guidance node */
    struct GuidanceNodeList *next; /* Pointer to the next guidance node */
} GuidanceNodeList;

typedef struct LabelNode {
    Token *label; /* The label identifier (NULL for guidance nodes without a label) */
    InstructionNodeList *instruction_list; /* List of instruction sentences (NULL for guidance labels) */
    GuidanceNodeList *guidance_list; /* List of guidance sentences (NULL for instruction labels) */
    unsigned int size; /* The memory size occupied by the label */
    unsigned int position; /* The memory position of the label */
} LabelNode;

typedef struct AssemblyStatement {
    union {
        DataNode data_node;
        StringNode string_node;
        InstructionNode instruction_node;
        EntryNode entry_node;
        ExternalNode external_node;
    } node; /* The current node (data, string, instruction, entry, or external) */
    NodeType type; /* The type of the node */
    bool has_parser_error; /* True if a parser error occurred in the corresponding node, false otherwise */
    unsigned int size; /* The memory size occupied by the node */
    unsigned int position; /* The position of the node in its respective image */
} AssemblyStatement;

typedef struct AssemblyStatementList {
    AssemblyStatement node; /* Current sentence */
    struct AssemblyStatementList *next; /* The next sentence in the sequence */
} AssemblyStatementList;

typedef struct LabelNodeList {
    LabelNode label; /* The current label */
    struct LabelNodeList *next; /* Next label */
} LabelNodeList;

typedef struct TranslationUnit {
    ExternalNodeList *external_list; /* The list of all the external nodes */
    EntryNodeList *entry_list; /* The list of all the entry nodes */
    LabelNodeList *instruction_label_list; /* The instructions label list */
    LabelNodeList *guidance_label_list; /* The guidance label list */
    ErrorHandler error_handler; /* The error handler of the translation unit */
    TokenNode *tokens; /* The token list reference from the lexer */
} TranslationUnit;

#endif /* NODE_H */