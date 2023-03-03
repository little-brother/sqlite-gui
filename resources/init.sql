create table if not exists prefs (name text not null, value text, primary key (name));
create table if not exists recents (path text not null, time real not null, primary key (path));
create table if not exists history (query text not null, time real not null, primary key (query));
create table if not exists gists (query text not null, time real not null, primary key (query));
create table if not exists generators (type text, value text);
create table if not exists refs (dbname text not null, schema text not null, tblname text not null, colname text not null, refname text, query text, primary key (dbname, schema, tblname, colname)); 
create table if not exists disabled (dbpath text not null, type text not null, name text not null, sql text, primary key (dbpath, type, name)); 
create table if not exists pinned (dbname text not null, name text not null, primary key (dbname, name));
create table if not exists cli ("time" real, dbname text not null, query text not null, elapsed integer, result text); 
create table if not exists diagrams (dbname text, tblname text, x integer, y integer, width integer, height integer, primary key (dbname, tblname));
create table if not exists main.encryption (dbpath text, param text, idc integer, value text, no integer, primary key (dbpath, param));
create table if not exists temp.encryption (dbpath text, param text, idc integer, value text, no integer, primary key (dbpath, param));
create table if not exists query_params (dbname text, name text, value text, primary key (dbname, name, value));
create table if not exists search_history ("time" real, what text, type integer, primary key (what, type));
create index if not exists idx_cli on cli ("time" desc, dbname);
create table if not exists help (word text primary key, type text, brief text, description text, example text, alt text, args json, nargs integer);
create table if not exists functions (id integer primary key autoincrement, name text, type integer default 0, language text default 'sql', code text, code2 text, code3 text, description text);
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