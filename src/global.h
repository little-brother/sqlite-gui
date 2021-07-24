#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define MAX_TAB_COUNT               16
#define MAX_RESULT_COUNT            32
#define MAX_ENTITY_COUNT          1024
#define MAX_TEXT_LENGTH          32000
#define MAX_TOOLTIP_LENGTH        1024
#define MAX_REFCOLUMN_COUNT        128
#define MAX_CHART_COLOR_COUNT       10
#define MAX_DIALOG_COUNT            32
#define MAX_COMPARE_RESULT          50

#define DLG_OK                      1
#define DLG_CANCEL                 -1
#define DLG_DELETE                  2

#define ACTION_SETDEFFONT           1
#define ACTION_DESTROY              2
#define ACTION_RESIZETAB            3
#define ACTION_UPDATETAB            4
#define ACTION_REDRAW               5

#define ROW_VIEW                    0
#define ROW_EDIT                    1
#define ROW_ADD                     2

#define TABLE                       1
#define VIEW                        2
#define INDEX                       3
#define TRIGGER                     4
#define COLUMN                      5

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

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
#include <math.h>

#include "missing.h"
#include "sqlite3.h"

extern sqlite3 *db;
extern HWND  hMainWnd, hTabWnd, hSortingResultWnd, hTooltipWnd;
extern HMENU hBlobMenu, hEditorMenu;

extern TCHAR editTableData16[255]; // filled on DataEdit Dialog
extern TCHAR searchString[255];

extern const char *TYPES8[6];
extern const TCHAR *TYPES16[6];
extern const TCHAR *TYPES16u[6];
extern const TCHAR *TYPES16p[6];

extern COLORREF GRIDCOLORS[8];

extern HFONT hDefFont;
extern WNDPROC cbOldListView, cbOldEdit;
LRESULT CALLBACK cbNewListView(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool CALLBACK cbEnumChildren (HWND hWnd, LPARAM action);
int CALLBACK cbListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

void setEditorFont(HWND hWnd);
void setEditorColor(HWND hWnd, COLORREF color, bool noEffects = false);
void setTreeFont(HWND hWnd);

bool attachDb(sqlite3** _db, const char* path8, const char* name8 = 0);
bool search(HWND hWnd);
void processHighlight(HWND hWnd, bool isRequireHighligth, bool isRequireParenthesisHighligth, bool isRequireOccurrenceHighlight);
bool processEditorEvents(MSGFILTER* pF);
bool processAutoComplete(HWND hParent, int key, bool isKeyDown);
bool processEditKeys(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
TCHAR* getWordFromCursor(HWND hWnd, bool isTable, int pos = -1);
bool toggleWordWrap(HWND hEditorWnd);
bool toggleTextCase (HWND hEditorWnd);
bool toggleComment (HWND hEditorWnd);
bool pasteText (HWND hEditorWnd);
void fixQuoteSelection(HWND hEditorWnd, SELCHANGE* pSc);
void switchDialog(HWND hDlg, bool isNext);
void showTooltip(int x, int y, TCHAR* text16);

int Toolbar_SetButtonState(HWND hToolbar, int id, byte state, LPARAM lParam = 0);
int ListView_SetData(HWND hListWnd, sqlite3_stmt *stmt, bool isRef = false);
int ListView_ShowRef(HWND hListWnd, int rowNo, int colNo);
int ListView_Sort(HWND hListWnd, int colNo);
int ListView_Reset(HWND hListWnd);
int ListView_GetColumnCount(HWND hListWnd);
int Header_GetItemText(HWND hWnd, int i, TCHAR* pszText, int cchTextMax);
int TabCtrl_GetItemText(HWND hWnd, int iItem, TCHAR* pszText, int cchTextMax);
LRESULT onListViewMenu(HWND hListWnd, int rowNo, int colNo, int cmd, bool ignoreLastColumn = false);
TCHAR* getDDL(TCHAR* name16, int type, bool withDrop = false);
bool showDbError(HWND hWnd);

#endif
