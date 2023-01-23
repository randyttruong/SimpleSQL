/*scanner.h*/

//
// Scanner for SimpleSQL programming language. The scanner reads the input
// stream and turns the characters into language Tokens, such as identifiers,
// keywords, and punctuation.
//
// Prof. Joe Hummel
// Northwestern University
// CS 211, Winter 2023
//

#pragma once

#include <stdio.h>
#include "token.h"

void         scanner_init(int* lineNumber, int* colNumber, char* value);
struct Token scanner_nextToken(FILE* input, int* lineNumber, int* colNumber, char* value);
