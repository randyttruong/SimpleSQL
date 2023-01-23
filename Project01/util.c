/*util.c*/

//
// Project: utility functions for SimpleSQL
// 
// Prof. Joe Hummel
// Northwestern University
// CS 211, Winter 2023
//

#include <stdio.h>
#include <stdlib.h>

#include "util.h"


void panic(char* msg)
{
   printf("**PANIC\n");
   printf("**PANIC: %s\n", msg);
   printf("**PANIC\n");

   exit(-1);
}
