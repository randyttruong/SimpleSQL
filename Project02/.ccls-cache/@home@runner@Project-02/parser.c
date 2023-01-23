/*parser.c*/

//
// Recursive-descent parsing functions for SimpleSQL programming language.
// The parser is responsible for checking if the input follows the syntax
// ("grammar") rules of SimpleSQL. If successful, a copy of the tokens is
// returned so an analysis can be performed, an AST built, and the query
// executed.
//
// Prof. Joe Hummel
// Northwestern University
// Winter 2023
//

#include <ctype.h>   // isspace
#include <stdbool.h> // true, false
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "scanner.h"
#include "token.h"
#include "tokenqueue.h"
#include "util.h"

//
// helper functions:
//
static void errorMsg(char *expecting, char *value, struct Token found) {
  printf("**SYNTAX ERROR: expecting '%s', found '%s' @ (%d, %d)\n", expecting,
         value, found.line, found.col);
}

static bool match(struct TokenQueue *tokens, int expectedID,
                  char *expectedValue) {
  //
  // does the token match the expected token?
  //
  struct Token curToken = tokenqueue_peekToken(tokens);
  char *curValue = tokenqueue_peekValue(tokens);

  if (curToken.id != expectedID) // no, => error
  {
    errorMsg(expectedValue, curValue, curToken);
    return false;
  }

  //
  // yes, it matched, so discard and return true:
  //
  tokenqueue_dequeue(tokens);

  return true;
}

//
// recursive-descent parsing functions:
//

// <column> ::= <table> . ID
//            | ID
//
static bool column(struct TokenQueue *tokens) {
  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_HASH) // must be a table name
  {
    match(tokens, SQL_HASH, "#");

    if (!match(tokens, SQL_IDENTIFIER, "identifier such as 'id' or 'title'"))
      return false;

    if (!match(tokens, SQL_DOT, "."))
      return false;

    if (!match(tokens, SQL_IDENTIFIER, "identifier such as 'id' or 'title'"))
      return false;

    return true;
  }

  //
  // otherwise it must be either ID or ID . ID
  //
  if (!match(tokens, SQL_IDENTIFIER, "identifier such as 'id' or 'title'"))
    return false;

  curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_DOT) {
    match(tokens, SQL_DOT, ".");

    if (!match(tokens, SQL_IDENTIFIER, "identifier such as 'id' or 'title'"))
      return false;

    return true;
  } else {
    // . ID was optional, so okay if missing
    return true;
  }
}

//
// <literal> ::= INT_LITERAL
//             | REAL_LITERAL
//             | STR_LITERAL
//
static bool literal(struct TokenQueue *tokens) {
  struct Token curToken = tokenqueue_peekToken(tokens);
  char *curValue = tokenqueue_peekValue(tokens);

  if (curToken.id == SQL_INT_LITERAL) {
    match(tokens, SQL_INT_LITERAL, "integer literal such as '10' or '123'");

    return true;
  } else if (curToken.id == SQL_REAL_LITERAL) {
    match(tokens, SQL_REAL_LITERAL, "real literal such as '12.' or '3.14'");

    return true;
  } else if (curToken.id == SQL_STR_LITERAL) {
    match(tokens, SQL_STR_LITERAL, "string literal such as 'The Matrix'");

    return true;
  } else {
    errorMsg("integer, real or string literal", curValue, curToken);
    return false;
  }
}

//
// <operator> ::= < | <= | > | >= | = | <>
//
static bool operator(struct TokenQueue *tokens) {
  struct Token curToken = tokenqueue_peekToken(tokens);
  char *curValue = tokenqueue_peekValue(tokens);

  if (curToken.id == SQL_LT) {
    match(tokens, SQL_LT, "<");

    return true;
  } else if (curToken.id == SQL_LTE) {
    match(tokens, SQL_LTE, "<=");

    return true;
  } else if (curToken.id == SQL_GT) {
    match(tokens, SQL_GT, ">");

    return true;
  } else if (curToken.id == SQL_GTE) {
    match(tokens, SQL_GTE, ">=");

    return true;
  } else if (curToken.id == SQL_EQUAL) {
    match(tokens, SQL_EQUAL, "=");

    return true;
  } else if (curToken.id == SQL_NOT_EQUAL) {
    match(tokens, SQL_NOT_EQUAL, "<>");

    return true;
  } else {
    errorMsg("<, <=, >, >=, =, or <>", curValue, curToken);
    return false;
  }
}

