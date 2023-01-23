//*scanner/ 
//
// Scanner for SimpleSQL programming language. The scanner reads the input
// stream and turns the characters into language Tokens, such as identifiers,
// keywords, and punctuation.
//
// Randy Truong 
// Northwestern University
// CS 211, Winter 2023
//
// Contributing author: Prof. Joe Hummel
//

#include <stdio.h>
#include <stdbool.h>  // true, false
#include <ctype.h>    // isspace, isdigit, isalpha
#include <string.h>   // stricmp
#include <strings.h>

#include "util.h"
#include "scanner.h"


//
// SimpleSQL keywords, in alphabetical order. Note that "static" means 
// the array / variable is not accessible outside this file, which is
// intentional and good design.
//
static char* keywords[] = { "asc", "avg", "by", "count", "delete", "desc", "from", "inner",
  "insert", "intersect", "into", "join", "like", "limit", "max", "min", "on", "order", 
  "select", "set", "sum", "union", "update", "values", "where" };

static int numKeywords = sizeof(keywords) / sizeof(keywords[0]);


//
// scanner_init
//
// Initializes line number, column number, and value before
// the start of the next input sequence. 
//
void scanner_init(int* lineNumber, int* colNumber, char* value)
{
  if (lineNumber == NULL || colNumber == NULL || value == NULL)
    panic("one or more parameters are NULL (scanner_init)");

  *lineNumber = 1;
  *colNumber  = 1;
  value[0]    = '\0';  // empty string ""
}


//
// scanner_nextToken
//
// Returns the next token in the given input stream, advancing the line
// number and column number as appropriate. The token's string-based 
// value is returned via the "value" parameter. For example, if the 
// token returned is an integer literal, then the value returned is
// the actual literal in string form, e.g. "123". For an identifer,
// the value is the identifer itself, e.g. "ID" or "title". For a 
// string literal, the value is the contents of the string literal
// without the quotes.
//
char prettyifyStringLiteral( char* value )
{
  char newValue[256];
  newValue[0] = '\0';
  int i;
  int counter = 0;
  char startingChar = value[0];

  for ( i = 0; i < strlen(value); i++)
    {
      if ( (value[i] != startingChar) && (value[i] != EOF) )
        {
          newValue[counter] = value[i];
          counter += 1;
        }
    }
  newValue[counter] = '\0';
  for ( i = 0; i < strlen(newValue); i++ )
    {
        value[i] = newValue[i];
    }
  value[strlen(newValue)] = '\0';
  return *value; 
}

char loopWord( FILE* input, char* value, int c) 
{ 
    value[0] = '\0';
    int counter = 0; 
    value[counter] = c;  

    while ( isalnum(c) != 0 || c == '_')
    {
      value[counter] = c;
      c = fgetc(input);
      counter++;
    }
    ungetc(c, input);
    value[counter] = '\0';
    // printf("the length of value is: %ld\n", strlen(value));
    return *value;
}

