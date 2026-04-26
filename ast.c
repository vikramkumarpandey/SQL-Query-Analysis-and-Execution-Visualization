

#include "ast.h"

static char *safe_strdup(const char *s) {
    if (!s) return NULL;
    char *dup = strdup(s);
    if (!dup) {
        fprintf(stderr, "Error: out of memory in safe_strdup\n");
        exit(EXIT_FAILURE);
    }
    return dup;
}

ASTNode *create_node(NodeType type, const char *value,
                     ASTNode *left, ASTNode *right) {
    ASTNode *n = (ASTNode *)malloc(sizeof(ASTNode));
    if (!n) {
        fprintf(stderr, "Error: out of memory in create_node\n");
        exit(EXIT_FAILURE);
    }
    n->type  = type;
    n->value = safe_strdup(value);
    n->left  = left;
    n->right = right;
    n->next  = NULL;
    return n;
}

ASTNode *create_table_node(const char *name) {
    return create_node(NODE_TABLE, name, NULL, NULL);
}

ASTNode *create_column_node(const char *name) {
    return create_node(NODE_COLUMN, name, NULL, NULL);
}

ASTNode *create_value_node(const char *val) {
    return create_node(NODE_VALUE, val, NULL, NULL);
}

ASTNode *create_star_node(void) {
    return create_node(NODE_STAR, "*", NULL, NULL);
}

ASTNode *create_condition_node(const char *col, const char *op,
                               const char *val) {

    size_t len = strlen(col) + strlen(op) + strlen(val) + 4;
    char *buf = (char *)malloc(len);
    if (!buf) {
        fprintf(stderr, "Error: out of memory in create_condition_node\n");
        exit(EXIT_FAILURE);
    }
    snprintf(buf, len, "%s %s %s", col, op, val);

    ASTNode *n = create_node(NODE_CONDITION, buf, NULL, NULL);

    n->left  = create_column_node(col);
    n->right = create_value_node(val);
    free(buf);
    return n;
}

ASTNode *create_and_node(ASTNode *left, ASTNode *right) {
    return create_node(NODE_AND, "AND", left, right);
}

ASTNode *create_or_node(ASTNode *left, ASTNode *right) {
    return create_node(NODE_OR, "OR", left, right);
}

ASTNode *create_select_node(ASTNode *columns, ASTNode *table,
                            ASTNode *where) {
    ASTNode *n = create_node(NODE_SELECT, "SELECT", columns, table);
    if (where) {

        table->next = where;
    }
    return n;
}

ASTNode *create_insert_node(ASTNode *table, ASTNode *values) {
    return create_node(NODE_INSERT, "INSERT", table, values);
}

ASTNode *create_update_node(ASTNode *table, ASTNode *set,
                            ASTNode *where) {
    ASTNode *n = create_node(NODE_UPDATE, "UPDATE", table, set);
    if (where) {
        set->next = where;
    }
    return n;
}

ASTNode *create_delete_node(ASTNode *table, ASTNode *where) {
    return create_node(NODE_DELETE, "DELETE", table, where);
}

ASTNode *append_column(ASTNode *list, ASTNode *col) {
    if (!list) return col;
    ASTNode *cur = list;
    while (cur->next) cur = cur->next;
    cur->next = col;
    return list;
}

ASTNode *append_value(ASTNode *list, ASTNode *val) {
    if (!list) return val;
    ASTNode *cur = list;
    while (cur->next) cur = cur->next;
    cur->next = val;
    return list;
}

const char *node_type_str(NodeType t) {
    switch (t) {
        case NODE_SELECT:      return "SELECT";
        case NODE_INSERT:      return "INSERT";
        case NODE_UPDATE:      return "UPDATE";
        case NODE_DELETE:      return "DELETE";
        case NODE_TABLE:       return "TABLE";
        case NODE_COLUMN:      return "COLUMN";
        case NODE_COLUMN_LIST: return "COLUMN_LIST";
        case NODE_VALUE:       return "VALUE";
        case NODE_VALUE_LIST:  return "VALUE_LIST";
        case NODE_CONDITION:   return "CONDITION";
        case NODE_AND:         return "AND";
        case NODE_OR:          return "OR";
        case NODE_STAR:        return "STAR";
        case NODE_SET:         return "SET";
        case NODE_WHERE:       return "WHERE";
        default:               return "UNKNOWN";
    }
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  ");

    printf("[%s]", node_type_str(node->type));
    if (node->value) {
        printf(" \"%s\"", node->value);
    }
    printf("\n");

    print_ast(node->left,  indent + 1);
    print_ast(node->right, indent + 1);

    print_ast(node->next,  indent);
}

void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->next);
    if (node->value) free(node->value);
    free(node);
}