//
// <expr> ::= <column> <operator> <literal>
//          | <column> like STR_LITERAL
//
static bool expr(struct TokenQueue *tokens) {
  if (!column(tokens))
    return false;

  //
  // what's next? If we see "like", then it's rule 2 like STR_LITERAL
  //
  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_KEYW_LIKE) // <column> like STR_LITERAL
  {
    match(tokens, SQL_KEYW_LIKE, "like");

    if (!match(tokens, SQL_STR_LITERAL, "string literal such as 'The Matrix'"))
      return false;

    return true;
  }

  //
  // otherwise it's rule 1 <operator> <literal>
  //
  if (!operator(tokens))
    return false;

  if (!literal(tokens))
    return false;

  return true;
}

//
// <function> ::= min | max | sum | avg | count
//
static bool function(struct TokenQueue *tokens) {
  struct Token curToken = tokenqueue_peekToken(tokens);
  char *curValue = tokenqueue_peekValue(tokens);

  if (curToken.id == SQL_KEYW_MIN) {
    match(tokens, SQL_KEYW_MIN, "min");

    return true;
  } else if (curToken.id == SQL_KEYW_MAX) {
    match(tokens, SQL_KEYW_MAX, "max");

    return true;
  } else if (curToken.id == SQL_KEYW_SUM) {
    match(tokens, SQL_KEYW_SUM, "sum");

    return true;
  } else if (curToken.id == SQL_KEYW_AVG) {
    match(tokens, SQL_KEYW_AVG, "avg");

    return true;
  } else if (curToken.id == SQL_KEYW_COUNT) {
    match(tokens, SQL_KEYW_COUNT, "count");

    return true;
  } else {
    errorMsg("min/max/avg/sum/count", curValue, curToken);
    return false;
  }
}

//
// <column_or_function> ::= <column>
//                        | <function> ( <column> )
//
static bool column_or_function(struct TokenQueue *tokens) {
  struct Token curToken = tokenqueue_peekToken(tokens);
  char *curValue = tokenqueue_peekValue(tokens);

  if (curToken.id == SQL_HASH) // must be a table name ==> column
  {
    if (!column(tokens))
      return false;

    return true;
  } else if (curToken.id == SQL_IDENTIFIER) // ID => column
  {
    if (!column(tokens))
      return false;

    return true;
  } else if (curToken.id == SQL_KEYW_MIN || curToken.id == SQL_KEYW_MAX ||
             curToken.id == SQL_KEYW_SUM || curToken.id == SQL_KEYW_AVG ||
             curToken.id == SQL_KEYW_COUNT) {
    if (!function(tokens))
      return false;

    if (!match(tokens, SQL_LEFT_PAREN, "("))
      return false;

    if (!column(tokens))
      return false;

    if (!match(tokens, SQL_RIGHT_PAREN, ")"))
      return false;

    return true;
  } else {
    errorMsg("column or function(column)", curValue, curToken);
    return false;
  }
}

//
// <columns_or_functions> ::= <column_or_function> [, <column_or_function>]*
//
static bool columns_or_functions(struct TokenQueue *tokens) {
  struct Token curToken;

  //
  // a column or function followed by 0 or more columns or functions...
  //
  while (true) {
    if (!column_or_function(tokens))
      return false;

    curToken = tokenqueue_peekToken(tokens);

    if (curToken.id != SQL_COMMA) // no more columns or functions, exit loop:
      break;
    else // match comma and process another column:
      match(tokens, SQL_COMMA, ",");
  }

  return true;
}

