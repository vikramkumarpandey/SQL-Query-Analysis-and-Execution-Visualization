
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

static int node_id;

static void ast_dot_node(FILE *fp, ASTNode *node, int parent_id) {
    if (!node) return;

    int my_id = node_id++;

    if (node->value)
        fprintf(fp, "  n%d [label=\"%s\\n%s\"];\n", my_id,
                node_type_str(node->type), node->value);
    else
        fprintf(fp, "  n%d [label=\"%s\"];\n", my_id,
                node_type_str(node->type));

    if (parent_id >= 0)
        fprintf(fp, "  n%d -> n%d;\n", parent_id, my_id);

    ast_dot_node(fp, node->left,  my_id);
    ast_dot_node(fp, node->right, my_id);
    ast_dot_node(fp, node->next,  my_id);
}

void generate_ast_dot(ASTNode *root) {
    if (!root) return;

    FILE *fp = fopen("output/ast.dot", "w");
    if (!fp) {
        fprintf(stderr, "Error: cannot create output/ast.dot\n");
        return;
    }

    node_id = 0;
    fprintf(fp, "digraph AST {\n");
    fprintf(fp, "  node [shape=box, style=filled, fillcolor=lightblue, fontname=Courier];\n");
    fprintf(fp, "  edge [color=gray40];\n");
    ast_dot_node(fp, root, -1);
    fprintf(fp, "}\n");
    fclose(fp);
    system("dot -Tpng output/ast.dot -o output/ast.png");
    system("start output/ast.png");
    printf("  [Graphviz] AST written to output/ast.dot\n");
}

static int plan_id;

static void plan_dot_cond(FILE *fp, ASTNode *node, char *buf, int bufsize) {
    if (!node) return;
    if (node->type == NODE_CONDITION) {
        int len = strlen(buf);
        snprintf(buf + len, bufsize - len, "%s", node->value);
    } else if (node->type == NODE_AND) {
        plan_dot_cond(fp, node->left, buf, bufsize);
        int len = strlen(buf);
        snprintf(buf + len, bufsize - len, " AND ");
        plan_dot_cond(fp, node->right, buf, bufsize);
    } else if (node->type == NODE_OR) {
        plan_dot_cond(fp, node->left, buf, bufsize);
        int len = strlen(buf);
        snprintf(buf + len, bufsize - len, " OR ");
        plan_dot_cond(fp, node->right, buf, bufsize);
    }
}

