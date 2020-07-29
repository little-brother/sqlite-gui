#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define _WIN32_IE	0x0500

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
#include <stdio.h>
#include <tchar.h>
#include <ctime>
#include <sys/stat.h>
#include <locale.h>

#include "sqlite3.h"

extern sqlite3 *db;
extern HWND  hMainWnd, hToolbarWnd, hStatusWnd, hTreeWnd, hEditorWnd, hTabWnd, hSortingResultWnd, hAutoComplete;

extern HTREEITEM treeItems[5]; // 0 - current
extern HMENU treeMenus[6]; // 0 - add/refresh menu
extern TCHAR treeEditName[255];
extern TCHAR editTableData16[255]; // filled on DataEdit Dialog

extern const char *TYPES8[5];
extern const TCHAR *TYPES16[5];
extern const TCHAR *TYPES16u[5];
extern const TCHAR *TYPES16p[5];

extern HFONT hDefFont;
bool CALLBACK cbEnumChildren (HWND hWnd, LPARAM action);

struct ListViewCell {
   HWND hListWnd;
   int iItem;
   int iSubItem;
};
extern ListViewCell currCell;

extern HMENU hEditDataMenu;

void setEditorFont(HWND hWnd);
void setTreeFont(HWND hWnd);

void updateHighlighting(HWND hEditorWnd);
bool processAutoComplete(MSGFILTER* pF);

bool executeCommandQuery(const TCHAR* query);
int setListViewData(HWND hListWnd, sqlite3_stmt *stmt);
TCHAR* getDbValue(const TCHAR* query16);
TCHAR* getDDL(TCHAR* name16, int type);
#endif
