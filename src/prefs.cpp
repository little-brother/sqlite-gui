#include <ctime>
#include "prefs.h"
#include "resource.h"

namespace prefs {
	sqlite3* db = NULL;

	const int ICOUNT = 91;
	const char* iprops[ICOUNT] = {
		"x", "y", "width", "height", "splitter-position-x", "splitter-position-y",
		"maximized", "font-size", "max-query-count", "exit-by-escape", "beep-query-duration", "synchronous-off",
		"cli-font-size", "cli-row-limit", "cli-preserve-inja",
		"backup-prefs", "ignore-readonly-prefs", "restore-db", "restore-editor", "use-highlight",
		"use-autocomplete", "autocomplete-by-tab", "disable-autocomplete-help", "use-foreign-keys", "use-legacy-rename", "editor-indent",
		"editor-tab-count", "editor-tab-current", "highlight-delay",
		"ask-delete", "word-wrap", "clear-values", "recent-count", "auto-filters", "edit-data-filter-mode",
		"csv-export-is-unix-line", "csv-export-delimiter", "csv-export-is-columns", "copy-to-clipboard-delimiter",
		"csv-import-encoding", "csv-import-delimiter", "csv-import-is-columns", "csv-import-is-create-table", "csv-import-is-truncate", "csv-import-is-replace", "csv-import-trim-values", "csv-import-skip-empty", "csv-import-abort-on-error",
		"odbc-strategy",
		"sql-export-multiple-insert", "extended-query-plan",
		"row-limit", "show-preview", "preview-width", "show-filters", "max-column-width",
		"chart-grid-size-x", "chart-grid-size-y", "dialog-row-width", "dialog-row-maximized",
		"cipher-legacy", "retain-passphrase",
		"check-update", "last-update-check",
		"case-sensitive",
		"help-version",
		"http-server", "http-server-port", "http-server-debug",
		"color-null", "color-blob", "color-text", "color-integer", "color-real", "color-current-cell",
		"color-keyword", "color-function", "color-quoted", "color-comment", "color-parenthesis", "color-pragma",
		"data-generator-row-count", "data-generator-truncate",
		"show-fk-links", "show-column-types", "show-isolation-reasons",
		"format-keyword-case", "format-function-case",
		"transpose-is-first-column", "transpose-is-row-col-names",
		"use-logger"
	};

	int ivalues[ICOUNT] = {
		100, 100, 800, 600, 200, 200,
		0, 10, 1000, 1, 3000, 1,
		8, 10, 1,
		0, 0, 1, 1, 1,
		1, 1, 0, 0, 0, 0, // use
		1, 0, 30,
		0, 0, 0, 10, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 1, 0, 1, 1, 1, 0, // csv-import
		0,
		0, 0,
		10000, 0, 200, 0, 400,
		100, 50, 400, 0, // chart and dialog-row size
		0, 0,
		1, 0,
		0,
		0,
		0, 3000, 0,
		// colors are stored in reverse order BGR
		0xFFF0F0, 0xFFF0FF, 0xF9F9F9, 0xF0F9FF, 0xF0FFF0, 0xA66046,
		0xC80000, 0xFF5C00, 0x00C800, 0x0000FF, 0xFFFF7F, 0x404080,
		100, 0,
		1, 1, 1,
		1, 1,
		0, 0,
		0
	};

	int get(const char* name) {
		for (int i = 0; i < ICOUNT; i++)
			if (!strcmp(iprops[i], name))
				return ivalues[i];

		return -1;
	}

	bool set(const char* name, int value) {
		bool res = false;
		for (int i = 0; i < ICOUNT; i++)
			if (!strcmp(iprops[i], name)) {
				ivalues[i] = value;
				res = true;
			}

		return res;
	}

