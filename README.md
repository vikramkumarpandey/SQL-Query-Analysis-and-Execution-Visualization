
# SQL Query Analysis and Execution Visualization

A compiler design based project for:

* understanding how SQL queries are processed internally
* visualizing query structure using AST
* generating execution plan step-by-step
* converting queries into relational algebra
* visualizing AST and execution plan using Graphviz



## Tech stack

* Language: C
* Lexical Analysis: Flex
* Syntax Analysis: Bison
* Visualization: Graphviz



## Project structure

* `lexer.l` → Tokenization using Flex
* `parser.y` → Grammar rules using Bison
* `ast.c / ast.h` → AST creation and handling
* `semantic.c` → Schema-based validation
* `execution_plan.c` → Execution steps generation
* `relational.c` → Relational algebra conversion
* `graphviz.c` → AST & plan visualization
* `schema.txt` → Table and column definitions
* `main.c` → Main driver program



## How to run

### 1) Install dependencies

Make sure you have:

* Flex
* Bison
* GCC
* Graphviz



### 2) Compile

```bash
flex lexer.l
bison -d parser.y
gcc lex.yy.c parser.tab.c ast.c semantic.c execution_plan.c relational.c graphviz.c main.c -o sql_analyzer
```

---

### 3) Run

```bash
./sql_analyzer
```

Enter a SQL query (end with `;` optional).



## Features

* Lexical and Syntax Analysis using Flex and Bison
* AST (Abstract Syntax Tree) Generation
* Semantic Analysis using schema validation
* Execution Plan Generation (Scan, Filter, Project)
* Relational Algebra Representation
* Graphviz Visualization (AST & Execution Plan)
* Supports basic SQL queries (SELECT, INSERT, UPDATE, DELETE)
* Handles errors for invalid queries and schema mismatches



## Notes

* The project focuses on explaining internal query processing rather than executing real database operations
* Designed mainly for learning compiler design concepts
* Supports single-table queries (joins and nested queries not fully implemented yet)
* Graphviz must be installed for visualization output