//
// <columns> ::= <column> [, <column>]*
//
static bool columns(struct TokenQueue *tokens) {
  struct Token curToken;

  //
  // a column followed by 0 or more columns...
  //
  while (true) {
    if (!column(tokens))
      return false;

    curToken = tokenqueue_peekToken(tokens);

    if (curToken.id != SQL_COMMA) // no more columns, exit loop:
      break;
    else // match comma and process another column:
      match(tokens, SQL_COMMA, ",");
  }

  return true;
}

//
// <table> ::= ID
//           | # ID
//
static bool table(struct TokenQueue *tokens) {
  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_HASH) {
    match(tokens, SQL_HASH, "#");
  }

  //
  // must have ID:
  //
  if (!match(tokens, SQL_IDENTIFIER, "identifier such as 'id' or 'title'"))
    return false;

  return true;
}

//
// <literals> ::= <literal> [, <literal>]*
//
static bool literals(struct TokenQueue *tokens) {
  struct Token curToken;

  //
  // a literal followed by 0 or more literals...
  //
  while (true) {
    if (!literal(tokens))
      return false;

    curToken = tokenqueue_peekToken(tokens);

    if (curToken.id != SQL_COMMA) // no more literals, exit loop:
      break;
    else // match comma and process another literal:
      match(tokens, SQL_COMMA, ",");
  }

  return true;
}

//
// <into> ::= into # ID
//
static bool into(struct TokenQueue *tokens) {
  if (!match(tokens, SQL_KEYW_INTO, "into"))
    return false;

  if (!match(tokens, SQL_HASH, "#"))
    return false;

  if (!match(tokens, SQL_IDENTIFIER, "temporary table name"))
    return false;

  return true;
}

//
// <limit> ::= limit INT_LITERAL
//
static bool limit(struct TokenQueue *tokens) {
  if (!match(tokens, SQL_KEYW_LIMIT, "limit"))
    return false;

  if (!match(tokens, SQL_INT_LITERAL, "integer literal such as '10' or '123'"))
    return false;

  return true;
}

//
// <orderby> ::= order by <column_or_function> [ asc | desc ]
//
static bool orderby(struct TokenQueue *tokens) {
  if (!match(tokens, SQL_KEYW_ORDER, "order"))
    return false;

  if (!match(tokens, SQL_KEYW_BY, "by"))
    return false;

  if (!column_or_function(tokens))
    return false;

  //
  // optional asc or desc keyword:
  //
  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_KEYW_ASC) {
    // ascending order
    match(tokens, SQL_KEYW_ASC, "asc");

    return true;
  } else if (curToken.id == SQL_KEYW_DESC) {
    // descending order
    match(tokens, SQL_KEYW_DESC, "desc");

    return true;
  } else {
    // optional, default is ascending order:
    return true;
  }
}

//
// <where> ::= where <expr>
//
static bool where(struct TokenQueue *tokens) {
  if (!match(tokens, SQL_KEYW_WHERE, "where"))
    return false;

  if (!expr(tokens))
    return false;

  return true;
}

//
// <join> ::= inner join <table> on <column> = <column>
//
static bool join(struct TokenQueue *tokens) {
  if (!match(tokens, SQL_KEYW_INNER, "inner"))
    return false;

  if (!match(tokens, SQL_KEYW_JOIN, "join"))
    return false;

  if (!table(tokens))
    return false;

  if (!match(tokens, SQL_KEYW_ON, "on"))
    return false;

  if (!column(tokens))
    return false;

  if (!match(tokens, SQL_EQUAL, "="))
    return false;

  if (!column(tokens))
    return false;

  //
  // if get here, parsed successfully:
  //
  return true;
}

//
// <selectable_columns> ::= *
//                        | <columns_or_functions>
//
static bool selectable_columns(struct TokenQueue *tokens) {
  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_ASTERISK) {
    match(tokens, SQL_ASTERISK, "*");

    return true;
  } else {
    if (!columns_or_functions(tokens))
      return false;

    return true;
  }
}

