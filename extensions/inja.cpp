/*
	inja(template)
	Parse mustache template to plain sql.
	select inja('{% for i in range (2) %}select {{ i }}; {% endfor %}'); --> select 1; select 2;
	select inja('select {{ argv.1 }}; select {{ argv.2 }}', 'a', 'b'); --> select a; select b;
	select inja('{% for row in query("select * from books") %} {{ row.author }} {% endfor %}'); --> Brandon Sanderson  Cristin Terrill ...
*/
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <stdio.h>
#include "inja.hpp"

static void _inja(sqlite3_context *ctx, int argc, sqlite3_value **argv){		
	if (sqlite3_value_type(argv[0]) == SQLITE_NULL) 
		return sqlite3_result_null(ctx);
	
	sqlite3* db = sqlite3_context_db_handle(ctx);
	
	inja::Environment env;
	env.add_callback("query", 1, [db](inja::Arguments& args) {
		inja::json res = inja::json::array();
		
		sqlite3_stmt* stmt;
		bool rc = SQLITE_OK == sqlite3_prepare_v2(db, (args.at(0)->get<std::string>()).c_str(), -1, &stmt, 0);
		if (rc) {
			int colCount = sqlite3_column_count(stmt);
			while (SQLITE_ROW == sqlite3_step(stmt)) {
				inja::json row;
				for (int colNo = 0; colNo < colCount; colNo++) 
					row[(const char*)sqlite3_column_name(stmt, colNo)] = (const char*)sqlite3_column_text(stmt, colNo);
	
				res.push_back(row);			
			}
		}
		sqlite3_finalize(stmt);
		
		if (!rc)
			throw std::logic_error((const char*)sqlite3_errmsg(db));

		return res;
	});	
	
	env.add_callback("query_row", 1, [db](inja::Arguments& args) {
		inja::json res;
		
		sqlite3_stmt* stmt;
		bool rc = SQLITE_OK == sqlite3_prepare_v2(db, (args.at(0)->get<std::string>()).c_str(), -1, &stmt, 0);
		if (rc) {
			int colCount = sqlite3_column_count(stmt);
			if (SQLITE_ROW == sqlite3_step(stmt)) {
				for (int colNo = 0; colNo < colCount; colNo++) 
					res[(const char*)sqlite3_column_name(stmt, colNo)] = (const char*)sqlite3_column_text(stmt, colNo);	
			}
		}
		sqlite3_finalize(stmt);
		
		if (!rc)
			throw std::logic_error((const char*)sqlite3_errmsg(db));

		return res;
	});	
	
	env.add_callback("query_value", 1, [db](inja::Arguments& args) {
		std::string res;
		
		sqlite3_stmt* stmt;
		bool rc = SQLITE_OK == sqlite3_prepare_v2(db, (args.at(0)->get<std::string>()).c_str(), -1, &stmt, 0);
		if (rc) {
			if (SQLITE_ROW == sqlite3_step(stmt)) 
				res = (const char*)sqlite3_column_text(stmt, 0);	
		}
		sqlite3_finalize(stmt);
		
		if (!rc)
			throw std::logic_error((const char*)sqlite3_errmsg(db));

		return res;
	});			

	env.add_callback("call", 2, [env](inja::Arguments& args) mutable {
		inja::json data = args.at(1)->get<inja::json>();
		return env.render(args.at(0)->get<std::string>(), data);
	});
		
	const char* tmpl = (const char*)sqlite3_value_text(argv[0]);	
	inja::json data;
	for (int i = 0; i < argc; i++)
		data["argv"][i] = (const char*)sqlite3_value_text(argv[i]);
		
	try {
		sqlite3_result_text(ctx, env.render(tmpl, data).c_str(), -1, SQLITE_TRANSIENT);
	} catch(const std::exception& err){
		sqlite3_result_error(ctx, err.what(), -1);
	}
}

extern "C"
{
#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_inja_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
	SQLITE_EXTENSION_INIT2(pApi);
	(void)pzErrMsg;  /* Unused parameter */
	return sqlite3_create_function(db, "inja", -1, SQLITE_UTF8, 0, _inja, 0, 0);
}
}