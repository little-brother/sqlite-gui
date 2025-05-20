#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <stdlib.h>
#include <wininet.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <io.h>
#include "dbutils.h"
#include "utils.h"

namespace dbutils {
	bool isSQLiteDatabase(TCHAR* path16) {
		FILE *f = _tfopen (path16, TEXT("rb"));
		if(!f)
			return 0;

		char buf[16] = {0};
		fread(buf, 16, 1, f);
		fclose(f);

		return strncmp(buf, "SQLite format 3", 15) == 0;
	}

	BYTE detectSqliteType(const char* value8, bool forceToText) {
		if (value8 == 0)
			return SQLITE_NULL;

		int len = strlen(value8);
		if (len == 0)
			return SQLITE_NULL;

		if (forceToText)
			return SQLITE_TEXT;

		if (len == 1 && value8[0] == '0')
			return SQLITE_INTEGER;

		bool isNum = true;
		int dotCount = 0;
		for (int i = +(value8[0] == '-' /* is negative */); i < len; i++) {
			isNum = isNum && (isdigit(value8[i]) || value8[i] == '.');
			dotCount += value8[i] == '.';// || value8[i] == ',';
		}

		if (isNum && dotCount == 0) {
			return SQLITE_INTEGER;
		}

		double d = 0;
		if (isNum && dotCount == 1 && utils::isNumber(value8, &d))
			return SQLITE_FLOAT;

		return SQLITE_TEXT;
	}

	int bind_variant(sqlite3_stmt* stmt, int pos, const char* value8, bool forceToText) {
		BYTE type = detectSqliteType(value8, forceToText);
		if (type == SQLITE_NULL)
			return sqlite3_bind_null(stmt, pos);

		if (type == SQLITE_TEXT)
			return sqlite3_bind_text(stmt, pos, value8, strlen(value8), SQLITE_TRANSIENT);

		if (type == SQLITE_FLOAT) {
			double d = 0;
			utils::isNumber(value8, &d);
			return sqlite3_bind_double(stmt, pos, d);
		}

		if (type == SQLITE_INTEGER) {
			return strlen(value8) < 10 ? // 2147483647
				sqlite3_bind_int(stmt, pos, atoi(value8)) :
				sqlite3_bind_int64(stmt, pos, atoll(value8));
		}

		return SQLITE_ERROR;
	}

	BYTE sqlite3_type(const char* decltype8) {
		if (decltype8 == 0)
			return SQLITE_NULL;

		char type8[strlen(decltype8) + 1]{0};
		strcpy(type8, decltype8);
		strlwr(type8);

		return strstr(type8, "char") != NULL || strstr(type8, "text") != NULL ? SQLITE_TEXT :
			strstr(type8, "int") != NULL ? SQLITE_INTEGER :
			strstr(type8, "float") != NULL || strstr(type8, "double") != NULL || strstr(type8, "real") != NULL || strstr(type8, "numeric") != NULL ? SQLITE_FLOAT :
			strcmp(type8, "blob") == 0 ? SQLITE_BLOB :
			SQLITE_TEXT;
	}

