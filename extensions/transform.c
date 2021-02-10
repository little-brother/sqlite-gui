/*
	pivot(query, target)
	Calculate a pivot of query result and put it into a target table. If target table exists then it will be recreated.
	Example
	select pivot('select manager, strftime('01-%m-%Y', saledate) date, qty from orders group by 2', 'mytable')

	unpivot(query, target)
	Calculate a unpivot of query result and put it into a target table. If the target table exists then it will be recreated.
	unpivot(pivot(query)) should return the same data.
	Example
	select pivot('select manager, "01-01-2020", "01-02-2020", "01-03-2020", ... from pivot_data', 'mytable')

	jsontable(json, target)
	Parse an input json and put a resultset to a target table. If the target table exists then it will be recreated.
	Column names are c1, c2, etc.
	select jsontable('["a", "b"]', 'mytable')
	select jsontable('[{"a": 10, "b": 20}, {"a": 100, "b": 200}]', 'mytable')
	select jsontable('[["10", "20"], ["100", "200"]]', 'mytable')

	txttable(text, target)
	Split text by lines and put a resultset to a target table. If the target table exists then it will be recreated.
	The target table always has one column c1.
	select txttable('line1'|| x'0d' || x'0a' || 'line2'|| x'0a' || 'line3', 'mytable') 
*/
#define UNICODE
#define _UNICODE

#define MAX_DATA_LENGTH 32000
#define MAX_COLUMN_LENGTH 2000

#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>

char* getName(const char* in, BOOL isSchema) {
	char* res = (char*)calloc(strlen(in) + 5, sizeof(char));
	if (!strlen(in)) 
		return strcat(res, isSchema ? "main" : "");

	const char* p = in;
	while (p[0] && !_istgraph(p[0]))
		p++;

	char* q = p ? strchr("'`\"[", p[0]) : 0;
	if (q) {
		char* q2 = strchr(p + 1, q[0] == '[' ? ']' : q[0]);
		if (q2 && ((isSchema && q2[1] == '.') || (!isSchema && q2[1] != '.')))
			strncpy(res, p + 1, strlen(p) - strlen(q2) - 1);

		if (q2 && !isSchema && q2[1] == '.' && q2[2] != 0) {
			free(res);
			return getName(q2 + 2, FALSE);
		}

	} else {
		char* d = p ? strchr(p, '.') : 0;
		if (d && isSchema)
			strncpy(res, p, strlen(p) - strlen(d));
		if (d && !isSchema) {
			free(res);
			return getName(d + 1, FALSE);
		}
	}

	if (!res[0])
		strcat(res, isSchema ? "main" : in);

	return res;
}

static void onError(sqlite3* db, sqlite3_context *ctx, const char* info) {
	char result[strlen(sqlite3_errmsg(db)) + strlen(info) + 128];
	sprintf(result, "{\"error\": \"%s\", \"info\": \"%s\"}", sqlite3_errmsg(db), info);
	sqlite3_result_text(ctx, result, strlen(result), SQLITE_TRANSIENT);
}

