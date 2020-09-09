#include <ctime>
#include "prefs.h"
#include "sqlite3.h"

namespace prefs {
	sqlite3* db;

	const int ICOUNT = 28;
	const char* iprops[ICOUNT] = {
		"x", "y", "width", "height", "splitter-width", "splitter-height",
		"maximized", "font-size", "max-query-count",
		"autoload-extensions", "restore-db", "restore-editor", "use-highlight", "use-legacy-rename", "editor-indent", "editor-tab-count",
		"csv-export-is-unix-line", "csv-export-delimiter",
		"csv-import-encoding", "csv-import-delimiter", "csv-import-is-columns",
		"row-limit",
		"data-generator-row-count", "data-generator-truncate",
		"exit-by-escape",
		"link-fk", "link-view", "link-trigger"
	};

	int ivalues[ICOUNT] = {
		100, 100, 800, 600, 200, 200,
		0, 10, 1000,
		1, 1, 1, 1, 0, 0, 1,
		0, 0,
		0, 0, 1,
		10000,
		100, 0,
		1,
		1, 0, 0
	};

	int get(const char* name) {
		for (int i = 0; i < ICOUNT; i++)
			if (!strcmp(iprops[i], name))
				return ivalues[i];

		return 0;
	}

	void set(const char* name, int value) {
		for (int i = 0; i < ICOUNT; i++)
			if (!strcmp(iprops[i], name))
				ivalues[i] = value;
	}

	char* get(const char* name, const char* def) {
		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare(db, "select value from 'prefs' where name = ?;", -1, &stmt, 0))
			return NULL;

		sqlite3_bind_text(stmt, 1, name, strlen(name),  SQLITE_TRANSIENT);
		const char* val = SQLITE_ROW == sqlite3_step(stmt) ? (char*)sqlite3_column_text(stmt, 0) : def;
		char* value = new char[strlen(val) + 1]{0};
		strcpy(value, val);

