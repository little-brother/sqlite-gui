insert or replace into help (word, type, brief, description, example, alt, args, nargs) values 

-- CORE 
('abs', 'function', 'abs (n)', 'Returns the absolute value of the numeric argument N.', 'select abs(-5); --> 5', '', null, 1),
('changes', 'function', 'changes ()', 'Returns the number of database rows that were changed or inserted or deleted 
by the most recently completed INSERT, DELETE, or UPDATE statement, exclusive of statements in lower-level triggers.', 'select changes();', '', null, 0),
('char', 'function', 'char (n1 [, n2, ...])', 'Returns a string composed of characters having the unicode code point values of integers.', 'select char(83, 81, 76, 105, 116, 101); --> SQLite', '', 'N1|N2|N3|N4|N5', -1),
('coalesce', 'function', 'coalesce (any , any [, any, ...])', 'Returns a copy of its first non-NULL argument, or NULL if all arguments are NULL.', ' select coalesce(null, 3 , ''a''); --> 3', '', null, -1),
('glob', 'function', 'glob (pattern, str)', 'Determines whether a string matches a specific pattern.
Similar to the LIKE operator but it''s case sensitive and uses the UNIX wildcards.
Also it''s equivalent to the expression "str GLOB pattern", but the arguments are reversed in the glob() function relative to the infix GLOB operator. 
The following expressions are equivalent: <name GLOB ''*helium*''> and <glob(''*helium*'',name)>', 'select * from books where glob(''*we*'', title);', '', 'Pattern|Str', 2),
('hex', 'function', 'hex (n)', 'Interprets n as a BLOB and returns a string which is the upper-case hexadecimal rendering of the content of that blob', 'select hex(''abc''); --> 616263', '', null, 1),
('ifnull', 'function', 'ifnull (any, any2)', 'Returns the first non-NULL argument, or NULL if both arguments are NULL.', 'select ifnull(null, 5); --> 5', '', null, 2),
('iif', 'function', 'iif (expr, any1, any2)', 'Returns the value Any1 if Expr is true, and Any2 otherwise', 'select iif(1 = 2, ''Yes'', ''No''); --> No', '', null, 3),
('instr', 'function', 'instr (str, substring)', 'Searches a substring in a string and returns an integer that indicates the position of the substring, which is the first character of the Substring.
If the Substring does not appear in the string, the INSTR function returns 0.  
In case either Str or Substring is NULL, the INSTR function returns a NULL value.', 
'select instr(''SQLite Tutorial'', ''Tutorial'') position; --> 8', '', 'Str|Substring', 2),
('last_insert_rowid', 'function', 'last_insert_rowid ()', 'Returns the ROWID of the last row insert from the database connection which invoked the function.', 'select last_insert_rowid();', '', null, 0),
('length', 'function', 'length (str)', 'Returns the number of characters in a string or the number of bytes in a BLOB', 'select length(''abc'') --> 3', '', null, 1),
('like', 'function', 'like (pattern, str [, esc])', 'Is used to implement the "str LIKE pattern [ESCAPE esc]" expression. 
If the optional ESCAPE clause is present, then the like() function is invoked with three arguments. 
Otherwise, it is invoked with two arguments only. 
Note that the Pattern and Str parameters are reversed in the like() function relative to the infix LIKE operator. 
The following expressions are equivalent: <name LIKE ''%neon%''> and <like(''%neon%'', name)>', 'select * from books where like(''%we%'', title);', '', 'Pattern|Str|Escape-clause', 3),
('likelihood', 'function', 'likelihood (x, ratio)', 'Returns argument X unchanged. The Ratio must be between 0.0 and 1.0, inclusive. 
The purpose of the likelihood(x, ratio) function is to provide a hint to the query planner that the argument X is a boolean 
that is true with a probability of approximately ratio. 
The unlikely(x) function is short-hand for likelihood(X,0.0625). 
The likely(x) function is short-hand for likelihood(x, 0.9375).', 'select likelihood(10, 0.9375); --> 10', '', 'X|Ratio', 2),
('likely', 'function', 'likely(x)', 'Returns the argument X unchanged. It is equivalent to likelihood(x, 0.9375).', 'select likely(x)', '', null, 1),
('load_extension', 'function', 'load_extension (path [, entrypoint])', 'Loads SQLite extensions out of the shared library. 
The result of load_extension() is always a NULL.', 'select load_extension(''./extensions/fileio.dll'')', '', 'Path|EntryPoint', 2),
('lower', 'function', 'lower (str)', 'Returns a copy of a string with all letter characters converted to lowercase. 
Doesn''t work for non-ANSI symbols without icu extension.', 'select lower(''SQLite''); --> sqlite', '', null, 1),
('ltrim', 'function', 'ltrim (str [, characters = '' ''])', 'Removes specified Characters at the beginning of a string Str.', 
'select ltrim(''   SQLite   ''); --> SQLite___
select ltrim(''@@SQLite@@'', ''@''); --> SQLite@@
select ltrim(''@12SQLite@@'', ''1@2''); --> SQLite@@', '', 'Str|Characters', 2),
('nullif', 'function', 'nullif (any, any2)', 'Returns its first argument if the arguments are different and NULL if the arguments are the same.', 'select nullif(10, 12); --> 10', '', null, 2),
('printf', 'function', 'printf (format, ...)', 'Works like the printf() function from the standard C library.
Format is synonim.', 'select printf(''Hello %s!''); --> Hello world!', 'format', 'Format', -1), 
('quote', 'function', 'quote (str)', 'Returns the text of an SQL literal which is the value of its argument suitable for inclusion into an SQL statement. 
Strings are surrounded by single-quotes with escapes on interior quotes as needed. 
BLOBs are encoded as hexadecimal literals.', 'select quote(''aa''''bb''); --> ''aa''''bb''', '', null, 1),
('random', 'function', 'random ()', 'Returns a pseudo-random integer between -9223372036854775808 and +9223372036854775807', 'select random();', '', null, 0),
('randomblob', 'function', 'randomblob (n)', 'Returns an N-byte blob containing pseudo-random bytes. 
If n is less than 1 then a 1-byte random blob is returned.', 'select randomblob();', '', null, 1),
('replace', 'function', 'replace (str, pattern, replacement)', 'Returns a copy of a string with each instance of a substring replaced by another substring.', 'select replace(''This is London'', ''London'', ''Sparta''); --> This is Sparta', '', 'Str|Pattern|Replacement', 3),
('round', 'function', 'round (n [, scale])', 'Returns a floating-point value N rounded to Scale digits to the right of the decimal point.', 'select round(12.345, 2); --> 12.35', '', 'N|Scale', 2),
('rtrim', 'function', 'rtrim (str [, character = '' ''])', 'Removes specified characters at the end of a string.', 
'select rtrim(''   SQLite   ''); --> ___SQLite
select rtrim(''@@SQLite@@'', ''@''); --> @@SQLite
select rtrim(''@@SQLite@12'', ''1@2''); --> @@SQLite', '', 'Str|Characters', 2),
('sign', 'function', 'sign (n)', 'Returns the sign of n: -1 if n < 0, 0 if n = 0, and 1 if n > 0.', 'select sign(-5); --> -1', '', null, 2),
('soundex', 'function', 'soundex (str)', 'Returns a string that is the soundex encoding of the string X. 
The string "?000" is returned if the argument is NULL or contains no ASCII alphabetic characters. 
This function is omitted from SQLite by default.', 'select soundex(''cat'');', '', null, 1),
('sqlite_compileoption_get', 'function', 'sqlite_compileoption_get (n)', 'Returns the N-th compile-time option used to build SQLite or 
NULL if N is out of range.', 'select sqlite_compileoption_get(5)', '', null, 1),
('sqlite_compileoption_used', 'function', 'sqlite_compileoption_used (opt)', 'When the argument is a string which is the name of a compile-time option, 
this routine returns true (1) or false (0) depending on whether or not that option was used during the build.', 'select sqlite_compileoption_used(''SQLITE_ENABLE_MATH_FUNCTIONS''); --> 1', '', null, 1),
('sqlite_source_id', 'function', 'sqlite_source_id ()', ' Returns the date and time that the source code was checked', 'select sqlite_source_id();', '', null, 0),
('sqlite_version', 'function', 'sqlite_version()', 'Returns the version string for the SQLite library that is running.', 'select sqlite_version();', '', null, 0),
('substr', 'function', 'substr (str, start [, length])', 'Returns a substring from a string starting at a specified position with a predefined length. 
Substring is synonim.', 
'select substr(''SQLite example'', 3); --> Lite example
select substr(''SQLite example'', 3, 4); --> Lite
select substr(''SQLite example'', 3, -2); --> SQ', 'substring', 'Str|Start|Length', 3),
('total_changes', 'function', 'total_changes()', 'Returns the number of row changes caused by INSERT, UPDATE or DELETE statements 
since the current database connection was opened.', 'select total_changes();', '', null, 0),
('trim', 'function', 'trim (str [, character = '' ''])', 'Removes specified characters at the beginning and the end of a string.', 
'select trim(''   SQLite   ''); --> SQLite
select trim(''@@SQLite@@'', ''@''); --> SQLite
select trim(''@12@SQLite@12'', ''1@2''); --> SQLite', '', 'Str|Characters', 2),
('typeof', 'function', 'typeof (any)', 'Determines the data type of a value', 
'select typeof(10); --> integer
select typeof(10.0); --> real
select typeof(''10'') --> text
select typeof(x''100'') --> blob
select typeof(null); --> null', '', null, 1),
('unicode', 'function', 'unicode(str)', 'Returns the numeric unicode code point corresponding to the first character of the Str. 
If the argument is not a string then the result is undefined.', 'select unicode(''S''); --> 83', '', null, 1),
('unlikely', 'function', 'unlikely(x)', 'Returns the argument x unchanged. 
The unlikely(x) is equivalent to likelihood(x, 0.0625). ', 'select unlikely(3);', '', null, 1),
('upper', 'function', 'upper (str)', 'Returns a copy of a string with all letter characters converted to uppercase. 
Doesn''t work for non-ANSI symbols without icu extension.', 'select upper(''SQLite''); --> SQLITE', '', null, 1),
('zeroblob', 'function', 'zeroblob(n)', 'Returns a BLOB consisting of n bytes of 0x00.', 'select zeroblob(5);', '', null, 1),

-- MATH
('acos', 'function', 'acos (n)', 'Returns the arc cosine of N (in the range of 0 to pi, expressed in radians).
The n must be in the range of -1 to 1.', 'select acos(0.3); --> 1.2661036727795', '', null, 1),
('acosh', 'function', 'acosh (n)', 'Compute the inverse hyperbolic cosine of N.', 'select acosh(3); --> 1.76274717403909', '', null, 1),
('asin', 'function', 'asin (n)', 'Returns the arc sine of N (in the range of 0 to pi, expressed in radians).
The n must be in the range of -1 to 1.', 'select asin(0.3); --> 0.304692654015397', '', null, 1),
('asinh', 'function', 'asinh (n)', 'Compute the inverse hyperbolic sine of N.', 'select asinh(3); --> 1.81844645923207', '', null, 1),
('atan', 'function', 'atan (n)', 'Returns the arc tangent of N (in the range of -pi/2 to pi/2, expressed in radians).', 'select atan(0.3); --> 0.291456794477867', '', null, 1),
('atan2', 'function', 'atan2 (n1, n1)', 'Returns the arc tangent of N1 and N2 (in the range of -pi to pi, depending on the signs of N1 and N2, expressed in radians).
The argument n1 can be in an unbounded range. 
atan2(n1, n2) is the same as atan(n1/n2).', 'select atan2(0.3, 0.2); --> 0.982793723247329', '', 'N1|N2', 2),
('atanh', 'function', 'atanh (n)', 'Compute the inverse hyperbolic tangent of N.', 'select atanh(0.3); --> 0.309519604203112', '', null, 1),
('ceil', 'function', 'ceil (n)', 'Returns smallest integer greater than or equal to N.
Ceiling is synonim.', 'select ceil(10.3); --> 11', 'ceiling', null, 1),
('cos', 'function', 'cos (n)', 'Returns the cosine of N (an angle expressed in radians)', 'select cos(pi()/2); --> ~0', '', null, 1),
('cosh', 'function', 'cosh (n)', 'Returns the hyperbolic cosine of N', 'select cosh(0.3); --> 1.04533851412886', '', null, 1),
('degrees', 'function', 'degrees (n)', 'Convert Radians into Degrees.', 'select degrees(pi()/2); --> 90', '', null, 1),
('exp', 'function', 'exp (n)', 'Returns e (~ 2.71828183) raised to the Nth power', 'select exp(2); --> ~7.4', '', null, 1),
('floor', 'function', 'floor (n)', 'Returns largest integer equal to or less than N', 'select floor(10.3); --> 10', '', null, 1),
('ln', 'function', 'log (n)', 'Returns the natural logarithm (base e) of the specified number.', 'select ln(2.72); --> ~1', '', null, 1),
('log', 'function', 'log ([base = 10,] n)', 'Returns the Base logarithm of the specified number.', 'select log(100); --> 2
log(10, 100); --> 2', 'log10', null, 2),
('log2', 'function', 'log2 (n)', 'Returns the base-2 logarithm of the specified number.', 'select log2(3); --> ~1.6', '', null, 1),
('mod', 'function', 'mod (n, m)', 'Returns the remainder after dividing N by M.
This is similar to the % operator, except that it works for non-integer arguments.', 'select mod(23, 7); --> 2', '', 'N|M', 2),
('pi', 'function', 'pi ()', 'Returns pi number.', 'select pi(); --> 3.14159265358979', '', null, 0),
('power', 'function', 'power (n1, n2)', 'Returns N1 raised to the N2 power. 
Pow is synonim.', 'select power(2, 3); --> 8', 'pow', 'N1|N2', 2),
('radians', 'function', 'radians (n)', 'Convert Radians into Degrees.', 'select radians(180); --> 3.14159265358979', '', null, 1),
('sin', 'function', 'sin (n)', 'Returns the sine of N (an angle expressed in radians)', 'select sin(pi()/2); --> 1.0', '', null, 1),
('sinh', 'function', 'sinh (n)', 'Returns the hyperbolic sine of N', 'select sinh(0.3); --> 0.304520293447143', '', null, 1),
('sqrt', 'function', 'sqrt (n)', 'Returns the square root of the specified number.', 'select sqrt(9); --> 3', '', null, 1),
('tan', 'function', 'tan (n)', 'Returns the tangent of N (an angle expressed in radians)', 'select tan(pi()/4); --> 1', '', null, 1),
('tanh', 'function', 'tanh (n)', 'Returns the hyperbolic tangent of N.', 'select tanh(0.3); --> 0.291312612451591', '', null, 1),
('trunc', 'function', 'trunc (n)', 'Returns the representable integer in between N and 0 (inclusive) that is furthest away from zero. 
Or, in other words, return the integer part of N, rounding toward zero. 
The trunc() function is similar to ceiling(n) and floor(n) except that it always rounds toward zero 
whereas ceiling(n) and floor(n) round up and down, respectively. ', 'select trunc(-11.5); --> -11', '', null, 1),

-- DATETIME FUNCTIONS
('time', 'function', 'time (timestring [, modifier, ...])', 'Accepts a time string and one or more modifiers. 
Returns a string that represents a specific time in HH:MM:SS format', 'select time(''10:20'',''+2 hours''); --> 12:20:00', '', 'TimeString|Modifier1|Modifier2|Modifier3|Modifier4|Modifier5', -1),
('date', 'function', 'date (timestring [, modifier, ...])', 'Accepts a time string and zero or more modifiers as arguments.
Returns a date string in YYYY-MM-DD format.', 'select date(''now'', ''start of month'', ''+1 month'', ''-1 day'');
select date(''2021-07-31'', ''+1 month'', ''-1 day''); --> 2021-08-30', '', 'TimeString|Modifier1|Modifier2|Modifier3|Modifier4|Modifier5', -1),
('datetime', 'function', 'datetime (timestring [, modifier, ...])', 'Accepts a time string and zero or more modifiers as arguments.
Returns a date string in YYYY-MM-DD HH:MM:SS format.', 'select datetime(''2021-08-14 13:15'', ''-1 day'', ''+2 hour''); --> 2012-08-13 15:15:00', '', 'TimeString|Modifier1|Modifier2|Modifier3|Modifier4|Modifier5', -1),
('julianday', 'function', 'julianday (timestring [, modifier, ...])', 'Accepts a time string and zero or more modifiers as arguments.
Returns the number of days since noon in Greenwich on November 24, 4714 B.C', 'select julianday(''2021-07-04''); --> 2459399.5', '', 'TimeString|Modifier1|Modifier2|Modifier3|Modifier4|Modifier5', -1),
('unixepoch', 'function', 'unixepoch (timestring [, modifier, ...])', 'Returns a unix timestamp - the number of seconds since 1970-01-01 00:00:00 UTC', 'select unixepoch(''2021-07-04''); --> 1625356800', '', 'TimeString|Modifier1|Modifier2|Modifier3|Modifier4|Modifier5', -1),
('strftime', 'function', 'strftime (format, timestring [, modifier, ...])', 'Used to format a datetime value based on a specified format', 'select strftime(''%Y-%m-%d %H:%M'', ''now''); --> current datetime
select strftime(''%Y-%m-%d %H:%M'', ''now'', ''localtime''); --> local datetime
select strftime(''%s'', ''now''); --> current Unix-time  
select strftime(''%s'', ''now'', ''+2 day''); --> current Unix-time + 2 days

-- Convert Unix-time 1605961514 to local date-time 21-11-2020 15:25:14
select strftime(''%d-%m-%Y %H:%M:%S'', 1605961514, ''unixepoch'', ''localtime'')', '', 'Format|TimeString|Modifier1|Modifier2|Modifier3|Modifier4|Modifier5', -1),
('unixepoch', 'function', 'unixepoch (timestring [, modifier, ...])', 'Returns a unix timestamp - the number of seconds since 1970-01-01 00:00:00 UTC. The return value is always an integer, even if the input time-value has millisecond precision.', 'select unixepoch(''2021-08-14 13:15''); --> 1628946900', '', 'TimeString|Modifier1|Modifier2|Modifier3|Modifier4|Modifier5', -1),

-- AGGREGATE FUNCTIONS
('min', 'function', 'min (any [, any, ...])', 'Returns the minimum value in arguments/a group. Only one argument is allowed for aggregation.', 'select available > 5 avail, min(price) "min" from books group by 1;
select min(5, 3, 10, 7); --> 3', '', null, -1),
('max', 'function', 'max (any [, any, ...])', 'Returns the maximum value in arguments/a group. Only one argument is allowed for aggregation.', 'select available > 5 avail, max(price) "max" from books group by 1;
select max(5, 3, 10, 7); --> 10', '', null, -1),
('avg', 'function', 'avg (any)', 'An aggregate SQL function that returns the average value of a group', 'select avg(price) from books; --> ~12.63
select min(distinct price) from books; -> ~13.1', '', null, 1),
('sum', 'function', 'sum (any)', 'An aggregate SQL function that returns the sum of values. 
If there are no non-NULL input rows then returns NULL.', 'select sum(price) from books;', '', null, 1),
('total', 'function', 'total (any)', 'An aggregate SQL function that returns the sum of values.
If there are no non-NULL input rows then returns 0.0.', 'select sum(price) from books;', '', null, 1),
('count', 'function', 'count (val)', 'An aggregate SQL function that returns a count of the number of times that Val is not NULL in a group. 
The count(*) function (with no arguments) returns the total number of rows in the group.', 'select count(*) from books;', '', null, 1),
('group_concat', 'function', 'group_concat (expr [, separator = ''''])', 'An aggregate SQL function that returns a string that is the concatenation of all non-NULL values of the input Expr separated by the Separator', 'select iif(price >= 5, ''expensive'', ''cheap'') "group", group_concat(title, '', '') titles from books group by price >= 5', '', 'Expr|Separator', 2),

-- REGEXP
('regexp', 'function', 'regexp (pattern, str)', 'Returns 1 if Str is matched to Pattern and returns 0 otherwise.
The following regular expression syntax is supported:
     X*      zero or more occurrences of X
     X+      one or more occurrences of X
     X?      zero or one occurrences of X
     X{p,q}  between p and q occurrences of X
     (X)     match X
     X|Y     X or Y
     ^X      X occurring at the beginning of the string
     X$      X occurring at the end of the string
     .       Match any single character
     \c      Character c where c is one of \{}()[]|*+?.
     \c      C-language escapes for c in afnrtv.  ex: \t or \n
     \uXXXX  Where XXXX is exactly 4 hex digits, unicode value XXXX
     \xXX    Where XX is exactly 2 hex digits, unicode value XX
     [abc]   Any single character from the set abc
     [^abc]  Any single character not in the set abc
     [a-z]   Any single character in the range a-z
     [^a-z]  Any single character not in the range a-z
     \b      Word boundary
     \w      Word character.  [A-Za-z0-9_]
     \W      Non-word character
     \d      Digit
     \D      Non-digit
     \s      Whitespace character
     \S      Non-whitespace character
  
The following expressions are equivalent: <url REGEXP ''^google.*''> and <regexp(''^google.*'', url)>', 'select regexp(''^google.*'', ''google.com'')', '', 'Pattern|Str', 2),

-- JSON1
('json', 'function', 'json (str)', 'Verifies that Str is a valid JSON string and returns a minified version.
If str is not a well-formed JSON string, then this function throws an error.', 'select json(''{"a":    100}''); --> {"a":100}', '', null, 1),
('json_valid', 'function', 'json_valid (str)', 'Returns 1 if Str is well-formed JSON and returns 0 otherwise.', 'select json_valid(''{"a":100}''); --> 1', '', null, 1),
('json_object', 'function', 'json_object ([key, value, ...])', 'Accepts zero or more pairs of arguments and returns a well-formed JSON object that is composed from those arguments.
The first argument of each pair is the Key and the second argument of each pair is the Value.
If any argument is a BLOB then an error is thrown.', 'select json_object(''a'', ''foo'', ''b'', ''bar''); --> {"a":"foo","b":"bar"}', '', 'Key1|Value1|Key2|Value2|Key3|Value3|Key4|Value4|Key5|Value5', -1),
('json_array', 'function', 'json_array ([any, ...])', 'Accepts zero or more arguments and returns a well-formed JSON array. 
If any argument is a BLOB then an error is thrown.', 'select json_array(1, 2, ''3'', 4); --> [1,2,"3",4]
select json_array(json_array(1,2)); --> [[1,2]]', '', null, -1),
('json_array_length', 'function', 'json_array_length (json [, path])', 'Returns the number of elements in the JSON array json (text), or 0 if json is not an array.', 'select json_array_length(''[1, 2, 3, 4]''); --> 4
select json_array_length(''{"one":[1,2,3]}'', ''$.one''); --> 3', '', 'Json|Path', 2),
('json_extract', 'function', 'json_extract (json, path [, path2, ...])', 'Extracts and returns one or more values from the well-formed JSON at Json.', 'select json_extract(''{"a": 2, "c": [4, 5, {"f": 7}]}'', ''$.c''); --> [4,5,{"f":7}]
select json_extract(''{"a": 2, "c": [4, 5], "f":7}'', ''$.c[#-1]'') --> 5', '', 'Json|Path|Path2|Path3|Path4|Path5', -1),
('json_insert', 'function', 'json_insert (json, path, value [, path2, value2, ...])', 'Returns a new JSON string formed by updating the input JSON by the path/value pairs. Ignore existing values.', 'select json_insert(''{"a": 2,"c": 4}'', ''$.b'', 50); --> {"a":2,"c":4,"b":50}
select json_insert(''{"a": 2,"c": 4}'', ''$.a'', 99); --> {"a":2,"c":4}', '', null, -1),
('json_replace', 'function', 'json_replace (json, path, value [, path2, value2, ...])', 'Returns a new JSON string formed by updating the input JSON by the path/value pairs. Ignore non-existing values.', 'select json_replace(''{"a": 2,"c": 4}'', ''$.b'', 50); --> {"a":2,"c":4}
select json_replace(''{"a": 2,"c": 4}'', ''$.a'', 99); --> {"a":99,"c":4}', '', 'Json|Path|Value|Path2|Value2|Path3|Value3|Path4|Value4|Path5|Value5', -1),
('json_set', 'function', 'json_set (json, path, value [, path2, value2, ...])', 'Returns a new JSON string formed by updating the input JSON by the path/value pairs. Create non-existing values, update existing.', 'select json_set(''{"a": 2,"c": 4}'', ''$.b'', 50, ''$.c'', 40); --> {"a":2,"c":40,"b":50}', '', 'Json|Path|Value|Path2|Value2|Path3|Value3|Path4|Value4|Path5|Value5', -1),
('json_patch', 'function', 'json_patch (json1, json2)', 'Returns patched json1 by json2 keys.', 'select json_patch(''{"a": [1, 2], "b":3, "c": 4}'', ''{"a": 9, "d": 5}''); --> {"a":9,"b":3,"c":4,"d":5}', '', 'Json1|Json2', 2),
('json_remove', 'function', 'json_remove (json, path [, path2, ...])', 'Returns a copy of the json with all the elements identified by path arguments removed. 
Paths that select elements not found in json are silently ignored.', 'select json_remove(''[0, 1, 2, 3, 4]'', ''$[2]'', ''$[0]''); --> [1,3,4]', '', 'Json|Path|Path2|Path3|Path4|Path5', -1),
('json_type', 'function', 'json_type (json [, path])', 'Returns the "type" of the outermost element of Json by Path. 
It''s one of: null, true, false, integer, real, text, array, or object. 
If the Path selects an element that does not exist in Json, then returns NULL.', 'select json_type(''{"a": [2, 3.5, true, false, null, "x"]}'',''$.a[5]''); --> text', '', 'Json|Path', 2),

('json_group_array', 'function', 'json_group_array (n)', 'An aggregate SQL function that returns a JSON array comprised of all N values in the aggregation', 'select json_group_array(n) from (select 1 n union select 2 n); --> [1,2]', '', null, 1),
('json_group_object', 'function', 'json_group_object (key, value)', 'An aggregate SQL function that returns a JSON object comprised of all key/value pairs in the aggregation', 'select json_group_object(key, value) from (select ''a'' key, ''foo'' value union select ''b'', ''bar''); --> {"a":"foo","b":"bar"}', '', 'Key|Value', 2),

('json_each', 'module', 'json_each (json [, path])', 'Expands the JSON text into a set of key/value pairs', 
'select key, value from json_each(''{"a":"foo", "b":"bar"}''); --> 2 rows: (a, foo) and (b, bar)
select value from json_each(''["a", "b"]''); --> 2 rows: (a) and (b)
select value from json_each(''{"a": ["foo", "bar"]}'', ''$.a''); --> 2 rows: (foo) and (bar)', '', 'Json|Path', 2),
('json_tree', 'module', 'json_tree(json [, path])', 'Walks the json and returns one row for each element.', 'select value from json_tree(''{"a": ["foo", "bar"]}'', ''$.a''); --> 3 rows: (["foo","bar"]), (foo) and (bar)', '', 'Json|Path', 2),

-- FILEIO
('readfile', 'function', ' readfile (path)', 'Reads and returns the contents of file as BLOB from disk', 'select readfile(''D:/data.txt'');', '', null, 1),
('writefile', 'function', ' readfile (path, data [, mode [, mtime]])', 'Writes blob DATA to file. If successful, the number of bytes written is returned.
If an error occurs, NULL is returned.', 'select writefile(''D:/data.txt'', ''Hello world'');', '', 'Path|Data|Mode|Mtime', 4),
('fsdir', 'module', 'fsdir (path [, dir])', 'Returns one row for the directory, and one row for each file within the hierarchy rooted at path.', 'select * from fsdir(''C:/WINDOWS'');', '', 'Path|Dir', 2),

-- EXEC
('exec', 'function', 'exec (command [, codepage = UTF32])', 'Executes an internal command and returns its output as the result.
The codepage is one of: ANSI, CP437, UTF7, UTF8, UTF16.
If command starts from "powershell" and codepage is empty then CP437 is applied.', 
'select exec(''powershell Get-Content C:/data.txt'', ''CP437''); -- as one value
select * from exec(''powershell Get-Content C:/data.txt'', ''CP437''); -- as each a line as a row', '', 'Command|Codepage', 2),

-- ODBC
('odbc_read', 'function', 'odbc_read (connectionString, query, target)', 
'Reads data via ODBC from an external source and write it to SQLite table. 
If the target table doesn''t exist, it''ll be created.
Use TABLES as the query to obtain a table list.
Use TYPES as the query to obtain support types.', 
'select odbc_read(''DSN=myDSN'', ''TABLES'', ''imported_data'');
select odbc_read(''DSN=myDSN'', ''select * from dbo.users'', ''imported_users'');', '', 'ConnectionString|Query|Target', 3),
('odbc_write', 'function', 'odbc_write (query, connectionString, target)', 
'Upload query resultset from SQLite to external database.
If the target doesn''t exist then it will be created. 
Otherwise, query columns should be complete the same with target coiumns.', 
'select odbc_write(''select * from books'', ''DSN=myMSSQL'', ''remote_books'');', '', 'Query|ConnectionString|Target', 3), 
('odbc_query', 'function', 'odbc_query (connectionString, query)', 'Execute query on external database e.g. to create target table.'
, 'select odbc_query(''DSN=myMSSQL'', ''drop database ABC;'')', '', 'ConnectionString|Query', 2),
('odbc_dsn', 'function', 'odbc_dsn ()', 'Returns local DSN list as json array: {"result": ["MyData", "Csv", ...], ...}', 'select odbc_dsn();', '', null, 0),

-- ORA
('rownum', 'function', 'rownum ([start])', 'Returns a row number in the result.', 'select rownum() from books;', '', 'Start', 1),
('concat', 'function', 'concat ([str, ...])', 'Concatenates input strings. Equals to str1 || str2 || ...', 'select concat(1, ''a'', 2.5) --> 1a2.5', '', 'Str|Str2|Str3|Str4|Str5', -1),
('decode', 'function', 'decode (expr [, search, result, ...], def)', 'Compares expr to each Search one by one.
If expr is equal to a Search, then returns the corresponding Result. 
If no match is found, then returns Def. 
If Def is omitted, then returns NULL.', 'select decode(1 < 2, false, ''NO'', true, ''YES'', ''???''); --> YES', '', 'Expr|Search|Result|Search2|Result2|Search3|Result3|Search4|Result4|Search5|Result5', -1),
('crc32', 'function', 'crc32 (str)', 'Calculate crc32 checksum', 'select crc32(''hello''); --> 907060870', '', null, 1),
('md5', 'function', 'md5 (str)', 'Calculate md5 checksum.', 'select md5(''hello''); --> 5d41402abc4b2a76b9719d911017c592', '', null, 1),
('base64_encode', 'function', 'base64_encode (str)', 'Encodes the given string with base64.', 'select base64_encode(''foobar''); --> Zm9vYmFy', '', null, 1),
('base64_decode', 'function', 'base64_decode (str)', 'Decodes a base64 encoded string.', 'select base64_encode(''Zm9vYmFy''); --> foobar', '', null, 1),
('strpart', 'function', 'strpart(str, delimiter, partno)', 'Returns substring for a delimiter and a part number', 
'select strpart(''ab-cd-ef'', ''-'', 2); --> cd
select strpart(''20.01.2021'', ''.'', 3); --> 2021
select strpart(''20-01/20/21'', ''-/'', -2); --> 20
select strpart(''D:/Docs/Book1.xls'', ''.'', -2); --> Book1', '', 'Str|Delimiter|PartNo', 3),
('conv', 'function', 'conv (num, from_base, to_base)', 'Converts a number from one numeric base number system to another numeric base number system.
After the conversion, the function returns a string representation of the number.
The minimum base is 2 and the maximum base is 36. Only positive numbers are supported.', 'select conv(15, 10, 2); --> 1111', '', 'Num|From base|To base', 3),
('tosize', 'function', 'tosize (nBytes)', 'Returns a human readable size.', 'select tosize(1024); --> 1.00KB
select tosize(2 * 1024 * 1024); --> 2.00MB', '', null, 1),
('levenshtein', 'function', 'levenshtein (str1, str2)', 'Calculates Levenshtein distance between two strings.', 'select levenshtein(''9128 LEEWARD CIR, INDIANAPOLIS, IN'', upper(''9128 Leeward Circle, Indianapolis, IN'')) --> 3', '', 'Str1|Str2', 2),

-- SERIES
('generate_series', 'module', 'generate_series (start, [stop [, step = 1]])', 'Generate a series of values, from start to stop with a step size of step', 'select value from generate_series(1, 10, 2); --> 5 rows', '', 
'Start|Stop|Step', 3),
 
-- UUID
('uuid', 'function', 'uuid ()', 'Generates a version 4 UUID as a string', 'select uuid();', '', null, 0),
('uuid_str', 'function', 'uuid_str (uuid)', 'Converts a UUID X into a well-formed UUID string', 'select uuid_str(''f34449d3f2e9448db2cfb49805ff1552''); --> f34449d3-f2e9-448d-b2cf-b49805ff1552', '', null, 1),
('uuid_blob', 'function', 'uuid_blob (uuid)', 'Converts a UUID X into a 16-byte blob', 'select uuid_blob(''f34449d3f2e9448db2cfb49805ff1552'');', '', null, 1),

-- XML
('xml_valid', 'function', 'xml_valid (xml)', 'Returns 1 if the argument is well-formed XML and 0, otherwise.', 'select xml_valid(''<a>A</a>''); --> 1
select xml_valid(''<a>A/a>''); --> 0', '', null, 1),
('xml_extract', 'function', 'xml_extract (xml, xpath [, sep = ''''])', 'Extracts a node content or an attribute value.', 'select xml_extract(''<a>A</a>'', ''a''); --> <a>A</a>
select xml_extract(''<a>A</a>'', ''a/text()''); --> A
select xml_extract(''<a>A</a><a>B</a>'', ''a/text()'', '',''); --> A,B
select xml_extract(''<a id = "1">A</a>'', ''a/@id''); --> 1', '', 'Xml|XPath|Sep', 3),
('xml_append', 'function', 'xml_append (xml, xpath, insertion [, pos = after])', 'Appends a node or an attribute based on the pos (one of: first, before, after, last, child or "child first"/"child last").
The "child" is ignored for the attribute. The insertion should be valid (there is no check).', 
'select xml_append(''<a>A</a><a>B</a><a>C</a>'', ''a[2]'', ''<b>D</b>'', ''after'') xml; --> <a>A</a><a>B</a><b>D</b><a>C</a>
select xml_append(''<a>A</a><a>B</a><a>C</a>'', ''a[2]'', ''<b>D</b>'', ''child'') xml; --> <a>A</a><a>B<b>D</b></a><a>C</a>
select xml_append(''<a>A</a><a id="1">B</a><a id="2">C</a>'', ''a/@id'', ''x="2"'', ''first'') xml; --> <a>A</a><a x="2" id="1">B</a><a x="2" id="2">C</a>', '', 'Xml|XPath|Insertion|Pos', 4),
('xml_update', 'function', 'xml_update (xml, xpath, replacement)', 'Updates nodes or attributes. The replacement should be valid (there is no check).
If the replacement is NULL then the call equals to xml_remove (xml, path).', 
'select xml_update(''<a>A</a><a id="1">B</a><a id="2">C</a>'', ''a[2]'', ''<b>D</b>''); --> <a>A</a><b>D</b><a id="2">C</a>
select xml_update(''<a>A</a><a id="1">B</a><a id="2">C</a>'', ''a/@id'', ''3''); --> <a>A</a><a id="3">B</a><a id="3">C</a>', '', 'Xml|XPath|Replacement', 3),
('xml_remove', 'function', 'xml_remove (xml, xpath)', 'Removes nodes or attributes.', 
'select xml_remove(''<a>A</a><a id="1">B</a><a id="2">C</a>'', ''a[2]''); --> <a>A</a><a id="2">C</a>
select xml_remove(''<a>A</a><a id="1">B</a><a id="2">C</a>'', ''a/@id''); --> <a>A</a><a>B</a><a>C</a>', '', 'Xml|XPath', 2),
('xml_each', 'module', 'xml_each (xml, xpath)', 'Walks the xml and returns one row for each element.', 'select * from xml_each(''<a>A</a><a>B</a><a>C</a>'', ''a/text()''); --> 3 rows', '', 'Xml|XPath', 2),

-- PRAGMAS
('analysis_limit', 'pragma', 'pragma analysis_limit = N', 'Query or change a limit on the approximate ANALYZE setting. This is approximate number of rows examined in each index by the ANALYZE command. If the argument N is omitted, then the analysis limit is unchanged. If the limit is zero, then the analysis limit is disabled and the ANALYZE command will examine all rows of each index. If N is greater than zero, then the analysis limit is set to N and subsequent ANALYZE commands will stop analyzing each index after it has examined approximately N rows. If N is a negative number or something other than an integer value, then the pragma behaves as if the N argument was omitted. In all cases, the value returned is the new analysis limit used for subsequent ANALYZE commands.', '', 'pragma_analysis_limit', null, 0),
('application_id', 'pragma', 'pragma schema.application_id = N', 'Query or set the 32-bit signed big-endian "Application ID" integer located at offset 68 into the database header', '', 'pragma_application_id', null, 0),

('auto_vacuum', 'pragma', 'schema.auto_vacuum = 0 | NONE | 1 | FULL | 2 | INCREMENTAL', 'Query or set the auto-vacuum status in the database.

The default setting is 0 or "none" (disabled): data is deleted data from a database, the database file remains the same size. Unused database file pages are added to a "freelist" and reused for subsequent inserts. So no database file space is lost. 

When the mode is 1 or "full", the freelist pages are moved to the end of the database file and the database file is truncated to remove the freelist pages at every transaction commit. Note, however, that auto-vacuum only truncates the freelist pages from the file.

When the value of auto-vacuum is 2 or "incremental" then the additional information needed to do auto-vacuuming is stored in the database file but auto-vacuuming does not occur automatically at each commit as it does with auto_vacuum=full. In incremental mode, the separate incremental_vacuum pragma must be invoked to cause the auto-vacuum to occur.', '', 'pragma_auto_vacuum', null, 0),

('automatic_index', 'pragma', 'pragma automatic_index = 0 | 1', 'Query, set, or clear the automatic indexing capability.', '', 'pragma_automatic_index', null, 0),

('busy_timeout', 'pragma', 'pragma busy_timeout = N', 'Query or change the setting of the busy timeout in milliseconds.', '', 'pragma_busy_timeout', null, 0),

('cache_size', 'pragma', 'pragma schema.cache_size = N', 'Query or change the suggested maximum number of database disk pages that SQLite will hold in memory at once per open database file. If the argument N is positive then the suggested cache size is set to N. If the argument N is negative, then the number of cache pages is adjusted to be a number of pages that would use approximately abs(N*1024) bytes of memory based on the current page size.', 'select * from temp.pragma_cache_size', 'pragma_cache_size', null, 0),
('cache_spill', 'pragma', 'pragma schema.cache_spill = 0 | 1 | N', 'Enables or disables the ability of the pager to spill dirty cache pages to the database file in the middle of a transaction. The N-form sets a minimum cache size threshold required for spilling to occur. The number of pages in cache must exceed both the cache_spill threshold and the maximum cache size set by the PRAGMA cache_size statement in order for spilling to occur.', '', 'pragma_cache_spill', null, 0),

('case_sensitive_like', 'pragma', 'pragma case_sensitive_like = 0 | 1', 'The default behavior of the LIKE operator is to ignore case for ASCII characters. Hence, by default ''a'' LIKE ''A'' is true. The case_sensitive_like pragma installs a new application-defined LIKE function that is either case sensitive or insensitive depending on the value of the case_sensitive_like pragma. When case_sensitive_like is disabled, the default LIKE behavior is expressed. When case_sensitive_like is enabled, case becomes significant.', 'select * from pragma_case_sensitive_like', 'pragma_case_sensitive_like', null, 0),

('cell_size_check ', 'pragma', 'pragma cell_size_check = 0 | 1', 'Enables or disables additional sanity checking on database b-tree pages as they are initially read from disk. With cell size checking enabled, database corruption is detected earlier and is less likely to "spread". However, there is a small performance hit for doing the extra checks and so cell size checking is turned off by default. ', '', 'pragma_cell_size_check', null, 0),

('checkpoint_fullfsync', 'pragma', 'pragma checkpoint_fullfsync = 0 | 1', 'Query or change the fullfsync flag for checkpoint operations. If this flag is set, then the F_FULLFSYNC syncing method is used during checkpoint operations on systems that support F_FULLFSYNC. The default value is off. Only Mac OS-X supports F_FULLFSYNC.', '', 'pragma_checkpoint_fullfsync', null, 0),

('collation_list', 'pragma', 'pragma collation_list', 'Return a list of the collating sequences defined for the current database connection.', '', 'pragma_collation_list', null, 0),

('compile_options', 'pragma', 'pragma compile_options', 'Returns the names of compile-time options used when building SQLite, one option per row. The "SQLITE_" prefix is omitted from the returned option names.', '', 'pragma_compile_options', null, 0),

('count_changes', 'pragma', 'pragma count_changes = 0 | 1', 'DEPRECATED

Query or change the count-changes flag. Normally, when the count-changes flag is not set, INSERT, UPDATE and DELETE statements return no data. When count-changes is set, each of these commands returns a single row of data consisting of one integer value - the number of rows inserted, modified or deleted by the command. The returned change count does not include any insertions, modifications or deletions performed by triggers, any changes made automatically by foreign key actions, or updates caused by an upsert.', '', 'pragma_count_changes', null, 0),

('data_store_directory', 'pragma', 'pragma data_store_directory = Path', 'DEPRECATED

Query or change the value of the sqlite3_data_directory global variable, which windows operating-system interface backends use to determine where to store database files specified using a relative pathname.', '', 'pragma_data_store_directory', null, 0),

('data_version', 'pragma', 'pragma schema.data_version', 'Provides an indication that the database file has been modified. Interactive programs that hold database content in memory or that display database content on-screen can use the PRAGMA data_version command to determine if they need to flush and reload their memory or update the screen display.', '', 'pragma_data_version', null, 0),

('database_list', 'pragma', 'pragma database_list', 'Returns one row for each database attached to the current database connection.', '', 'pragma_database_list', null, 0),

('default_cache_size', 'pragma', 'pragma schema.default_cache_size', 'DEPRECATED

This pragma queries or sets the suggested maximum number of pages of disk cache that will be allocated per open database file. The difference between this pragma and cache_size is that the value set here persists across database connections. The value of the default cache size is stored in the 4-byte big-endian integer located at offset 48 in the header of the database file.', '', 'pragma_default_cache_size', null, 0),

('defer_foreign_keys', 'pragma', 'pragma defer_foreign_keys = 0 | 1', 'When is on, enforcement of all foreign key constraints is delayed until the outermost transaction is committed. The pragma defaults to OFF so that foreign key constraints are only deferred if they are created as "DEFERRABLE INITIALLY DEFERRED". The defer_foreign_keys pragma is automatically switched off at each COMMIT or ROLLBACK. Hence, the defer_foreign_keys pragma must be separately enabled for each transaction. This pragma is only meaningful if foreign key constraints are enabled.', '', 'pragma_defer_foreign_keys', null, 0),

('empty_result_callbacks', 'pragma', 'pragma empty_result_callbacks = 0 | 1', 'DEPRECATED

Query or change the empty-result-callbacks flag.', '', 'pragma_empty_result_callbacks', null, 0),

('encoding', 'pragma', 'pragma encoding = ''UTF-8'' | ''UTF-16'' | ''UTF-16le'' | ''UTF-16be''', 'Returns or set the text encoding used by the main database. The ''UTF-16'' is interpreted as "UTF-16 encoding using native machine byte-ordering". Once an encoding has been set for a database, it cannot be changed.', '', 'pragma_encoding', null, 0),

('foreign_key_check', 'pragma', 'pragma schema.foreign_key_check([table-name])', 'Checks the database, or the table called "table-name", for foreign key constraints that are violated. Returns one row output for each foreign key violation.', '', 'pragma_foreign_key_check', null, 0),

('foreign_key_list', 'pragma', 'pragma foreign_key_list([table-name])', 'Returns one row for each foreign key constraint created by a REFERENCES clause in the CREATE TABLE statement of table "table-name".', '', 'pragma_foreign_key_list', null, 0),

('foreign_keys', 'pragma', 'pragma foreign_keys = 0 | 1', 'Query, set, or clear the enforcement of foreign key constraints.
This pragma is a no-op within a transaction; foreign key constraint enforcement may only be enabled or disabled when there is no pending BEGIN or SAVEPOINT.', '', 'pragma_foreign_keys', null, 0),

('freelist_count', 'pragma', 'pragma schema.freelist_count', 'Returns the number of unused pages in the database file', '', 'pragma_freelist_count', null, 0),

('full_column_names', 'pragma', 'pragma full_column_names = 0 | 1', 'DEPRECATED

Query or change the full_column_names flag. This flag together with the short_column_names flag determine the way SQLite assigns names to result columns of SELECT statements.', '', 'pragma_full_column_names', null, 0),

('fullfsync', 'pragma', 'pragma fullfsync = 0 | 1', 'Queries or changes the fullfsync flag. This flag determines whether or not the F_FULLFSYNC syncing method is used on systems that support it. The default value is off. Only Mac OS X supports F_FULLFSYNC.', '', 'pragma_fullfsync', null, 0),

('function_list', 'pragma', 'pragma function_list', 'Returns a list of SQL functions known to the database connection.', '', 'pragma_function_list', null, 0),

('hard_heap_limit', 'pragma', 'pragma hard_heap_limit = N', 'Invokes the sqlite3_hard_heap_limit64() interface with the argument N, if N is specified and N is a positive integer that is less than the current hard heap limit.', '', 'pragma_hard_heap_limit', null, 0),

('ignore_check_constraints', 'pragma', 'pragma ignore_check_constraints = 0 | 1', 'Enables or disables the enforcement of CHECK constraints. The default setting is off.', '', 'pragma_ignore_check_constraints', null, 0),

('incremental_vacuum', 'pragma', 'pragma schema.incremental_vacuum(N)', 'Causes up to N pages to be removed from the freelist. The database file is truncated by the same amount.', '', 'pragma_incremental_vacuum', null, 0),

('index_info', 'pragma', 'pragma schema.index_info(index-name)', 'Returns one row for each key column in the named index.', '', 'pragma_index_info', null, 0),

('index_list', 'pragma', 'pragma schema.index_list(table-name)', 'Returns one row for each index associated with the given table.', '', 'pragma_index_list', null, 0),

('index_xinfo', 'pragma', 'pragma schema.index_xinfo(index-name)', 'Returns information about every column in an index. Unlike this index_info pragma, this pragma returns information about every column in the index, not just the key columns.', '', 'pragma_index_xinfo', null, 0),

('integrity_check', 'pragma', 'pragma schema.integrity_check(table-name | N)', 'Does a low-level formatting and consistency check of the database.', '', 'pragma_integrity_check', null, 0),

('journal_mode', 'pragma', 'pragma schema.journal_mode = DELETE | TRUNCATE | PERSIST | MEMORY | WAL | OFF', 'Queries or sets the journal mode for databases associated with the current database connection.', '', 'pragma_journal_mode', null, 0),

('journal_size_limit', 'pragma', 'pragma schema.journal_size_limit = N', 'The pragma may be used to limit the size of rollback-journal and WAL files left in the file-system after transactions or checkpoints. Each time a transaction is committed or a WAL file resets, SQLite compares the size of the rollback journal file or WAL file left in the file-system to the size limit set by this pragma and if the journal or WAL file is larger it is truncated to the limit.', '', 'pragma_journal_size_limit', null, 0),

('legacy_alter_table', 'pragma', 'pragma legacy_alter_table = 0 | 1', 'sets or queries the value of the legacy_alter_table flag. When this flag is on, the ALTER TABLE RENAME command (for changing the name of a table) works as it did in SQLite 3.24.0 (2018-06-04) and earlier. More specifically, when this flag is on the ALTER TABLE RENAME command only rewrites the initial occurrence of the table name in its CREATE TABLE statement and in any associated CREATE INDEX and CREATE TRIGGER statements. Other references to the table are unmodified.', '', 'pragma_legacy_alter_table', null, 0),

('locking_mode', 'pragma', 'pragma schema.locking_mode = NORMAL | EXCLUSIVE', 'Sets or queries the database connection locking-mode. In NORMAL locking-mode (the default), a database connection unlocks the database file at the conclusion of each read or write transaction. When the locking-mode is set to EXCLUSIVE, the database connection never releases file-locks. The first time the database is read in EXCLUSIVE mode, a shared lock is obtained and held. The first time the database is written, an exclusive lock is obtained and held.', '', 'pragma_locking_mode', null, 0),

('max_page_count', 'pragma', 'pragma schema.max_page_count = N', 'Queries or sets the maximum number of pages in the database file.', '', 'pragma_max_page_count', null, 0),

('mmap_size', 'pragma', 'pragma schema.mmap_size = N', 'Queries or changes the maximum number of bytes that are set aside for memory-mapped I/O on a single database.', '', 'pragma_mmap_size', null, 0),

('module_list', 'pragma', 'pragma module_list', 'Returns a list of virtual table modules registered with the database connection.', '', 'pragma_module_list', null, 0),

('optimize', 'pragma', 'pragma schema.optimize(MASK)', 'Attempt to optimize the database.', '', 'pragma_optimize', null, 0),

('page_count', 'pragma', 'pragma schema.page_count = N', 'Queries or sets the page size of the database. The page size must be a power of two between 512 and 65536 inclusive. When a new database is created, SQLite assigns a page size to the database based on platform and filesystem.', '', 'pragma_page_count', null, 0),

('pragma_list', 'pragma', 'pragma pragma_list', 'Returns a list of PRAGMA commands known to the database connection. ', '', 'pragma_pragma_list', null, 0),

('quick_check', 'pragma', 'pragma schema.quick_check(table-name | N)', 'The pragma is like integrity_check except that it does not verify UNIQUE constraints and does not verify that index content matches table content. By skipping UNIQUE and index consistency checks, quick_check is able to run faster. PRAGMA quick_check runs in O(N) time whereas PRAGMA integrity_check requires O(NlogN) time where N is the total number of rows in the database. Otherwise the two pragmas are the same.', '', 'pragma_quick_check', null, 0),

('read_uncommitted', 'pragma', 'pragma read_uncommitted = 0 | 1', 'Queries, sets, or clears READ UNCOMMITTED isolation. The default isolation level for SQLite is SERIALIZABLE. Any process or thread can select READ UNCOMMITTED isolation, but SERIALIZABLE will still be used except between connections that share a common page and schema cache. Cache sharing is enabled using the sqlite3_enable_shared_cache() API. Cache sharing is disabled by default. ', '', 'pragma_read_uncommitted', null, 0),

('recursive_triggers', 'pragma', 'pragma recursive_triggers = 0 | 1', 'Queries, sets, or clears the recursive trigger capability. Changing the recursive_triggers setting affects the execution of all statements prepared using the database connection, including those prepared before the setting was changed.', '', 'pragma_recursive_triggers', null, 0),

('reverse_unordered_selects', 'pragma', 'pragma reverse_unordered_selects = 0 | 1', 'When enabled, this PRAGMA causes many SELECT statements without an ORDER BY clause to emit their results in the reverse order from what they normally would. This can help debug applications that are making invalid assumptions about the result order. The reverse_unordered_selects pragma works for most SELECT statements, however the query planner may sometimes choose an algorithm that is not easily reversed, in which case the output will appear in the same order regardless of the reverse_unordered_selects setting. ', '', 'pragma_reverse_unordered_selects', null, 0),

('schema_version', 'pragma', 'pragma schema.schema_version = N', 'Gets or sets the value of the schema-version integer at offset 40 in the database header. Warning: Misuse of this pragma can result in database corruption.', '', 'pragma_schema_version', null, 0),

('secure_delete', 'pragma', 'pragma schema.secure_delete = 0 | 1 | FAST', 'Queries or changes the secure-delete setting. When secure_delete is on, SQLite overwrites deleted content with zeros. The default mode is determined by the SQLITE_SECURE_DELETE compile-time option and is normally off. The off setting for secure_delete improves performance by reducing the number of CPU cycles and the amount of disk I/O. Applications that wish to avoid leaving forensic traces after content is deleted or updated should enable the secure_delete pragma prior to performing the delete or update, or else run VACUUM after the delete or update.

The "fast" setting for secure_delete is an intermediate setting in between "on" and "off". When secure_delete is set to "fast", SQLite will overwrite deleted content with zeros only if doing so does not increase the amount of I/O. In other words, the "fast" setting uses more CPU cycles but does not use more I/O. This has the effect of purging all old content from b-tree pages, but leaving forensic traces on freelist pages. ', '', 'pragma_secure_delete', null, 0),

('short_column_names', 'pragma', 'pragma short_column_names = 0 | 1', 'DEPRECATED

Queries or changes the short-column-names flag. This flag affects the way SQLite names columns of data returned by SELECT statements.', '', 'pragma_short_column_names', null, 0),

('shrink_memory', 'pragma', 'pragma shrink_memory', 'Causes the database connection on which it is invoked to free up as much memory as it can, by calling sqlite3_db_release_memory().', '', 'pragma_shrink_memory', null, 0),

('soft_heap_limit', 'pragma', 'pragma soft_heap_limit = N', 'Invokes the sqlite3_soft_heap_limit64() interface with the argument N, if N is specified and is a non-negative integer.', '', 'pragma_soft_heap_limit', null, 0),

('stats', 'pragma', 'pragma stats', 'Returns auxiliary information about tables and indices. The intended use of this pragma is only for testing and validation of SQLite.', '', 'pragma_stats', null, 0),

('synchronous', 'pragma', 'pragma schema.synchronous = 0 | OFF | 1 | NORMAL | 2 | FULL | 3 | EXTRA', 'Queries or changes the setting of the "synchronous" flag:
* EXTRA (3) synchronous is like FULL with the addition that the directory containing a rollback journal is synced after that journal is unlinked to commit a transaction in DELETE mode. EXTRA provides additional durability if the commit is followed closely by a power loss.
* When synchronous is FULL (2), the SQLite database engine will use the xSync method of the VFS to ensure that all content is safely written to the disk surface prior to continuing. This ensures that an operating system crash or power failure will not corrupt the database. FULL synchronous is very safe, but it is also slower. FULL is the most commonly used synchronous setting when not in WAL mode.
* When synchronous is NORMAL (1), the SQLite database engine will still sync at the most critical moments, but less often than in FULL mode. There is a very small (though non-zero) chance that a power failure at just the wrong time could corrupt the database in journal_mode=DELETE on an older filesystem. WAL mode is safe from corruption with synchronous=NORMAL, and probably DELETE mode is safe too on modern filesystems. WAL mode is always consistent with synchronous=NORMAL, but WAL mode does lose durability. A transaction committed in WAL mode with synchronous=NORMAL might roll back following a power loss or system crash. Transactions are durable across application crashes regardless of the synchronous setting or journal mode. The synchronous=NORMAL setting is a good choice for most applications running in WAL mode.
* With synchronous OFF (0), SQLite continues without syncing as soon as it has handed data off to the operating system. If the application running SQLite crashes, the data will be safe, but the database might become corrupted if the operating system crashes or the computer loses power before that data has been written to the disk surface. On the other hand, commits can be orders of magnitude faster with synchronous OFF.', '', 'pragma_synchronous', null, 0),

('table_info', 'pragma', 'pragma schema.table_info(table-name)', 'Returns one row for each column in the named table', '', 'pragma_table_info', null, 0),

('table_list', 'pragma', 'pragma schema.table_list
pragma table_list(table-name)', 'Returns information about the tables and views in the schema, one table per row of output', '', 'pragma_table_list', null, 0),

('table_xinfo', 'pragma', 'pragma schema.table_xinfo(table-name)', 'Returns one row for each column in the named table, including hidden columns in virtual tables.', '', 'pragma_table_xinfo', null, 0),

('temp_store', 'pragma', 'pragma temp_store = 0 | DEFAULT | 1 | FILE | 2 | MEMORY', 'Queries or changes the setting of the "temp_store" parameter. When temp_store is DEFAULT (0), the compile-time C preprocessor macro SQLITE_TEMP_STORE is used to determine where temporary tables and indices are stored. When temp_store is MEMORY (2) temporary tables and indices are kept in as if they were pure in-memory databases memory. When temp_store is FILE (1) temporary tables and indices are stored in a file.', '', 'pragma_temp_store', null, 0),

('temp_store_directory', 'pragma', 'pragma temp_store_directory = ''dir-name''', 'DEPRECATED

Queries or changes the value of the sqlite3_temp_directory global variable, which many operating-system interface backends use to determine where to store temporary tables and indices.', '', 'pragma_temp_store_directory', null, 0),

('threads', 'pragma', 'pragma threads = N', 'Queries or changes the value of the sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, ...) limit for the current database connection. This limit sets an upper bound on the number of auxiliary threads that a prepared statement is allowed to launch to assist with a query. The default limit is 0.', '', 'pragma_threads', null, 0),

('trusted_schema', 'pragma', 'pragma trusted_schema = 0 | 1', 'The trusted_schema setting is a per-connection boolean that determines whether or not SQL functions and virtual tables that have not been security audited are allowed to be run by views, triggers, or in expressions of the schema such as CHECK constraints, DEFAULT clauses, generated columns, expression indexes, and/or partial indexes. In order to maintain backwards compatibility, this setting is ON by default.', '', 'pragma_trusted_schema', null, 0),

('user_version', 'pragma', 'pragma schema.user_version = N', 'The user_version pragma will to get or set the value of the user-version integer at offset 60 in the database header. The user-version is an integer that is available to applications to use however they want. SQLite makes no use of the user-version itself.', '', 'pragma_user_version', null, 0),

('wal_autocheckpoint', 'pragma', 'pragma wal_autocheckpoint = N', 'Queries or sets the write-ahead log auto-checkpoint interval. When the write-ahead log is enabled (via the journal_mode pragma) a checkpoint will be run automatically whenever the write-ahead log equals or exceeds N pages in length. Setting the auto-checkpoint size to zero or a negative value turns auto-checkpointing off.', '', 'pragma_wal_autocheckpoint', null, 0),

('wal_checkpoint', 'pragma', 'pragma schema.wal_checkpoint(PASSIVE | FULL | RESTART | TRUNCATE)', 'If the write-ahead log is enabled (via the journal_mode pragma), this pragma causes a checkpoint operation to run on database database, or on all attached databases if database is omitted. If write-ahead log mode is disabled, this pragma is a harmless no-op.
* PASSIVE: Checkpoint as many frames as possible without waiting for any database readers or writers to finish. Sync the db file if all frames in the log are checkpointed. This mode is the same as calling the sqlite3_wal_checkpoint() C interface. The busy-handler callback is never invoked in this mode. 
* FULL: This mode blocks (invokes the busy-handler callback) until there is no database writer and all readers are reading from the most recent database snapshot. It then checkpoints all frames in the log file and syncs the database file. FULL blocks concurrent writers while it is running, but readers can proceed. 
* RESTART: This mode works the same way as FULL with the addition that after checkpointing the log file it blocks (calls the busy-handler callback) until all readers are finished with the log file. This ensures that the next client to write to the database file restarts the log file from the beginning. RESTART blocks concurrent writers while it is running, but allowed readers to proceed. 
* TRUNCATE: This mode works the same way as RESTART with the addition that the WAL file is truncated to zero bytes upon successful completion. ', '', 'pragma_wal_checkpoint', null, 0),

('writable_schema', 'pragma', 'pragma writable_schema = 0 | 1 | RESET', 'When this pragma is on, and the SQLITE_DBCONFIG_DEFENSIVE flag is off, then the sqlite_schema table can be changed using ordinary UPDATE, INSERT, and DELETE statements. If the argument is "RESET" then schema writing is disabled (as with "PRAGMA writable_schema = OFF") and, in addition, the schema is reloaded. Warning: misuse of this pragma can easily result in a corrupt database file.', '', 'pragma_writable_schema', null, 0)
;