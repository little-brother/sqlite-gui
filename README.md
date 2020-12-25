Sqlite-gui is a lightweight GUI for [SQLite](https://www.sqlite.org/index.html) powered by C++, WinAPI and [Code::Blocks 17](http://www.codeblocks.org/).  
Recent binaries are available at [Releases page](https://github.com/little-brother/sqlite-gui/releases).


![View](resources/image.webp)


### Features
* Syntax highlighting and code completion
* Store an execution history and user scripts
* Export/Import data
* Database diagram
* Database comparison
* Search text in the whole database
* [Workflow manager](https://github.com/little-brother/sqlite-wf) (ETL)
* Quick data references ([video](https://youtu.be/XL1_lFhzLKo))
* Terminal mode
* Data generator
* Most usefull extensions are included
* Demo database "Bookstore" for beginners
* Does not require installation

### Cons
* Only utf-8 is supported
* NULL is displayed as an empty string and an empty string is set to NULL when data is edit


<details>
 <summary>How to build</summary>
  
  * sqlite3.dll + sqlite3.def
    ```
    gcc -shared -Wl,--output-def=sqlite3.def sqlite3.c -o sqlite3.dll -DSQLITE_ENABLE_DBSTAT_VTAB -DSQLITE_ENABLE_COLUMN_METADATA  -s -O
    ```

 * libsqlite3.a
    ```
    dlltool -d sqlite3.def -l libsqlite3.a -D sqlite3.dll
    ```

 * Extension e.g. `json1`
    ```
    gcc -I ../include -g -shared json1.c -o json1.dll -s
    ```
    `series.c` and `regexp.c` should be compiled with additional flag `-Os` to minimize [VirusTotal alerts](https://github.com/little-brother/sqlite-gui/issues/3)	
 
 * Use Code::Blocks 17 to build the app.   
</details>


If you liked the project, press the like-button [here](https://alternativeto.net/software/sqlite-gui/) to support it.<br>
If you have any problems, comments or suggestions, just let me know <a href="mailto:lb.im@yandex.ru?subject=sqlite-gui">lb.im@yandex.ru</a>.