static void pivot(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	const char* query = sqlite3_value_text(argv[0]);
	const char* target = sqlite3_value_text(argv[1]);
	sqlite3* db = sqlite3_context_db_handle(ctx);

	srand(time(NULL));
	int sid = rand();

	char buf[strlen(query) + 255];
	sprintf(buf, "create table temp.pivot%i as select * from (%s)", sid, query); 
	if (SQLITE_OK != sqlite3_exec(db, buf, 0, 0, 0)) 
		return onError(db, ctx, buf);

	BOOL rc = TRUE;
	sqlite3_stmt* stmt;
	char sbuf[256];

	char a[MAX_COLUMN_LENGTH + 1], b[MAX_COLUMN_LENGTH + 1], c[MAX_COLUMN_LENGTH + 1];
	memset(a, 0, MAX_COLUMN_LENGTH + 1);
	memset(b, 0, MAX_COLUMN_LENGTH + 1);
	memset(c, 0, MAX_COLUMN_LENGTH + 1);

	sprintf(sbuf, "select * from temp.pivot%i where 1 = 2", sid);
	rc = (SQLITE_OK == sqlite3_prepare_v2(db, sbuf, -1, &stmt, 0)) && (SQLITE_DONE == sqlite3_step(stmt)); 
	if (rc && sqlite3_column_count(stmt) != 3) {
		sprintf(sbuf, "Invalid column count in the query. Should be exactly 3.");
		rc = FALSE;
	}

	if (rc) {
		strncpy(a, sqlite3_column_name(stmt, 0), MAX_COLUMN_LENGTH);
		strncpy(b, sqlite3_column_name(stmt, 1), MAX_COLUMN_LENGTH);
		strncpy(c, sqlite3_column_name(stmt, 2), MAX_COLUMN_LENGTH);
	}

	sqlite3_finalize(stmt);

	if (!rc)
		return onError(db, ctx, sbuf);

	sprintf(sbuf, "select distinct \"%s\" from temp.pivot%i where \"b\" is not null", b, sid, b);
	rc = SQLITE_OK == sqlite3_prepare_v2(db, sbuf, -1, &stmt, 0); 
	char q[MAX_DATA_LENGTH + 1];
	memset(q, 0, MAX_DATA_LENGTH + 1);
	if (rc) {
		char* schema = getName(target, TRUE);
		char* tablename = getName(target, FALSE);
		sprintf(q, "drop table if exists \"%s\".\"%s\";create table \"%s\".\"%s\" as select \"%s\"", schema, tablename, schema, tablename, a);
		free(schema);
		free(tablename);

		while (SQLITE_ROW == sqlite3_step(stmt)) {
			const char* val = sqlite3_column_text(stmt, 0);
			char buf[strlen(val)* 2 + 64];
			sprintf(buf, ", max(%s) filter(where \"%s\" = '%s') \"%s\"", c, b, val, val);
			strcat(q, buf);
		}
		sprintf(sbuf, " from temp.pivot%i group by 1", sid);
		strcat(q, sbuf);

		rc = SQLITE_OK == sqlite3_exec(db, q, 0, 0, 0);

		sprintf(sbuf, "drop table temp.pivot%i", sid);
		sqlite3_exec(db, sbuf, 0, 0, 0);
	}
	sqlite3_finalize(stmt);

	if (rc) {
		char result[] = "{\"result\": \"ok\"}";
		sqlite3_result_text(ctx, result, strlen(result), SQLITE_TRANSIENT);
	} else {
		onError(db, ctx, q);
	}
}

static void unpivot(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	const char* query = sqlite3_value_text(argv[0]);
	const char* target = sqlite3_value_text(argv[1]);
	sqlite3* db = sqlite3_context_db_handle(ctx);

	srand(time(NULL));
	int sid = rand();

	char buf[strlen(query) + 255];
	sprintf(buf, "create table temp.unpivot%i as select * from (%s)", sid, query); 
	if (SQLITE_OK != sqlite3_exec(db, buf, 0, 0, 0)) 
		return onError(db, ctx, buf);

	BOOL rc = TRUE;
	sqlite3_stmt* stmt;
	char sbuf[256];

	sprintf(sbuf, "select * from temp.unpivot%i where 1 = 2", sid);
	rc = (SQLITE_OK == sqlite3_prepare_v2(db, sbuf, -1, &stmt, 0)) && (SQLITE_DONE == sqlite3_step(stmt)); 

	int colCount = sqlite3_column_count(stmt);
	if (rc && colCount < 3) {
		sprintf(sbuf, "Invalid column count in the query. Should be 3 or more.");
		rc = FALSE;
	}

	if (rc) {
		const unsigned char* a = sqlite3_column_name(stmt, 0);

		int size = 255;
		for (int colNo = 1; colNo < colNo; colNo++)
			size += strlen(a) + 3 * strlen(sqlite3_column_name(stmt, colNo)) + 255;

		unsigned char q[size + 1];
		char* schema = getName(target, TRUE);
		char* tablename = getName(target, FALSE);
		sprintf(q, "drop table if exists \"%s\".\"%s\";create table \"%s\".\"%s\" as ", schema, tablename, schema, tablename);
		free(schema);
		free(tablename);

		for (int colNo = 1; colNo < colCount; colNo++) {
			if (colNo > 1) 
				strcat(q, " union all ");
			
			const unsigned char* b = sqlite3_column_name(stmt, colNo); 
			int size = strlen(a) + 3 * strlen(b) + 255; 
			unsigned char buf[size + 1];
			sprintf(buf, "select %s, '%s' c1, \"%s\" c2 from temp.unpivot%i where \"%s\" is not null", a, b, b, sid, b);
			strcat(q, buf);
		}

		rc = SQLITE_OK == sqlite3_exec(db, q, 0, 0, 0);
		if (rc) {
			char result[] = "{\"result\": \"ok\"}";
			sqlite3_result_text(ctx, result, strlen(result), SQLITE_TRANSIENT);
		} else {
			onError(db, ctx, q);
		}	
	} else {
		return onError(db, ctx, sbuf);
	}

	sqlite3_finalize(stmt);
}

