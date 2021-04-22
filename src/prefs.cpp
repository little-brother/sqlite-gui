#include <ctime>
#include "prefs.h"

namespace prefs {
	sqlite3* db = NULL;

	const int ICOUNT = 46;
	const char* iprops[ICOUNT] = {
		"x", "y", "width", "height", "splitter-width", "splitter-height",
		"maximized", "font-size", "max-query-count", "exit-by-escape", "beep-query-duration", "synchronous-off",
		"cli-font-size", "cli-row-limit", "cli-max-width",
		"backup-prefs", "autoload-extensions", "restore-db", "restore-editor", "use-highlight", "use-legacy-rename", "editor-indent", "editor-tab-count", "editor-tab-current",
		"ask-delete", "word-wrap", "clear-values", "recent-count",
		"csv-export-is-unix-line", "csv-export-delimiter",
		"csv-import-encoding", "csv-import-delimiter", "csv-import-is-columns", "odbc-strategy",
		"row-limit",
		"cipher-legacy",
		"color-null", "color-blob", "color-text", "color-integer", "color-real",
		"data-generator-row-count", "data-generator-truncate",
		"link-fk", "link-view", "link-trigger"
	};

	int ivalues[ICOUNT] = {
		100, 100, 800, 600, 200, 200,
		0, 10, 1000, 1, 3000, 1,
		8, 10, 20,
		0, 1, 1, 1, 1, 0, 0, 1, 0,
		0, 0, 0, 10,
		0, 0,
		0, 0, 1, 0,
		10000,
		0,
		0x00FFF0F0, 0xFFF0FF, 0xF9F9F9, 0xF0F9FF, 0xF0FFF0, // reverse order BGR
		100, 0,
		1, 0, 0
	};

	int get(const char* name) {
		for (int i = 0; i < ICOUNT; i++)
			if (!strcmp(iprops[i], name))
				return ivalues[i];

		return 0;
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
		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare_v2(db, "select value from 'prefs' where name = ?;", -1, &stmt, 0))
			return NULL;

		sqlite3_bind_text(stmt, 1, name, strlen(name),  SQLITE_TRANSIENT);
		const char* val = SQLITE_ROW == sqlite3_step(stmt) ? (char*)sqlite3_column_text(stmt, 0) : def;
		char* value = new char[strlen(val) + 1]{0};
		strcpy(value, val);

		sqlite3_finalize(stmt);
		return value;
	}

	bool set(const char* name, const char* value) {
		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare_v2(db, "replace into 'prefs' (name, value) values (?1, ?2);", -1, &stmt, 0))
			return false;

		sqlite3_bind_text(stmt, 1, name, strlen(name),  SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, value, strlen(value),  SQLITE_TRANSIENT);

		bool res = SQLITE_OK == sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		return res;
	}

	bool load(char* path) {
		bool isOpen = SQLITE_OK == sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, 0);
		bool isReadWrite = isOpen && !sqlite3_db_readonly(db, 0);
		if (!isOpen || !isReadWrite) {
			sqlite3_close_v2(db);
			sqlite3_open_v2(":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, 0);
			MessageBox(0, TEXT("Can't open prefs.sqlite for writing.\nSettings will not be saved."), 0, 0);

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

		char sql8[] = "" \
			"begin;\n" \
			"create table if not exists prefs (name text not null, value text, primary key (name));" \
			"create table if not exists recents (path text not null, time real not null, primary key (path));" \
			"create table if not exists history (query text not null, time real not null, primary key (query));" \
			"create table if not exists gists (query text not null, time real not null, primary key (query));" \
			"create table if not exists generators (type text, value text);" \
			"create table if not exists refs (dbname text not null, schema text not null, tblname text not null, colname text not null, query text, primary key (dbname, schema, tblname, colname)); " \
			"create table if not exists disabled (dbpath text not null, type text not null, name text not null, sql text, primary key (dbpath, type, name)); " \
			"create table if not exists cli (\"time\" real, dbname text not null, query text not null, elapsed integer, result text); " \
			"create table if not exists diagrams (dbname text, tblname text, x integer, y integer, width integer, height integer, primary key (dbname, tblname));" \
			"create table if not exists main.encryption (dbpath text, param text, idc integer, value text, no integer, primary key (dbpath, param));" \
			"create table if not exists temp.encryption (dbpath text, param text, idc integer, value text, no integer, primary key (dbpath, param));" \
			"create table if not exists query_params (dbname text, name text, value text, primary key (dbname, name, value));" \
			"create index if not exists idx_cli on cli (\"time\" desc, dbname);" \
			"commit;";

		if (SQLITE_OK != sqlite3_exec(db, sql8, 0, 0, 0))
			return false;

		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare_v2(db, "select name, value from prefs where value GLOB '*[0-9]*'", -1, &stmt, 0)) {
			sqlite3_finalize(stmt);
			return false;
		}

		while(sqlite3_step(stmt) == SQLITE_ROW)
			set((char*) sqlite3_column_text(stmt, 0), atoi((char*) sqlite3_column_text(stmt, 1)));

		sqlite3_finalize(stmt);
		return true;
	}

	bool save() {
		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare_v2(db, "replace into 'prefs' (name, value) values (?1, ?2);", -1, &stmt, 0))
			return false;

		for(int i = 0; i < ICOUNT; i++) {
			sqlite3_bind_text(stmt, 1, iprops[i], strlen(iprops[i]),  SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, ivalues[i]);
			sqlite3_step(stmt);
			sqlite3_reset(stmt);
		}

		sqlite3_finalize(stmt);
		sqlite3_close(db);
		db = NULL;
		return true;
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
}
