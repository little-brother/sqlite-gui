/* 
	rownum(startBy)
	Returns a row number starting from a passed argument
	select *, rownum(0) from mytable
*/
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <assert.h>
#include <string.h>

static void rownum(sqlite3_context *context, int argc, sqlite3_value **argv){
	int *pCounter = (int*)sqlite3_get_auxdata(context, 0);
	if (pCounter == 0) {
		pCounter = sqlite3_malloc(sizeof(*pCounter));
		if (pCounter == 0) {
			sqlite3_result_error_nomem(context);
			return;
		}
		
		*pCounter = sqlite3_value_int(argv[0]);
		sqlite3_set_auxdata(context, 0, pCounter, sqlite3_free);
	} else {
		++*pCounter;
	}
	
	sqlite3_result_int(context, *pCounter);
}

__declspec(dllexport) int sqlite3_rownum_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
	int rc = SQLITE_OK;
	SQLITE_EXTENSION_INIT2(pApi);
	(void)pzErrMsg;  /* Unused parameter */
	return sqlite3_create_function(db, "rownum", 1, SQLITE_UTF8, 0, rownum, 0, 0);
}