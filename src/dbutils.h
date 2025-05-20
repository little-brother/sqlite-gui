#ifndef __DBUTILS_H__
#define __DBUTILS_H__

#include <windows.h>
#include "sqlite3.h"

namespace dbutils {
	bool isSQLiteDatabase(TCHAR* path16);
	int bind_variant(sqlite3_stmt* stmt, int pos, const char* value8, bool forceToText = false);
	BYTE detectSqliteType(const char* value8, bool forceToText = false);
	BYTE sqlite3_type(const char* type8);
	void userDefinedFunction (sqlite3_context *ctx, int argc, sqlite3_value **argv);

	void rownum(sqlite3_context *ctx, int argc, sqlite3_value **argv);
	void md5 (sqlite3_context *ctx, int argc, sqlite3_value **argv);
	void tosize (sqlite3_context *ctx, int argc, sqlite3_value **argv);
	void double_quote (sqlite3_context *ctx, int argc, sqlite3_value **argv);

	char* removeComments(const char* in8);
	bool isSqlQuery(const char* text8);
	char* queryCSV(sqlite3* db, const char* query8, bool isColumns, const char* delimiter8, bool isUnixNewLine, int* rowCount = 0, const char* null8 = 0);
	TCHAR* getQueryError(sqlite3* db, const TCHAR* query16);
}
#endif