static void jsontable(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	const char* json = sqlite3_value_text(argv[0]);
	const char* target = sqlite3_value_text(argv[1]);
	sqlite3* db = sqlite3_context_db_handle(ctx);

	unsigned char query[2000];
	char* schema = getName(target, TRUE);
	char* tablename = getName(target, FALSE);
	sprintf(query, "create table \"%s\".\"%s\" as select ", schema, tablename);		

	sqlite3_stmt* stmt;
	BOOL rc = SQLITE_OK == sqlite3_prepare_v2(db, "select type, value from json_each(?1) limit 1", -1, &stmt, 0);
	if (rc) { 
		sqlite3_bind_text(stmt, 1, json, strlen(json), SQLITE_TRANSIENT);
		if (SQLITE_ROW == sqlite3_step(stmt)) {
			const unsigned char* type = sqlite3_column_text(stmt, 0);
			if (strcmp(type, "object") == 0) {
				sqlite3_stmt* stmt2;
				rc = SQLITE_OK == sqlite3_prepare_v2(db, "select key from json_each(?1)", -1, &stmt2, 0);
				const unsigned char* value = sqlite3_column_text(stmt, 1); 
				sqlite3_bind_text(stmt2, 1, value, strlen(value), SQLITE_TRANSIENT);
				int colNo = 0;
				while (rc && SQLITE_ROW == sqlite3_step(stmt2)) {
					const unsigned char* key = sqlite3_column_text(stmt2, 0);
					unsigned char buf[strlen(key) + 64];
					sprintf(buf, "%sjson_extract(value, '$.%s') c%i", colNo ? ", " : "", key, colNo + 1);
					strcat(query, buf);					
					colNo++;
				}
				sqlite3_finalize(stmt2);
				rc = rc && (colNo > 0);
			} else if (strcmp(type, "array") == 0) {
				sqlite3_stmt* stmt2;
				rc = SQLITE_OK == sqlite3_prepare_v2(db, "select json_array_length(?1)", -1, &stmt2, 0);
				const unsigned char* value = sqlite3_column_text(stmt, 1); 
				sqlite3_bind_text(stmt2, 1, value, strlen(value), SQLITE_TRANSIENT);
				rc = rc && SQLITE_ROW == sqlite3_step(stmt2);
				if (rc) {
					int colCount = sqlite3_column_int(stmt2, 0); 
	
					for (int colNo = 0; colNo < colCount; colNo++) {
						unsigned char buf[64];
						sprintf(buf, "%sjson_extract(value, '$[%i]') c%i", colNo ? ", " : "", colNo, colNo + 1);
						strcat(query, buf);
					}
				}
				sqlite3_finalize(stmt2);
			} else {
				strcat(query, "value c1");
			}
			strcat(query, " from json_each(?1)");

			if (!rc)
				onError(db, ctx, query);
		} else {
			onError(db, ctx, "Invalid json or an array is empty");
			rc = FALSE;
		}
	} else {
		onError(db, ctx, json);
		rc = FALSE;
	}
	sqlite3_finalize(stmt);

	char buf[strlen(target) + 255];
	sprintf(buf, "drop table if exists \"%s\".\"%s\";", schema, tablename); 
	free(tablename);
	free(schema);

	if (!rc) 
		return;
	
	if (SQLITE_OK != sqlite3_exec(db, buf, 0, 0, 0)) 
		return onError(db, ctx, buf);

	if (SQLITE_OK == sqlite3_prepare_v2(db, query, -1, &stmt, 0) && 
		SQLITE_OK == sqlite3_bind_text(stmt, 1, json, strlen(json), SQLITE_TRANSIENT) &&
		SQLITE_DONE == sqlite3_step(stmt)) {
		sprintf(buf, "{\"result\": \"ok\"}");	
		sqlite3_result_text(ctx, buf, strlen(buf), SQLITE_TRANSIENT);
	} else {
		onError(db, ctx, query);
	}
	sqlite3_finalize(stmt);
}