//
// <baseselect> ::= select <selectable_columns> from <table>
//                  [<join>] [<where>] [<orderby>] [<limit>] [<into>]
//
static bool baseselect(struct TokenQueue *tokens) {
  if (!match(tokens, SQL_KEYW_SELECT, "select"))
    return false;

  if (!selectable_columns(tokens))
    return false;

  if (!match(tokens, SQL_KEYW_FROM, "from"))
    return false;

  if (!table(tokens))
    return false;

  //
  // optional inner join? where? order by? limit? into?
  //
  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_KEYW_INNER) {
    if (!join(tokens))
      return false;

    curToken = tokenqueue_peekToken(tokens);
  }

  if (curToken.id == SQL_KEYW_WHERE) {
    if (!where(tokens))
      return false;

    curToken = tokenqueue_peekToken(tokens);
  }

  if (curToken.id == SQL_KEYW_ORDER) {
    if (!orderby(tokens))
      return false;

    curToken = tokenqueue_peekToken(tokens);
  }

  if (curToken.id == SQL_KEYW_LIMIT) {
    if (!limit(tokens))
      return false;

    curToken = tokenqueue_peekToken(tokens);
  }

  if (curToken.id == SQL_KEYW_INTO) {
    if (!into(tokens))
      return false;

    curToken = tokenqueue_peekToken(tokens);
  }

  //
  // if we get here, we parsed successfully:
  //
  return true;
}

//
// <select> ::= <baseselect>
//            | <baseselect> union <baseselect>
//            | <baseselect> intersect <baseselect>
//
static bool parser_select(struct TokenQueue *tokens) {
  if (!baseselect(tokens))
    return false;

  //
  // now peek ahead and see if optional component exists:
  //
  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_KEYW_UNION) {
    match(tokens, SQL_KEYW_UNION, "union");

    return baseselect(tokens);
  } else if (curToken.id == SQL_KEYW_INTERSECT) {
    match(tokens, SQL_KEYW_INTERSECT, "intersect");

    return baseselect(tokens);
  } else {
    //
    // union and intersect are optional, so if we get here, successful parse:
    //
    return true;
  }
}

//
// <update> ::= update <table> set <column> = <expr> [ <where> ]
//
static bool update(struct TokenQueue *tokens) {
  if (!match(tokens, SQL_KEYW_UPDATE, "update"))
    return false;

  if (!table(tokens))
    return false;

  if (!match(tokens, SQL_KEYW_SET, "set"))
    return false;

  if (!column(tokens))
    return false;

  if (!match(tokens, SQL_EQUAL, "="))
    return false;

  if (!expr(tokens))
    return false;

  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_KEYW_WHERE) {
    // optional where clause
    return where(tokens);
  } else {
    // where is optional, so okay if missing:
    return true;
  }
}

//
// <insert> ::= insert into <table> ( <columns> ) values ( <literals> )
//
static bool insert(struct TokenQueue *tokens) {
  if (!match(tokens, SQL_KEYW_INSERT, "insert"))
    return false;

  if (!match(tokens, SQL_KEYW_INTO, "into"))
    return false;

  if (!table(tokens))
    return false;

  if (!match(tokens, SQL_LEFT_PAREN, "("))
    return false;

  if (!columns(tokens))
    return false;

  if (!match(tokens, SQL_RIGHT_PAREN, ")"))
    return false;

  if (!match(tokens, SQL_KEYW_VALUES, "values"))
    return false;

  if (!match(tokens, SQL_LEFT_PAREN, "("))
    return false;

  if (!literals(tokens))
    return false;

  if (!match(tokens, SQL_RIGHT_PAREN, ")"))
    return false;

  return true;
}

//
// <delete> ::= delete from <table> [ <where> ]
//
static bool delete (struct TokenQueue *tokens) {
  if (!match(tokens, SQL_KEYW_DELETE, "delete"))
    return false;

  if (!match(tokens, SQL_KEYW_FROM, "from"))
    return false;

  if (!table(tokens))
    return false;

  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == SQL_KEYW_WHERE) {
    // optional where clause
    return where(tokens);
  } else {
    // where is optional, so okay if missing:
    return true;
  }
}