void generate_plan_dot(ASTNode *root) {
    if (!root) return;

    FILE *fp = fopen("output/plan.dot", "w");
    if (!fp) {
        fprintf(stderr, "Error: cannot create output/plan.dot\n");
        return;
    }

    plan_id = 0;
    fprintf(fp, "digraph ExecutionPlan {\n");
    fprintf(fp, "  node [shape=box, style=filled, fillcolor=lightyellow, fontname=Courier];\n");
    fprintf(fp, "  edge [color=gray40];\n");
    fprintf(fp, "  rankdir=TB;\n");

    switch (root->type) {

    case NODE_SELECT: {
        ASTNode *table = root->right;
        ASTNode *where = NULL;
        if (table && table->next && table->next->type == NODE_WHERE)
            where = table->next->left;

        int scan_id = plan_id++;
        fprintf(fp, "  n%d [label=\"SCAN\\n%s\", fillcolor=lightgreen];\n",
                scan_id, table->value);

        int prev_id = scan_id;

        if (where) {
            char buf[512] = "";
            plan_dot_cond(fp, where, buf, sizeof(buf));
            int filter_id = plan_id++;
            fprintf(fp, "  n%d [label=\"FILTER\\n%s\"];\n", filter_id, buf);
            fprintf(fp, "  n%d -> n%d;\n", prev_id, filter_id);
            prev_id = filter_id;
        }

        int proj_id = plan_id++;
        if (root->left && root->left->type == NODE_STAR) {
            fprintf(fp, "  n%d [label=\"PROJECT\\n*\"];\n", proj_id);
        } else {
            char cols[512] = "";
            ASTNode *col = root->left;
            while (col) {
                int len = strlen(cols);
                snprintf(cols + len, sizeof(cols) - len, "%s%s",
                         col->value, col->next ? ", " : "");
                col = col->next;
            }
            fprintf(fp, "  n%d [label=\"PROJECT\\n%s\"];\n", proj_id, cols);
        }
        fprintf(fp, "  n%d -> n%d;\n", prev_id, proj_id);

        int ret_id = plan_id++;
        fprintf(fp, "  n%d [label=\"RESULT\", fillcolor=lightsalmon];\n", ret_id);
        fprintf(fp, "  n%d -> n%d;\n", proj_id, ret_id);
        break;
    }

    case NODE_INSERT: {
        ASTNode *table = root->left;
        int open_id = plan_id++;
        fprintf(fp, "  n%d [label=\"OPEN\\n%s\", fillcolor=lightgreen];\n",
                open_id, table->value);

        char vals[512] = "";
        ASTNode *v = root->right;
        while (v) {
            int len = strlen(vals);
            snprintf(vals + len, sizeof(vals) - len, "%s%s",
                     v->value, v->next ? ", " : "");
            v = v->next;
        }
        int ins_id = plan_id++;
        fprintf(fp, "  n%d [label=\"INSERT\\n(%s)\"];\n", ins_id, vals);
        fprintf(fp, "  n%d -> n%d;\n", open_id, ins_id);

        int wr_id = plan_id++;
        fprintf(fp, "  n%d [label=\"WRITE\"];\n", wr_id);
        fprintf(fp, "  n%d -> n%d;\n", ins_id, wr_id);

        int conf_id = plan_id++;
        fprintf(fp, "  n%d [label=\"CONFIRM\", fillcolor=lightsalmon];\n", conf_id);
        fprintf(fp, "  n%d -> n%d;\n", wr_id, conf_id);
        break;
    }

    case NODE_UPDATE: {
        ASTNode *table = root->left;
        ASTNode *set   = root->right;
        ASTNode *where = NULL;
        if (set && set->next && set->next->type == NODE_WHERE)
            where = set->next->left;

        int scan_id = plan_id++;
        fprintf(fp, "  n%d [label=\"SCAN\\n%s\", fillcolor=lightgreen];\n",
                scan_id, table->value);

        int prev_id = scan_id;

        if (where) {
            char buf[512] = "";
            plan_dot_cond(fp, where, buf, sizeof(buf));
            int filter_id = plan_id++;
            fprintf(fp, "  n%d [label=\"FILTER\\n%s\"];\n", filter_id, buf);
            fprintf(fp, "  n%d -> n%d;\n", prev_id, filter_id);
            prev_id = filter_id;
        }

        int upd_id = plan_id++;
        fprintf(fp, "  n%d [label=\"UPDATE\\n%s = %s\"];\n",
                upd_id, set->left->value, set->right->value);
        fprintf(fp, "  n%d -> n%d;\n", prev_id, upd_id);

        int wr_id = plan_id++;
        fprintf(fp, "  n%d [label=\"WRITE\"];\n", wr_id);
        fprintf(fp, "  n%d -> n%d;\n", upd_id, wr_id);

        int conf_id = plan_id++;
        fprintf(fp, "  n%d [label=\"CONFIRM\", fillcolor=lightsalmon];\n", conf_id);
        fprintf(fp, "  n%d -> n%d;\n", wr_id, conf_id);
        break;
    }

    case NODE_DELETE: {
        ASTNode *table = root->left;
        ASTNode *where = NULL;
        if (root->right && root->right->type == NODE_WHERE)
            where = root->right->left;

        int scan_id = plan_id++;
        fprintf(fp, "  n%d [label=\"SCAN\\n%s\", fillcolor=lightgreen];\n",
                scan_id, table->value);

        int prev_id = scan_id;

        if (where) {
            char buf[512] = "";
            plan_dot_cond(fp, where, buf, sizeof(buf));
            int filter_id = plan_id++;
            fprintf(fp, "  n%d [label=\"FILTER\\n%s\"];\n", filter_id, buf);
            fprintf(fp, "  n%d -> n%d;\n", prev_id, filter_id);
            prev_id = filter_id;
        }

        int del_id = plan_id++;
        fprintf(fp, "  n%d [label=\"DELETE\"];\n", del_id);
        fprintf(fp, "  n%d -> n%d;\n", prev_id, del_id);

        int wr_id = plan_id++;
        fprintf(fp, "  n%d [label=\"WRITE\"];\n", wr_id);
        fprintf(fp, "  n%d -> n%d;\n", del_id, wr_id);

        int conf_id = plan_id++;
        fprintf(fp, "  n%d [label=\"CONFIRM\", fillcolor=lightsalmon];\n", conf_id);
        fprintf(fp, "  n%d -> n%d;\n", wr_id, conf_id);
        break;
    }

    default:
        fprintf(fp, "  n0 [label=\"UNKNOWN\"];\n");
        break;
    }

    fprintf(fp, "}\n");
    fclose(fp);
    system("dot -Tpng output/plan.dot -o output/plan.png");
    system("start output/plan.png");
    printf("  [Graphviz] Execution plan written to output/plan.dot\n");
}
