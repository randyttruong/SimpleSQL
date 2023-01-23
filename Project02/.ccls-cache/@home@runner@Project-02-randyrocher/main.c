/*main.c*/

//
// This file serves as an SQL parser and analyzer that allows 
// the user to process .meta and .data files through 
// user-inputted database names and search queries.  
// The file will print out the schema of the database as well as 
// an abstract syntrax tree relating to the inputted search query. 
//
// Randy Truong
// Northwestern University 
// CS 211, Winter 2023 

// Header Files  
#include <stdio.h>
#include <stdlib.h> // malloc()
#include <string.h> // strcat(), strchr() 
#include <strings.h> // strcasecmp() 
// Project02-exclusive header files 
#include "database.h" 
#include "parser.h"
#include "analyzer.h"
#include "ast.h"
#include "scanner.h"

// 
// HELPER FUNCTION
// determineColType 
// 
// Parameters: 
// i - (int) An integer representing an enumerated literal type.  
//
// Returns: 
// A character pointer 
//
// Given an integer i, determines the 'litType' of a given expression from the 
// EXPR struct in ast.h, where each integer represnts an enumerated 'litType' (literal
// type. 
// 
// Returns "unknown" if literal type is not specified (but should not get to that point).
//
const char* determineColType( int i ) 
{
  if ( i == 1 )
     return "int"; 
    
  else if ( i == 2)
    return "real";
    
  else if ( i == 3 )
    return "string";
  
 return "unknown"; 
}

//
// HELPER FUNCTION
// determineIndexType 
//
// Parameters: 
// i - (int) some integer that reperesents an enumerated 'indexType' from the 
// struct Database in database.h 
//
// Return: 
// A character pointer 
// 
// Given some integer, returns the enumerated index type in the form of a character 
// pointer. 
//
// Returns "unknown" if index-type is not found (but should not get to that point). 
// 
const char* determineIndexType( int i )
{
  if ( i == 0 )
    return "non-indexed";
    
  else if ( i == 1 )
    return "indexed";
    
  else if ( i == 2 )
    return "unique indexed";
  
  return "unknown";
}

//
// REQUIRED FUNCTION 
// print_schema 
//
// Parameters: 
// dbFile - (struct Database*) a Database* object  
//
// Return: 
// void 
// 
// Given a Database* object, print_schema prints out the schema of the database
// object. The function returns all possible query inputs, returning (NULL) whenever such
// input does not exist in the inputted query. 
//
void print_schema(struct Database* dbFile)
{
  // Declaring counters for traversing the Database* linked lists
  int i; 
  int n; 

  // Printing the schema 
  printf("**DATABASE SCHEMA**\n");
  printf("Database: %s\n", dbFile->name);
  for ( i = 0;i < dbFile->numTables; i++)
  {
    struct TableMeta currentTable = dbFile->tables[i];
    printf("Table: %s\n", currentTable.name);
    printf("  Record size: %d\n", currentTable.recordSize);
    for ( n = 0;n < currentTable.numColumns; n++ )
    {
      printf(
        "  Column: %s, %s, %s\n", 
        currentTable.columns[n].name, 
        determineColType(currentTable.columns[n].colType), 
        determineIndexType(currentTable.columns[n].indexType));
    }
  }
  printf("**END OF DATABASE SCHEMA**\n");
}

// 
// HELPER FUNCTION
// translateOperator 
//
// Parameters:
// n - (int) An integer from the 'operator' from the EXPR object in the 'ast.h' file.
//
// Return: 
// A character pointer. 
//
// Given an inputted integer, this function returns a character pointer based on the inputted
// integer. The character pointer is dependent on the AST_EXPR_OPERATORS object in the 'ast.h' file. 
// 
// Returns 0 if the inputted integer does not exist within the enumerated list of operators
// (but should not get to that point).
//
char* translateOperator( int n )
{
  if ( n == 0 )
    return "<";
    
  else if ( n == 1 )
    return "<=";
    
  else if ( n == 2 )
    return ">";
    
  else if ( n == 3 )
    return ">=";
    
  else if ( n == 4 )
    return "=";
    
  else if ( n == 5 )
    return "<>";
    
  else if ( n == 6 )
    return "like";
  
  return 0;
}

