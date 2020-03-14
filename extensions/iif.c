#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <assert.h>
#include <string.h>

static void iif(sqlite3_context *context, int argc, sqlite3_value **argv){
	assert(3 == argc);
	
	int idx = 2;
	sqlite3_value* expr = argv[0];
	int expr_type = sqlite3_value_type(expr);
	if (expr_type == SQLITE_INTEGER)
		idx = sqlite3_value_int64(expr) ? 1 : 2;
	if (expr_type == SQLITE_FLOAT)
		idx = sqlite3_value_double(expr) ? 1 : 2;
	if (expr_type == SQLITE_TEXT)
		idx = strlen(sqlite3_value_text(expr)) ? 1 : 2;

	sqlite3_value* res = argv[idx];
	switch (sqlite3_value_type(res)) {
		case SQLITE_NULL:
			sqlite3_result_null(context);
			break;
		case SQLITE_INTEGER:
			sqlite3_result_int64(context, sqlite3_value_int64(res));
			break;
		case SQLITE_FLOAT:
			sqlite3_result_double(context, sqlite3_value_double(res));
			break; 
		case SQLITE_TEXT: {
			int len  = sqlite3_value_bytes(res);
		    unsigned char* text = sqlite3_malloc(len + 1);
		    strcpy((char*)text, (char*)sqlite3_value_text(res)); 
			sqlite3_result_text(context, (char*)text, -1, SQLITE_TRANSIENT);
			sqlite3_free(text);
			break;
		}
		default:
			sqlite3_result_error(context, "Error unknown type", -1);
	}	
}

__declspec(dllexport) int sqlite3_iif_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);
  (void)pzErrMsg;  /* Unused parameter */
  return sqlite3_create_function(db, "iif", 3, SQLITE_UTF8|SQLITE_INNOCUOUS|SQLITE_DETERMINISTIC, 0, iif, 0, 0);
}