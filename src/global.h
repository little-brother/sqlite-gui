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
#define MAX_SCHEMA_COUNT            20
#define MAX_RESULT_COLUMN_COUNT     96
#define MAX_TRANSPOSE_ROWS         512
#define MAX_PLUGIN_COUNT           128
#define MAX_MODIFIER_OUTPUT_LEN   1024
#define MAX_MODIFIER_ERROR_LEN     255

#define DLG_OK                      1
#define DLG_CANCEL                 -1
#define DLG_DELETE                  2

#define ACTION_SETFONT              1
#define ACTION_SETMENUFONT          2
#define ACTION_SETPARENTFONT        3
#define ACTION_DESTROY              4
#define ACTION_RESIZETAB            5
#define ACTION_UPDATETAB            6
#define ACTION_REDRAW               7
#define ACTION_SET_THEME            8
#define ACTION_RESET_MODIFIERS      9

#define LOGGER_HIGHLIGHT            1
#define LOGGER_OCCURRENCE           2
#define LOGGER_PARENTHESIS          3
#define LOGGER_FORMAT               4

#define ROW_VIEW                    0
#define ROW_EDIT                    1
#define ROW_ADD                     2

#define TABLE                       1
#define VIEW                        2
#define INDEX                       3
#define TRIGGER                     4
#define COLUMN                      5

#define ADDON_SQLITE_EXTENSION      0
#define ADDON_VALUE_VIEWER          1
#define ADDON_COLUMN_MODIFIER       2

#define EDITOR_HIGHLIGHT            TEXT("HIGHLIGHT")
#define EDITOR_PARENTHESIS          TEXT("PARENTHESIS")
#define EDITOR_OCCURRENCE           TEXT("OCCURRENCE")
#define EDITOR_HASOCCURRENCE        TEXT("HASOCCURRENCE")
#define EDITOR_SELECTION_START      TEXT("SELECTION_START")

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CLAMP(x, lower, upper) (MIN(upper, MAX(x, lower)))

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
#include <uxtheme.h>

#include "sqlite3.h"

extern sqlite3 *db;
extern HWND  hMainWnd, hLoggerWnd;
extern HMENU hEditorMenu, hPreviewMenu;

extern TCHAR searchString[255];
extern TCHAR APP_PATH[MAX_PATH];
extern TCHAR TMP_PATH[MAX_PATH];

extern const char *TYPES8[6];
extern const TCHAR *TYPES16[6];
extern const TCHAR *TYPES16u[6];
extern const TCHAR *TYPES16p[6];
extern const TCHAR* ADDON_EXTS16[3];

extern COLORREF GRIDCOLORS[8];
extern HFONT hFont;
extern HFONT hMenuFont;
extern HIMAGELIST hIconsImageList;

typedef HWND (WINAPI *pluginView)(HWND hPreviewWnd, const unsigned char* data, int dataLen, int dataType, TCHAR* outInfo16, TCHAR* outExtension16);
typedef int (WINAPI* pluginClose)(HWND hPluginWnd);
typedef BOOL (WINAPI* pluginActivate)(HWND hListWnd, int colNo, TCHAR* err16);
typedef BOOL (WINAPI* pluginDeactivate)(HWND hListWnd, int colNo);
typedef BOOL (WINAPI *pluginSetText)(HWND hListWnd, int colNo, const unsigned char* data, int dataLen, int dataType, TCHAR* output16);
typedef BOOL (WINAPI *pluginSetColor)(NMLVCUSTOMDRAW* pCustomDraw, const unsigned char* data, int dataLen, int dataType);
typedef BOOL (WINAPI *pluginRender)(NMLVCUSTOMDRAW* pCustomDraw, const unsigned char* data, int dataLen, int dataType);
typedef int (WINAPI* pluginGetPriority)();

struct TPlugin {
	HMODULE hModule;
	pluginView view;
	pluginClose close;
	pluginActivate activate;
	pluginDeactivate deactivate;
	pluginSetText setText;
	pluginSetColor setColor;
	pluginRender render;
	int type;
	int priority;
	TCHAR name[256];
};
extern TPlugin plugins[MAX_PLUGIN_COUNT + 1];

typedef struct TDlgParam {
	const TCHAR* s1;
	const TCHAR* s2;
	const TCHAR* s3;
	const TCHAR* s4;
} TDlgParam;

typedef struct TValue {
	const unsigned char* data;
	int dataLen;
	int dataType;
	LONG_PTR extra;
} TValue;

