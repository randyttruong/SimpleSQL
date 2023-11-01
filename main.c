/*main.c*/

//
// Program to open a database and "execute" select queries
// against this database. For now we just output the database
// schema, the AST for each query, and the first 5 lines
// of the references table.
//
// Randy Truong
//

#include <assert.h>  // assert
#include <stdbool.h> // true, false
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strcpy, strcat
#include <strings.h>

#include "execute.h"
//
// main
//
// Prompt for database, open, and then input and
// execute SimpleSQL queries...
//
int main() {
  struct Database *db = NULL;

  //
  // first we need the database name, and then let's
  // try to open it:
  //
  char database[DATABASE_MAX_ID_LENGTH + 1]; // +1 for null terminator

  printf("database? ");
  scanf("%s", database);

  db = database_open(database);

  //
  // Did the database open successfully?
  //
  if (db == NULL) // no
  {
    printf("**Error: unable to open database '%s'\n", database);
    exit(-1);
  }

  //
  // print the schema:
  //
  // print_schema(db);

  //
  // now let's start parsing and analyzing each query:
  //
  parser_init();

  while (true) {
    printf("query? ");

    struct TokenQueue *tokens = NULL;

    //
    // first we check for syntax errors / EOF:
    //
    tokens = parser_parse(stdin);

    if (tokens == NULL) {
      //
      // EOF, or syntax error (error msg already output by parser if so):
      //
      if (parser_eof())
        break;
      else // syntax error, loop around and try another query;
        continue;
    } else // successful parse:
    {
      //
      // analyze the query for semantic errors, and build AST if successful:
      //
      struct QUERY *query = NULL;

      query = analyzer_build(db, tokens);

      tokenqueue_destroy(tokens); // done with the tokens, free memory:

      if (query == NULL) // semantic error, msg already output
      {
        //
        // nothing to do, ignore and loop around and try again:
        //
        continue;
      } else {
        //
        // print AST and output first 5 lines of the table
        // given in the FROM clause of the query:
        //
        // print_ast(query);

        // Creating a resultset struct
        struct ResultSet *rSet;
        rSet = resultset_create();

        // Executing the query
        execute_query(db, query, rSet);

        // Freeing memory associated with the query
        analyzer_destroy(query);

        // Freeing memory associated with the Resultset
        resultset_destroy(rSet);
      }
    } // else
  }   // while

  //
  // done!
  //

  // Freeing memory associated with the database
  database_close(db);

  return 0;
}
