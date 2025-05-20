create table if not exists prefs (name text primary key, value text);
create table if not exists recents (path text primary key, time real not null);
create table if not exists attached_recents (path text primary key, time real not null);
create table if not exists odbc_recents (alias text primary key, driver text, options text, time real not null);
create table if not exists addons (name text, type integer, enable integer, version text, primary key (name, type));
create table if not exists history (query text primary key, time real not null);
create table if not exists gists (query text primary key, time real not null);
create table if not exists generators (type text, value text);
create table if not exists refs (dbname text not null, schema text not null, tblname text not null, colname text not null, refname text, query text, primary key (dbname, schema, tblname, colname)); 
create table if not exists disabled (dbpath text not null, type text not null, name text not null, sql text, primary key (dbpath, type, name)); 
create table if not exists pinned (dbname text not null, name text not null, primary key (dbname, name));
create table if not exists pinned_results (id integer primary key autoincrement, tab_no integer, caption text, query text, elapsed_time text);
create table if not exists cli ("time" real, dbname text not null, query text not null, elapsed integer, result text); 
create table if not exists diagrams (dbname text, tblname text, x integer, y integer, width integer, height integer, primary key (dbname, tblname));
create table if not exists main.encryption (dbpath text, param text, idc integer, value text, no integer, primary key (dbpath, param));
create table if not exists temp.encryption (dbpath text, param text, idc integer, value text, no integer, primary key (dbpath, param));
create table if not exists query_params (dbname text, name text, value text, primary key (dbname, name, value));
create table if not exists search_history ("time" real, what text, type integer, primary key (what, type));
create index if not exists idx_cli on cli ("time" desc, dbname);
create table if not exists help (keyword text, type text, signature text, description text, example text, alias text, args json, nargs integer, source text, primary key (keyword, type));
create table if not exists functions (id integer primary key autoincrement, name text, type integer default 0, language text default 'sql', code text, code2 text, code3 text, description text, help text);
create table if not exists sheets (sheet_id text primary key, "time" real);
create table if not exists shortcuts as 
select cast('Alt + F1' as text) name, cast(0x70 as integer) key, cast(0 as integer) ctrl, cast(1 as integer) alt, 
'-- Columns
pragma table_info(''$SUB$'');

-- Foreign keys
pragma foreign_key_list(''$SUB$'');

-- DDL
select * from sqlite_master where tbl_name = ''$SUB$'';

-- First 10 rows
select * from ("$SUB$") limit 10;

-- Memory usage
select tosize(SUM(payload)) payload, tosize(SUM(pgsize)) total from dbstat where name = ''$SUB$'';' query 
union select 'Ctrl + 1', 0x31, 1, 0, 
'-- $SUB$
select * from "$SUB$" limit 100' 
union select 'Ctrl + 2', 0x32, 1, 0, 
'-- $SUB$
select count(*) from "$SUB$"' 
union select 'Ctrl + 3', 0x33, 1, 0, NULL 
union select 'Ctrl + 4', 0x34, 1, 0, NULL 
union select 'Ctrl + 5', 0x35, 1, 0, NULL 
union select 'Ctrl + 6', 0x36, 1, 0, NULL 
union select 'Ctrl + 7', 0x37, 1, 0, NULL 
union select 'Ctrl + 8', 0x38, 1, 0, NULL 
union select 'Ctrl + 9', 0x39, 1, 0, NULL 
union select 'Ctrl + 0', 0x30, 1, 0, NULL;

create table if not exists ai_messages (id integer primary key autoincrement, sid integer, role text, content text, details text, is_aux integer default 0);
create index if not exists ai_messages_details_prompt on ai_messages(sid, json_extract(details, '$.prompt-id'));
create index if not exists ai_messages_details_database on ai_messages(sid, json_extract(details, '$.database'));
create table if not exists ai_config (model text not null default 'default', param text not null, value text, primary key (model, param));
insert or replace into ai_config (model, param, value) values 
('default', 'request-options', '{}'),
('default', 'request-headers', 'Content-Type: application/json' || char(10) || 'Accept: application/json' || char(10)),

('default', 'default-init-prompt', 'You are an AI SQL assistant in SQLite editor. Your primary job is to build SQL queries for a current SQLite database. The database has the next tables: {{TABLES}} and additional functions: {{FUNCTIONS}}. If the request is unclear, missing key details, or requires dynamic values (like specific IDs or another column values to filters), fetch the necessary data first by responding with "GET:your-sql-query" to retrieve the missing information e.g. "GET:select * from mytable" to obtain data from mytable. Don''t repeat a GET-answer if you already asked it before. Always analyze the database schema (tables and columns) before generating any SQL query and verify it after generating.'),