	char* get(const char* name, const char* def) {
		char* value = NULL;
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db, "select value from prefs where name = ?1", -1, &stmt, 0)) {
			sqlite3_bind_text(stmt, 1, name, strlen(name),  SQLITE_TRANSIENT);
			const char* val = SQLITE_ROW == sqlite3_step(stmt) ? (char*)sqlite3_column_text(stmt, 0) : def;
			value = new char[strlen(val) + 1]{0};
			strcpy(value, val);
		}

		sqlite3_finalize(stmt);
		return value;
	}

	bool set(const char* name, const char* value, bool strict) {
		bool res = false;

		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db,
				strict ?
					"update prefs set value = ?2 where name = ?1" :
					"replace into prefs (name, value) values (?1, ?2)", -1, &stmt, 0)) {
			sqlite3_bind_text(stmt, 1, name, strlen(name),  SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, value, strlen(value),  SQLITE_TRANSIENT);

			res = SQLITE_DONE == sqlite3_step(stmt) && sqlite3_changes(db) == 1;
		}
		sqlite3_finalize(stmt);

		return res;
	}

	bool loadSqlResource(int id) {
		bool res = false;
		HMODULE hInstance = GetModuleHandle(0);
		HRSRC rc = FindResource(hInstance, MAKEINTRESOURCE(id), RT_RCDATA);
		HGLOBAL rcData = LoadResource(hInstance, rc);
		int len = SizeofResource(hInstance, rc);
		LPVOID data = LockResource(rcData);
		if (len > 0 && data) {
			char* sql8 = new char[len + 1]{0};
			memcpy(sql8, (const char*)data, len);
			res = SQLITE_OK == sqlite3_exec(db, "begin;", 0, 0, 0);
			res = res && (SQLITE_OK == sqlite3_exec(db, sql8, 0, 0, 0));
			if (!res)
				MessageBoxA(0, sqlite3_errmsg(db), 0, 0);

			sqlite3_exec(db, res ? "commit;" : "rollback;", 0, 0, 0);

			delete [] sql8;
		}
		FreeResource(rcData);

		return res;
	}

	bool load(char* path) {
		bool isOpen = SQLITE_OK == sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, 0);
		bool isReadWrite = isOpen && !sqlite3_db_readonly(db, 0);
		if (!isOpen || !isReadWrite) {
			sqlite3_close_v2(db);
			sqlite3_open_v2(":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, 0);

			if (isOpen) {
				sqlite3* file;
				sqlite3_open_v2(path, &file, SQLITE_OPEN_READONLY | SQLITE_OPEN_URI, 0);
				sqlite3_backup *pBackup;
				pBackup = sqlite3_backup_init(db, "main", file, "main");
				if (pBackup) {
					(void)sqlite3_backup_step(pBackup, -1);
					(void)sqlite3_backup_finish(pBackup);
				}
				sqlite3_close_v2(file);
			}
		}

		// migration from 1.8.2 and earlier versions
		if (SQLITE_OK == sqlite3_exec(db, "select 1 from help where 1 = 2", 0, 0, 0) &&
			SQLITE_OK != sqlite3_exec(db, "select source from help where 1 = 2", 0, 0, 0)) {
			MessageBox(0, TEXT("The help structure was changed. You need reinstall all extensions\nto update help. Open Database > Extension manager and uncheck and check again all checked entries.\n\n(Re-)Install odbc-extension to enable all ODBC features."), TEXT("Information"), MB_ICONINFORMATION);
			sqlite3_exec(db, "drop table help", 0, 0, 0);
		}
		sqlite3_exec(db, "alter table functions add column help text", 0, 0, 0);

		if (!loadSqlResource(IDR_INIT))
			return false;

		// migration from 1.7.1 and earlier versions
		if (SQLITE_OK != sqlite3_exec(db, "select refname from refs where 1 = 2", 0, 0, 0)) {
			sqlite3_exec(db, "alter table refs add column refname text", 0, 0, 0);
			if(IDYES == MessageBox(0, TEXT("Reference format in table prefs.refs has been changed.\nClear the table to rebuild references automatically?\n\nVisit Wiki to get details."), TEXT("Confirmation"), MB_YESNO))
				sqlite3_exec(db, "delete from refs", 0, 0, 0);
		}

		// migration from 1.9.0 to 1.9.1
		if (SQLITE_OK == sqlite3_exec(db, "select 1 from extensions where 1 = 2", 0, 0, 0)) {
			sqlite3_exec(db, "insert into addons (name, type, enable, version) select name, 0, enable, version from extensions", 0, 0, 0);
			sqlite3_exec(db, "drop table extensions", 0, 0, 0);
		}

		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db, "select name, value from prefs where value GLOB '*[0-9]*'", -1, &stmt, 0)) {
			while(sqlite3_step(stmt) == SQLITE_ROW)
				set((char*) sqlite3_column_text(stmt, 0), sqlite3_column_int(stmt, 1));
		}
		sqlite3_finalize(stmt);

		if (HELP_VERSION != get("help-version") && loadSqlResource(IDR_HELP))
			set("help-version", HELP_VERSION);

		if (!isReadWrite && !get("ignore-readonly-prefs"))
				MessageBox(0, TEXT("Can't open prefs.sqlite for writing.\nSettings will not be saved."), 0, 0);

		return true;
	}

	bool save() {
		sqlite3_stmt* stmt;
		bool rc = SQLITE_OK == sqlite3_prepare_v2(db, "replace into 'prefs' (name, value) values (?1, ?2);", -1, &stmt, 0);
		if (rc) {
			for(int i = 0; i < ICOUNT; i++) {
				sqlite3_bind_text(stmt, 1, iprops[i], strlen(iprops[i]),  SQLITE_TRANSIENT);
				sqlite3_bind_int(stmt, 2, ivalues[i]);
				sqlite3_step(stmt);
				sqlite3_reset(stmt);
			}
		}
		sqlite3_finalize(stmt);
		return rc;
	}

	bool backup() {
		const char* dbpath = sqlite3_db_filename(db, 0);
		int len = strlen(dbpath);
		char backup8[len + 64]{0};
		strcat(backup8, "vacuum into '");
		strncat(backup8, dbpath, len - strlen(".sqlite"));
		strcat(backup8, ".backup'");
		return SQLITE_OK == sqlite3_exec(db, backup8, 0, 0, 0);
	}

	bool setSyncMode(int mode) {
		char query[255];
		sprintf(query, "pragma synchronous = %i;", mode);
		return SQLITE_DONE == sqlite3_exec(db, query, 0, 0, 0);
	}

	bool applyHelp(const char* json, const char* source) {
		bool rc = true;

		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db,
			"with j (type, signature, description, example, alias, args) as (select " \
			"json_extract(value, '$.type'), json_extract(value, '$.signature'), json_extract(value, '$.description'), "	\
			"json_extract(value, '$.example'), json_extract(value, '$.alias'), json_extract(value, '$.args') from json_each(?1))" \
			"insert or replace into \"help\" (keyword, \"type\", signature, description, example, alias, args, nargs, source) " \
			"select " \
			"trim(substr(signature, 0, instr(signature, '('))), type, signature, description, example, alias, args, " \
			"iif(instr(signature, '...') = 0, length(args) - length(replace(args, '|', '')) + (instr(args, '()') = 0), -1), ?2 " \
			"from j;", -1, &stmt, 0)) {
			sqlite3_bind_text(stmt, 1, json, strlen(json),  SQLITE_TRANSIENT);
			if (source) {
				sqlite3_bind_text(stmt, 2, source, strlen(source),  SQLITE_TRANSIENT);
			} else {
				sqlite3_bind_null(stmt, 2);
			}

			rc = SQLITE_DONE == sqlite3_step(stmt);
		} else {
			printf(sqlite3_errmsg(db));
		}
		sqlite3_finalize(stmt);

		return rc;
	}
}
