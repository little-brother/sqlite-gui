/*
	odbc_read(connectionString, query, target)
	Read data via ODBC from external source and write it to SQLite table. If the target table doesn't exist, it'll be created.
	Use TABLES as the query to obtain a table list.
	Use TYPES as the query to obtain support types.

	odbc_write(query, connectionString, target)
	Upload query resultset from SQLite to external database.
	If the target doesn't exist then it will be created. 
	Otherwise, query columns should be complete the same with target coiumns.

	odbc_query(connectionString, query)
	Execute query on external database e.g. to create target table.

	odbc_dsn()
	Returns local DSN list as json array: {"result": ["MyData", "Csv", ...], ...}

	Returns json
		OK - {"result": "ok", ...}
		Error - {"error": "<msg>"}

	Cons
	* Some driver has read-only mode e.g. Microsoft Text Driver (*.csv, *.txt)
	* Supports only basic types - numbers and text. 
	* BLOB, datetime and etc are unsupported.

	Remarks
	* Use 32bit ODBC manager: C:\Windows\SysWOW64\odbcad32.exe

	To-Do: 
		* add opts: row limit, time limit, rejected rows limit, commit each X rows, clear before load data, custom column brackets 
		* update/insert/replace modes (requires pk) for odbc_write
		* a table for rejected rows %trg%_rjt (has all columns as text)
		* Mask quotes inside error message
*/
#define UNICODE
#define _UNICODE

#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include <ctype.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <assert.h>

#define MAX_DATA_LENGTH 32000
#define MAX_COLUMN_LENGTH 2000
#define MAX_ERROR_LENGTH 2000

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

char* utf16to8(const TCHAR* in) {
	char* out;
	if (!in || _tcslen(in) == 0) {
		out = (char*)calloc (1, sizeof(char));
	} else {
		int len = WideCharToMultiByte(CP_UTF8, 0, in, -1, NULL, 0, 0, 0);
		out = (char*)calloc (len, sizeof(char));
		WideCharToMultiByte(CP_UTF8, 0, in, -1, out, len, 0, 0);
	}
	return out;
}

TCHAR* utf8to16(const char* in) {
	TCHAR *out;
	if (!in || strlen(in) == 0) {
		out = (TCHAR*)calloc (1, sizeof (TCHAR));
	} else {
		DWORD size = MultiByteToWideChar(CP_UTF8, 0, in, -1, NULL, 0);
		out = (TCHAR*)calloc (size, sizeof (TCHAR));
		MultiByteToWideChar(CP_UTF8, 0, in, -1, out, size);
	}
	return out;
}

static void onError(sqlite3_context *ctx, const char* err) {
	char result[strlen(err) + 128];
	sprintf(result, "{\"error\": \"%s\"}", err);
	sqlite3_result_text(ctx, result, strlen(result), SQLITE_TRANSIENT);
}

static void showDbError(SQLHANDLE hStmt) {
	SQLWCHAR err16[MAX_ERROR_LENGTH + 1];	
	SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, NULL, NULL, err16, MAX_ERROR_LENGTH, NULL);
	MessageBox(0, err16, 0, 0);
}

