Sqlite-gui is an another one GUI for [SQLite](https://www.sqlite.org/index.html) powered by C++, WinAPI and [Code::Blocks](http://www.codeblocks.org/).  
Recent binaries are available at [Releases page](https://github.com/little-brother/sqlite-gui/releases).


### How to build
* sqlite3.dll + sqlite3.def
  ```
  gcc -shared -Wl,--output-def=sqlite3.def sqlite3.c -o sqlite3.dll
  ```

* libsqlite3.a
  ```
  dlltool -d sqlite3.def -l libsqlite3.a -D sqlite3.dll
  ```

* Extension e.g. iif
  ```
  gcc -I ../include -g -shared iif.c -o iif.dll
  ```