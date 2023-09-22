#ifndef __GLOBAL_H__
#define __GLOBAL_H__

// missing
#define LVS_EX_AUTOSIZECOLUMNS      0x10000000

#define MAX_TAB_COUNT               16
#define MAX_RESULT_COUNT            32
#define MAX_ENTITY_COUNT          1024
#define MAX_TEXT_LENGTH          32000
#define MAX_TOOLTIP_LENGTH        1024
#define MAX_REFCOLUMN_COUNT        128
#define MAX_CHART_COLOR_COUNT       10
#define MAX_DIALOG_COUNT            32
#define MAX_COMPARE_RESULT          50
#define MAX_SCHEMA_COUNT            20
#define MAX_RESULT_COLUMN_COUNT     96
#define MAX_TRANSPOSE_ROWS         512

#define DLG_OK                      1
#define DLG_CANCEL                 -1
#define DLG_DELETE                  2

#define ACTION_SETFONT              1
#define ACTION_SETDEFFONT           2
#define ACTION_DESTROY              3
#define ACTION_RESIZETAB            4
#define ACTION_UPDATETAB            5
#define ACTION_REDRAW               6

#define ROW_VIEW                    0
#define ROW_EDIT                    1
#define ROW_ADD                     2

#define TABLE                       1
#define VIEW                        2
#define INDEX                       3
#define TRIGGER                     4
#define COLUMN                      5

#define EDITOR_HIGHLIGHT            TEXT("HIGHLIGHT")
#define EDITOR_PARENTHESIS          TEXT("PARENTHESIS")
#define EDITOR_OCCURRENCE           TEXT("OCCURRENCE")
#define EDITOR_HASOCCURRENCE        TEXT("HASOCCURRENCE")
#define EDITOR_SELECTION_START      TEXT("SELECTION_START")

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>
#include <shlwapi.h>
#include <gdiplus.h>
#include <stdio.h>
#include <tchar.h>
#include <ctime>
#include <sys/stat.h>
#include <locale.h>
#include <math.h>

#include "sqlite3.h"

extern sqlite3 *db;
extern HWND  hMainWnd;
extern HMENU hBlobMenu, hEditorMenu;

extern TCHAR searchString[255];

extern const char *TYPES8[6];
extern const TCHAR *TYPES16[6];
extern const TCHAR *TYPES16u[6];
extern const TCHAR *TYPES16p[6];

extern COLORREF GRIDCOLORS[8];

extern HFONT hDefFont;
extern HFONT hFont;
extern HPEN hCurrentCellPen;

LRESULT CALLBACK cbNewListView(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool CALLBACK cbEnumChildren (HWND hWnd, LPARAM action);
int CALLBACK cbListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

void setEditorFont(HWND hWnd);
void setEditorColor(HWND hWnd, COLORREF color, bool noEffects = false);

bool attachDb(sqlite3** _db, const char* path8, const char* name8 = 0);
bool search(HWND hWnd);
void processHighlight(HWND hWnd, bool isRequireHighlight, bool isRequireParenthesisHighlight, bool isRequireOccurrenceHighlight);
bool processEditorEvents(MSGFILTER* pF);
bool processAutoComplete(HWND hParent, int key, bool isKeyDown);
bool processEditKeys(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
TCHAR* getCurrentText(HWND hWnd);
bool wrapSelectedText(HWND hEditorWnd, int key);
bool toggleWordWrap(HWND hEditorWnd);
bool toggleTextCase (HWND hEditorWnd);
bool toggleComment (HWND hEditorWnd);
bool pasteText (HWND hEditorWnd, bool detectCSV = false);
bool formatQuery (HWND hEditorWnd);
void switchDialog(HWND hDlg, bool isNext);
void createTooltip(HWND hWnd);
void showTooltip(int x, int y, TCHAR* text16);
void hideTooltip();

bool openBlobAsFile(const unsigned char* data, int size, bool isTxt = false);

int Toolbar_SetButtonState(HWND hToolbar, int id, byte state, LPARAM lParam = 0);
int ListView_SetData(HWND hListWnd, sqlite3_stmt *stmt, bool isRef = false);
int ListView_ShowRef(HWND hListWnd, int rowNo, int colNo);
int ListView_Sort(HWND hListWnd, int colNo);
int ListView_Reset(HWND hListWnd);
int ListView_GetColumnCount(HWND hListWnd);
int Header_GetItemText(HWND hWnd, int i, TCHAR* pszText, int cchTextMax);
int Header_SetItemText(HWND hWnd, int i, TCHAR* pszText);
void Menu_SetItemText(HMENU hMenu, UINT wID, const TCHAR* caption);
void Menu_SetItemState(HMENU hMenu, UINT wID, UINT fState);
void Menu_InsertItem(HMENU hMenu, UINT uPosition, UINT wID, UINT fState, const TCHAR* pszText);
void Menu_SetData(HMENU hMenu, ULONG_PTR data);
ULONG_PTR Menu_GetData(HMENU hMenu);

COLORREF RichEdit_GetTextColor (HWND hWnd, int pos);
int TabCtrl_GetItemText(HWND hWnd, int iItem, TCHAR* pszText, int cchTextMax);
LRESULT onListViewMenu(HWND hListWnd, int rowNo, int colNo, int cmd, bool ignoreLastColumn = false);
TCHAR* getDDL(const TCHAR* schema16, const TCHAR* name16, int type, bool withDrop = false);
bool showDbError(HWND hWnd);

#endif
