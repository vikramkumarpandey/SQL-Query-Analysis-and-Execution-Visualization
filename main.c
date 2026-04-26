
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int   yyparse(void);
extern ASTNode *parsed_ast;

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buf);

extern int  load_schema(const char *filename);
extern int  analyze(ASTNode *root);

extern void generate_execution_plan(ASTNode *root);
extern void generate_relational_algebra(ASTNode *root);
extern void generate_ast_dot(ASTNode *root);
extern void generate_plan_dot(ASTNode *root);

static void print_banner(void) {
    printf("\n");
    printf("=====================================================\n");
    printf("  SQL Query Analysis and Execution Visualization\n");
    printf("  Using Compiler Design Concepts\n");
    printf("  (Flex + Bison + C)\n");
    printf("=====================================================\n\n");
}

static void print_separator(void) {
    printf("-----------------------------------------------------\n\n");
}

int main(int argc, char *argv[]) {
    print_banner();

    if (load_schema("schema.txt") != 0) {
        fprintf(stderr, "Warning: could not load schema; "
                "semantic analysis will be skipped.\n\n");
    }

    char query[1024];
    printf("Enter a SQL query (end with ;):\n> ");
    fflush(stdout);

    if (!fgets(query, sizeof(query), stdin)) {
        fprintf(stderr, "Error: could not read input.\n");
        return EXIT_FAILURE;
    }

    query[strcspn(query, "\n")] = '\0';
    if (query[strlen(query)-2] != ';') {
    strcat(query, ";");
    }

    printf("\n");
    print_separator();

    printf("=== PHASE 1: LEXICAL & SYNTAX ANALYSIS ===\n\n");
    printf("  Input: %s\n\n", query);

    YY_BUFFER_STATE buf = yy_scan_string(query);
    parsed_ast = NULL;

    int parse_result = yyparse();
    yy_delete_buffer(buf);

    if (parse_result != 0 || !parsed_ast) {
        fprintf(stderr, "\nParsing failed. Please check your SQL syntax.\n");
        return EXIT_FAILURE;
    }

    printf("  Parsing ... SUCCESS\n\n");
    print_separator();

    printf("=== PHASE 2: ABSTRACT SYNTAX TREE ===\n\n");
    print_ast(parsed_ast, 1);
    printf("\n");
    print_separator();

    int sem_errors = analyze(parsed_ast);
    print_separator();

    if (sem_errors == 0) {
        printf("=== PHASE 4: EXECUTION PLAN ===\n\n");
        generate_execution_plan(parsed_ast);
        print_separator();
        generate_relational_algebra(parsed_ast);
        print_separator();
        printf("=== PHASE 6: GRAPHVIZ OUTPUT ===\n\n");
        generate_ast_dot(parsed_ast);
        generate_plan_dot(parsed_ast);
        printf("\n");
        print_separator();
    }

    printf("Done.\n\n");
    free_ast(parsed_ast);

    return EXIT_SUCCESS;
}
