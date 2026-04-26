
#include <stdio.h>
#include "ast.h"

static void print_ra_cond(ASTNode *node) {
    if (!node) return;
    if (node->type == NODE_CONDITION) {
        printf("%s", node->value);
    } else if (node->type == NODE_AND) {
        print_ra_cond(node->left);
        printf(" AND ");
        print_ra_cond(node->right);
    } else if (node->type == NODE_OR) {
        print_ra_cond(node->left);
        printf(" OR ");
        print_ra_cond(node->right);
    }
}

void generate_relational_algebra(ASTNode *root) {
    if (!root) return;
    if (root->type != NODE_SELECT) return;

    printf("=== PHASE 5: RELATIONAL ALGEBRA ===\n\n");

    ASTNode *table = root->right;
    ASTNode *where = NULL;
    if (table && table->next && table->next->type == NODE_WHERE)
        where = table->next->left;

    printf("  ");

    int has_projection = 0;
    if (root->left && root->left->type != NODE_STAR) {
        has_projection = 1;
        printf("PI ");
        ASTNode *col = root->left;
        while (col) {
            printf("%s", col->value);
            if (col->next) printf(", ");
            col = col->next;
        }
        printf(" ( ");
    }

    if (where) {
        printf("SIGMA ");
        print_ra_cond(where);
        printf(" ( %s )", table->value);
    } else {
        printf("%s", table->value);
    }

    if (has_projection) {
        printf(" )");
    }

    printf("\n\n");
}
