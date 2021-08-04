#include <windows.h>
#include <process.h>
#include <ws2tcpip.h>
#include <shlwapi.h>
#include <stdio.h>
#include "utils.h"

#define MAX_CLIENT_COUNT        30
#define MAX_REQUEST_LENGTH   32000
#define DEF_RESPONSE_LENGTH   2000
#define MAX_URL_LENGTH        2000

#define URI_UNSUPPORTED 0
#define URI_SELECT_LIST 1
#define URI_SELECT_ITEM 2
#define URI_SELECT_REFS 3
#define URI_INSERT_ITEM 4
#define URI_UPDATE_ITEM 5
#define URI_DELETE_ITEM 6

namespace http {
	static bool isRun = false;
	static HANDLE hThread = 0;
	static sqlite3* db;
	static int port = 0;
	static bool isDebug = false;

	const char header[] = "HTTP/1.1 %s\nServer: sqlite-gui\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Methods: *\nAccess-Control-Allow-Headers: *\nContent-Type: application/json; charset=utf-8\nContent-Length: %i\n\n%s";
	const char index[] = "HTTP/1.1 200 OK\nServer: sqlite-gui\nContent-Type: text/html; charset=utf-8\n\n" \
		"<html>" \
		"<head>" \
		"<link rel=\"shortcut icon\" href=\"data:image/x-icon;base64,AAABAAEAEBAQAAAAAAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAACAAAAAgIAAgAAAAIAAgACAgAAAgICAAMDAwAAAAP8AAP8AAAD//wD/AAAA/wD/AP//AAD///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACZkAAACZmQmZkAmZkJmZCZmZCZmQAJkJkJkJkACZmQmQmQmQAJmZCZCZCZAAmQAJkJkJkACZmQmZmQmQAJmZCZmZCZAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD//wAA//8AAP//AAD+PwAACGEAAAghAADJJwAACScAAAknAAA5JwAACCcAAAgnAAD//wAA//8AAP//AAD//wAA\" type=\"image/x-icon\"> " \
		"<meta charset=\"UTF-8\"/><meta charset=\"UTF-8\"/><title>sqlite-gui</title>" \
		"<style>html, body {height: 100%%;} html {display: table; margin: auto;} body {display: table-cell; vertical-align: middle;} #help {display: block; text-align: right; font-size: 12px; margin-top: 5px;}</style>" \
		"</head>" \
		"<body>" \
		"<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\">" \
		"<caption>SQLite REST API web server for<br>%s<br>&nbsp;</caption>" \
		"<tr><th>Method</th><th>Url</th><th>Description</th></tr>" \
		"<tr><td>GET</td><td>/api/:table?:tail</td><td>Returns all :table rows</td></tr>" \
		"<tr><td>GET</td><td>/api/:table/:id</td><td>Get row by :id</td></tr>" \
		"<tr><td>GET</td><td>/api/:table/:id/:fktable</td><td>Get linked rows in :fktable by foreign key</td></tr>" \
		"<tr><td>POST</td><td>/api/:table</td><td>Insert row into :table</td></tr>" \
		"<tr><td>PUT</td><td>/api/:table/:id</td><td>Update row by :id</td></tr>" \
		"<tr><td>DELETE</td><td>/api/:table/:id</td><td>Remove row by :id</td></tr>" \
		"</table>" \
		"<div id = \"help\">Visit <a href=\"https://github.com/little-brother/sqlite-gui/wiki#rest-api-web-server\">Wiki</a> to get details</div>" \
		"</body>" \
		"</html>";

	void debug(const char *fmt, ...) {
		if (!isDebug)
			return;

		FILE* out = fopen ("http-debug.txt", "ab+");
		va_list args;
		va_start(args, fmt);
		vfprintf(out, fmt, args);
		va_end(args);

		fputc('\n', out);
		fflush(out);
		fclose(out);
	}

	unsigned int __stdcall runServer (void* data);
	bool stopServer();

	char *getHTTPResponse(const char *HTTPRequest);
	void initSocketDescriptors(struct fd_set *socketDescriptors, SOCKET ServerSocket, SOCKET ClientSocketList[]);
	int acceptNewConnection(SOCKET ServerSocket, SOCKET ClientSocketList[]);
	int processSocketActivity(SOCKET ClientSocketList[], int socketIndex);