static void odbc_read(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	int res = 0;
	SQLHANDLE hEnv;
	SQLHANDLE hConn;
	SQLHANDLE hStmt = 0;

	sqlite3* db = sqlite3_context_db_handle(ctx);

	srand(time(NULL));
	int sid = rand();

	int rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	res = rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;

	rc = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	res = res && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

	rc = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hConn);
	res = res && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

	if (res) {
		int len  = sqlite3_value_bytes(argv[0]);
		char connectionString8[len + 1];
		strcpy((char*)connectionString8, (char*)sqlite3_value_text(argv[0]));
		TCHAR* connectionString16 = utf8to16(connectionString8);	
		rc = SQLDriverConnect(hConn, NULL, connectionString16, _tcslen(connectionString16), 0, 0, NULL, SQL_DRIVER_NOPROMPT);
		res = rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;
		free(connectionString16);
		if (!res)
			onError(ctx, "DSN is invalid");

		rc = SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
		res = res && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

		len  = sqlite3_value_bytes(argv[1]);
		char query8[len + 1];
		strcpy((char*)query8, (char*)sqlite3_value_text(argv[1]));
		if (strcmp(query8, "TABLES") == 0) {
			rc = SQLTables(hStmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0);
		} else if (strcmp(query8, "TYPES") == 0) {
			rc = SQLGetTypeInfo(hStmt, SQL_ALL_TYPES);
		} else {
			TCHAR* query16 = utf8to16(query8);
			rc = SQLExecDirect(hStmt, query16, SQL_NTS);
			free(query16);
		}
		res = res && (rc != SQL_ERROR);

		if (rc == SQL_ERROR) {
			SQLWCHAR err16[MAX_ERROR_LENGTH + 1];	
			SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, NULL, NULL, err16, MAX_ERROR_LENGTH, NULL);

			char* err8 = utf16to8(err16);
			onError(ctx, err8);
			free(err8);
		}

		const char* target8 = (const char*)sqlite3_value_text(argv[2]);
		char* tablename8 = getName(target8, FALSE);
		TCHAR* tablename16 = utf8to16(tablename8);
		free(tablename8);
	 
		SQLSMALLINT colCount = 0;
		SQLNumResultCols(hStmt, &colCount);

		int colTypes[colCount];
		int maxLength = colCount * MAX_COLUMN_LENGTH + 512; 
		TCHAR create16[maxLength];
		TCHAR insert16[maxLength];
		
		_sntprintf(create16, maxLength, TEXT("create table if not exists temp.\"%ls%i\" (\""), tablename16, sid);
		_sntprintf(insert16, maxLength, TEXT("insert into temp.\"%ls%i\" (\""), tablename16, sid);

		for (int colNo = 1; colNo <= colCount; colNo++) {
			TCHAR colName[MAX_COLUMN_LENGTH + 1];
			SQLSMALLINT colType = 0;
			SQLDescribeCol(hStmt, colNo, colName, MAX_COLUMN_LENGTH, 0, &colType, 0, 0, 0);
			colTypes[colNo] = colType;
			_tcscat(create16, colName);

			_tcscat(create16, 
				colType == SQL_DECIMAL || colType == SQL_NUMERIC || colType == SQL_REAL || colType == SQL_FLOAT || colType == SQL_DOUBLE ? TEXT("\" real") : 
				colType == SQL_SMALLINT || colType == SQL_INTEGER || colType == SQL_BIT || colType == SQL_TINYINT || colType == SQL_BIGINT ? TEXT("\" integer") : 
				colType == SQL_CHAR || colType == SQL_VARCHAR || colType == SQL_LONGVARCHAR || colType == SQL_WCHAR || colType == SQL_WVARCHAR || colType == SQL_WLONGVARCHAR ? TEXT("\" text") : 
				colType == SQL_BINARY || colType == SQL_VARBINARY ? TEXT("\" blob") : 
				TEXT("\""));
	
			_tcscat(insert16, colName);

			if (colNo != colCount) {
				_tcscat(create16, TEXT(",\""));
				_tcscat(insert16, TEXT("\",\""));
			}
		}

		_tcscat(create16, TEXT(")"));
		_tcscat(insert16, TEXT("\") values ("));
		for (int colNo = 0; colNo < colCount - 1; colNo++)
			_tcscat(insert16, TEXT("?, "));
		_tcscat(insert16, TEXT("?)"));

		char* create8 = utf16to8(create16);
		res = SQLITE_OK == sqlite3_exec(db, create8, 0, 0, 0);
		free(create8);

		if (res) {
			char* insert8 = utf16to8(insert16);
			sqlite3_stmt* stmt;
			rc = SQLITE_OK == sqlite3_prepare_v2(db, insert8, -1, &stmt, 0);
			free(insert8);

			if (!rc) 
				onError(ctx, sqlite3_errmsg(db));

			int insertedRows = 0;
			int rejectedRows = 0;
			SQLLEN res = 0;	
			while (rc && (SQLFetch(hStmt) == SQL_SUCCESS)) {
				for (int colNo = 1; colNo <= colCount; colNo++) {
					int colType = colTypes[colNo];
					if (colType == SQL_DECIMAL || colType == SQL_NUMERIC || colType == SQL_REAL || colType == SQL_FLOAT || colType == SQL_DOUBLE) {
						double val = 0;
						SQLGetData(hStmt, colNo, SQL_C_DOUBLE, &val, sizeof(double), &res);
						if (res != SQL_NULL_DATA)
							sqlite3_bind_double(stmt, colNo, val);
						else
							sqlite3_bind_null(stmt, colNo);
					} else if (colType == SQL_SMALLINT || colType == SQL_INTEGER || colType == SQL_BIT || colType == SQL_TINYINT || colType == SQL_BIGINT) {
						int val = 0;
						SQLGetData(hStmt, colNo, SQL_C_SLONG, &val, sizeof(int), &res);
						if (res != SQL_NULL_DATA)
							sqlite3_bind_int(stmt, colNo, val);
						else
							sqlite3_bind_null(stmt, colNo);
					} else {
						SQLWCHAR val16[MAX_DATA_LENGTH + 1];
						SQLGetData(hStmt, colNo, SQL_WCHAR, val16, MAX_DATA_LENGTH * sizeof(TCHAR), &res); 
						char* val8 = utf16to8(val16);
						if (res != SQL_NULL_DATA)
							sqlite3_bind_text(stmt, colNo, val8, strlen(val8), SQLITE_TRANSIENT);
						else
							sqlite3_bind_null(stmt, colNo);
						free(val8);
					} 
				}

				if (SQLITE_DONE == sqlite3_step(stmt)) {
					insertedRows++;
				} else {
					rejectedRows++;
				}

				sqlite3_reset(stmt);
			}

			if (rc) {
				int len = 2 * strlen(target8) + 128;
				char create8[len];
				char insert8[len];
				char* schema8 = getName(target8, TRUE);
				char* tablename8 = getName(target8, FALSE);

				sprintf(create8, "create table \"%s\".\"%s\" as select * from temp.\"%s%i\"", schema8, tablename8, tablename8, sid);
				sprintf(insert8, "insert into \"%s\".\"%s\" select * from temp.\"%s%i\"", schema8, tablename8, tablename8, sid);

				free(schema8);
				free(tablename8);

				if (SQLITE_OK == sqlite3_exec(db, create8, 0, 0, 0) || SQLITE_OK == sqlite3_exec(db, insert8, 0, 0, 0)) {
					char result[512];
					sprintf(result, "{\"result\":\"ok\", \"read\": %i, \"inserted\": %i, \"rejected\": %i}", insertedRows + rejectedRows, insertedRows, rejectedRows);
					sqlite3_result_text(ctx, result, strlen(result), SQLITE_TRANSIENT);
				} else {
					onError(ctx, sqlite3_errmsg(db));
				}				
			}
			sqlite3_finalize(stmt);
		}
	} else {
		onError(ctx, "Couldn't get access to ODBC driver");
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	SQLDisconnect(hConn);
	SQLFreeHandle(SQL_HANDLE_DBC, hConn);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

static void odbc_write(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	sqlite3* db = sqlite3_context_db_handle(ctx);
	sqlite3_stmt* stmt;
	if (SQLITE_OK != sqlite3_prepare_v2(db, (const char*)sqlite3_value_text(argv[0]), -1, &stmt, 0)) {
		onError(ctx, sqlite3_errmsg(db));
		return;
	}

	int res = 0;
	SQLHANDLE hEnv;
	SQLHANDLE hConn;
	SQLHANDLE hStmt = 0;

	int rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	res = rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;

	rc = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	res = res && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

	rc = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hConn);
	res = res && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

	if (res) {
		int len  = sqlite3_value_bytes(argv[1]);
		char connectionString8[len + 1];
		strcpy((char*)connectionString8, (char*)sqlite3_value_text(argv[1]));
		TCHAR* connectionString16 = utf8to16(connectionString8);	
		rc = SQLDriverConnect(hConn, NULL, connectionString16, _tcslen(connectionString16), 0, 0, NULL, SQL_DRIVER_NOPROMPT);
		res = rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;
		free(connectionString16);
		if (!res)
			onError(ctx, "DSN is invalid");
		
		TCHAR driverName16[255];	
		SQLGetInfo(hConn, SQL_DBMS_NAME, driverName16, 255, NULL);
		SQLSetConnectAttr(hConn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF,  SQL_IS_UINTEGER);

		char* target8 = getName((const char*)sqlite3_value_text(argv[2]), FALSE);
		TCHAR* target16 = utf8to16(target8);
		int colCount = sqlite3_column_count(stmt);
		SQLSMALLINT colTypes[colCount + 1];
		BOOL isTargetMissing = FALSE; 
		
		SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
		int maxLength = _tcslen(target16) + 255;
		TCHAR check16[maxLength];
		_sntprintf(check16, maxLength, TEXT("select * from \"%ls\" where 1 = 2"), target16);
		rc = SQLExecDirect(hStmt, (SQLWCHAR*)check16, SQL_NTS);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

		if (rc == SQL_ERROR) {
			int maxLength = colCount * MAX_COLUMN_LENGTH + 512;
			TCHAR create16[maxLength];
			_sntprintf(create16, maxLength, TEXT("create table \"%ls\" ("), target16);
					
			for (int colNo = 0; colNo < colCount; colNo++) {
				TCHAR* colName16 = utf8to16((const char*)sqlite3_column_name(stmt, colNo));			
				TCHAR* colType16 = utf8to16((const char*)sqlite3_column_decltype(stmt, colNo));
				int maxLength = MAX_COLUMN_LENGTH + 255;
				TCHAR field16[maxLength];
				
				if (colType16) 
					_tcslwr(colType16);
					
				BOOL isNumeric = colType16 != NULL && (
					_tcscmp(colType16, TEXT("integer")) == 0 || 
					_tcscmp(colType16, TEXT("real")) == 0 || 
					_tcscmp(colType16, TEXT("int")) == 0 || 
					_tcscmp(colType16, TEXT("numeric")) == 0
				);	
								
				_sntprintf(field16, maxLength, TEXT("\"%ls\" %ls%ls"), 
					colName16, 
					isNumeric ? TEXT("numeric") : TEXT("text"),
					colNo != colCount - 1 ? TEXT(", ") : TEXT(""));
				_tcscat(create16, field16);	

				free(colName16);
				free(colType16);				
			}
			_tcscat(create16, TEXT(")"));
			
			SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
			isTargetMissing = SQL_ERROR == SQLExecDirect(hStmt, create16, SQL_NTS);
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		}		

		SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
		maxLength = _tcslen(target16) + 2;
		TCHAR _target16[maxLength];
		_sntprintf(_target16, maxLength, TEXT("%ls%ls"), target16, _tcscmp(driverName16, TEXT("EXCEL")) == 0 ? TEXT("$") : TEXT(""));
		rc = SQLColumns(hStmt, NULL, 0, NULL, 0, (SQLWCHAR*)_target16, SQL_NTS, NULL, 0);
		
		maxLength = colCount * MAX_COLUMN_LENGTH + 512;
		TCHAR insert16[maxLength];
		_sntprintf(insert16, maxLength, TEXT("insert into \"%ls\" (\""), _target16);	
		
		int colCount2 = 0;
		if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {			
			SQLSMALLINT dataType;
			SQLWCHAR colName[MAX_COLUMN_LENGTH];  

			SQLLEN cbDataType;
			SQLLEN cbColName;
			
			SQLBindCol(hStmt, 4, SQL_C_WCHAR, colName, MAX_COLUMN_LENGTH, &cbColName); 			
			SQLBindCol(hStmt, 5, SQL_C_SSHORT, &dataType, 0, &cbDataType); 
			
			while (colCount2 < colCount) {
				int rc = SQLFetch(hStmt);
				if (SQL_SUCCESS != rc && SQL_SUCCESS_WITH_INFO != rc) 
					break;
					
				colCount2++;
				colTypes[colCount2] = dataType;
				
				_tcscat(insert16, (TCHAR*)colName);	

				if (colCount2 < colCount) 
					_tcscat(insert16, TEXT("\",\""));
			}
		
			_tcscat(insert16, TEXT("\") values ("));
			for (int colNo = 0; colNo < colCount2 - 1; colNo++)
				_tcscat(insert16, TEXT("?, "));
			_tcscat(insert16, TEXT("?)")); // last placeholder here
		}		
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		free(target16);
		
		if (colCount == colCount2) {
			SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
			if(SQLPrepare (hStmt, insert16, SQL_NTS) != SQL_ERROR) {
				int insertedRows = 0;
				int rejectedRows = 0;
				int rowNo = 0;	
					
				while(SQLITE_ROW == sqlite3_step(stmt)) {
					SQLPOINTER data[colCount + 1];
					
					rowNo++;
					for (int colNo = 1; colNo <= colCount; colNo++) {											
						BOOL isNULL = sqlite3_column_type(stmt, colNo - 1) == SQLITE_NULL;
						SQLLEN cbData = SQL_NTS;
						SQLLEN cbNull = SQL_NULL_DATA;

						int type = colTypes[colNo];
						BOOL isNumeric = type == SQL_DECIMAL || type == SQL_NUMERIC || type == SQL_FLOAT || type == SQL_REAL || type == SQL_DOUBLE;								
						isNumeric = isNumeric && !isNULL;
							
						if (isNumeric) {
							double* d = (double*)calloc(1, sizeof(double));
							*d = sqlite3_column_double(stmt, colNo - 1);
							data[colNo] = (SQLPOINTER)d;
						} else {							
							const char* val8 = (const char*)sqlite3_column_text(stmt, colNo - 1);
							data[colNo] = (SQLPOINTER)utf8to16(val8);
						}
						
						int rc = isNULL ? SQLBindParameter(hStmt, colNo, SQL_PARAM_INPUT, SQL_C_NUMERIC, SQL_NUMERIC, 0, 0, NULL, 0, &cbNull) :
							isNumeric ? SQLBindParameter(hStmt, colNo, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, data[colNo], 0, 0) :
							SQLBindParameter(hStmt, colNo, SQL_PARAM_INPUT, SQL_C_WCHAR, type, _tcslen((TCHAR*)data[colNo]) + 1, 0, data[colNo], 0, &cbData);
					}
					
					if (SQL_ERROR != SQLExecute(hStmt))
						insertedRows++;
					else 
						rejectedRows++;
						
					SQLFreeStmt(hStmt, SQL_RESET_PARAMS);
						
					for (int colNo = 1; colNo < colCount; colNo++)	
						free(data[colNo]);
				}
				SQLEndTran(SQL_HANDLE_DBC, hConn, SQL_COMMIT);

				char result[512];
				sprintf(result, "{\"result\":\"ok\", \"read\": %i, \"inserted\": %i, \"rejected\": %i}", insertedRows + rejectedRows ,insertedRows, rejectedRows);
				sqlite3_result_text(ctx, result, strlen(result), SQLITE_TRANSIENT);
			} else {
				onError(ctx, "Invalid insert statement");
			}
		} else {
			char err8[255];
			if (isTargetMissing)
				sprintf(err8, "Can't create target table");	
			else if (colCount2 != colCount)
				sprintf(err8, "Can't fetch target columns");
			else	
				sprintf(err8, "Column count mismatches: query returns %i, target has %i", sqlite3_column_count(stmt), colCount);
		
			onError(ctx, err8);
		}		
	} else {
		onError(ctx, "Couldn't get access to ODBC driver");
	}

	sqlite3_finalize(stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	SQLDisconnect(hConn);
	SQLFreeHandle(SQL_HANDLE_DBC, hConn);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

static void odbc_query(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	int res = 0;
	SQLHANDLE hEnv;
	SQLHANDLE hConn;
	SQLHANDLE hStmt = 0;

	sqlite3* db = sqlite3_context_db_handle(ctx);

	srand(time(NULL));
	int sid = rand();

	int rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	res = rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;

	rc = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	res = res && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

	rc = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hConn);
	res = res && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

	if (res) {
		int len  = sqlite3_value_bytes(argv[0]);
		char connectionString8[len + 1];
		strcpy((char*)connectionString8, (char*)sqlite3_value_text(argv[0]));
		TCHAR* connectionString16 = utf8to16(connectionString8);	
		rc = SQLDriverConnect(hConn, NULL, connectionString16, _tcslen(connectionString16), 0, 0, NULL, SQL_DRIVER_NOPROMPT);
		res = rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;
		free(connectionString16);
		
		if (res) {
			rc = SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
			res = res && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
	
			len  = sqlite3_value_bytes(argv[1]);
			char query8[len + 1];
			strcpy((char*)query8, (char*)sqlite3_value_text(argv[1]));
			TCHAR* query16 = utf8to16(query8);
			rc = SQLExecDirect(hStmt, query16, SQL_NTS);
			res = res && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
			free(query16);
			if (rc != SQL_ERROR) {
				sqlite3_result_text(ctx, "{\"result\": \"ok\"}", -1, SQLITE_TRANSIENT);		
			} else {
				SQLWCHAR err16[MAX_ERROR_LENGTH + 1];	
				SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, NULL, NULL, err16, MAX_ERROR_LENGTH, NULL);
	
				char* err8 = utf16to8(err16);
				onError(ctx, err8);
				free(err8);	
			}
		} else {
			onError(ctx, "DSN is invalid");
		}
	} else {
		onError(ctx, "Couldn't get access to ODBC driver");
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);		
	SQLDisconnect(hConn);
	SQLFreeHandle(SQL_HANDLE_DBC, hConn);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

static void odbc_dsn(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	SQLHANDLE hEnv;
	int rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	rc = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		int cnt = 0;
		rc = SQLDataSources(hEnv, SQL_FETCH_FIRST, NULL, 0, 0, NULL, 0, 0);	
		while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
			cnt++;
			rc = SQLDataSources(hEnv, SQL_FETCH_NEXT, NULL, 0, 0, NULL, 0, 0);
		}

		if (cnt > 0) {
			TCHAR dsn16[SQL_MAX_DSN_LENGTH + 1];
			short len = 0;

			char query8[MAX_DATA_LENGTH];
			sprintf(query8, "select json_object('result', json_array(");
			for (int i = 0; i < cnt; i++)
				strcat(query8, i > 0 ? ", ?" : "?");
			strcat(query8, "))");

			sqlite3* db = sqlite3_context_db_handle(ctx);
			sqlite3_stmt* stmt;

			if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
				int no = 1;	
				rc = SQLDataSources(hEnv, SQL_FETCH_FIRST, dsn16, SQL_MAX_DSN_LENGTH + 1, &len, NULL, 0, 0);	
				while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
					char* dsn8 = utf16to8(dsn16);
					sqlite3_bind_text(stmt, no, dsn8, strlen(dsn8), SQLITE_TRANSIENT);
					free(dsn8);
					no++;
	
					rc = SQLDataSources(hEnv, SQL_FETCH_NEXT, dsn16, SQL_MAX_DSN_LENGTH, &len, NULL, 0, 0);
				}

				if (SQLITE_ROW == sqlite3_step(stmt)) {
					sqlite3_result_text(ctx, (const char*)sqlite3_column_text(stmt, 0), -1, SQLITE_TRANSIENT);
				} else {
					onError(ctx, sqlite3_errmsg(db));
				}
			}

			sqlite3_finalize(stmt);
		} else {
			sqlite3_result_text(ctx, "{\"result\": []}", -1, SQLITE_TRANSIENT);
		} 
	} else {
		onError(ctx, "Couldn't get access to ODBC manager");
	}

	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

__declspec(dllexport) int sqlite3_odbc_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
	SQLITE_EXTENSION_INIT2(pApi);
	(void)pzErrMsg;  /* Unused parameter */

	return (SQLITE_OK == sqlite3_create_function(db, "odbc_read", 3, SQLITE_UTF8, 0, odbc_read, 0, 0)) &&
		(SQLITE_OK == sqlite3_create_function(db, "odbc_write", 3, SQLITE_UTF8, 0, odbc_write, 0, 0)) &&
		(SQLITE_OK == sqlite3_create_function(db, "odbc_query", 2, SQLITE_UTF8, 0, odbc_query, 0, 0)) &&
		(SQLITE_OK == sqlite3_create_function(db, "odbc_dsn", 0, SQLITE_UTF8, 0, odbc_dsn, 0, 0)) ? SQLITE_OK : SQLITE_ERROR;
}