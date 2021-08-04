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
('printf', 'function', 'printf (format, ...)', 'Works like the printf() function from the standard C library.', 'select printf(''Hello %s!''); --> Hello world!', '', 'Format', -1), 
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
('strftime', 'function', 'strftime (format, timestring [, modifier, ...])', 'Used to format a datetime value based on a specified format', 'select strftime(''%Y-%m-%d %H:%M'', ''now''); --> current datetime
select strftime(''%Y-%m-%d %H:%M'', ''now'', ''localtime''); --> local datetime
select strftime(''%s'', ''now''); --> current Unix-time  
select strftime(''%s'', ''now'', ''+2 day''); --> current Unix-time + 2 days

-- Convert Unix-time 1605961514 to local date-time 21-11-2020 15:25:14
select strftime(''%d-%m-%Y %H:%M:%S'', 1605961514, ''unixepoch'', ''localtime'')', '', 'Format|TimeString|Modifier1|Modifier2|Modifier3|Modifier4|Modifier5', -1),

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
('xml_each', 'module', 'xml_each (xml, xpath)', 'Walks the xml and returns one row for each element.', 'select * from xml_each(''<a>A</a><a>B</a><a>C</a>'', ''a/text()''); --> 3 rows', '', 'Xml|XPath', 2)

;