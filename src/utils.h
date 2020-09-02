#ifndef __UTILS_H__
#define __UTILS_H__

#include <tchar.h>
#include <windows.h>
#include "sqlite3.h"

namespace utils {
	TCHAR* trim(TCHAR *in);
	TCHAR* maskQuotes(TCHAR* in);
	TCHAR* replace(const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start = 0);
	TCHAR* replaceAll(const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start = 0);

	TCHAR* utf8to16(const char* in);
	char* utf16to8(const TCHAR* in);

	void setClipboardText(const TCHAR* text);
	int openFile(TCHAR* path, const TCHAR* filter);
	int saveFile(TCHAR* path, const TCHAR* filter);
	bool isFileExists(const TCHAR* path);

	bool isNumber(TCHAR* str, double *out);

	int sqlite3_bind_variant(sqlite3_stmt* stmt, int pos, const char* value8);
}
#endif