//
// HELPER FUNCTION 
// translateAsc
//
// Parameters:
// asc - (bool) A boolean value from the ORDERBY object. 

// Return: 
// A character pointer. 
//
// Given an inputted boolean value, this function returns a character pointer 
// that either determines the variant as ascending or descending, based on boolean value.
//
char* translateAsc( bool asc )
{
  if ( asc == true )
    return "ASC";
  
  return "DESC";
}

//
// HELPER FUNCTION 
// translateFunc
//
// Parameters:
// n - (int)
//
// Return: 
// A character pointer.  
//
// Given an integer 'n', this function will return a character poiner based on the integer value. 
// The integer value represents a COLUMN_FUNC_TYPE from the 'ast.h' file and a variable from the 
// COLUMN object . 
//
// Returns 0 if the inputted integer does not exist within the enumerated list of expression
// functions (but should not get to that point).
//
char* translateFunc( int n )
{
  if ( n == 0)
    return "MIN";
    
  else if ( n == 1)
    return "MAX";
    
  else if ( n == 2)
    return "SUM";
    
  else if ( n == 3)
    return "AVG";
    
  else if ( n == 4)
    return "COUNT";

  return 0;
}

//
// REQUIRED FUNCTION 
// print_ast
//
// Parameters: 
// inputAST - (struct QUERY*) An abstract syntax tree that is derived from the inputted
// search query. 
//
// Return:
// void  
//
// Given an inputted QUERY object, prints an abstract syntax tree for all possible queries.
// If a query is equal to NULL, then the function prints out (NULL). The function will NOT accept 
// action queries INSERT, UPDATE, and DELETE. 
// 
// NOTE: The values from the QUERY* object are COMPLETELY user-inputted. Therefore, we 
// cannot rely on the QUERY* object for finding names of tables and columns. 
//
void print_ast( struct QUERY* inputAST )
{
  int i; 

  // Initializing variables for easy access of the SELECT object as well
  // as clauses of select. 
  struct SELECT* querySelect = inputAST->q.select;
  struct JOIN* selectJoin = querySelect->join;
  struct WHERE* selectWhere = querySelect->where; 
  struct ORDERBY* selectOrderBy = querySelect->orderby; 
  struct LIMIT* selectLimit = querySelect->limit; 
  struct INTO* selectInto = querySelect->into; 

  // Iniializing a cursor that traverses the COLUMN struct, which possesses a linked list. 
  struct COLUMN* cursor = querySelect->columns;
  
  // Print QUERY START  
  printf("**QUERY AST**\n"); 

  // Printing Table and Columns by iteratin through linked list. 
  printf("Table: %s\n", querySelect->table);
  if ( querySelect->columns == NULL )
    printf("Columns: (NULL)");
    
  else
  {
  while ( cursor != NULL )
  {
    if ( cursor->function == -1 )
    {
      printf(
        "Select column: %s.%s\n", 
        cursor->table, 
        cursor->name);
      cursor = cursor->next;
    }
      else 
      {
        printf(
          "Select column: %s(%s.%s)\n", 
          translateFunc(cursor->function),
          cursor->table, 
          cursor->name);
        cursor = cursor->next;
      }
  }
  }
  
  // 
  // Printing JOIN* object by taking into consideration table name and column name. 
  //
  if ( selectJoin == NULL ) 
    printf("Join (NULL)\n");
  else 
    printf(
      "Join %s on %s.%s = %s.%s \n", 
      selectJoin->table,
      selectJoin->left->table,
      selectJoin->left->name,
      selectJoin->right->table,
      selectJoin->right->name);

  //
  // Printing WHERE* object by taking into account the table name, column name, expression operator,
  // and the value (which is a string literal).
  //
  // We must consider whether the EXPR object in the WHERE object. From the EXPR object, 
  // we must consider the 'litType' (literal type) of the inputted value, as well as the 
  // (if any). 
  //
  if ( selectWhere == NULL )  
    printf("Where (NULL)\n");
    
  else 
  {
    struct EXPR* selectExpr = selectWhere->expr;
    
    if ( selectExpr->litType == 2 ) 
    {
      
      if ( strchr( selectExpr->value, '\'' ) != NULL ) 
      {
      printf(
        "Where %s.%s %s \"%s\"\n", 
        selectExpr->column->table, 
        selectExpr->column->name,
        translateOperator(selectExpr->operator),
        selectExpr->value);
      }
        
      else 
      {
      printf(
        "Where %s.%s %s \'%s\'\n", 
        selectExpr->column->table, 
        selectExpr->column->name,
        translateOperator(selectExpr->operator),
        selectExpr->value);
      }
    }
      
    else 
    {
      printf(
        "Where %s.%s %s %s\n", 
        selectExpr->column->table, 
        selectExpr->column->name,
        translateOperator(selectExpr->operator),
        selectExpr->value);
    }
  }
  
  //
  // Printing ORDERBY* object by extracting table name, column name, the function
  // and the desired order (ascending or descending).
  // 
  // We want to be able to determine whether the query wants to order the columns
  // by ascending/descending order. 
  //
  if ( selectOrderBy == NULL )
    printf("Order By (NULL)\n");
    
  else if ( selectOrderBy->column->function == -1 ) // Accounts for if there is no "orderby" function 
      printf(
        "Order By %s.%s %s\n", 
        selectOrderBy->column->table, 
        selectOrderBy->column->name, 
        translateAsc(selectOrderBy->ascending));
    
    else 
    {
      printf(
        "Order By %s(%s.%s) %s\n", 
        translateFunc( selectOrderBy->column->function ), 
        selectOrderBy->column->table, 
        selectOrderBy->column->name, 
        translateAsc( selectOrderBy->ascending ));
    }
  //
  // Printing LIMIT* object by extracting integer.
  //
  // We only want to consider the int N. 
  //
  if ( selectLimit == NULL )
    printf("Limit (NULL)\n");
  else 
    printf("Limit %d\n", selectLimit->N);
  
  //
  // Printing INTO* object by extracting table name. 
  //
  if ( selectInto == NULL )
    printf("Into (NULL)\n");
  else 
    printf("Into %s\n", selectInto->table);
  
  printf("**END OF QUERY AST**\n");
}

