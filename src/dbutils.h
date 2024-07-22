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
}
#endif
