/*
	lower2(str)
	upper2(str)
*/
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <assert.h>
#include <string.h>
#include <windows.h>

char* transform(const char* in8, boolean isToLower) {
	int len8 = strlen(in8) + 1;
	char* res8 = malloc(sizeof(char) * len8);
	memset(res8, 0, sizeof(char) * len8);

	if (len8 > 1) {
		int len16 = MultiByteToWideChar(CP_UTF8, 0, in8, -1, NULL, 0);
		wchar_t* in16 = malloc(sizeof(wchar_t) * len16);
		memset(in16, 0, sizeof(wchar_t) * len16);
		MultiByteToWideChar(CP_UTF8, 0, in8, -1, in16, len16);
	
		wchar_t* res16 = isToLower ? _wcslwr(in16) : _wcsupr(in16);	

		WideCharToMultiByte(CP_UTF8, 0, res16, -1, res8, len8, 0, 0);
		free(res16);
	}

	return res8;
}

static void lower(sqlite3_context *context, int argc, sqlite3_value **argv){
	const char* str = sqlite3_value_text(argv[0]);
	char* res = (char*)transform(str, TRUE);
	sqlite3_result_text(context, res, -1, SQLITE_TRANSIENT);
	free(res);
}

static void upper(sqlite3_context *context, int argc, sqlite3_value **argv){
	const char* str = sqlite3_value_text(argv[0]);
	char* res = (char*)transform(str, FALSE);
	sqlite3_result_text(context, res, -1, SQLITE_TRANSIENT);
	free(res);
}


__declspec(dllexport) int sqlite3_icu_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
	SQLITE_EXTENSION_INIT2(pApi);
	(void)pzErrMsg;  /* Unused parameter */
	return sqlite3_create_function(db, "lower2", 1, SQLITE_UTF8, 0, lower, 0, 0) || 
		sqlite3_create_function(db, "upper2", 1, SQLITE_UTF8, 0, upper, 0, 0);
}