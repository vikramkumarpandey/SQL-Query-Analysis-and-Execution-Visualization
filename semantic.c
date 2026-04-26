

#include "ast.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

#define MAX_TABLES 20
#define MAX_COLUMNS 20
#define MAX_NAME 64

typedef struct {
  char name[MAX_NAME];
  char columns[MAX_COLUMNS][MAX_NAME];
  int col_count;
} TableSchema;

static TableSchema schema[MAX_TABLES];
static int schema_count = 0;

static int error_count = 0;

static void trim(char *s) {
  char *start = s;
  while (*start && isspace((unsigned char)*start))
    start++;
  if (start != s)
    memmove(s, start, strlen(start) + 1);
  char *end = s + strlen(s) - 1;
  while (end > s && isspace((unsigned char)*end))
    *end-- = '\0';
}

int load_schema(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Semantic Error: cannot open schema file '%s'\n", filename);
    return -1;
  }

  char line[512];
  schema_count = 0;

  while (fgets(line, sizeof(line), fp)) {

    line[strcspn(line, "\n")] = '\0';
    trim(line);
    if (strlen(line) == 0)
      continue;
    if (line[0] == '#')
      continue;

    char *colon = strchr(line, ':');
    if (!colon) {
      fprintf(stderr, "Semantic Warning: malformed schema line: %s\n", line);
      continue;
    }

    *colon = '\0';
    char *table_name = line;
    char *cols_str = colon + 1;
    trim(table_name);
    trim(cols_str);

    strncpy(schema[schema_count].name, table_name, MAX_NAME - 1);
    schema[schema_count].col_count = 0;

    char *tok = strtok(cols_str, " \t");
    while (tok && schema[schema_count].col_count < MAX_COLUMNS) {
      trim(tok);
      strncpy(schema[schema_count].columns[schema[schema_count].col_count], tok,
              MAX_NAME - 1);
      schema[schema_count].col_count++;
      tok = strtok(NULL, " \t");
    }

    schema_count++;
    if (schema_count >= MAX_TABLES)
      break;
  }

  fclose(fp);
  printf("[Schema] Loaded %d table(s) from '%s'\n\n", schema_count, filename);
  return 0;
}

static TableSchema *find_table(const char *name) {
  for (int i = 0; i < schema_count; i++) {
    if (strcasecmp(schema[i].name, name) == 0) {
      return &schema[i];
    }
  }
  return NULL;
}

static int column_exists(TableSchema *tbl, const char *col) {
  for (int i = 0; i < tbl->col_count; i++) {
    if (strcasecmp(tbl->columns[i], col) == 0) {
      return 1;
    }
  }
  return 0;
}

static void sem_error(const char *fmt, const char *detail) {
  fprintf(stderr, "  Semantic Error: ");
  fprintf(stderr, fmt, detail);
  fprintf(stderr, "\n");
  error_count++;
}

static void validate_columns(ASTNode *node, TableSchema *tbl) {
  while (node) {
    if (node->type == NODE_COLUMN) {
      if (!column_exists(tbl, node->value)) {
        char buf[256];
        snprintf(buf, sizeof(buf), "column '%s' does not exist in table '%s'",
                 node->value, tbl->name);
        sem_error("%s", buf);
      }
    }
    node = node->next;
  }
}

static void validate_condition(ASTNode *node, TableSchema *tbl) {
  if (!node)
    return;
  if (node->type == NODE_CONDITION) {

    if (node->left && node->left->type == NODE_COLUMN) {
      if (!column_exists(tbl, node->left->value)) {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "column '%s' in WHERE does not exist in table '%s'",
                 node->left->value, tbl->name);
        sem_error("%s", buf);
      }
    }
  }
  if (node->type == NODE_AND || node->type == NODE_OR) {
    validate_condition(node->left, tbl);
    validate_condition(node->right, tbl);
  }
}

int analyze(ASTNode *root) {
  if (!root) {
    fprintf(stderr, "Semantic Error: NULL AST\n");
    return 1;
  }

  error_count = 0;
  printf("=== SEMANTIC ANALYSIS ===\n\n");

  const char *table_name = NULL;
  TableSchema *tbl = NULL;

  switch (root->type) {

  case NODE_SELECT: {

    if (root->right && root->right->type == NODE_TABLE) {
      table_name = root->right->value;
    }
    if (!table_name) {
      sem_error("cannot determine table in SELECT query%s", "");
      break;
    }
    tbl = find_table(table_name);
    if (!tbl) {
      sem_error("table '%s' does not exist", table_name);
      break;
    }
    printf("  Table '%s' ... OK\n", table_name);

    if (root->left && root->left->type != NODE_STAR) {
      validate_columns(root->left, tbl);
    }

    if (root->right->next && root->right->next->type == NODE_WHERE) {
      validate_condition(root->right->next->left, tbl);
    }
    break;
  }

  case NODE_INSERT: {
    if (root->left && root->left->type == NODE_TABLE) {
      table_name = root->left->value;
    }
    if (!table_name) {
      sem_error("cannot determine table in INSERT query%s", "");
      break;
    }
    tbl = find_table(table_name);
    if (!tbl) {
      sem_error("table '%s' does not exist", table_name);
      break;
    }
    printf("  Table '%s' ... OK\n", table_name);

    int val_count = 0;
    ASTNode *v = root->right;
    while (v) {
      val_count++;
      v = v->next;
    }
    if (val_count != tbl->col_count) {
      char buf[256];
      snprintf(buf, sizeof(buf),
               "value count (%d) does not match column count (%d) "
               "for table '%s'",
               val_count, tbl->col_count, table_name);
      sem_error("%s", buf);
    } else {
      printf("  Value count (%d) matches column count ... OK\n", val_count);
    }
    break;
  }

  case NODE_UPDATE: {
    if (root->left && root->left->type == NODE_TABLE) {
      table_name = root->left->value;
    }
    if (!table_name) {
      sem_error("cannot determine table in UPDATE query%s", "");
      break;
    }
    tbl = find_table(table_name);
    if (!tbl) {
      sem_error("table '%s' does not exist", table_name);
      break;
    }
    printf("  Table '%s' ... OK\n", table_name);

    if (root->right && root->right->type == NODE_SET) {
      if (root->right->left && root->right->left->type == NODE_COLUMN) {
        if (!column_exists(tbl, root->right->left->value)) {
          char buf[256];
          snprintf(buf, sizeof(buf),
                   "column '%s' in SET does not exist in table '%s'",
                   root->right->left->value, table_name);
          sem_error("%s", buf);
        }
      }

      if (root->right->next && root->right->next->type == NODE_WHERE) {
        validate_condition(root->right->next->left, tbl);
      }
    }
    break;
  }

  case NODE_DELETE: {
    if (root->left && root->left->type == NODE_TABLE) {
      table_name = root->left->value;
    }
    if (!table_name) {
      sem_error("cannot determine table in DELETE query%s", "");
      break;
    }
    tbl = find_table(table_name);
    if (!tbl) {
      sem_error("table '%s' does not exist", table_name);
      break;
    }
    printf("  Table '%s' ... OK\n", table_name);

    if (root->right && root->right->type == NODE_WHERE) {
      validate_condition(root->right->left, tbl);
    }
    break;
  }

  default:
    sem_error("unknown query type for semantic analysis%s", "");
    break;
  }

  if (error_count == 0) {
    printf("\n  Result: PASSED (no semantic errors)\n\n");
  } else {
    printf("\n  Result: FAILED (%d error(s) found)\n\n", error_count);
  }

  return error_count;
}
