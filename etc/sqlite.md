## How to build sqlite
  
  * sqlite3.dll + sqlite3.def
    ```
    gcc -shared -Wl,--output-def=sqlite3.def sqlite3.c -o sqlite3.dll -DSQLITE_ENABLE_DBSTAT_VTAB -DSQLITE_ENABLE_COLUMN_METADATA  -s -O
    ```

 * libsqlite3.a
    ```
    dlltool -d sqlite3.def -l libsqlite3.a -D sqlite3.dll
    ```

 * Extension e.g. iif
    ```
    gcc -I ../include -g -shared json1.c -o json1.dll -s
    ```