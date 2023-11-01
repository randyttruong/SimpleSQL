# SimpleSQL
An experiment for creating a *simple* text-based relational
database management system that contains a subset of core
functionality from SQL.

## Currently Supported
- SELECT, INSERT
- GROUP BY
- WHERE + Binary Operators

## Components
1) Lexical Analyzer
2) Syntactic Analyzer
3) Semantic Analyzer
4) Executor

### Lexical Analyzer
- Files: `scanner.o`, `scanner.h`, `scanner.c`
This lexical analyzer/lexer tokenizes a given SQL query, tokenizing it
into the form of a tuple of `(token_key, row_start, column_start)`.

These tokens can be categorized as the following:
- Symbolic Characters (such as parentheses, periods, commas, etc)
- Identifiers (which are common SQL commands)
- String literals (strings that are encased in quotation marks)
- Integer literals (strings of numbers that are unsigned and without floating point)
- Real literals (strings of numbers that are signed with and without floating point)
- Unknown (an unknown identifier)

These tokens are then added into a Queue data structure known as the TokenQueue.
### Syntactic Analyzer
Receives a `TokenQueue` object and then compares tokens to Backus-Naur form in order to validate
syntactically acceptable SQL queries. If error, then break query. Otherwise, pass to semantic
analyzer.

### Semantic Analyzer
Receives a `TokenQueue` object containing a syntactically-valid SQL query, then
constructs an abstract syntax tree that can be categorized via the structure of SQL query.
Syntax tree is developed based on previous token. If semantic analysis
is successful, then return abstract syntax tree for query execution.

### Executor
Given an abstract syntax tree as well as a parsed database, construct
resulting database from query via linked list traversal.

