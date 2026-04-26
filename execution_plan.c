
#include <stdio.h>
#include "ast.h"

static void print_cond(ASTNode *node) {
    if (!node) return;
    if (node->type == NODE_CONDITION) {
        printf("%s", node->value);
    } else if (node->type == NODE_AND) {
        print_cond(node->left);
        printf(" AND ");
        print_cond(node->right);
    } else if (node->type == NODE_OR) {
        print_cond(node->left);
        printf(" OR ");
        print_cond(node->right);
    }
}

void generate_execution_plan(ASTNode *root) {
    if (!root) {
        printf("[Execution Plan] No AST provided.\n\n");
        return;
    }

    printf("=== EXECUTION PLAN ===\n\n");

    int step = 1;

    switch (root->type) {

    case NODE_SELECT: {
        ASTNode *table = root->right;
        ASTNode *where = NULL;
        if (table && table->next && table->next->type == NODE_WHERE)
            where = table->next->left;

        printf("  Step %d: SCAN table '%s'\n", step++, table->value);

        if (where) {
            printf("  Step %d: FILTER rows where ", step++);
            print_cond(where);
            printf("\n");
        }

        printf("  Step %d: PROJECT columns ", step++);
        if (root->left && root->left->type == NODE_STAR) {
            printf("(all columns)\n");
        } else {
            ASTNode *col = root->left;
            while (col) {
                printf("%s", col->value);
                if (col->next) printf(", ");
                col = col->next;
            }
            printf("\n");
        }

        printf("  Step %d: RETURN result set\n", step++);
        break;
    }

    case NODE_INSERT: {
        ASTNode *table = root->left;
        printf("  Step %d: OPEN table '%s'\n", step++, table->value);

        printf("  Step %d: INSERT values (", step++);
        ASTNode *v = root->right;
        while (v) {
            printf("%s", v->value);
            if (v->next) printf(", ");
            v = v->next;
        }
        printf(")\n");

        printf("  Step %d: WRITE to storage\n", step++);
        printf("  Step %d: CONFIRM insertion\n", step++);
        break;
    }

    case NODE_UPDATE: {
        ASTNode *table = root->left;
        ASTNode *set   = root->right;
        ASTNode *where = NULL;
        if (set && set->next && set->next->type == NODE_WHERE)
            where = set->next->left;

        printf("  Step %d: SCAN table '%s'\n", step++, table->value);

        if (where) {
            printf("  Step %d: FILTER rows where ", step++);
            print_cond(where);
            printf("\n");
        }

        printf("  Step %d: UPDATE column '%s' = '%s'\n", step++,
               set->left->value, set->right->value);
        printf("  Step %d: WRITE changes to storage\n", step++);
        printf("  Step %d: CONFIRM update\n", step++);
        break;
    }

    case NODE_DELETE: {
        ASTNode *table = root->left;
        ASTNode *where = NULL;
        if (root->right && root->right->type == NODE_WHERE)
            where = root->right->left;

        printf("  Step %d: SCAN table '%s'\n", step++, table->value);

        if (where) {
            printf("  Step %d: FILTER rows where ", step++);
            print_cond(where);
            printf("\n");
        }

        printf("  Step %d: DELETE matching rows\n", step++);
        printf("  Step %d: WRITE changes to storage\n", step++);
        printf("  Step %d: CONFIRM deletion\n", step++);
        break;
    }

    default:
        printf("  Unknown query type for execution plan.\n");
        break;
    }

    printf("\n");
}
