/*execute.c*/

//
// Project: Execution of queries for SimpleSQL
//
// Randy Truong
//

//
// #include any other system <.h> files?
//
#include <assert.h>
#include <stdbool.h> // true, false
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

//
// #include any other of our ".h" files?
//
#include "ast.h"
#include "database.h"
#include "resultset.h"
#include "util.h"

//
// execute_query
//
// execute a select query, which for now means print the resulting parts of a
// database reference in the query
//
void execute_query(struct Database *db, struct QUERY *query,
                   struct ResultSet *rSet) {

  // Ensuring the database and query exist
  if (db == NULL)
    panic("db is NULL (execute)");
  if (query == NULL)
    panic("query is NULL (execute)");

  // Ensuring that only the select type is in the query, since it is the focus
  // of this project
  if (query->queryType != SELECT_QUERY) {
    printf("**INTERNAL ERROR: execute() only supports SELECT queries.\n");
    return;
  }

  struct SELECT *select = query->q.select; // alias for less typing:

  //
  // the query has been analyzed and so we know it's correct: the
  // database exists, the table(s) exist, the column(s) exist, etc.
  //

  //
  // (1) we need a pointer to the table meta data, so find it:
  //
  struct TableMeta *tablemeta = NULL;

  // Going through database to find right table
  for (int t = 0; t < db->numTables; t++) {
    if (icmpStrings(db->tables[t].name, select->table) == 0) // found it:
    {
      tablemeta = &db->tables[t];
      break;
    }
  }

  // Ensuring that the table meta data exists
  assert(tablemeta != NULL);

  // Going through table meta data and inserting relevant columns into the
  // resultset
  for (int i = 0; i < tablemeta->numColumns; i++) {
    resultset_insertColumn(rSet, i + 1, tablemeta->name,
                           (tablemeta->columns[i]).name, NO_FUNCTION,
                           (tablemeta->columns[i]).colType);
  }

  //
  // (2) open the table's data file
  //
  // the table exists within a sub-directory under the executable
  // where the directory has the same name as the database, and with
  // a "TABLE-NAME.data" filename within that sub-directory:
  //
  char path[(2 * DATABASE_MAX_ID_LENGTH) + 10];

  strcpy(path, db->name); // name/name.data
  strcat(path, "/");
  strcat(path, tablemeta->name);
  strcat(path, ".data");

  FILE *datafile = fopen(path, "r");
  if (datafile == NULL) // unable to open:
  {
    printf("**INTERNAL ERROR: table's data file '%s' not found.\n", path);
    panic("execution halted");
    exit(-1);
  }

  //
  // (3) allocate a buffer for input, and start reading the data:
  //
  int dataBufferSize =
      tablemeta->recordSize + 3; // ends with $\n + null terminator
  char *dataBuffer = (char *)malloc(sizeof(char) * dataBufferSize);
  if (dataBuffer == NULL)
    panic("out of memory");

  // Going through each column and breaking each line into the relevant columns
  // Then inserting the relevant type of data into the corresponding column
  while (true) {
    fgets(dataBuffer, dataBufferSize, datafile);
    if (feof(datafile)) {
      break;
    }
    char *cp = dataBuffer;
    char *end = NULL;
    int rowNumber = resultset_addRow(rSet);
    for (int i = 0; i < tablemeta->numColumns; i++) {
      int colNumber = i + 1;
      if ((tablemeta->columns[i]).colType == COL_TYPE_INT) {
        end = strchr(cp, ' ');
        *end = '\0';
        int value = atoi(cp);
        resultset_putInt(rSet, rowNumber, colNumber, value);
        cp = end + 1;
      } else if (tablemeta->columns[i].colType == COL_TYPE_REAL) {
        end = strchr(cp, ' ');
        assert(end != NULL);
        *end = '\0';
        double value = atof(cp);
        resultset_putReal(rSet, rowNumber, colNumber, value);
        cp = end + 1;
      } else {
        char quote = *cp;
        end = cp;
        end++;
        while (*end != quote) {
          end++;
        }
        *end = '\0';
        resultset_putString(rSet, rowNumber, colNumber, cp + 1);
        cp = end + 2;
      }
    }
  }
  // Freeing memory associated with the buffer for reading each row
  free(dataBuffer);

  // Checking to see if there is a where clause
  if (select->where != NULL) {
    // And if there is, determining the relevant column
    int index = 0;
    for (int i = 0; i < tablemeta->numColumns; i++) {
      if (strcasecmp(tablemeta->columns[i].name,
                     select->where->expr->column->name) == 0) {
        index = i;
      }
    }
    // And deleting the row from the resultset if the data doesn't satisfy the
    // conditions from the query
    if (tablemeta->columns[index].colType == COL_TYPE_INT) {
      int rh_val = atoi(select->where->expr->value);

      for (int i = rSet->numRows; i > 0; i--) {
        int lh_val = resultset_getInt(rSet, i, index + 1);
        bool result = NULL;
        switch (select->where->expr->operator) {
        case EXPR_LT:
          result = lh_val < rh_val;
          break;
        case EXPR_LTE:
          result = lh_val <= rh_val;
          break;
        case EXPR_GT:
          result = lh_val > rh_val;
          break;
        case EXPR_GTE:
          result = lh_val >= rh_val;
          break;
        case EXPR_EQUAL:
          result = lh_val == rh_val;
          break;
        case EXPR_NOT_EQUAL:
          result = lh_val != rh_val;
          break;
        }

        if (result == false) {
          resultset_deleteRow(rSet, i);
        }
      }
    } else if (tablemeta->columns[index].colType == COL_TYPE_REAL) {
      double rh_val = atof(select->where->expr->value);

      for (int i = rSet->numRows; i > 0; i--) {
        double lh_val = resultset_getReal(rSet, i, index + 1);
        bool result = NULL;
        switch (select->where->expr->operator) {
        case EXPR_LT:
          result = lh_val < rh_val;
          break;
        case EXPR_LTE:
          result = lh_val <= rh_val;
          break;
        case EXPR_GT:
          result = lh_val > rh_val;
          break;
        case EXPR_GTE:
          result = lh_val >= rh_val;
          break;
        case EXPR_EQUAL:
          result = lh_val == rh_val;
          break;
        case EXPR_NOT_EQUAL:
          result = lh_val != rh_val;
          break;
        }

        if (result == false) {
          resultset_deleteRow(rSet, i);
        }
      }
    } else {
      char *rh_val = select->where->expr->value;
      int result = 0;

      for (int i = rSet->numRows; i > 0; i--) {
        char *lh_val = resultset_getString(rSet, i, index + 1);
        bool result = NULL;
        switch (select->where->expr->operator) {
        case EXPR_LT:
          result = (strcasecmp(lh_val, rh_val) < 0);
          break;
        case EXPR_LTE:
          result = (strcasecmp(lh_val, rh_val) <= 0);
          break;
        case EXPR_GT:
          result = (strcasecmp(lh_val, rh_val) > 0);
          break;
        case EXPR_GTE:
          result = (strcasecmp(lh_val, rh_val) >= 0);
          break;
        case EXPR_EQUAL:
          result = (strcasecmp(lh_val, rh_val) == 0);
          break;
        case EXPR_NOT_EQUAL:
          result = (strcasecmp(lh_val, rh_val) != 0);
          break;
        }

        if (result == false) {
          resultset_deleteRow(rSet, i);
        }
        // Freeing memory associated with copying strings from the resultset
        free(lh_val);
      }
    }
  }

  // Now checking whether the column is in the query, and if it isn't, deleting
  // it from the resultset
  bool inAST = false;
  for (int i = tablemeta->numColumns; i > 0; i--) {
    struct COLUMN *column = query->q.select->columns;
    while (column != NULL) {
      if (strcasecmp(column->name, tablemeta->columns[i - 1].name) == 0) {
        inAST = true;
      }
      column = column->next;
    }
    if (inAST == false) {
      resultset_deleteColumn(rSet, i);
    }
    inAST = false;
  }

  // And now changing the order of columns as they appear in the dataset
  struct COLUMN *column_pos = query->q.select->columns;
  int index_col_pos = 1;
  int col_pos = 0;
  while (column_pos != NULL) {
    col_pos = resultset_findColumn(rSet, index_col_pos, tablemeta->name,
                                   column_pos->name);
    resultset_moveColumn(rSet, col_pos, index_col_pos);
    index_col_pos++;
    column_pos = column_pos->next;
  }

  // And now adding in aggregate functions from the query to the dataset (if
  // there are any)
  struct COLUMN *agg_function = query->q.select->columns;
  int agg_func_pos = 1;
  while (agg_function != NULL) {
    if (agg_function->function != NO_FUNCTION) {
      resultset_applyFunction(rSet, agg_function->function, agg_func_pos);
    }
    agg_func_pos++;
    agg_function = agg_function->next;
  }

  // And lastly adding the limit clause, which deletes all rows past the limit
  if (select->limit != NULL) {
    for (int i = rSet->numRows; i > select->limit->N; i--) {
      resultset_deleteRow(rSet, i);
    }
  }

  resultset_print(rSet);
  // Freeing memory associated with the datafile
  fclose(datafile);

  //
  // done!
  //
}