char isKeyword( char* value, char** keywords, int numKeywords)
{
  char TokenID;
  if ( strcasecmp(value, "asc") == 0) 
    return SQL_KEYW_ASC; 
  else if ( strcasecmp(value, "avg") == 0) 
    return SQL_KEYW_AVG; 
  else if ( strcasecmp(value, "by") == 0)
    return SQL_KEYW_BY; 
  else if ( strcasecmp(value, "count") == 0) 
    return SQL_KEYW_COUNT; 
  else if ( strcasecmp(value, "delete") == 0) 
    return SQL_KEYW_DELETE; 
  else if ( strcasecmp(value, "desc") == 0) 
    return SQL_KEYW_DESC; 
  else if ( strcasecmp(value, "from") == 0) 
    return SQL_KEYW_FROM; 
  else if ( strcasecmp(value, "inner") == 0) 
    return SQL_KEYW_INNER; 
  else if ( strcasecmp(value, "insert") == 0) 
    return SQL_KEYW_INSERT; 
  else if ( strcasecmp(value, "intersect") == 0) 
    return SQL_KEYW_INTERSECT; 
  else if ( strcasecmp(value, "into") == 0) 
    return SQL_KEYW_INTO; 
  else if ( strcasecmp(value, "join") == 0) 
    return SQL_KEYW_JOIN; 
  else if ( strcasecmp(value, "like") == 0) 
    return SQL_KEYW_LIKE; 
  else if ( strcasecmp(value, "limit") == 0) 
    return SQL_KEYW_LIMIT; 
  else if ( strcasecmp(value, "max") == 0) 
    return SQL_KEYW_MAX; 
  else if ( strcasecmp(value, "min") == 0) 
    return SQL_KEYW_MIN; 
  else if ( strcasecmp(value, "on") == 0) 
    return SQL_KEYW_ON; 
  else if ( strcasecmp(value, "order") == 0) 
    return SQL_KEYW_ORDER; 
  else if ( strcasecmp(value, "select") == 0) 
    return SQL_KEYW_SELECT; 
  else if ( strcasecmp(value, "set") == 0) 
    return SQL_KEYW_SET; 
  else if ( strcasecmp(value, "sum") == 0) 
    return SQL_KEYW_SUM; 
  else if ( strcasecmp(value, "union") == 0) 
    return SQL_KEYW_UNION; 
  else if ( strcasecmp(value, "update") == 0) 
    return SQL_KEYW_UPDATE; 
  else if ( strcasecmp(value, "values") == 0) 
    return SQL_KEYW_VALUES; 
  else if ( strcasecmp(value, "where") == 0) 
    return SQL_KEYW_WHERE; 
  return SQL_IDENTIFIER;  
}

// loopStringLiteral 
// Objective: 
// Whenever a character is ' ' ' (a single quote )
// or ' " ' (a double quote )
// continue looping through everything 
//
//

int loopComment( FILE* input, char* value, int c, int* colNumber) 
{
  int counter = 0;
  c = fgetc(input);
  while (true) 
    {
      if (c == -1 || c == '$' || c == ';' || c == '\n')
      {
        printf("this is the final comment: %s\n", value);
        return 0; 
      }
      value[counter] = c;
      c = fgetc(input);
      counter++;
      printf("this is c: %d\n",c);
    }
  return 0; 
}

char loopDoubleStringLiteral( FILE* input, char* value, int c)
{
  int counter = 0;
  value[0] ='\0'; // resetting the string  
  value[counter] = c; 
  c = fgetc(input);
  counter = counter + 1;
  
  while ( true ) 
    {
      if (c == '\"' || c == EOF || c == '$' || c == '\n' ) 
      {
        value[counter] = c;
        value[counter + 1] = '\0';
        return *value;
      }
      value[counter] = c;
      c = fgetc(input); 
      counter = counter + 1;
    }
  return *value;
}

char loopSingleStringLiteral( FILE* input, char* value, int c)
{
  int counter = 0;
  value[0] ='\0'; // resetting the string  
  value[counter] = c; 
  c = fgetc(input);
  counter = counter + 1;
  
  while ( true ) 
    {
      if ( c == '\'' || c == EOF || c == '$' || c == '\n' ) 
      {
        value[counter] = c;
        value[counter + 1] = '\0';
        return *value;
      }
      value[counter] = c;
      c = fgetc(input); 
      counter = counter + 1;
    }
  return *value;
}

char validDoubleQuoteLiteral( char* value, int* lineNumber, int* colNumber)
{
  // printf("we are now testing doubles for the following value: \n%s\n", value);
  int quoteCounter = 0; 
  int i;
  for ( i = 0; i < strlen(value); i++ ) 
    {
      if ( value[i] == '\"') 
        quoteCounter = quoteCounter + 1; 
    }
  // printf("this is the quoteCounter: %d\n", quoteCounter);
  if ( quoteCounter >= 2 )
    return SQL_STR_LITERAL;
  // printf("WARNING: string literal @ (%d, %d) not terminated properly.\n", *lineNumber, *colNumber);
  return SQL_UNKNOWN;
}

