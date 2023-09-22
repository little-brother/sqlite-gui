#ifndef __UTILS_H__
#define __UTILS_H__

#include <tchar.h>
#include <windows.h>
#include "sqlite3.h"

namespace utils {
	TCHAR* trim(TCHAR *in);
	TCHAR* replace(const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start = 0, bool ignoreCase = false);
	TCHAR* replaceAll(const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start = 0, bool ignoreCase = false);
	bool hasString(const TCHAR* str, const TCHAR* sub);

	TCHAR* getTableName(const TCHAR* in, bool isSchema = false);
	TCHAR* getFullTableName(const TCHAR* schema, const TCHAR* tablename, bool isOmitMain = false);

	TCHAR* utf8to16(const char* in);
	char* utf16to8(const TCHAR* in);

	void setClipboardText(const TCHAR* text);
	TCHAR* getClipboardText();

	int openFile(TCHAR* path, const TCHAR* filter, HWND hWnd = 0);
	int saveFile(TCHAR* path, const TCHAR* filter, const TCHAR* defExt, HWND hWnd = 0);

	bool isFileExists(const TCHAR* path);
	bool isFileExists(const char* path);

	char* readFile(const char* path);
	char* getFileName(const char* path, bool noExt = false);
	TCHAR* getFileName(const TCHAR* path16, bool noExt = false);
	bool getFileExtension(const char* data, int len, TCHAR* out);

	TCHAR* toBlobSize(INT64 bSize);

	bool isNumber(const TCHAR* str, double *out);
	bool isNumber(const char* str, double *out);
	bool isDate(const TCHAR* str, double* utc);

	COLORREF blend(COLORREF c1, COLORREF c2, BYTE alpha);

	bool isSQLiteDatabase(TCHAR* path16);
	int sqlite3_bind_variant(sqlite3_stmt* stmt, int pos, const char* value8, bool forceToText = false);
	BYTE sqlite3_type(const char* type8);
	void urlDecode (char *dst, const char *src);

	bool isStartBy(const TCHAR* text, int pos, const TCHAR* test);
	bool isPrecedeBy(const TCHAR* text, int pos, const TCHAR* test);

	extern const UINT crc32_tab[];
	UINT crc32(const BYTE* data, int size);

	void mergeSort(int indexes[], void* data, int l, int r, BOOL isBackward, BOOL isNums);

	char* httpRequest(const char* method, const char* uri, const char* path, const char* data = 0);
}
#endif
