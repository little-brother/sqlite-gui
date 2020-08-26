/*
	exec(cmd, maxOutputLength = 32000)
	Executes shell command and returns console output as result.
	Can be used to import an external data or to execute smth in a trigger
	select exec('powershell -nologo "Get-Content data.json"')
*/
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <assert.h>
#include <string.h>
#include <windows.h>

static void exec(sqlite3_context *context, int argc, sqlite3_value **argv){
	if (argc == 0) {
		sqlite3_result_null(context);
		return;		
	}
		
	int len  = sqlite3_value_bytes(argv[0]);
	unsigned char cmd[len + 1];
	strcpy((char*)cmd, (char*)sqlite3_value_text(argv[0]));

	int MAXSIZE = argc > 1 && sqlite3_value_type(argv[1]) ==  SQLITE_INTEGER ? sqlite3_value_int(argv[1]) : 32000;
	int BUFSIZE = 32000;
	if (MAXSIZE <= 0)
		MAXSIZE = BUFSIZE;
	if (MAXSIZE < BUFSIZE)
		 BUFSIZE = MAXSIZE;
	
	SECURITY_ATTRIBUTES sa = {0};
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	
	HANDLE hStdOutRd, hStdOutWr;
	HANDLE hStdErrRd, hStdErrWr;
	
	if (!CreatePipe(&hStdOutRd, &hStdOutWr, &sa, 0) || !CreatePipe(&hStdErrRd, &hStdErrWr, &sa, 0)) {
		sqlite3_result_text(context, (char*)"Error to create streams", -1, SQLITE_TRANSIENT);
		return;
	}
	
	SetHandleInformation(hStdOutRd, HANDLE_FLAG_INHERIT, 0);
	SetHandleInformation(hStdErrRd, HANDLE_FLAG_INHERIT, 0);
	
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = hStdOutWr;
	si.hStdError = hStdErrWr;
	
	PROCESS_INFORMATION pi = {0};
	
	boolean rc = CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi); 

	CloseHandle(hStdOutWr);
	CloseHandle(hStdErrWr);

	if (rc) {
		char res[MAXSIZE];
		memset (res, 0, MAXSIZE);
	 
		char buf[BUFSIZE];
	
		for (int i = 0; i < 2; i++) {
			boolean bSuccess = FALSE;
			DWORD dwRead;
			HANDLE hRd = i == 0 ? hStdOutRd : hStdErrRd;

			for (;strlen(res) < MAXSIZE;) { 
				memset (buf, 0, BUFSIZE);
				bSuccess = ReadFile(hRd, buf, BUFSIZE - 1, &dwRead, NULL);
				if(!bSuccess) 
					break; 
			
				int rLen = strlen(res);
				int bLen = strlen(buf);
				strncat(res, buf, rLen + bLen < MAXSIZE ? bLen : MAXSIZE - rLen);
			}
		}

		sqlite3_result_text(context, (char*)res, -1, SQLITE_TRANSIENT);
	} else {
		sqlite3_result_text(context, (char*)"An error has occurred while creating a process", -1, SQLITE_TRANSIENT);
	}
	
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	CloseHandle(hStdOutRd);	
	CloseHandle(hStdErrRd);
}

__declspec(dllexport) int sqlite3_exec_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
	int rc = SQLITE_OK;
	SQLITE_EXTENSION_INIT2(pApi);
	(void)pzErrMsg;  /* Unused parameter */
	return sqlite3_create_function(db, "exec", -1, SQLITE_UTF8, 0, exec, 0, 0);
}