#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "utils.h"

namespace utils {
	TCHAR* trim(TCHAR *in) {
		auto isBlank = [](TCHAR ch) -> bool {
			return (ch == TEXT(' ')) || (ch == TEXT('\r')) || (ch == TEXT('\n'));
		};

		int start = 0;
		int end = !in ? 0 : _tcslen(in);

		while(start < end && isBlank(in[start]))
			start++;

		while(end > start && isBlank(in[end - 1]))
			end--;

		TCHAR* out = new TCHAR[end - start + 1];
		for (int i = 0; i < end - start; i++)
			out[i] = in[start + i];
		out[end - start] = TEXT('\0');

		return out;
	}

	TCHAR* replace (const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start, bool isAll) {
		int len = _tcslen(in);
		int nLen = _tcslen(newStr);
		int oLen = _tcslen(oldStr);

		if (start > len || len == 0)
			return new TCHAR[1]{0};

		TCHAR* res = new TCHAR[nLen <= oLen ? len + 1 : len * (nLen - oLen + 1)] {0};
		TCHAR* p = (TCHAR*)in + start;
		TCHAR* p2 = p;

		_tcsncat(res, in, start);

		while((p = _tcsstr(p, oldStr))) {
			_tcsncat(res, p2, p - p2);
			_tcsncat(res, newStr, nLen);
			p = p + oLen;
			p2 = p;

			if (!isAll)
				break;
		}

		_tcsncat(res, p2, len - (p2 - in));
		return res;
	}

	TCHAR* replace (const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start) {
		return replace(in, oldStr, newStr, start, false);
	}

	TCHAR* replaceAll (const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start) {
		return replace(in, oldStr, newStr, start, true);
	}

	TCHAR* getName(const TCHAR* in, bool isSchema) {
		TCHAR* res = new TCHAR[_tcslen(in) + 5]{0}; // `in` can be 1 char, but schema requires 4 ("main")
		if (!_tcslen(in))
			return _tcscat(res, isSchema ? TEXT("main") : TEXT(""));

		const TCHAR* p = in;
		while (p[0] && !_istgraph(p[0]))
			p++;

		TCHAR* q = p ? _tcschr(TEXT("'`\"["), p[0]) : 0;
		if (q) {
			TCHAR* q2 = _tcschr(p + 1, q[0] == TEXT('[') ? TEXT(']') : q[0]);
			if (q2 && ((isSchema && q2[1] == TEXT('.')) || (!isSchema && q2[1] != TEXT('.'))))
				_tcsncpy(res, p + 1, _tcslen(p) - _tcslen(q2) - 1);

			if (q2 && !isSchema && q2[1] == TEXT('.') && q2[2] != 0) {
				delete [] res;
				return getName(q2 + 2);
			}

		} else {
			TCHAR* d = p ? _tcschr(p, TEXT('.')) : 0;
			if (d && isSchema)
				_tcsncpy(res, p, _tcslen(p) - _tcslen(d));
			if (d && !isSchema) {
				delete [] res;
				return getName(d + 1);
			}
		}

		if (!res[0])
			_tcscat(res, isSchema ? TEXT("main") : in);

		return res;
	}

	TCHAR* utf8to16(const char* in) {
		TCHAR *out;
		if (!in || strlen(in) == 0) {
			out = new TCHAR[1]{0};
			out[0] = TEXT('\0');
		} else  {
			DWORD size = MultiByteToWideChar(CP_UTF8, 0, in, -1, NULL, 0);
			out = new TCHAR[size]{0};
			MultiByteToWideChar(CP_UTF8, 0, in, -1, out, size);
		}
		return out;
	}

	char* utf16to8(const TCHAR* in) {
		char* out;
		if (!in || _tcslen(in) == 0) {
			out = new char[1]{0};
			out[0] = '\0';
		} else  {
			int len = WideCharToMultiByte(CP_UTF8, 0, in, -1, NULL, 0, 0, 0);
			out = new char[len]{0};
			WideCharToMultiByte(CP_UTF8, 0, in, -1, out, len, 0, 0);
		}
		return out;
	}

