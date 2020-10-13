/* 
	rownum(startBy)
	Returns a row number starting from a passed argument
	select *, rownum(0) from mytable

	concat(str1, str2, ...)
	Concatenates strings. Equals to str1 || str2 || ...
	select concat(str1, str2, str3) from mytable

	decode(expr, key1, value1, ke2, value2, ..., defValue)
	Compares expr to each key one by one. If expr is equal to a key, then returns the corresponding value. 
	If no match is found, then returns defValue. If defValue is omitted, then returns null.
	decode(1 < 2, false, 'NO', true, 'YES', '???') --> YES

*/
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static void rownum(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	int *pCounter = (int*)sqlite3_get_auxdata(ctx, 0);
	if (pCounter == 0) {
		pCounter = sqlite3_malloc(sizeof(*pCounter));
		if (pCounter == 0) {
			sqlite3_result_error_nomem(ctx);
			return;
		}
		
		*pCounter = sqlite3_value_int(argv[0]);
		sqlite3_set_auxdata(ctx, 0, pCounter, sqlite3_free);
	} else {
		++*pCounter;
	}
	
	sqlite3_result_int(ctx, *pCounter);
}

static void concat (sqlite3_context *ctx, int argc, sqlite3_value **argv) {
	if (argc == 0)
		return sqlite3_result_null(ctx);

	for(int i = 0; i < argc; i++) {
		if(sqlite3_value_type(argv[i]) == SQLITE_NULL) 
			return sqlite3_result_null(ctx);
	}
	
	size_t len = 0;
	for(int i = 0; i < argc; i++) 
		len += strlen(sqlite3_value_text(argv[i]));
	
	char* all = (char*)calloc(sizeof(char), len + 1);	
	for(int i = 0; i < argc; i++) 
		strcat(all, sqlite3_value_text(argv[i]));
	
	sqlite3_result_text(ctx, all, -1, SQLITE_TRANSIENT);
	free(all);
}

static void decode (sqlite3_context *ctx, int argc, sqlite3_value **argv) {
	if (argc < 2)
		return sqlite3_result_error(ctx, "Too many values", -1);

	int keyCount = (argc - 1) / 2;
	const char* expr = sqlite3_value_text(argv[0]);
	for (int keyNo = 0; keyNo < keyCount; keyNo++) {
		if (strcmp(expr, sqlite3_value_text(argv[2 * keyNo + 1])) == 0)
			return sqlite3_result_text(ctx, sqlite3_value_text(argv[2 * keyNo + 2]), -1, SQLITE_TRANSIENT);
	} 
	
	return argc % 2 ? sqlite3_result_null(ctx) : sqlite3_result_text(ctx, sqlite3_value_text(argv[argc - 1]), -1, SQLITE_TRANSIENT);
}

__declspec(dllexport) int sqlite3_ora_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
	int rc = SQLITE_OK;
	SQLITE_EXTENSION_INIT2(pApi);
	(void)pzErrMsg;  /* Unused parameter */
	return SQLITE_OK == sqlite3_create_function(db, "rownum", 1, SQLITE_UTF8, 0, rownum, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "concat", -1, SQLITE_UTF8, 0, concat, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "decode", -1, SQLITE_UTF8, 0, decode, 0, 0) ? 
		SQLITE_OK : SQLITE_ERROR;
}