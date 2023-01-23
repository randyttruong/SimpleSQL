/*main.c*/

//
// Project 01: main program to test scanner for SimpleSQL
// 
// Prof. Joe Hummel
// Northwestern University
// CS 211, Winter 2023
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false

#include "token.h"    // token defs
#include "scanner.h"  // scanner
#include "util.h"     // panic


//
// consumeRestOfLine
// 
// Reads from given input stream until EOL or EOF reached.
//
void consumeRestOfLine(FILE* input)
{
  int c = fgetc(input);
  while (c != '\n' && c != EOF)
    c = fgetc(input);
}


//
// main
//
int main(int argc, char* argv[])
{
  FILE* input = NULL;
  bool  keyboardInput = false;

  //
  // where is input coming from, keyboard or sql
  // program file?
  //
  if (argc == 1)  // no command-line args, => keyboard input
  {
    // 
    // input from the keyboard, aka stdin:
    //
    input = stdin;
    keyboardInput = true;
  }
  else if (argc == 2)  // we have a program arg:
  {
    //
    // open input file as our source of input:
    //
    char* sqlfile = argv[1];  // 2nd arg

    //
    // can we open the file?
    //
    input = fopen(sqlfile, "r");
    if (input == NULL) // unable to open:
    {
      printf("**ERROR: unable to open sqlfile '%s' for input.\n", sqlfile);
      panic("usage: simplesql [sqlfile]\n");
    }

    keyboardInput = false;
  }
  else // too many program arguments, something is wrong:
  {
    printf("**ERROR: too many command-line arguments.\n");
    panic("usage: simplesql [sqlfile]\n");
  }

  //
  // input the tokens, either from keyboard or the 
  // given sql file; the "input" variable controls the 
  // source. We use the scanner to input the tokens one 
  // by one from the input stream. We'll do this over
  // and over again for testing purposes, until user 
  // enters $:
  //
  while (true)
  {
    int lineNumber = -1;
    int colNumber = -1;
    char value[256] = "";
    struct Token T;

    // setup to start scanning:
    scanner_init(&lineNumber, &colNumber, value);

    if (keyboardInput)  // prompt the user:
    {
      printf("input? ");
    }

    //
    // process the input token by token until we see ; or $
    //
    T = scanner_nextToken(input, &lineNumber, &colNumber, value);

    while (T.id != SQL_SEMI_COLON && T.id != SQL_EOS)
    {
      printf("%d (%d,%d): %s\n", T.id, T.line, T.col, value);
      T = scanner_nextToken(input, &lineNumber, &colNumber, value);
    }

    // output that last token:
    printf("%d (%d,%d): %s\n", T.id, T.line, T.col, value);

    //
    // Does the user want to stop?
    //
    if (T.id == SQL_EOS) // end of input stream, stop:
    {
      break;
    }
    else // consume what's left of input stream before we prompt again:
    {
      consumeRestOfLine(input);
    }
  }//while

  //
  // done:
  //
  return 0;
}