	void setClipboardText(const TCHAR* text) {
		int len = (_tcslen(text) + 1) * sizeof(TCHAR);
		HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), text, len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hMem);
		CloseClipboard();
	}

	TCHAR* getClipboardText() {
		if (OpenClipboard(NULL)) {
			HANDLE clip = GetClipboardData(CF_UNICODETEXT);
			TCHAR* str = (LPWSTR)GlobalLock(clip);
			TCHAR* res = new TCHAR[_tcslen(str) + 1]{0};
			_tcscpy(res, str);
			GlobalUnlock(clip);
			CloseClipboard();
			return res;
		}

		return new TCHAR[1]{0};
	}

	int openFile(TCHAR* path, const TCHAR* filter) {
		OPENFILENAME ofn = {0};

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = HWND_DESKTOP;
		ofn.lpstrFile = path;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
		return GetOpenFileName(&ofn);
	}

	int saveFile(TCHAR* path, const TCHAR* filter) {
		OPENFILENAME ofn = {0};

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = HWND_DESKTOP;
		ofn.lpstrFile = path;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
		return GetSaveFileName(&ofn);
	}

	bool isFileExists(const TCHAR* path) {
		WIN32_FIND_DATA FindFileData;
		HANDLE hFile = FindFirstFile(path, &FindFileData) ;
		int isFound = hFile != INVALID_HANDLE_VALUE;
		if (isFound)
			FindClose(hFile);

		return isFound;
	}

	char* readFile(const char* path) {
		FILE *fp = fopen (path , "rb");
		if(!fp)
			return 0;

		fseek(fp, 0L, SEEK_END);
		long size = ftell(fp);
		rewind(fp);

		char* buf = new char[size + 1]{0};
		int rc = fread(buf, size, 1 , fp);
		fclose(fp);

		if (!rc) {
			delete [] buf;
			return 0;
		}

		buf[size] = '\0';

		return buf;
	}

	char* getFileName(const char* path) {
		TCHAR* path16 = utils::utf8to16(path);
		TCHAR name16[255], ext16[32], name_ext16[300];
		_tsplitpath(path16, NULL, NULL, name16, ext16);
		_stprintf(name_ext16, TEXT("%s%s"), name16, ext16);
		char* name8 = utils::utf16to8(name_ext16);
		delete [] path16;
		return name8;
	}

	int sqlite3_bind_variant(sqlite3_stmt* stmt, int pos, const char* value8) {
		int len = strlen(value8);

		if (len == 0)
			return sqlite3_bind_null(stmt, pos);

		//if (len > 20) // 18446744073709551615
		//	return sqlite3_bind_text(stmt, pos, value8, strlen(value8), SQLITE_TRANSIENT);

		if (len == 1 && value8[0] == '0')
			return sqlite3_bind_int(stmt, pos, 0);

		bool isNum = true;
		int dotCount = false;
		for (int i = +(value8[0] == '-' /* is negative */); i < len; i++) {
			isNum = isNum && (isdigit(value8[i]) || value8[i] == '.' || value8[i] == ',');
			dotCount += value8[i] == '.' || value8[i] == ',';
		}

		if (isNum && dotCount == 0) {
			return len < 10 ? // 2147483647
				sqlite3_bind_int(stmt, pos, atoi(value8)) :
				sqlite3_bind_int64(stmt, pos, atoll(value8));
		}

		double d = 0;
		if (isNum && dotCount == 1 && isNumber(value8, &d)) {
			printf("%s -> %.30lf\n", value8, d);
			return sqlite3_bind_double(stmt, pos, d);
		}


		return sqlite3_bind_text(stmt, pos, value8, strlen(value8), SQLITE_TRANSIENT);
	}

	const TCHAR* sizes[] = { TEXT("b"), TEXT("KB"), TEXT("MB"), TEXT("GB"), TEXT("TB") };
	TCHAR* toBlobSize(INT64 bSize) {
		TCHAR* res = new TCHAR[64]{0};
        if (bSize == 0) {
			_stprintf(res, TEXT("(BLOB: empty)"));
			return res;
        }

        int mag = floor(log10(bSize)/log10(1024));
        double size = (double) bSize / (1L << (mag * 10));

        _stprintf(res, TEXT("(BLOB: %.2lf%s)"), size, mag < 5 ? sizes[mag] : TEXT("Error"));
        return res;
	}

	// Supports both 2.7 and 2,7
	bool isNumber(const TCHAR* str, double *out) {
		double d;
		TCHAR *endptr;
		errno = 0;
		d = _tcstod(str, &endptr);
		bool rc = !(errno != 0 || *endptr != '\0');
		if (rc && out != NULL)
				*out = d;

		if (rc)
			return true;

		int len = _tcslen(str);
		TCHAR str2[len + 1]{0};
		_tcscpy(str2, str);
		for (int i = 0; i < len; i++)
			if (str2[i] == TEXT('.'))
				str2[i] = TEXT(',');

		errno = 0;
		d = _tcstod(str2, &endptr);
		rc = !(errno != 0 || *endptr != '\0');
		if (rc && out != NULL)
			*out = d;

		return rc;
	}

	bool isNumber(const char* str, double *out) {
		double d;
		char *endptr;
		errno = 0;
		d = strtold(str, &endptr);
		bool rc = !(errno != 0 || *endptr != '\0');
		if (rc && out != NULL)
				*out = d;

		if (rc)
			return true;

		int len = strlen(str);
		char str2[len + 1]{0};
		strcpy(str2, str);
		for (int i = 0; i < len; i++)
			if (str2[i] == TEXT('.'))
				str2[i] = TEXT(',');

		errno = 0;
		d = strtold(str2, &endptr);
		rc = !(errno != 0 || *endptr != '\0');
		if (rc && out != NULL)
			*out = d;

		return rc;
	}
}
