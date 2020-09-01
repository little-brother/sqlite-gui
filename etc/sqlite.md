## How to build sqlite
  
  * sqlite3.dll + sqlite3.def
    ```
    gcc -shared -Wl,--output-def=sqlite3.def sqlite3.c -o sqlite3.dll -D SQLITE_ENABLE_DBSTAT_VTAB SQLITE_ENABLE_FTS5 -s
    ```

 * libsqlite3.a
    ```
    dlltool -d sqlite3.def -l libsqlite3.a -D sqlite3.dll
    ```

 * Extension e.g. iif
    ```
    gcc -I ../include -g -shared json1.c -o json1.dll -s
    ```