LRESULT CALLBACK cbNewListView(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool CALLBACK cbEnumChildren (HWND hWnd, LPARAM action);
bool CALLBACK cbEnumFixEditHeights (HWND hWnd, LPARAM height);
int CALLBACK cbListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

void setEditorPadding(HWND hWnd);
void setEditorFont(HWND hWnd);
void setEditorColor(HWND hWnd, COLORREF color, bool noEffects = false);

bool attachDb(sqlite3** _db, const char* path8, const char* name8 = 0);
bool doEditorSearch(HWND hWnd, bool isBackward);
void processHighlight(HWND hWnd, bool isRequireHighlight, bool isRequireParenthesisHighlight, bool isRequireOccurrenceHighlight);
bool processEditorEvents(MSGFILTER* pF);
bool processAutoComplete(HWND hParent, int key, bool isKeyDown);
bool processEditKeys(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
TCHAR* getCurrentText(HWND hWnd);
TCHAR* getEditorText(HWND hWnd);
void setEditorText(HWND hWnd, TCHAR* text16);
bool wrapSelectedText(HWND hEditorWnd, int key);
bool toggleWordWrap(HWND hEditorWnd);
bool toggleTextCase (HWND hEditorWnd);
bool toggleComment (HWND hEditorWnd);
bool pasteText (HWND hEditorWnd, bool detectCSV = false);
TCHAR* formatQuery (TCHAR* query16);
HWND openDialog(int IDD, DLGPROC proc, LPARAM lParam = 0);
void switchDialog(HWND hDlg, bool isNext);
void createTooltip(HWND hWnd);
void showTooltip(int x, int y, TCHAR* text16);
void showPopup(const TCHAR* text16, HWND hParentWnd = hMainWnd);
void hideTooltip();
void logger(ULONG_PTR type, TCHAR* msg16);

int aiGetSid();
TCHAR* aiRequest(const TCHAR* promptId, const TCHAR* arg1, const TCHAR* arg2 = 0, int sid = 0);

int Toolbar_SetButtonState(HWND hToolbar, int id, byte state, LPARAM lParam = 0);
DWORD_PTR Toolbar_GetButtonData(HWND hToolbar, int id);
LPARAM TreeView_GetItemParam (HWND hTreeWnd, HTREEITEM hItem);
int ListView_SetData(HWND hListWnd, sqlite3_stmt *stmt, bool isRef = false);
int ListView_ShowRef(HWND hListWnd, int rowNo, int colNo);
BOOL ListView_GetItemValue(HWND hListWnd, int rowNo, int colNo, TValue* cell, bool ignoreResultset = false);
TCHAR* ListView_GetItemValueText(HWND hListWnd, int rowNo, int colNo, bool ignoreResultset = false);
int ListView_Sort(HWND hListWnd, int colNo);
int ListView_Reset(HWND hListWnd);
int ListView_GetColumnCount(HWND hListWnd);
int Header_GetItemText(HWND hWnd, int i, TCHAR* pszText, int cchTextMax);
int Header_SetItemText(HWND hWnd, int i, TCHAR* pszText);
int Menu_GetItemPositionByName(HMENU hMenu, const TCHAR* itemName);
BOOL Menu_SetItemText(HMENU hMenu, UINT wID, const TCHAR* caption);
BOOL Menu_SetItemState(HMENU hMenu, UINT wID, UINT fState);
BOOL Menu_SetItemStateByPosition(HMENU hMenu, UINT pos, UINT fState);
BOOL Menu_InsertItem(HMENU hMenu, UINT uPosition, UINT wID, UINT fState, const TCHAR* pszText);
BOOL Menu_SetData(HMENU hMenu, ULONG_PTR data);
ULONG_PTR Menu_GetData(HMENU hMenu);
BOOL Menu_SetItemData(HMENU hMenu, UINT wID, ULONG_PTR lParam);
ULONG_PTR Menu_GetItemData(HMENU hMenu, UINT wID);

COLORREF RichEdit_GetTextColor (HWND hWnd, int pos);
int TabCtrl_GetItemText(HWND hWnd, int iItem, TCHAR* pszText, int cchTextMax);
LRESULT onListViewMenu(HWND hListWnd, int rowNo, int colNo, int cmd, bool ignoreLastColumn = false);
TCHAR* getDDL(const TCHAR* schema16, const TCHAR* name16, int type, int mode = 0);
bool showDbError(HWND hWnd);
#endif
