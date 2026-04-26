# SQL-Query-Analysis-and-Execution-Visualization

This project is a **compiler-based SQL query analyzer** built using **Flex (Lex) and Bison (Yacc)**. It simulates how a database processes SQL queries by implementing key phases of a compiler, including lexical analysis, parsing, semantic validation, and intermediate representation.

The system takes SQL queries as input and generates:

* An **Abstract Syntax Tree (AST)** for structural representation
* **Relational Algebra expressions** (π, σ) for logical interpretation
* A **step-by-step execution plan** (SCAN → FILTER → PROJECT → RESULT)
* **Graphical visualizations** of AST and execution plans using Graphviz

The project also includes schema-based validation to ensure that table and column references in queries are correct. It supports interactive input and automatically generates and displays visual outputs for better understanding.

---

## Features

* SQL parsing using Flex & Bison (LALR parser)
* AST generation and traversal
* Semantic analysis using schema validation
* Relational algebra conversion
* Execution plan generation
* Automatic Graphviz-based visualization (images generated and displayed)
* Supports queries with or without semicolon

---

## Tech Stack

* C Programming
* Flex (Lex)
* Bison (Yacc)
* Graphviz

 ## Purpose

This project demonstrates how **compiler design concepts** can be applied to **database query processing**, providing insight into how SQL queries are analyzed, validated, and executed internally.



Just tell 👍

