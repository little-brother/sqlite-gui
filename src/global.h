#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define _WIN32_IE	0x0500
#define MAX_RECENT_COUNT 10
#define MAX_TAB_COUNT 32
#define MAX_RESULT_COUNT 32
#define MAX_TEXT_LENGTH 32000


#define ACTION_SETDEFFONT           1
#define ACTION_DESTROY              2
#define ACTION_RESIZETAB            3
#define ACTION_UPDATETAB            4

#define ROW_VIEW                    0
#define ROW_EDIT                    1
#define ROW_ADD                     2

#define TABLE                       1
#define VIEW                        2
#define INDEX                       3
#define TRIGGER                     4
#define COLUMN                      5

#define DLG_OK                      1
#define DLG_CANCEL                 -1

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>
#include <shlwapi.h>
#include <stdio.h>
#include <tchar.h>
#include <ctime>
#include <sys/stat.h>
#include <locale.h>

#include "sqlite3.h"

extern sqlite3 *db;
extern HWND  hMainWnd, hToolbarWnd, hStatusWnd, hTreeWnd, hEditorWnd, hTabWnd, hSortingResultWnd;

extern HTREEITEM treeItems[5]; // 0 - current
extern HMENU treeMenus[6]; // 0 - add/refresh menu
extern TCHAR treeEditName[255];
extern TCHAR editTableData16[255]; // filled on DataEdit Dialog
extern TCHAR searchString[255];

extern const char *TYPES8[5];
extern const TCHAR *TYPES16[5];
extern const TCHAR *TYPES16u[5];
extern const TCHAR *TYPES16p[5];

extern HFONT hDefFont;
bool CALLBACK cbEnumChildren (HWND hWnd, LPARAM action);

int CALLBACK cbListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

struct ListViewCell {
	HWND hListWnd;
	int iItem;
	int iSubItem;
};
extern ListViewCell currCell;

extern HMENU hEditDataMenu, hResultMenu;

void setEditorFont(HWND hWnd);
void setTreeFont(HWND hWnd);

void processHightlight(HWND hEditorWnd, bool isRequireHighligth, bool isRequireParenthesisHighligth);
bool processEditorKey(MSGFILTER* pF);
bool processAutoComplete(HWND hParent, int key, bool isKeyDown);
TCHAR* getWordFromCursor(HWND hWnd, int pos = -1);

bool executeCommandQuery(const TCHAR* query);
int setListViewData(HWND hListWnd, sqlite3_stmt *stmt);
bool sortListView(HWND hListWnd, int colNo);
LRESULT onListViewMenu(int cmd);
TCHAR* getDbValue(const TCHAR* query16);
TCHAR* getDDL(TCHAR* name16, int type);
bool isQueryValid(const char* query);
bool showDbError(HWND hWnd, char* err8 = NULL);

#endif