//
// <action> ::= <insert>
//            | <update>
//            | <delete>
//
static bool action(struct TokenQueue *tokens) {
  struct Token curToken = tokenqueue_peekToken(tokens);
  char *curValue = tokenqueue_peekValue(tokens);

  if (curToken.id == SQL_KEYW_INSERT) {
    return insert(tokens);
  } else if (curToken.id == SQL_KEYW_UPDATE) {
    return update(tokens);
  } else if (curToken.id == SQL_KEYW_DELETE) {
    return delete (tokens);
  } else {
    errorMsg("insert/update/delete", curValue, curToken);
    return false;
  }
}

//
// <query> ::= <select> ;
//           | <action> ;
//           | $
//
static bool query(struct TokenQueue *tokens) {
  struct Token curToken = tokenqueue_peekToken(tokens);
  char *curValue = tokenqueue_peekValue(tokens);

  if (curToken.id == SQL_KEYW_SELECT) {
    if (!parser_select(tokens))
      return false;

    if (!match(tokens, SQL_SEMI_COLON, ";"))
      return false;

    return true;
  } else if (curToken.id == SQL_KEYW_INSERT || curToken.id == SQL_KEYW_UPDATE ||
             curToken.id == SQL_KEYW_DELETE) {
    if (!action(tokens))
      return false;

    if (!match(tokens, SQL_SEMI_COLON, ";"))
      return false;

    return true;
  } else if (curToken.id == SQL_EOS) {
    match(tokens, SQL_EOS, "$");

    return true;
  } else {
    errorMsg("select/insert/update/delete", curValue, curToken);
    return false;
  }
}

//
// public functions:
//
static bool parser_reachedEOF = false;

void parser_init(void) { parser_reachedEOF = false; }

bool parser_eof(void) { return parser_reachedEOF; }

struct TokenQueue *parser_parse(FILE *input) {
  if (input == NULL)
    panic("input stream is NULL (parser_parse)");

  //
  // Since we are parsing potentially one "program" after another,
  // there will be newline / whitespace between programs. So we
  // need to skip any whitespace before we start parsing:
  //
  int c = fgetc(input);

  while (isspace(c)) {
    c = fgetc(input);
  }

  ungetc(c, input); // put the non-space char back for processing:

  //
  // First, let's get all the tokens and store them
  // into a queue:
  //
  int lineNumber, colNumber;
  char value[256];
  struct Token token;
  struct TokenQueue *tokens;

  scanner_init(&lineNumber, &colNumber, value);

  token = scanner_nextToken(input, &lineNumber, &colNumber, value);
  tokens = tokenqueue_create();

  while (token.id != SQL_SEMI_COLON && token.id != SQL_EOS) {
    tokenqueue_enqueue(tokens, token, value);

    token = scanner_nextToken(input, &lineNumber, &colNumber, value);
  }

  // enqueue the final token:
  tokenqueue_enqueue(tokens, token, value);

  // tokenqueue_print(tokens);

  //
  // now duplicate the tokens so that we have a copy after the
  // parsing process is over --- we need a copy so we can return
  // in case the parsing is successful. The tokens are then used
  // for analysis and AST building.
  //
  struct TokenQueue *duplicate;

  duplicate = tokenqueue_duplicate(tokens);

  // tokenqueue_print(duplicate);

  //
  // okay, now let's parse the input tokens:
  //
  bool result = query(tokens);

  //
  // done, free memory and return AST or NULL:
  //
  tokenqueue_destroy(tokens);

  if (result) // parse was successful
  {
    struct Token T = tokenqueue_peekToken(duplicate);

    if (T.id == SQL_EOS) // end of input, no command to execute:
    {
      parser_reachedEOF = true; // let the user know

      tokenqueue_destroy(duplicate);

      return NULL;
    }

    //
    // otherwise we have a query, so return the duplicate tokens:
    //
    return duplicate;
  } else // syntax error, nothing to execute:
  {
    tokenqueue_destroy(duplicate);

    return NULL;
  }
}
