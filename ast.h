

#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {

    NODE_SELECT,
    NODE_INSERT,
    NODE_UPDATE,
    NODE_DELETE,

    NODE_TABLE,
    NODE_COLUMN,
    NODE_COLUMN_LIST,
    NODE_VALUE,
    NODE_VALUE_LIST,
    NODE_CONDITION,
    NODE_AND,
    NODE_OR,
    NODE_STAR,
    NODE_SET,
    NODE_WHERE
} NodeType;

typedef struct ASTNode {
    NodeType        type;
    char           *value;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *next;
} ASTNode;

ASTNode *create_node(NodeType type, const char *value,
                     ASTNode *left, ASTNode *right);

ASTNode *create_table_node(const char *name);
ASTNode *create_column_node(const char *name);
ASTNode *create_value_node(const char *val);
ASTNode *create_star_node(void);
ASTNode *create_condition_node(const char *col, const char *op,
                               const char *val);
ASTNode *create_and_node(ASTNode *left, ASTNode *right);
ASTNode *create_or_node(ASTNode *left, ASTNode *right);

ASTNode *create_select_node(ASTNode *columns, ASTNode *table,
                            ASTNode *where);
ASTNode *create_insert_node(ASTNode *table, ASTNode *values);
ASTNode *create_update_node(ASTNode *table, ASTNode *set,
                            ASTNode *where);
ASTNode *create_delete_node(ASTNode *table, ASTNode *where);

ASTNode *append_column(ASTNode *list, ASTNode *col);
ASTNode *append_value(ASTNode *list, ASTNode *val);

void print_ast(ASTNode *node, int indent);

void free_ast(ASTNode *node);

const char *node_type_str(NodeType t);

#endif
