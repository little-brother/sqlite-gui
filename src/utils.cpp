#include <windows.h>
#include <stdio.h>
#include "utils.h"

namespace utils {
	TCHAR* trim(TCHAR *in, bool trimSemicolon) {
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

	int countChar(TCHAR* str, TCHAR chr) {
		int count = 0;
		for(; *str; count += (*str++ == chr));
			return count;
	}

	TCHAR* maskQuotes(TCHAR* in) {
		int qCount = countChar(in, TEXT('"'));
		int len = _tcslen(in);
		TCHAR* res = new TCHAR[len + qCount + 1]{0};

		int j = 0;
		for (int i = 0; i < len; i++) {
			if (in[i] == TEXT('"')) {
				res[i + j] = TEXT('\\');
				j++;
			}
			res[i + j] = in[i];
		}

		res[len + qCount] = '\0';
		return res;
	}

	TCHAR* replace (const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start) {
		int len = _tcslen(in);
		int nLen = _tcslen(newStr);
		int oLen = _tcslen(oldStr);

		TCHAR* res = new TCHAR[len + nLen + 1]{0};
		_tcscpy(res, in);

		TCHAR* p;
		if (!(p = _tcsstr(res + start, oldStr)))
			return res;

		int pos = len - _tcslen(p);
		for (int i = 0; i < nLen; i++)
			res[pos + i] = newStr[i];

		for (int i = 0; i < len - pos - oLen ; i++)
			res[pos + nLen + i] = in[pos + oLen + i];

		for (int i = len - oLen + nLen; i < len + nLen + 1; i++)
			res[i] = 0;

		return res;
	}

	TCHAR* utf8to16(const char* in) {
		TCHAR *out;
		if (!in || strlen(in) == 0) {
			out = new TCHAR[1]{0};
			out[0] = TEXT('\0');
		} else  {
			DWORD size = MultiByteToWideChar(CP_UTF8, 0, in, -1, NULL, 0);
			out = new TCHAR[size + 1]{0};
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
			out = new char[len + 1]{0};
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

	int sqlite3_bind_variant(sqlite3_stmt* stmt, int pos, const char* value8) {
		long lValue = strtol(value8, NULL, 10);
		double dValue = strtod(value8, NULL);

		if (strlen(value8) == 1 && value8[0] == '0')
			return sqlite3_bind_int(stmt, pos, 0);

		if (lValue && dValue)
			return lValue == dValue ? sqlite3_bind_int64(stmt, pos, lValue) : sqlite3_bind_double(stmt, pos, dValue);

		return strlen(value8) ? sqlite3_bind_text(stmt, pos, value8, strlen(value8),  SQLITE_TRANSIENT) : sqlite3_bind_null(stmt, pos);
	}
}
