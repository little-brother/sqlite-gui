#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <stdlib.h>
#include <wininet.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <io.h>
#include "utils.h"

namespace utils {
	TCHAR* trim(TCHAR *in) {
		auto isBlank = [](TCHAR ch) -> bool {
			return (ch == TEXT(' ')) || (ch == TEXT('\r')) || (ch == TEXT('\n')) || (ch == TEXT('\t'));
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

	TCHAR* _tcsistr(TCHAR* str, const TCHAR* strSearch) {
		int len = _tcslen(str);
		int slen = _tcslen(strSearch);
		if (len == 0 || slen == 0 || slen > len)
			return NULL;

		TCHAR lstr[len + 1]{0};
		_tcscpy(lstr, str);
		_tcslwr(lstr);

		TCHAR lstrSearch[slen + 1]{0};
		_tcscpy(lstrSearch, strSearch);
		_tcslwr(lstrSearch);

		TCHAR* res = _tcsstr(lstr, lstrSearch);
		return res ? str + len - _tcslen(res) : NULL;
	}

	TCHAR* replace (const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start, bool isAll, bool ignoreCase) {
		int len = _tcslen(in);
		int nLen = _tcslen(newStr);
		int oLen = _tcslen(oldStr);

		if (start > len || len == 0)
			return new TCHAR[1]{0};

		TCHAR* res = new TCHAR[nLen <= oLen ? len + 1 : len * (nLen - oLen + 1)] {0};
		TCHAR* p = (TCHAR*)in + start;
		TCHAR* p2 = p;

		_tcsncat(res, in, start);

		while((p = (ignoreCase ? _tcsistr(p, oldStr) : _tcsstr(p, oldStr)))) {
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

	TCHAR* replace (const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start, bool ignoreCase) {
		return replace(in, oldStr, newStr, start, false, ignoreCase);
	}

	TCHAR* replaceAll (const TCHAR* in, const TCHAR* oldStr, const TCHAR* newStr, int start, bool ignoreCase) {
		return replace(in, oldStr, newStr, start, true, ignoreCase);
	}

	bool hasString(const TCHAR* str, const TCHAR* sub) {
		bool res = false;

		if (!str || _tcslen(str) == 0 || !sub || _tcslen(sub) == 0)
			return res;

		TCHAR* lstr = _tcsdup(str);
		_tcslwr(lstr);
		TCHAR* lsub = _tcsdup(sub);
		_tcslwr(lsub);

		res = _tcsstr(lstr, lsub) != 0;

		free(lstr);
		free(lsub);

		return res;
	}

	char* double_quote(const char *str) {
		if (!str || strlen(str) == 0)
			return new char[3] {'"', '"', 0};

		int len = strlen(str) * 2 + 3;
		char* res = new char[len] {0};
		char q = '"';
		int j = 0;
		res[0] = q;
		j++;

		for (int i = 0; i < (int)strlen(str); i++) {
			res[j] = str[i];
			j++;

			if (str[i] == q) {
				res[j] = str[i];
				j++;
			}
		}

		res[j] = q;

		return res;
	}

	TCHAR* double_quote(const TCHAR *str) {
		if (!str || _tcslen(str) == 0)
			return new TCHAR[3] {'"', '"', 0};

		char* str8 = utf16to8(str);
		char* res8 = double_quote(str8);
		TCHAR* res16 = utf8to16(res8);
		delete [] str8;
		delete [] res8;

		return res16;
	}

	TCHAR* getTableName(const TCHAR* in, bool isSchema) {
		TCHAR* res = new TCHAR[_tcslen(in) + 5]{0}; // `in` can be 1 char, but schema requires 4 ("main")
		if (!_tcslen(in))
			return _tcscat(res, isSchema ? TEXT("main") : TEXT(""));

		const TCHAR* p = in;
		while (p[0] && !_istgraph(p[0]))
			p++;

		TCHAR* q = p ? _tcschr(TEXT("'`\"["), p[0]) : 0;
		if (q && q[0]) {
			TCHAR* q2 = _tcschr(p + 1, q[0] == TEXT('[') ? TEXT(']') : q[0]);
			while (q2[0] == TEXT('"') && q2[1] == TEXT('"'))
				q2 = _tcschr(q2 + 2, TEXT('"'));

			if (q2 && ((isSchema && q2[1] == TEXT('.')) || (!isSchema && q2[1] != TEXT('.')))) {
				if (q[0] == TEXT('"')) {
					int j = 0;
					for (size_t i = 0; i < _tcslen(p) - _tcslen(q2) - 1; i++) {
						res[j] = (p + 1 + i)[0];
						j++;

						if ((p + 1 + i)[0] == TEXT('"') && (p + 1 + i + 1)[0] == TEXT('"'))
							i++;
					}
				} else {
					_tcsncpy(res, p + 1, _tcslen(p) - _tcslen(q2) - 1);
				}
			}

			if (q2 && !isSchema && q2[1] == TEXT('.') && q2[2] != 0) {
				delete [] res;
				return getTableName(q2 + 2);
			}

		} else {
			TCHAR* d = p ? _tcschr(p, TEXT('.')) : 0;
			if (d && isSchema)
				_tcsncpy(res, p, _tcslen(p) - _tcslen(d));
			if (d && !isSchema) {
				delete [] res;
				return getTableName(d + 1);
			}
		}

		if (!res[0])
			_tcscat(res, isSchema ? TEXT("main") : in);

		return res;
	}

	TCHAR* getFullTableName(const TCHAR* schema, const TCHAR* tablename, bool isOmitMain) {
		int len = 2 * (_tcslen(schema) + _tcslen(tablename)) + 3;
		TCHAR* res = new TCHAR[len + 1]{0};

		bool isSQ = !_istalpha(schema[0]);
		for (int i = 0; !isSQ && (i < (int)_tcslen(schema)); i++)
			isSQ = isSQ || !(_istalnum(schema[i]) || schema[i] == TEXT('_'));

		bool isTQ = !_istalpha(tablename[0]);
		for (int i = 0; !isTQ && (i < (int)_tcslen(tablename)); i++)
			isTQ = isTQ || !(_istalnum(tablename[i]) || tablename[i] == TEXT('_'));

		TCHAR* qSchema = double_quote(schema);
		TCHAR* qTablename = double_quote(tablename);
		if (isOmitMain && _tcscmp(schema, TEXT("main")) == 0) {
			_sntprintf(res, len, isTQ ? qTablename : tablename);
		} else {
			_sntprintf(res, len, TEXT("%ls.%ls"), isSQ ? qSchema : schema, isTQ ? qTablename : tablename);
		}
		delete [] qSchema;
		delete [] qTablename;

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
		TCHAR* res = 0;
		if (OpenClipboard(NULL)) {
			HANDLE clip = GetClipboardData(CF_UNICODETEXT);
			TCHAR* str = (LPWSTR)GlobalLock(clip);
			if (str != 0) {
				res = new TCHAR[_tcslen(str) + 1]{0};
				_tcscpy(res, str);
			}
			GlobalUnlock(clip);
			CloseClipboard();
		}

		return res ? res : new TCHAR[1]{0};
	}

	int openFile(TCHAR* path, const TCHAR* filter, HWND hWnd) {
		OPENFILENAME ofn = {0};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = path;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_EXPLORER;

		if (!GetOpenFileName(&ofn))
			return false;

		if (utils::isFileExists(path))
			return true;

		TCHAR ext[32]{0};
		_tsplitpath(path, NULL, NULL, NULL, ext);
		if (_tcslen(ext) == 0)
			_tcscat(path, TEXT(".sqlite"));

		return true;
	}

	int saveFile(TCHAR* path, const TCHAR* filter, const TCHAR* defExt, HWND hWnd) {
		OPENFILENAME ofn = {0};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = path;
		ofn.lpstrFile[_tcslen(path) + 1] = '\0';
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

		if (!GetSaveFileName(&ofn))
			return false;

		if (_tcslen(path) == 0)
			return saveFile(path, filter, defExt, hWnd);

		TCHAR ext[32]{0};
		TCHAR path2[MAX_PATH + 1]{0};

		_tsplitpath(path, NULL, NULL, NULL, ext);
		if (_tcslen(ext) > 0)
			_tcscpy(path2, path);
		else
			_sntprintf(path2, MAX_PATH, TEXT("%ls%ls%ls"), path, _tcslen(defExt) > 0 ? TEXT(".") : TEXT(""), defExt);

		bool isFileExists = utils::isFileExists(path2);
		if (isFileExists && IDYES != MessageBox(hWnd, TEXT("Overwrite file?"), TEXT("Confirmation"), MB_YESNO))
			return saveFile(path, filter, defExt, hWnd);

		if (isFileExists)
			DeleteFile(path2);

		_tcscpy(path, path2);

		return true;
	}

	bool isFileExists(const TCHAR* path) {
		return _taccess(path, 0) == 0;
	}

	bool isFileExists(const char* path) {
		return _access(path, 0) == 0;
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

	char* getFileName(const char* path, bool noExt) {
		TCHAR* path16 = utils::utf8to16(path);
		TCHAR name16[MAX_PATH], ext16[32], name_ext16[MAX_PATH + 32];
		_tsplitpath(path16, NULL, NULL, name16, ext16);
		if (noExt)
			_sntprintf(name_ext16, MAX_PATH + 31, TEXT("%ls"), name16);
		else
			_sntprintf(name_ext16, MAX_PATH + 31, TEXT("%ls%ls"), name16, ext16);
		char* name8 = utils::utf16to8(name_ext16);
		delete [] path16;
		return name8;
	}

	TCHAR* getFileName(const TCHAR* path16, bool noExt) {
		TCHAR* name16 = new TCHAR[MAX_PATH];
		TCHAR ext16[32], name_ext16[MAX_PATH + 32];
		_tsplitpath(path16, NULL, NULL, name16, ext16);
		if (noExt)
			_sntprintf(name_ext16, MAX_PATH + 31, TEXT("%ls"), name16);
		else
			_sntprintf(name_ext16, MAX_PATH + 31, TEXT("%ls%ls"), name16, ext16);

		return name16;
	}

	bool detectFileExtension(const char* data, int len, TCHAR* out) {
		_sntprintf(out, 9,
			len < 10 ? TEXT("") :
			// https://en.wikipedia.org/wiki/List_of_file_signatures
			strncmp(data, "\x47\x49\x46\x38\x37\x61", 6) == 0 ? TEXT("gif") :
			strncmp(data, "\x47\x49\x46\x38\x39\x61", 6) == 0 ? TEXT("gif") :
			strncmp(data, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8) == 0 ? TEXT("png") :
			strncmp(data, "\xFF\xD8\xFF\xDB", 4) == 0 ? TEXT("jpg") :
			strncmp(data, "\xFF\xD8\xFF\xEE", 4) == 0 ? TEXT("jpg") :
			strncmp(data, "\xFF\xD8\xFF\xE0", 4) == 0 ? TEXT("jpg") :
			strncmp(data, "\xFF\xD8\xFF\xE1", 4) == 0 ? TEXT("jpg") :
			strncmp(data, "\x42\x4D", 2) == 0 ? TEXT("bmp") :

			strncmp(data, "\x50\x4B \x03\x04", 4) == 0 ? TEXT("zip") :
			strncmp(data, "\x52\x61\x72\x21\x1A\x07", 6) == 0 ? TEXT("rar") :
			strncmp(data, "\x37\x7A\xBC\xAF\x27\x1C", 6) == 0 ? TEXT("7z") :
			strncmp(data, "\x1F\x8B", 2) == 0 ? TEXT("gz") :

			strncmp(data, "\x52\x49\x46\x46", 4) == 0 ? TEXT("avi") :
			strncmp(data, "\x1A\x45\xDF\xA3", 4) == 0 ? TEXT("mkv") :
			strncmp(data, "\x00\x00\x01\xBA", 4) == 0 ? TEXT("mpg") :
			strncmp(data, "\x00\x00\x01\xB3", 4) == 0 ? TEXT("mpg") :
			strncmp(data, "\x66\x74\x79\x70\x69\x73\x6F\x6D", 8) == 0 ? TEXT("mp4") :

			strncmp(data, "\x52\x49\x46\x46", 4) == 0 ? TEXT("wav") :
			strncmp(data, "\x4F\x67\x67\x53", 4) == 0 ? TEXT("ogg") :
			strncmp(data, "\x4D\x54\x68\x64", 4) == 0 ? TEXT("mid") :

			strncmp(data, "\x25\x50\x44\x46\x2D", 5) == 0 ? TEXT("pdf") :
			strncmp(data, "\x7B\x5C\x72\x74\x66\x31", 6) == 0 ? TEXT("rtf") :

			TEXT("")
		);

		return true;
	}

	const TCHAR* sizes[] = { TEXT("b"), TEXT("KB"), TEXT("MB"), TEXT("GB"), TEXT("TB") };
	TCHAR* toBlobSize(INT64 bSize) {
		TCHAR* res = new TCHAR[64]{0};
        if (bSize == 0) {
			_sntprintf(res, 63, TEXT("(BLOB: empty)"));
			return res;
        }

        int mag = floor(log10(bSize)/log10(1024));
        double size = (double) bSize / (1L << (mag * 10));

        _sntprintf(res, 63, TEXT("(BLOB: %.2lf%ls)"), size, mag < 5 ? sizes[mag] : TEXT("Error"));
        return res;
	}

	unsigned char* toBlob(INT64 dataSize, const unsigned char* data) {
		int blobSize = dataSize + 4;
		unsigned char* blob = new unsigned char[blobSize];
		blob[0] = (blobSize >> 24) & 0xFF;
		blob[1] = (blobSize >> 16) & 0xFF;
		blob[2] = (blobSize >> 8) & 0xFF;
		blob[3] = blobSize & 0xFF;

		memcpy(blob + 4, data, dataSize);
		return blob;
	}

	// Supports both 2.7 and 2,7
	bool isNumber(const TCHAR* str, double *out) {
		int len = _tcslen(str);
		if (len == 0)
			return false;

		double d;
		TCHAR *endptr;
		errno = 0;
		d = _tcstod(str, &endptr);
		bool rc = !(errno != 0 || *endptr != '\0');
		if (rc && out != NULL)
			*out = d;

		if (rc)
			return true;

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
		int len = strlen(str);
		if (len == 0)
			return false;

		double d;
		char *endptr;
		errno = 0;
		d = strtold(str, &endptr);
		bool rc = !(errno != 0 || *endptr != '\0');
		if (rc && out != NULL)
			*out = d;

		if (rc)
			return true;

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

	bool isDate(const TCHAR* str, double* utc) {
		if (_tcslen(str) < 8)
			return false;

		struct tm tm{0};
		if (_stscanf(str, TEXT("%d%*c%d%*c%d %d%*c%d%*c%d"), &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec) < 3)
			return false;

		tm.tm_year -= 1900;
		tm.tm_mon -= 1;
		tm.tm_isdst = 0;
		*utc = (double)mktime(&tm);

		return true;
	}

	COLORREF blend(COLORREF c1, COLORREF c2, BYTE alpha) {
		BYTE av = alpha;
		BYTE rem = 255 - av;

		BYTE r1 = GetRValue(c1);
		BYTE g1 = GetGValue(c1);
		BYTE b1 = GetBValue(c1);

		BYTE r2 = GetRValue(c2);
		BYTE g2 = GetGValue(c2);
		BYTE b2 = GetBValue(c2);

		BYTE r = (r1*rem + r2*av) / 255;
		BYTE g = (g1*rem + g2*av) / 255;
		BYTE b = (b1*rem + b2*av) / 255;

		return RGB(r, g, b);
	}

	// https://stackoverflow.com/a/25427128/6121703
	bool isColorDark(COLORREF color) {
		return GetRValue(color) * 0.2126 + GetGValue(color) * 0.7152 + GetBValue(color) * 0.0722 < 255 / 2;
	}

	// https://stackoverflow.com/a/14530993/6121703
	void urlDecode (char *dst, const char *src) {
		char a, b;
		while (*src) {
			if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
				if (a >= 'a')
					a -= 'a'-'A';
				if (a >= 'A')
					a -= ('A' - 10);
				else
					a -= '0';
				if (b >= 'a')
					b -= 'a'-'A';
				if (b >= 'A')
					b -= ('A' - 10);
				else
					b -= '0';
				*dst++ = 16 * a + b;
				src+=3;
			} else if (*src == '+') {
				*dst++ = ' ';
				src++;
			} else {
				*dst++ = *src++;
			}
		}
		*dst++ = '\0';
	}

	bool isStartBy(const TCHAR* text, int pos, const TCHAR* test) {
		int len = _tcslen(test);
		TCHAR c = text[pos + len];
		return _tcsnicmp(text + pos, test, len) == 0 && !_istalnum(c) && c != TEXT('_');
	}

	bool isPrecedeBy(const TCHAR* text, int pos, const TCHAR* test) {
		while (pos > 0 && _tcschr(TEXT(" \t\r\n"), text[pos]))
			pos--;

		if (pos == 0 || pos < (int)_tcslen(test))
			return false;

		pos -= _tcslen(test) + 1;
		for (int i = 0; i < (int)_tcslen(test); i++)
			if (_totlower(text[pos + i]) != _totlower(test[i]))
				return false;

		return true;
	}

	const UINT crc32_tab[] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	UINT crc32(const BYTE* data, int size) {
		UINT crc = ~0U;
		while (size--)
			crc = crc32_tab[(crc ^ *data++) & 0xFF] ^ (crc >> 8);

		return crc ^ ~0U;
	}

	// https://github.com/pod32g/MD5/blob/master/md5.c
	const UINT k[64] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee ,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501 ,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be ,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821 ,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa ,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8 ,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed ,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a ,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c ,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70 ,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05 ,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665 ,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039 ,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1 ,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1 ,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};

	const UINT r[] = {
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
	};

	#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

	void to_bytes(UINT val, UINT8 *bytes) {
		bytes[0] = (UINT8) val;
		bytes[1] = (UINT8) (val >> 8);
		bytes[2] = (UINT8) (val >> 16);
		bytes[3] = (UINT8) (val >> 24);
	}

	UINT to_int32(const UINT8 *bytes) {
		return (UINT) bytes[0] | ((UINT) bytes[1] << 8) | ((UINT) bytes[2] << 16) | ((UINT) bytes[3] << 24);
	}

	void md5(const UINT8 *initial_msg, size_t initial_len, UINT8 *digest) {
		UINT h0, h1, h2, h3;
		UINT8 *msg = NULL;

		size_t new_len, offset;
		UINT w[16];
		UINT a, b, c, d, i, f, g, temp;

		h0 = 0x67452301;
		h1 = 0xefcdab89;
		h2 = 0x98badcfe;
		h3 = 0x10325476;

		for (new_len = initial_len + 1; new_len % (512/8) != 448/8; new_len++);

		msg = (UINT8*)malloc(new_len + 8);
		memcpy(msg, initial_msg, initial_len);
		msg[initial_len] = 0x80;
		for (offset = initial_len + 1; offset < new_len; offset++)
			msg[offset] = 0;


		to_bytes(initial_len*8, msg + new_len);
		to_bytes(initial_len>>29, msg + new_len + 4);

		for (offset=0; offset<new_len; offset += (512/8)) {
			for (i = 0; i < 16; i++)
				w[i] = to_int32(msg + offset + i*4);

			a = h0;
			b = h1;
			c = h2;
			d = h3;

			// Main loop:
			for(i = 0; i < 64; i++) {
				if (i < 16) {
					f = (b & c) | ((~b) & d);
					g = i;
				} else if (i < 32) {
					f = (d & b) | ((~d) & c);
					g = (5*i + 1) % 16;
				} else if (i < 48) {
					f = b ^ c ^ d;
					g = (3*i + 5) % 16;
				} else {
					f = c ^ (b | (~d));
					g = (7*i) % 16;
				}

				temp = d;
				d = c;
				c = b;
				b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
				a = temp;
			}

			h0 += a;
			h1 += b;
			h2 += c;
			h3 += d;
		}
		free(msg);

		to_bytes(h0, digest);
		to_bytes(h1, digest + 4);
		to_bytes(h2, digest + 8);
		to_bytes(h3, digest + 12);
	}

	unsigned int read_le16(const unsigned char *p) {
		return ((unsigned int) p[0]) | ((unsigned int) p[1] << 8);
	}

	unsigned int read_le32(const unsigned char *p) {
		return ((unsigned int) p[0]) | ((unsigned int) p[1] << 8) | ((unsigned int) p[2] << 16) | ((unsigned int) p[3] << 24);
	}

	void mergeSortJoiner(int indexes[], void* data, int l, int m, int r, BOOL isBackward, BOOL isNums) {
		int n1 = m - l + 1;
		int n2 = r - m;

		int* L = new int[n1];
		int* R = new int[n2];

		for (int i = 0; i < n1; i++)
			L[i] = indexes[l + i];
		for (int j = 0; j < n2; j++)
			R[j] = indexes[m + 1 + j];

		int i = 0, j = 0, k = l;
		while (i < n1 && j < n2) {
			int cmp = isNums ? ((double*)data)[L[i]] <= ((double*)data)[R[j]] : _tcscmp(((TCHAR**)data)[L[i]], ((TCHAR**)data)[R[j]]) <= 0;
			if (isBackward)
				cmp = !cmp;

			if (cmp) {
				indexes[k] = L[i];
				i++;
			} else {
				indexes[k] = R[j];
				j++;
			}
			k++;
		}

		while (i < n1) {
			indexes[k] = L[i];
			i++;
			k++;
		}

		while (j < n2) {
			indexes[k] = R[j];
			j++;
			k++;
		}

		delete [] L;
		delete [] R;
	}

	void mergeSort(int indexes[], void* data, int l, int r, BOOL isBackward, BOOL isNums) {
		if (l < r) {
			int m = l + (r - l) / 2;
			mergeSort(indexes, data, l, m, isBackward, isNums);
			mergeSort(indexes, data, m + 1, r, isBackward, isNums);
			mergeSortJoiner(indexes, data, l, m, r, isBackward, isNums);
		}
	}


	// http://www.geekhideout.com/urlcode.shtml
	char *urlEncode(const char *str) {
		auto toHex = [](char code) -> char {
			static char hex[] = "0123456789abcdef";
			return hex[code & 15];
		};

		const char *pstr = str;
		char* buf = (char*)malloc(strlen(str) * 3 + 1);
		char* pbuf = buf;
		while (*pstr) {
			if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
				*pbuf++ = *pstr;
			else if (*pstr == ' ')
				*pbuf++ = '+';
			else
				*pbuf++ = '%', *pbuf++ = toHex(*pstr >> 4), *pbuf++ = toHex(*pstr & 15);
			pstr++;
		}
		*pbuf = '\0';

		return buf;
	}

	char* httpRequest(const char* method, const char* uri, const char* headers, const char* data, int* readBytes, DWORD* statusCode, int timeout) {
		DWORD resSize = 1;
		char* res8 = 0;

		char* headers8 = new char[1024] {0};
		strncpy(headers8, headers ? headers : "Accept: application/json\r\n", 1023);

		const char* uri8 = uri;
		if (strstr(uri, "https://"))
			uri8 = uri + 8;

		int len = strlen(uri8);
		char* hostname8 = new char[len + 1] {0};
		char* path8 = new char[len + 1] {0};
		int port = INTERNET_DEFAULT_HTTPS_PORT;

		int pos = 0;
		while (pos < len && (strchr(":/?", uri8[pos]) == 0))
			pos++;

		strncpy(hostname8, uri8, pos);

		if (uri8[pos] == ':') {
			pos++;
			char buf[6] {0};
			while (pos < len && (isdigit(uri8[pos])) && strlen(buf) < 5) {
				buf[strlen(buf)] = uri8[pos];
				pos++;
			}

			port = atoi(buf);
		}

		strcpy(path8, uri8 + pos);

		HINTERNET hInet = InternetOpenA("Mozilla/4.0 (compatible; MSIE 6.0b; Windows NT 5.0; .NET CLR 1.0.2914)", INTERNET_OPEN_TYPE_PRECONFIG, "", "", 0);
		char timeout8[32];
		sprintf(timeout8, "%i", timeout);
		InternetSetOptionA(hInet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOptionA(hInet, INTERNET_OPTION_CONTROL_SEND_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOptionA(hInet, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOptionA(hInet, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOptionA(hInet, INTERNET_OPTION_DATA_SEND_TIMEOUT, &timeout, sizeof(timeout));

		HINTERNET hSession = InternetConnectA(hInet, hostname8, port, "", "", INTERNET_SERVICE_HTTP, 0, 0);
		HINTERNET hRequest = HttpOpenRequestA(hSession, method, path8, NULL, NULL, 0, INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);

		if (hRequest && HttpSendRequestA(hRequest, headers8, strlen(headers8), (char*)data, data ? strlen(data) : 0)) {
			bool isDone = false;

			while (!isDone) {
				DWORD read = 0;
				char* buf8 = new char[32000]{0};
				InternetReadFile(hRequest, buf8, 32000, &read);

				resSize += read;
				res8 = res8 ? (char*)realloc(res8, resSize) : new char[resSize]{0};
				for (DWORD i = 0; i < read; i++)
					res8[resSize - read + i - 1] = buf8[i];
				delete [] buf8;

				isDone = read == 0;
			}

			if (statusCode) {
				DWORD len = sizeof(DWORD);
				HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, statusCode, &len, NULL);
			}
		}

		delete [] headers8;
		delete [] hostname8;
		delete [] path8;

		InternetCloseHandle(hRequest);
		InternetCloseHandle(hSession);
		InternetCloseHandle(hInet);

		// The last char is 0. Hmmm...
		if (res8)
			res8[resSize - 1] = 0;

		if (readBytes)
			*readBytes = resSize - 1;

		return res8;
	}

	bool downloadFile(const TCHAR* url16, const TCHAR* path16, bool unpack) {
		bool rc = false;

		HINTERNET hInet = InternetOpen(TEXT("Mozilla/4.0 (compatible; MSIE 6.0b; Windows NT 5.0; .NET CLR 1.0.2914)"), INTERNET_OPEN_TYPE_PRECONFIG, TEXT(""), TEXT(""), 0);
		HINTERNET hUrl = InternetOpenUrl(hInet, url16, 0, 0, INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);

		if (hUrl)  {
			DWORD resSize = 1;
			char* res8 = 0;

			bool isDone = false;
			while (!isDone) {
				DWORD read = 0;
				char* buf8 = new char[32000]{0};
				InternetReadFile(hUrl, buf8, 32000, &read);

				resSize += read;
				res8 = res8 ? (char*)realloc(res8, resSize) : new char[resSize]{0};
				for (DWORD i = 0; i < read; i++)
					res8[resSize - read + i - 1] = buf8[i];
				delete [] buf8;

				isDone = read == 0;
			}

			TCHAR folder16[MAX_PATH + 1] = {0};
			_tcscpy(folder16, path16);
			PathRemoveFileSpec(folder16);
			SHCreateDirectoryEx(0, folder16, 0);

			FILE* f = _tfopen(path16, TEXT("wb"));
			if (f) {
				fwrite(res8, 1, resSize - 1, f);
				fclose(f);

				rc = true;
			}

			delete [] res8;
		}

		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInet);

		return rc;
	}

	SIZE getTextSize(HFONT hFont, const TCHAR* text) {
		HDC hDC = GetDC(HWND_DESKTOP);
		HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

		SIZE s = {0};
		GetTextExtentPoint32(hDC, text, _tcslen(text), &s);
		SelectObject(hDC, hOldFont);
		ReleaseDC(HWND_DESKTOP, hDC);

		return s;
	}

	POINTFLOAT getDlgScale(HWND hWnd) {
		RECT rc = {0, 0, 1000, 1000};
		MapDialogRect(hWnd, &rc);

		return (POINTFLOAT){rc.right/1000.f, rc.bottom/1000.f};
	}

	POINTFLOAT getWndScale(HWND hWnd) {
		HDC hDC = GetDC(hWnd);
		POINTFLOAT z = {GetDeviceCaps(hDC, LOGPIXELSX) / 96.f, GetDeviceCaps(hDC, LOGPIXELSY) / 96.f};
		ReleaseDC(hWnd, hDC);

		return z;
	}

	int getEditHeight(HWND hWnd) {
		float z = getWndScale(hWnd).x;
		return (13 - (z >= 1.25f) - (z >= 1.5f)) * getDlgScale(hWnd).y;
	}
	int getEditHeightByFont(HWND hWnd) {
		RECT rc = {0};
		HFONT hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);

		HDC hDC = GetDC(hWnd);
		HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
		DrawText(hDC, TEXT("AgqW"), 5, &rc, DT_CALCRECT);
		SelectObject(hDC, hOldFont);
		ReleaseDC(hWnd, hDC);

		return rc.bottom;
	}

	void alignDialog(HWND hDlgWnd, HWND hParentWnd, bool doLess, bool doMore) {
		RECT rc, prc;
		GetWindowRect(hDlgWnd, &rc);
		if (hParentWnd) {
			GetWindowRect(hParentWnd, &prc);
		} else {
			prc = (RECT){0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
		}

		if (doMore || doLess) {
			int w = rc.right - rc.left;
			int pw = prc.right - prc.left;
			int W = (doLess && w > pw) || (doMore && w < pw) ?  pw - 60 : w;

			int h = rc.bottom - rc.top;
			int ph = prc.bottom - prc.top;
			int H = (doLess && h > ph) || (doMore && h < ph) ?  ph - 60 : h;
			SetWindowPos(hDlgWnd, 0, 0, 0, W, H, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
		}

		GetWindowRect(hDlgWnd, &rc);
		POINT c = {(prc.right + prc.left) / 2, (prc.bottom + prc.top) / 2};
		SetWindowPos(hDlgWnd, 0, c.x - (rc.right - rc.left) / 2, c.y - (rc.bottom - rc.top) / 2, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	// Read first 4 bytes
	int getBlobSize (const unsigned char* data) {
		int size = 0;
		for (int i = 0; i < 4; i++) {
			size <<= 8;
			size |= data[i];
		}
		return size;
	}
}
