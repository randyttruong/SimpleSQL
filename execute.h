/*execute.h*/

//
// Project: Execution of queries for SimpleSQL
//
// Randy Truong
//

#pragma once

#include "analyzer.h"
#include "ast.h"
#include "database.h"
#include "execute.h"
#include "parser.h"
#include "resultset.h"
#include "scanner.h"
#include "token.h"
#include "tokenqueue.h"
#include "util.h"

//
// #include header files needed to compile function declarations
//

//
// function declarations:
//

// Executing the query
void execute_query(struct Database *db, struct QUERY *query,
                   struct ResultSet *rSet);