	unsigned int __stdcall runServer (void* data) {
		WSADATA wsaData;
		struct addrinfo *serverInfo = NULL;
		struct addrinfo hints = {AI_PASSIVE, AF_INET, SOCK_STREAM, IPPROTO_TCP};
		SOCKET ServerSocket = INVALID_SOCKET;
		SOCKET ClientSocketList[MAX_CLIENT_COUNT];
		fd_set socketDescriptors;
		int socketActivity;

		for (int i = 0; i < MAX_CLIENT_COUNT; i++)
			ClientSocketList[i] = 0;

		debug("Running web server on port %i...", port);

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
			return stopServer();

		char cport[10];
		sprintf(cport, "%i", port);
		if (getaddrinfo(NULL, cport, &hints, &serverInfo) != 0)
			return stopServer();

		ServerSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
		if (ServerSocket == INVALID_SOCKET) {
			debug("Can't open socket");
			freeaddrinfo(serverInfo);
			return stopServer();
		}

		if (bind(ServerSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == SOCKET_ERROR) {
			debug("Can't bind socket");
			freeaddrinfo(serverInfo);
			closesocket(ServerSocket);
			return stopServer();
		}

		freeaddrinfo(serverInfo);

		if (listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR) {
			debug("Can't listen socket");
			closesocket(ServerSocket);
			return stopServer();
		}

		isRun = true;
		MessageBeep(0);
		debug("OK");

		while(isRun) {
			initSocketDescriptors(&(socketDescriptors), ServerSocket, ClientSocketList);
			socketActivity = select(0, &(socketDescriptors), NULL, NULL, NULL);

			if (socketActivity == SOCKET_ERROR) {
				debug("Can't determine the status of socket");
				closesocket(ServerSocket);
				return stopServer();
			}

			if (FD_ISSET(ServerSocket, &(socketDescriptors)) && !acceptNewConnection(ServerSocket, ClientSocketList)) {
				debug("Can't accept a new connection");
				closesocket(ServerSocket);
				return stopServer();
			}

			for (int i = 0; i < MAX_CLIENT_COUNT; i++) {
				if (FD_ISSET(ClientSocketList[i], &(socketDescriptors)) && !processSocketActivity(ClientSocketList, i)) {
					debug("Can't process a client activity");
					closesocket(ClientSocketList[i]);
					closesocket(ServerSocket);
					return stopServer();
				}
			}
		}

		return stopServer();
	}

	bool stopServer() {
		WSACleanup();
		isRun = false;

		if (db) {
			sqlite3_close(db);
			db = 0;
			port = 0;
		}

		return true;
	}

	/* ?1 = table8, ?2 = id8, ?3 = postData8, ?4 = fktable8, ?5 = search8 */
	const char* QUERY_GENERATORS[] = {
		/* URI_UNSUPPORTED                       */ 0,
		/* URI_SELECT_LIST: GET    /:table?:tail */ "select 'select json_group_array(json_object(' || group_concat('''' || name || ''', ' || name, ', ') || ')) from (select * from ' || ?1 || ' ' || coalesce(?5, '') || ')' from pragma_table_info(?1) order by cid",
		/* URI_SELECT_ITEM: GET    /:table/:id   */ "select 'select json_object(' || group_concat('''' || name || ''', ' || name, ', ') || ') from ' || ?1 || ' where ' || group_concat(iif(pk, name, ''), null) || ' = ' || ?2 from pragma_table_info(?1) order by cid",
		/* URI_SELECT_REFS: GET    /:table/:id/:fktable */ "select 'select json_group_array(json_object(' || group_concat('''' || name || ''', ' || name, ', ') || ')) from ' || ?4 || ' where ''' || ?2 || ''' = \"' || (select \"from\" from pragma_foreign_key_list(?4) where \"table\" = ?1) || '\"' from pragma_table_info(?4) order by cid",
		/* URI_INSERT_ITEM: POST   /:table       */ "with pti as (select group_concat(name, ', ') name from pragma_table_info(?1) where pk = 1 limit 1), je as (select key, replace(value, '''', '''''') value from json_each(?3)) select 'insert into ' || ?1 || ' (\"' || group_concat(key, '\", \"') || '\") values (''' || group_concat(value, ''', ''') || ''') returning json_object(''' || (select name from pti) || ''', ' || (select name from pti) || ')' from je where key in (select name from pragma_table_info(?1))",
		/* URI_UPDATE_ITEM: PUT    /:table/:id   */ "with pti as (select group_concat(name, ', ') name from pragma_table_info(?1) where pk = 1 limit 1), je as (select key, replace(value, '''', '''''') value from json_each(?3)) select 'update ' || ?1 || ' set ' || group_concat('\"' || key || '\" = ''' || value || '''', ', ') || ' where \"' || (select name from pti) || '\" = ''' || ?2 || ''' returning json_object(''' || (select name from pti) || ''', ' || (select name from pti) || ')' from je where key in (select name from pragma_table_info(?1) where pk = 0)",
		/* URI_DELETE_ITEM: DELETE /:table/:id   */ "select 'delete from ' || ?1 || ' where ' || group_concat(iif(pk, name, ''), null) || ' = ' || ?2 || ' returning json_object(''' || group_concat(iif(pk, name, ''), null) || ''',' || ?2 || ')' from pragma_table_info(?1) order by cid"
	};

	char *getHTTPResponse(const char *HTTPRequest) {
		char *start = strchr(HTTPRequest, '/');
		int len = strchr(HTTPRequest, '\n') - start;
		while (len > 0 && ((start + len)[0] != ' '))
			len--;

		char rawpath8[len + 1]{0}, path8[len + 1]{0};
		strncpy(rawpath8, start, len);
		utils::urlDecode(path8, rawpath8);

		if (strcmp(path8, "/") == 0) {
			const char* dbpath8 = sqlite3_db_filename(db, 0);
			char* response = (char *) calloc((strlen(index) + strlen(dbpath8) + 1), sizeof(char));
			sprintf(response, index, dbpath8);
			return response;
		}

		char* status = (char *) calloc(200, sizeof(char));
		char* content = (char *) calloc(DEF_RESPONSE_LENGTH, sizeof(char));

		char table8[MAX_URL_LENGTH]{0};
		char id8[MAX_URL_LENGTH]{0};
		char fktable8[MAX_URL_LENGTH]{0};
		char search8[MAX_URL_LENGTH]{0};
		sscanf(path8, "/api/%[^/]/%[^/]/%[^/?]", table8, id8, fktable8);
		if (strlen(fktable8) == 0)
			sscanf(path8, "/api/%[^/]/%[^/?]?%s", table8, id8, search8);
		if (strlen(id8) == 0)
			sscanf(path8, "/api/%[^?/]?%[^,]", table8, search8);

		char* postData8 = strstr(HTTPRequest, "\n\n");
		if (postData8 == NULL)
			postData8 = strstr(HTTPRequest, "\r\n\r\n");

		debug(HTTPRequest);
		debug("\nUrl: %s, table: %s, id: %s, fk: %s, search: %s", path8, table8, id8, fktable8, search8);

		int uri =
			strncmp(HTTPRequest, "GET", 3) == 0 && strlen(table8) > 0 && strlen(id8) == 0 ? URI_SELECT_LIST :
			strncmp(HTTPRequest, "GET", 3) == 0 && strlen(table8) > 0 && strlen(id8) > 0 && strlen(fktable8) == 0 ? URI_SELECT_ITEM :
			strncmp(HTTPRequest, "GET", 3) == 0 && strlen(table8) > 0 && strlen(id8) > 0 && strlen(fktable8) > 0 ? URI_SELECT_REFS :
			strncmp(HTTPRequest, "POST", 4) == 0 && strlen(table8) > 0 && strlen(id8) == 0 ? URI_INSERT_ITEM :
			strncmp(HTTPRequest, "PUT", 3) == 0 && strlen(table8) > 0 && strlen(id8) > 0 ? URI_UPDATE_ITEM :
			strncmp(HTTPRequest, "DELETE", 6) == 0 && strlen(table8) > 0 && strlen(id8) > 0 ? URI_DELETE_ITEM :
			URI_UNSUPPORTED;

		auto error = [status, content](const char* code, const char* msg) {
			sprintf(status, code);
			sprintf(content, "{\"error:\": \"%s\"}", msg);
		};

		auto isJsonValid = [](const char* json8) {
			bool res = true;
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, "select json_valid(?1)", -1, &stmt, 0);
			sqlite3_bind_text(stmt, 1, json8, strlen(json8), SQLITE_TRANSIENT);
			sqlite3_step(stmt);
			res = sqlite3_column_int(stmt, 0) > 0;
			sqlite3_finalize(stmt);
			return res;
		};

		if (strlen(path8) > MAX_URL_LENGTH) {
			error("414 Request-URI Too Long", "Request-URI Too Long");
		} else if (uri == URI_UNSUPPORTED) {
			error("501 Not Implemented", "Not implemented");
		} else if ((uri == URI_INSERT_ITEM || uri == URI_UPDATE_ITEM) && !isJsonValid(postData8)) {
			error("400 Bad Request", "Request body should be a valid JSON object");
		} else {
			sqlite3_stmt* stmt;
			debug("\nUrl generator: %s", QUERY_GENERATORS[uri]);
			if (SQLITE_OK == sqlite3_prepare_v2(db, QUERY_GENERATORS[uri], -1, &stmt, 0)) {
				sqlite3_bind_text(stmt, 1, table8, strlen(table8), SQLITE_TRANSIENT);
				if (strlen(id8))
					sqlite3_bind_text(stmt, 2, id8, strlen(id8), SQLITE_TRANSIENT);

				if (postData8 != NULL)
					sqlite3_bind_text(stmt, 3, postData8, strlen(postData8), SQLITE_TRANSIENT);

				if (strlen(fktable8) > 0)
					sqlite3_bind_text(stmt, 4, fktable8, strlen(fktable8), SQLITE_TRANSIENT);

				if (strlen(search8) > 0)
					sqlite3_bind_text(stmt, 5, search8, strlen(search8), SQLITE_TRANSIENT);

				debug("\nExpanded url generator: %s\n", sqlite3_expanded_sql(stmt));
				bool rc = (SQLITE_ROW == sqlite3_step(stmt)) && (sqlite3_column_bytes(stmt, 0) > 0);
				if (rc) {
					const char* query8 = (const char*)sqlite3_column_text(stmt, 0);
					debug("\nGenerated query: %s\n", query8);
					sqlite3_stmt* stmt2;
					rc = (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt2, 0)) && (SQLITE_ROW == sqlite3_step(stmt2));
					if (rc) {
						const char* res = (const char*)sqlite3_column_text(stmt2, 0);
						if (strlen(res) > DEF_RESPONSE_LENGTH - 10) {
							free(content);
							content = (char *) calloc(strlen(res) + 1, sizeof(char));
						}
						sprintf(content, res);
					}
					sqlite3_finalize(stmt2);
				}

				if (!rc)
					error("404 Not found", "Not found");
			} else {
				debug("Error: %s", sqlite3_errmsg(db));
				error("500 Internal Server Error", "Internal Server Error");
			}

			sqlite3_finalize(stmt);
		}