	void userDefinedFunction (sqlite3_context *ctx, int argc, sqlite3_value **argv) {
		const char* code8 = (const char*)sqlite3_user_data(ctx);
		sqlite3* _db = sqlite3_context_db_handle(ctx);

		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(_db, code8, -1, &stmt, 0)) {
			for (int i = 0; i < argc; i++) {
				int type = sqlite3_value_type(argv[i]);
				if (type == SQLITE_TEXT) {
					const char* val = (const char*)sqlite3_value_text(argv[i]);
					sqlite3_bind_text(stmt, i + 1, val, strlen(val), SQLITE_TRANSIENT);
				} else if (type == SQLITE_INTEGER) {
					sqlite3_bind_int(stmt, i + 1, sqlite3_value_int(argv[i]));
				} else if (type == SQLITE_FLOAT) {
					sqlite3_bind_double(stmt, i + 1, sqlite3_value_double(argv[i]));
				} else if (type == SQLITE_BLOB) {
					sqlite3_bind_blob(stmt, i + 1, sqlite3_value_blob(argv[i]), -1, SQLITE_TRANSIENT);
				} else {
					sqlite3_bind_null(stmt, i + 1);
				}
			}

			if (SQLITE_ROW == sqlite3_step(stmt)) {
				int type = sqlite3_column_type(stmt, 0);
				if (type == SQLITE_TEXT) {
					sqlite3_result_text(ctx, (const char*)sqlite3_column_text(stmt, 0), -1, SQLITE_TRANSIENT);
				} else if (type == SQLITE_INTEGER) {
					sqlite3_result_int(ctx, sqlite3_column_int(stmt, 0));
				} else if (type == SQLITE_FLOAT) {
					sqlite3_result_double(ctx, sqlite3_column_double(stmt, 0));
				} else if (type == SQLITE_BLOB) {
					sqlite3_result_blob(ctx, sqlite3_column_blob(stmt, 0), -1, SQLITE_TRANSIENT);
				} else {
					sqlite3_result_null(ctx);
				}
			}
		}
		sqlite3_finalize(stmt);
	}

	void rownum(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
		int *pCounter = (int*)sqlite3_get_auxdata(ctx, 0);
		if (pCounter == 0) {
			pCounter = (int*)sqlite3_malloc(sizeof(*pCounter));
			if (pCounter == 0) {
				sqlite3_result_error_nomem(ctx);
				return;
			}

			*pCounter = argc == 0 || sqlite3_value_type(argv[0]) == SQLITE_NULL ? 0 : sqlite3_value_int(argv[0]);
			sqlite3_set_auxdata(ctx, 0, pCounter, sqlite3_free);
		} else {
			++*pCounter;
		}

		sqlite3_result_int(ctx, *pCounter);
	}

	char const hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	void md5 (sqlite3_context *ctx, int argc, sqlite3_value **argv) {
		if (sqlite3_value_type(argv[0]) == SQLITE_NULL)
			return sqlite3_result_null(ctx);

		UINT8 res[16];
		utils::md5(sqlite3_value_text(argv[0]), strlen((const char*)sqlite3_value_text(argv[0])), res);

		char buf[33];
		for(int i = 0; i < 16; i++) {
			char byte = res[i];
			buf[2 * i] = hex_chars[(byte & 0xF0) >> 4];
			buf[2 * i + 1] = hex_chars[(byte & 0x0F) >> 0];
		}
		buf[32] = 0;

		sqlite3_result_text(ctx, buf, -1, SQLITE_TRANSIENT);
	}

	const char* sizes[] = {"b", "KB", "MB", "GB", "TB"};

	void tosize (sqlite3_context *ctx, int argc, sqlite3_value **argv) {
		if (sqlite3_value_type(argv[0]) == SQLITE_NULL)
			return sqlite3_result_null(ctx);

		double size = sqlite3_value_double(argv[0]);
		char res[64];
		if (size != 0) {
			int mag = floor(log10(size)/log10(1024));
			double tosize = (double) size / (1L << (mag * 10));
			sprintf(res, "%.2lf%s", tosize, mag < 5 ? sizes[mag] : "Error");
		} else {
			sprintf(res, "0b");
		}

		sqlite3_result_text(ctx, res, -1, SQLITE_TRANSIENT);
	}

	void double_quote (sqlite3_context* ctx, int argc, sqlite3_value **argv) {
		if (sqlite3_value_type(argv[0]) == SQLITE_NULL)
			return sqlite3_result_null(ctx);

		const char* arg = (const char*)sqlite3_value_text(argv[0]);
		char* res = utils::double_quote(arg);
		sqlite3_result_text(ctx, res, -1, SQLITE_TRANSIENT);
		delete [] res;
	}

	char* removeComments(const char *in8) {
		if (in8 == 0)
			return 0;

		int len = strlen(in8);

		char* out8 = new char[len + 1] {0};
		int pos = 0;
		int opos = 0;

		while (pos < len) {
			if (in8[pos] == '/' && in8[pos + 1] == '*') {
				pos += 2;
				while (pos < len && !(in8[pos - 2] == '*' && in8[pos - 1] == '/'))
					pos++;
			}

			if (in8[pos] == '-' && in8[pos + 1] == '-') {
				pos += 2;
				while (pos < len && in8[pos] != '\n')
					pos++;
			}

			if (in8[pos] == '"' || in8[pos] == '\'' || in8[pos] == '`') {
				char q = in8[pos];

				do  {
					out8[opos] = in8[pos];
					opos++;
					pos++;
				} while (pos < len && in8[pos] != q);
			}

			if (pos < len) {
				out8[opos] = in8[pos];
				opos++;
				pos++;
			}
		}

		return out8;
	}

	bool isSqlQuery(const char* in8) {
		if (in8 == 0)
			return false;

		char* text8 = removeComments(in8);

		int len = strlen(text8);
		int pos = 0;
		while (!isprint(text8[pos]) && pos < len)
			pos++;

		char* txt8 = new char[len + 1];
		strcpy(txt8, text8 + pos);
		strlwr(txt8);

		bool res = (strstr(txt8, "with") == txt8 && strstr(txt8, "select")) ||
			(strstr(txt8, "select") == txt8) ||
			((strstr(txt8, "insert") == txt8 || strstr(txt8, "replace") == txt8) && strstr(txt8, "into")) ||
			(strstr(txt8, "update") == txt8 && strstr(txt8, "set") && strstr(txt8, "=")) ||
			(strstr(txt8, "delete") == txt8 && strstr(txt8, "from")) ||
			((strstr(txt8, "create") == txt8 || (strstr(txt8, "drop") == txt8)) && (strstr(txt8, "index") || strstr(txt8, "table") || strstr(txt8, "view") || strstr(txt8, "trigger"))) ||
			(strstr(txt8, "alter") == txt8) ||
			(strstr(txt8, "begin") == txt8) ||
			(strstr(txt8, "commit") == txt8) ||
			(strstr(txt8, "rollback") == txt8) ||
			(strstr(txt8, "attach") == txt8) ||
			(strstr(txt8, "detach") == txt8) ||
			(strstr(txt8, "vacuum") == txt8) ||
			(strstr(txt8, "pragma") == txt8) ||
			(strstr(txt8, "explain") == txt8 && strstr(txt8, "plan"));

		delete [] text8;
		delete [] txt8;
		return res;
	}

	char* queryCSV(sqlite3* db, const char* query8, bool isColumns, const char* delimiter8, bool isUnixNewLine, int* rowCount, const char* null8) {
		int capacity = 1024;
		char* result8 = new char[capacity] {0};
		int size = 0;

		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
			int colCount = sqlite3_column_count(stmt);
			char BLOB8[] = "(BLOB)";
			char NULL8[] = "";

			while (isColumns || (SQLITE_ROW == sqlite3_step(stmt))) {
				int len = 0;

				for(int i = 0; i < colCount; i++)
					len += isColumns ? strlen((char*)sqlite3_column_name(stmt, i)) :
						sqlite3_column_type(stmt, i) == SQLITE_TEXT ?
						sqlite3_column_bytes(stmt, i) + 1 :
						20; // int, real, BLOB and null less 20

				len = 2 * len + 2 * colCount; // a"b --> "a""b". See https://en.wikipedia.org/wiki/Comma-separated_values
				char* line8 = new char[len + 1] {0};
				int pos = 0;
				for(int colNo = 0; colNo < colCount; colNo++) {
					int type = sqlite3_column_type(stmt, colNo);
					const char* value8 = isColumns ? (const char *)sqlite3_column_name(stmt, colNo)	:
						type == SQLITE_BLOB ? BLOB8 :
						type == SQLITE_NULL ? (null8 ? null8 : NULL8) :
						(const char *)sqlite3_column_text(stmt, colNo);


					bool isQ = strchr(value8, '"') || strchr(value8, '\n');

					if (isQ) {
						line8[pos] = '"';
						pos++;
					}

					for (int i = 0; i < (int)strlen(value8); i++) {
						line8[pos] = value8[i];
						pos++;

						if (value8[i] == '"') {
							line8[pos] = '"';
							pos++;
						}
					}

					if (isQ) {
						line8[pos] = '"';
						pos++;
					}

					if (colNo < colCount - 1) {
						line8[pos] = ',';
						pos++;
					}
				}

				if (!isUnixNewLine) {
					line8[pos] = '\r';
					pos++;
				}
				line8[pos] = '\n';
				pos++;

				*rowCount += !isColumns;
				isColumns = false;

				int lsize = pos;
				if (capacity - size < lsize + 100) {
					capacity *= 2;
					result8 = (char*) realloc(result8, capacity);
				}

				for (int i = 0; i < lsize; i++)
					result8[size + i] = line8[i];
				size += lsize;

				delete [] line8;
			}
		}

		sqlite3_finalize(stmt);

		if (size > 2)
			result8[size - 1 - !isUnixNewLine] = '\0';

		return result8;
	}

	TCHAR* getQueryError(sqlite3* db, const TCHAR* query16) {
		char* query8 = utils::utf16to8(query16);

		TCHAR* err16 = 0;

		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare_v2(db, query8, -1, &stmt, 0))
			err16 = utils::utf8to16(sqlite3_errmsg(db));
		sqlite3_finalize(stmt);

		delete [] query8;

		return err16;
	}
}