// 
// HELPER FUNCTION
// findFilename 
//
// Parameters: 
// inputDB - (struct Database*)
// inputAST - (struct QUERY*)
//
// Return: 
// tableName - (char*)
// 
// Given a Database and QUERY object, findFilename will compare the name of the table listed
// in the QUERY object and compare it to the tables within the Database object and return the 
// properly-cased table name (tableName) in the form of a character pointer. 
// 
// Returns 0 if the function cannot create a file path (but should not get to that point).
//
char* findFilename(struct Database* inputDB, struct QUERY* inputAST)
{
  char* tableName = inputAST->q.select->table;
  
  int i; 

  for ( i = 0; i < inputDB->numTables; i++ ) 
  {
    if ( strcasecmp( tableName, inputDB->tables[i].name ) == 0 )
    {
      tableName = inputDB->tables[i].name;
      return tableName;
    }
  }
  return 0;
}

// 
// HELPER FUNCTION 
//
// findBufferSize 
// 
// Parameters: 
// inputDB - (struct Database*) A user-inputted Database object 
// inputAST - (struct QUERY*) A user-inputed QUERY object 
//
// Return: 
// int 
// 
// Given a Database object and a QUERY object, determines the size of the buffer based on the 
// number of records in a given table. Buffer size is returned in the form of an integer. 
// 
// Returns 0 if a buffer size cannot be determined (but should not get to that point).
//
int findBufferSize( struct Database* inputDB, struct QUERY* inputAST )
{
  int i;
  for ( i = 0; i < inputDB->numTables; i++ ) 
  {
    if ( strcasecmp( inputAST->q.select->table, inputDB->tables[i].name ) == 0 )
      return inputDB->tables[i].recordSize + 3 ;
  }
  return 0;
}