		char* response = (char *) calloc((strlen(header) + strlen(content) + strlen(status) + 200), sizeof(char));
		sprintf(response, header, status, strlen(content), content);
		free(status);
		free(content);

		return response;
	}

	void initSocketDescriptors(fd_set *socketDescriptors, SOCKET ServerSocket, SOCKET ClientSocketList[]) {
		FD_ZERO(socketDescriptors);
		FD_SET(ServerSocket, socketDescriptors);
		for (int i = 0; i < MAX_CLIENT_COUNT; i++)
			if (ClientSocketList[i] > 0)
				FD_SET(ClientSocketList[i], socketDescriptors);
	}

	int acceptNewConnection(SOCKET ServerSocket, SOCKET ClientSocketList[]) {
		SOCKET RecentSocket = 0;
		if ((RecentSocket = accept(ServerSocket, NULL, NULL)) == INVALID_SOCKET)
			return 0;

		for (int i = 0; i < MAX_CLIENT_COUNT; i++) {
			if (ClientSocketList[i] == 0) {
				ClientSocketList[i] = RecentSocket;
				break;
			}
		}

		return 1;
	}

	int processSocketActivity(SOCKET ClientSocketList[], int socketIndex) {
		SOCKET ClientSocketItem = ClientSocketList[socketIndex];
		char* HTTPRequest = (char *) malloc((MAX_REQUEST_LENGTH + 1) * sizeof(char));
		bool rc = recv(ClientSocketItem, HTTPRequest, MAX_REQUEST_LENGTH, 0) != 0;
		if (rc) {
			char* HTTPResponse = getHTTPResponse(HTTPRequest);
			rc = rc && (send(ClientSocketItem, HTTPResponse, strlen(HTTPResponse), 0) != SOCKET_ERROR);
			rc = rc && (shutdown(ClientSocketItem, SD_SEND) != SOCKET_ERROR);
			free(HTTPResponse);
		}

		closesocket(ClientSocketItem);
		ClientSocketList[socketIndex] = 0;
		return rc;
	}

	bool start(int _port, sqlite3* _db, bool _debug) {
		if (isRun)
			return false;

		db = _db;
		port = _port;
		isDebug = _debug;

		debug("\n\Execute in terminal \".set http-server-debug 0\" to stop debugging\n\n");

		hThread = (HANDLE)_beginthreadex(0, 0, &runServer, 0, 0, 0);
		return true;
	}

	bool stop() {
		isRun = false;
		if (hThread) {
			TerminateThread(hThread, 0);
			CloseHandle(hThread);
			hThread = 0;
		}

		stopServer();
		return true;
	}
}
