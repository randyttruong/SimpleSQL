# SimpleSQL
A *simple* text-based relational database management system that contains a
subset of core functionality from SQL.

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
### Semantic Analyzer
### Executor