static void txttable(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	const char* text = sqlite3_value_text(argv[0]);
	const char* target = sqlite3_value_text(argv[1]);
	sqlite3* db = sqlite3_context_db_handle(ctx);

	srand(time(NULL));
	int sid = rand();

	char buf[512 + strlen(target) * 2];
	sprintf(buf, "create table temp.txttable%i (c1 text)", sid); 
	if (SQLITE_OK != sqlite3_exec(db, buf, 0, 0, 0)) 
		return onError(db, ctx, buf);

	BOOL rc = TRUE;
	sqlite3_stmt* stmt;
	sprintf(buf, "insert into temp.txttable%i (c1) values (?1)", sid);
	rc = (SQLITE_OK == sqlite3_prepare_v2(db, buf, -1, &stmt, 0));
	unsigned char* pos = (unsigned char*)text;
	while (rc && pos) {
		unsigned char* end = strchr(pos, '\n');
		int len = strlen(pos) - (end ? strlen(end) : 0);
		char line[len + 1];
		memset(line, 0, len + 1);
		memcpy(line, pos, len - (end && (end - 1)[0] == '\r')); /* Remove \r\n or \n */
		line[len] = 0;
		pos = end ? end + 1 : 0;

		sqlite3_bind_text(stmt, 1, line, strlen(line), SQLITE_TRANSIENT);
		rc = SQLITE_DONE == sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);

	char* schema = getName(target, TRUE);
	char* tablename = getName(target, FALSE);
	sprintf(buf, "drop table if exists \"%s\".\"%s\";create table \"%s\".\"%s\" as select * from temp.txttable%i; drop table temp.txttable%i;", 
		schema, tablename, schema, tablename, sid, sid);
	free(schema);
	free(tablename);

	if (rc && SQLITE_OK == sqlite3_exec(db, buf, 0, 0, 0)) {
		sprintf(buf, "{\"result\": \"ok\"}");	
		sqlite3_result_text(ctx, buf, strlen(buf), SQLITE_TRANSIENT);
	} else {
		onError(db, ctx, buf);
	}
}

__declspec(dllexport) int sqlite3_transform_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
	int rc = SQLITE_OK;
	SQLITE_EXTENSION_INIT2(pApi);
	(void)pzErrMsg;  /* Unused parameter */
	return SQLITE_OK == sqlite3_create_function(db, "pivot", 2, SQLITE_UTF8, NULL, pivot, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "unpivot", 2, SQLITE_UTF8, NULL, unpivot, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "jsontable", 2, SQLITE_UTF8, NULL, jsontable, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "txttable", 2, SQLITE_UTF8, NULL, txttable, 0, 0) ?
		SQLITE_OK : SQLITE_ERROR;
}