// 
// REQUIRED FUNCTION 
//
// execute_query 
// 
// Parameters:
// inputDB - (struct Database*) A user-inputted Database object 
// inputAST - (struct QUERY*) A user-inputted QUERY object
// databaseName - (char*) The name of the database 
// 
// Return:
// void 
// 
// Given a Database object, a QUERY object, as well as a databaseName character poiner, 
// finds a filename path given the objects, opens it, and returns the first five lines of the 
// file. 
//  
// NOTE: databaseNames as well as file paths MUST be 31 characters long. 
void execute_query(struct Database* inputDB, struct QUERY* inputAST, char * databaseName)
{
  //
  // Initializing an empty character array based on the DATABASE_MAX_ID_LENGTH value (32), while 
  // still accounting for the null terminator at the end of the string. 
  char directory[32] = ""; 

  int i;
  //
  // Building the final file path in the format "{database_name}/{table_name}.data"
  // by combining the name of the database, as well as the case-corrected name of the table. 
  strcpy(directory, inputDB->name);
  strcat(directory, "/");
  strcat(directory, findFilename(inputDB, inputAST));
  strcat(directory, ".data");
  
  FILE* currentTable = fopen( directory, "r");

  //
  // Dynamic allocation of the buffer based on the number of characters in the table as well as the 
  // number of records in a given table. 
  char* buffer = (char*) malloc( sizeof(char) * findBufferSize(inputDB, inputAST) );

  // Printing out the first five lines of each file using the adjusted buffer size.
  for ( i = 0; i < 5; i++)
  {
    fgets( buffer, findBufferSize(inputDB, inputAST) , currentTable );
    printf("%s", buffer);
  }
}

int main()
{
  struct Database* mainFile; 
  char databaseName[32]; // Max_ID_Length = 31; therefore array size should be 32 
  printf("database? ");
  scanf( "%s", databaseName ); 

  mainFile = database_open(databaseName);
  if ( mainFile == NULL ) 
  { 
    printf("**Error: unable to open database '%s'.\n", databaseName); 
    return 0; 
  }

  print_schema(mainFile);

  // Initializing parser with parser_init() and TokenQueue and QUERY objects. 
  parser_init();
  struct TokenQueue* userTokens;
  struct QUERY* newAnalyzer;
  
  // Initializing the main query loop and prompter and prints out an abstract syntax tree
  // as well as a 5-line peek of a table. 
  // 
  // Ensures that as long as a EOF character is not inputted, the
  // program will keep asking for queries, even when faced 
  // with a syntax or semantic error. 
  //
  // NOTE: All queries MUST end in a ; (semicolon)
  while (true)
  {
    printf("query? ");
    userTokens = parser_parse(stdin);

    //
    // Breaks the loop if a EOF character is detected ie: "$"
    // and continues prompting the user if no Tokens can be derived from the 
    // input. 
    if ( parser_eof() == true ) 
        break;

    else if ( userTokens == NULL ) 
      continue;
      
    else 
    {
      newAnalyzer = analyzer_build(mainFile, userTokens);
      if ( newAnalyzer == NULL )
        continue;

      print_ast(newAnalyzer);
      execute_query(mainFile, newAnalyzer, databaseName);
    }
  }

  database_close(mainFile);

  return 0; 
}