char validSingleQuoteLiteral( char* value, int* lineNumber, int* colNumber)
{
  // printf("we are now testing singles for the following value: \n%s\n", value);
  int quoteCounter = 0; 
  int i;
  for ( i = 0; i < strlen(value); i++ ) 
    {
      if ( value[i] == '\'' ) 
        quoteCounter = quoteCounter + 1; 
    }
  // printf("this is the quoteCounter: %d\n", quoteCounter);
  if ( quoteCounter >= 2 )
    return SQL_STR_LITERAL;
  // printf("WARNING: string literal @ (%d, %d) not terminated properly.\n", *lineNumber, *colNumber);
  return SQL_UNKNOWN;
}
      // real integer literals loop 
      // - is a sequence of one or more digits, with an optional positive or negative sign to start.
      // Examples: “000”, “2”, “+123”, "-20”
    
      // integer literals loop 
      // - containsan optional positive or negative sign at the start, followed by 1 or more digits,
      // followed by a decimal point, followed by 0 or more digits
      // Examples: "123.", "3.14", "-2.0" 
    
char loopIntLiteral( FILE* input, char* value, int c)
{
  int counter = 1;
  value[counter] = c; 
  while ( isdigit(c) != 0 || c == '.' || c == '+' || c == '-')
    {
      if ( c == '*' || c == ',' || c == '#' || c == '<' || c == '>' || c == '(' || c == ')' || c == '_')
      {
        ungetc(c, input);
        value[counter] = '\0';
        return *value;
      }
      value[counter] = c;

      c = fgetc(input);
      counter = counter + 1;
    }
  ungetc(c, input);
  value[counter] = '\0';
  return *value; 
}

char loopIntLiteralNoSign( FILE* input, char* value, int c )
{
  int counter = 0;
  while ( isdigit(c) != 0 || c == '.' || c == '+' || c == '-' || isspace(c) == 0 )
    {
      if ( c == '*' || c == ',' || c == '#' || c == '<' || c == '>' || c == '(' || c == ')' || c == '_')
      {
        ungetc(c, input);
        value[counter] = '\0';
        return *value;
      }

      value[counter] = c;
      c = fgetc(input);      
      if ( (c == '.') && ( value[counter] == '.') ) 
      {
        ungetc(c, input);
        value[counter + 1] = '\0';
        return *value;
      }
      counter = counter + 1;
    }
  ungetc(c, input);
  value[counter] = '\0';
  // printf("the length of value is: %ld\n", strlen(value));
  return *value; 
}

char isReal( char* value )
{
  char TokenID;
  int i;
  int len = strlen(value); 
  for ( i = 0; i < strlen(value); i++) 
    { 
      if (value[i] == '.') 
      {
        return SQL_REAL_LITERAL; 
      }
    }
  return SQL_INT_LITERAL;
}

// Next Steps 
// 1. If c == '+' or c == '-', start the loopIntLiteral() loop 
// 2. If isdigit(c) != 0, start the loopIntLiteral() loop 


struct Token scanner_nextToken(FILE* input, int* lineNumber, int* colNumber, char* value)
{
  if (input == NULL)
    panic("input stream is NULL (scanner_nextToken)");
  if (lineNumber == NULL || colNumber == NULL || value == NULL)
    panic("one or more parameters are NULL (scanner_nextToken)");

  struct Token T;