('default', 'default-no-access-init-prompt', 'You are an AI SQL assistant in SQLite editor. Your primary job is to build SQL queries for a current SQLite database. The database has the next tables: {{TABLES}} and additional functions: {{FUNCTIONS}}.'),

('default', 'chat-init-prompt', 'You are an AI assistant for an SQLite database. Follow these rules exactly. 1. If the request is unclear, missing key details, or requires dynamic values (like specific IDs or another column values for filters) then request the necessary data first by responding with "GET:your-sql-query" (use strictly this format) to retrieve the missing information e.g. "GET:select * from mytable" to obtain data from mytable. Don''t repeat a GET-answer if you already asked it before. Use GET:-request to obtain any necessary data e.g. table''s metadata from the database. 2. Always analyze the database schema (tables and columns) before generating any SQL query and verify it after generating. 3. Your primary strategy for answers is to send a text or an query resultset. Send a SQL query (without any explanation) ONLY if the user explicitly asks it in the current request (don''t expand this strategy for another requests). Always prioritize giving the result of the query or a direct textual answer unless strictly instructed otherwise. If any part of the process requires preparatory queries, present only the final result or message to the user. 4. If the answer is a single SQL query, you have to modify it to the next format "QUERY:sql-query"; don''t send a single SQL query without "QUERY:"-header; don''t add "QUERY:"-header if the entire answer is not a single query. 5. Never join explanation and "GET:query" answers. 6. Never use markdown for answers.'),

('default', 'chat-no-access-init-prompt', 'You are an AI assistant for an SQLite database with the next tables {{TABLES}} and functions {{FUNCTIONS}}. Follow these rules exactly. 1. Your primary strategy for answers is to send a text or an query resultset. Send a SQL query (without any explanation) ONLY if the user explicitly asks it in the current request (don''t expand this strategy for another requests). Always prioritize giving the result of the query or a direct textual answer unless strictly instructed otherwise. If any part of the process requires preparatory queries, present only the final result or message to the user. 2. If the answer is a single SQL query, you have to modify it to the next format "QUERY:sql-query"; don''t send a single SQL query without "QUERY:"-header; don''t add "QUERY:"-header if the entire answer is not a single query. 3. Never use markdown for answers. 4. You don''t have a direct access to the database but you can ask an user to send to you missing data.'),

('default', 'chat-prompt', '{{QUERY}}'),

('default', 'execute-prompt', 'Process the next request "{{QUERY}}". Before building a filter over column values analyze column values. If the column doesn''t have  matched values then you have to make assumption that the request for a category. For example, if a column contains different animals and the request has mammal then you have to select all mammals from this column instead of using "LIKE ''%mammal%''"-filter. Use own internal knowledge to categorize values. The answer should be strictly in this format "QUERY:final-sql-query" or "GET:clarifying-query". Don''t use markdown. Don''t provide ANY explanations.'),
('default', 'explain-sql-prompt', 'Explain the next SQLite query "{{QUERY}}". The result should contain only an explanation.'),
('default', 'explain-nosql-prompt', 'Explain the next "{{QUERY}}". The using programming laguage is SQL. Don''t provide examples for other languages. The result should contain only an explanation.'),
('default', 'result-confirm-prompt', 'The query "{{QUERY}}" returns "{{ERROR}}". If the result has zero rows because of LIKE-filters, try to modify them as it is a category. If that result is a correct answer for "{{PROMPT}}"-request then return "CONFIRMED". Otherwise, try to generate another query.'),
('default', 'on-error-prompt', 'Your request "{{QUERY}}" failed with "{{ERROR}}"-error. Make another request to avoid this error.'),
('default', 'explain-sql-error-prompt', 'Explain why the SQLite query {{QUERY}} returns {{ERROR}}-error. The result should contain only a short explanation. '),

('default', 'format-prompt', 'Format the next SQLite query "{{QUERY}}". The result should contain only a formatted query. Use lower case for columns and keywords. Break long lines. Don''t do changes if the request is a not SQL-query. Don''t use markdown.'),
('default', 'fix-error-prompt', 'The query "{{QUERY}}" returns "{{ERROR}}"-error. Fix the query. The result should contain only corrected SQL-query. Don''t use markdown. The corrected query should be formatted as the original query.'),
('default', 'fix-error-next-prompt', 'The fixed query "{{QUERY}}" fails with error "{{ERROR}}". Suggest the next fix. The result should contain only corrected SQL-query. Don''t use markdown. The corrected query should be formatted as the original query.'),

('default', 'on-change-db-prompt', 'The database was changed and the previous table list is not actual anymore. Be carefull, now the database has only the next tables: {{TABLES}}. ')

on conflict (model, param) do nothing;