		sqlite3_finalize(stmt);
		return value;
	}

	bool set(const char* name, char* value) {
		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare(db, "replace into 'prefs' (name, value) values (?1, ?2);", -1, &stmt, 0))
			return false;

		sqlite3_bind_text(stmt, 1, name, strlen(name),  SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, value, strlen(value),  SQLITE_TRANSIENT);

		bool res = SQLITE_OK == sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		return res;
	}

	bool load() {
		if (SQLITE_OK != sqlite3_open("prefs.sqlite", &db))
			return false;

		char sql8[] = "" \
			"begin;\n" \
			"create table if not exists prefs (name text not null unique, value text not null, primary key (name));" \
			"create table if not exists recents (path text not null unique, time real not null, primary key (path));" \
			"create table if not exists history (query text not null unique, time real not null, primary key (query));" \
			"create table if not exists gists (query text not null unique, time real not null, primary key (query));" \
			"create table if not exists generators (type text, value text);" \
			"create table if not exists diagrams (dbname text, tblname text, x integer, y integer, width integer, height integer, primary key (dbname, tblname));" \
			"create unique index if not exists idx_history on history (query);" \
			"create unique index if not exists idx_gists on gists (query);" \
			"commit;" \
			"pragma synchronous = 0;";

		if (SQLITE_OK != sqlite3_exec(db, sql8, 0, 0, 0))
			return false;

		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare(db, "select name, value from prefs where value GLOB '*[0-9]*'", -1, &stmt, 0)) {
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
		if (SQLITE_OK != sqlite3_prepare(db, "replace into 'prefs' (name, value) values (?1, ?2);", -1, &stmt, 0))
			return false;

		for(int i = 0; i < ICOUNT; i++) {
			sqlite3_bind_text(stmt, 1, iprops[i], strlen(iprops[i]),  SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, ivalues[i]);
			sqlite3_step(stmt);
			sqlite3_reset(stmt);
		}

		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return true;
	}

	bool setRecent(char* path) {
		sqlite3_stmt* stmt;
		int rc = sqlite3_prepare(db, "replace into 'recents' (path, time) values (?1, ?2);", -1, &stmt, 0);
		if (rc != SQLITE_OK) {
			sqlite3_finalize(stmt);
			return false;
		}

		sqlite3_bind_text(stmt, 1, path, strlen(path),  SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 2, std::time(0));
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		return true;
	}

	int getRecents(char** recents) {
		sqlite3_stmt* stmt;
		int rc = sqlite3_prepare(db, "select path from recents order by time desc limit 100", -1, &stmt, 0);
		if (SQLITE_OK != rc) {
			sqlite3_finalize(stmt);
			return 0;
		}

		int count = 0;
		while(sqlite3_step(stmt) == SQLITE_ROW) {
			char* path8 = (char*) sqlite3_column_text(stmt, 0);
			recents[count] = new char[strlen(path8) + 1]{0};
			strcpy(recents[count], path8);
			count++;
		}

		sqlite3_finalize(stmt);
		return count;
	}

	bool setQuery(const char* table, const char* query) {
		char buf[256];
		sprintf(buf, "replace into %s (query, time) values (?1, ?2);", table);

		sqlite3_stmt* stmt;
		int rc = sqlite3_prepare(db, buf, -1, &stmt, 0);
		if (rc != SQLITE_OK) {
			sqlite3_finalize(stmt);
			return false;
		}

		sqlite3_bind_text(stmt, 1, query, strlen(query), SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 2, std::time(0));

		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		return (rc == SQLITE_OK);
	}

	int deleteQuery(const char* table, const char* query) {
		char buf[256];
		sprintf(buf, "delete from %s where query = ?1;", table);

		sqlite3_stmt* stmt;
		int rc = sqlite3_prepare(db, buf, -1, &stmt, 0);
		if (rc != SQLITE_OK) {
			sqlite3_finalize(stmt);
			return false;
		}

		sqlite3_bind_text(stmt, 1, query, strlen(query), SQLITE_TRANSIENT);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		return (rc == SQLITE_OK);
	}

	int getQueries(const char* table, const char* filter, char** queries) {
		char buf[256];
		sprintf(buf, "select strftime('%%d-%%m-%%Y %%H:%%M', time, 'unixepoch') || '\t' || query from %s %s order by time desc limit %i", table, strlen(filter) ? "where query like ?1" : "", get("max-query-count"));

		sqlite3_stmt* stmt;
		int rc = sqlite3_prepare(db, buf, -1, &stmt, 0);

		if (SQLITE_OK != rc) {
			char* err8 = (char*)sqlite3_errmsg(db);
			printf("\nERROR: %s\n",err8);
			sqlite3_finalize(stmt);
			return 0;
		}

		if (strlen(filter)) {
			char* filter8 = new char[strlen(filter) + 3]{0};
			sprintf(filter8, "%%%s%%", filter);
			sqlite3_bind_text(stmt, 1, filter8, strlen(filter8), SQLITE_TRANSIENT);
			delete [] filter8;
		}

		int count = 0;
		while(sqlite3_step(stmt) == SQLITE_ROW) {
			char* sql8 = (char*) sqlite3_column_text(stmt, 0);
			queries[count] = new char[strlen(sql8) + 1]{0};
			strcpy(queries[count], sql8);
			count++;
		}

		sqlite3_finalize(stmt);
		return count;
	}

	bool getDiagramRect(const char* dbname, const char* table, RECT* rect) {
		sqlite3_stmt * stmt;
		int rc = sqlite3_prepare(db, "select x, y, width, height from 'diagrams' where dbname = ?1 and tblname = ?2", -1, &stmt, 0);
		if (rc == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, dbname, strlen(dbname), SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, table, strlen(table), SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				rect->left = sqlite3_column_int(stmt, 0);
				rect->top = sqlite3_column_int(stmt, 1);
				rect->right = sqlite3_column_int(stmt, 2);
				rect->bottom = sqlite3_column_int(stmt, 3);
			}
		}
		sqlite3_finalize(stmt);

		return rc == SQLITE_DONE || rc == SQLITE_OK;
	}
	bool setDiagramRect(const char* dbname, const char* table, RECT rect) {
		sqlite3_stmt* stmt;
		int rc = sqlite3_prepare(db, "replace into 'diagrams' (dbname, tblname, x, y, width, height) values (?1, ?2, ?3, ?4, ?5, ?6)", -1, &stmt, 0);
		if (rc == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, dbname, strlen(dbname), SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, table, strlen(table), SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 3, rect.left);
			sqlite3_bind_int(stmt, 4, rect.top);
			sqlite3_bind_int(stmt, 5, rect.right);
			sqlite3_bind_int(stmt, 6, rect.bottom);

			rc = sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);

		return rc == SQLITE_DONE || rc == SQLITE_OK;
	}
}