  //
  // repeatedly input characters one by one until a token is found:
  //
  while (true)
  {
    int c = fgetc(input);
    // printf("this is c: %d at this position: %d\n", c, *colNumber);
    
		// end of line 
    if (c == EOF)  // no more input, return EOS:
    {
      T.id = SQL_EOS;
      T.line = *lineNumber;
      *lineNumber = *lineNumber + 1;
      T.col = *colNumber;
      *colNumber=  1; 

      value[0] = '$';
      value[1] = '\0';

      return T;
    }
    else if (c == '\n')  // no more input, return EOS:
    {
      T.id = SQL_EOS;
      T.line = *lineNumber;
      *lineNumber = *lineNumber + 1;
      T.col = *colNumber;
      *colNumber=  1; 
    }
		// '$' end of line 
    else if (c == '$')  // this is also EOF / EOS
    {
      T.id = SQL_EOS;
      T.line = *lineNumber;
      *lineNumber = *lineNumber + 1;
      T.col = *colNumber;
      *colNumber=  1; 

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }
		
		// semicolon 
    else if (c == ';')
    {
      T.id = SQL_SEMI_COLON;
      T.line = *lineNumber;
      T.col = *colNumber;
      *colNumber=  *colNumber + 1; 

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }
		
		// greater than symbol 
    else if (c == '>')  // could be > or >=
    {
      //
      // peek ahead to the next char:
      //
      c = fgetc(input);

      if (c == '=')
      {
        T.id = SQL_GTE;
        T.line = *lineNumber;
        T.col = *colNumber;
        *colNumber=  *colNumber + 2; 

        value[0] = '>';  
        value[1] = '=';
        value[2] = '\0';

        return T;
      }

      //
      // if we get here, then next char did not form a token, so 
      // we need to put char back to be processed on next call:
      //
      ungetc(c, input);

      T.id = SQL_GT;
      T.line = *lineNumber;
      T.col = *colNumber;
      *colNumber=  *colNumber + 1; 

      value[0] = '>';
      value[1] = '\0';

      return T;
    }
	
	// left paren 
	else if ( c == '(' )
	{
		T.id = SQL_LEFT_PAREN;
		T.line = *lineNumber; 
		T.col = *colNumber;
    *colNumber=  *colNumber + 1; 
		
		value[0] = '(';
		value[1] = '\0';
		
		return T; 
	}

	// right paren 
	else if ( c == ')' )
	{
		T.id = SQL_RIGHT_PAREN;
		T.line = *lineNumber; 
		T.col = *colNumber;
    *colNumber=  *colNumber + 1; 
		value[0] = ')';
		value[1] = '\0';
		
		return T; 
	}

	// asterisk 
	else if ( c == '*' )
	{
		T.id = SQL_ASTERISK;
		T.line = *lineNumber; 
		T.col = *colNumber;
    *colNumber=  *colNumber + 1; 

		value[0] = (char)c;
		value[1] = '\0';
		
		return T; 
	}

	// dot 
	else if ( c == '.' )
	{
		T.id = SQL_DOT;
		T.line = *lineNumber; 
		T.col = *colNumber;
    *colNumber=  *colNumber + 1; 
		value[0] = '.';
		value[1] = '\0';
		
		return T; 
	}

	// hash
	else if ( c == '#' )
	{
		T.id = SQL_HASH;
		T.line = *lineNumber; 
		T.col = *colNumber;
    *colNumber=  *colNumber + 1; 
		
		value[0] = '#';
		value[1] = '\0';
		
		return T; 
	}

	// "," comma 
	else if ( c == ',' )
	{
		T.id = SQL_COMMA;
		T.line = *lineNumber; 
		T.col = *colNumber;
    *colNumber=  *colNumber + 1; 

		value[0] = ',';
		value[1] = '\0';
		
		return T; 
	}

	// "=" equal 
	else if ( c == '=' )
	{
		T.id = SQL_EQUAL;
		T.line = *lineNumber; 
		T.col = *colNumber;
    *colNumber=  *colNumber + 1; 
		
		value[0] = '=';
		value[1] = '\0';
		
		return T; 
	}

	// "<" less than  
	else if ( c == '<' )
	{
		// peeking 
		c = fgetc(input);
		
		// if there is a "=" after 
		if ( c == '=' ) 
		{
		
			T.id = SQL_LTE; 
			T.line = *lineNumber; 
			T.col = *colNumber; 
      *colNumber=  *colNumber + 2; 
		
			value[0] = '<';
			value[1] = '=';
			value[2] = '\0'; 
			
			return T; 
		}
		
		// if there is a ">" after 
		else if ( c == '>' ) 
		{
			
			T.id = SQL_NOT_EQUAL; 
			T.line = *lineNumber; 
		  T.col = *colNumber;
      *colNumber=  *colNumber + 2; 
			
			value[0] = '<';
			value[1] = '>';
			value[2] = '\0'; 
			
			return T; 
		}
		
		c = ungetc(c, input); 
			
		T.id = SQL_LT;
		T.line = *lineNumber; 
		T.col = *colNumber;
    *colNumber=  *colNumber + 1; 

		value[0] = '<';
		value[1] = '\0';
			
		return T; 
	}

	// whitespace 
	else if ( isspace(c) != 0 )  
	{
    // printf("whitespace at %d\n", *colNumber);
    *colNumber=  *colNumber + 1; 
	}

	// copy of the top but just for the \n 
    else if (c == '\\')  
    {
      //
      // peek ahead to the next char:
      //
      c = fgetc(input);

      if (c == 'n')
      {
        T.id = SQL_GTE;
        T.line = *lineNumber;
        *colNumber=  *colNumber + 1; 
        T.col = *colNumber;

        value[0] = '\\';
        value[1] = 'n';
        value[2] = '\0';

        return T;
      }

      //
      // if we get here, then next char did not form a token, so 
      // we need to put char back to be processed on next call:
      // TODO: FIX THIS 
      ungetc(c, input);

      T.id = SQL_UNKNOWN;
      T.line = *lineNumber;
      T.col = *colNumber;
      *colNumber=  *colNumber + 1; 

      value[0] = (char)c;
      value[1] = '\0';


      return T;
    }
    else if ( isalpha(c) > 0) 
    { 
      *value = loopWord(input, value, c);
      T.id = isKeyword(value, keywords, numKeywords);
      T.line = *lineNumber;
      T.col = *colNumber;
      *colNumber=  *colNumber + strlen(value); 
      return T;
    }
    else if ( c == '\"')  // 39 = ' in ASCII and 34 = " in ASCII
    {
      value[0] = '\0';
      *value = loopDoubleStringLiteral( input, value, c );
      T.id = validDoubleQuoteLiteral(value, lineNumber, colNumber);
      T.line = *lineNumber;
      T.col = *colNumber;
      *colNumber = *colNumber + strlen(value);
      *value = prettyifyStringLiteral(value); 

      return T; 
    }
    else if ( c == '\'' ) 
    {
      value[0] = '\0';
      value[0] = c; 
      *value = loopSingleStringLiteral( input, value, c );
      T.id = validSingleQuoteLiteral(value, lineNumber, colNumber);
      T.line = *lineNumber;
      T.col = *colNumber;
      *colNumber = *colNumber + strlen(value);
      *value = prettyifyStringLiteral(value); 
      return T;
    }
    else if ( c == '+' ) 
    {
      char original = c;
      value[0] = '\0';
      value[0] = c;
      c = fgetc(input);
      if ( isdigit(c) != 0 )
      {
        *value = loopIntLiteral(input, value, c);
        T.id = isReal(value);
        T.line = *lineNumber;
        T.col = *colNumber;
        *colNumber = *colNumber + strlen(value);
        return T;
      }
      else
      { 
        ungetc(c, input);
        T.id = SQL_UNKNOWN;
        T.line = *lineNumber;
        T.col = *colNumber;
        *colNumber = *colNumber + 1;

        value[0] = '+';
        value[1] = '\0';
        return T;
      }
      return T;
    }
      // TODO: 
      // Add funtionality for comments 
      // Condition, if the character after '-' is '-',
      // continue looping through the current line 
    else if ( c == '-' )
    {      
      value[0] = '\0';
      value[0] = c;
      c = fgetc(input);
      if ( isdigit(c) != 0 )
      {
        *value = loopIntLiteral(input, value, c);
        T.id = isReal(value);
        T.line = *lineNumber;
        T.col = *colNumber;
        *colNumber = *colNumber + strlen(value);
        return T;
      }
      else if ( c == '-' ) 
      {
        while ( (c != -1) && (c != '\n') ) 
        {
          c = fgetc(input);
        }
        *colNumber = 1;
        *lineNumber = *lineNumber + 1 ;
      }
      else
      { 
        ungetc(c, input);
        T.id = SQL_UNKNOWN;
        T.line = *lineNumber;
        T.col = *colNumber;
        *colNumber = *colNumber + 1;

        value[0] = '-';
        value[1] = '\0';
        return T;
      }
    }
    else if ( isdigit(c) != 0 )
    {
      *value = loopIntLiteralNoSign(input, value, c); 
      T.id = isReal(value);
      T.line = *lineNumber;
      T.col = *colNumber;
      *colNumber = *colNumber + strlen(value);

      return T; 
    }
    else
    {
      //
      // if we get here, then char denotes an UNKNOWN token:
      //
      T.id = SQL_UNKNOWN;
      T.line = *lineNumber;
      T.col = *colNumber;
      *colNumber=  *colNumber + 1; 

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }
	
	
	// design a while loop that for as long as the next character is not a non-alpha numeric character, continue reading it as a single word 
	
  }//while

  //
  // execution should never get here, return occurs
  // from within loop
  //
}

