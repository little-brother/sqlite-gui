#include "global.h"
#include "resource.h"
#include "prefs.h"
#include "http.h"
#include "utils.h"
#include "dbutils.h"
#include "tools.h"
#include "dialogs.h"

#include <process.h>

#include "tom.h"
#include <richole.h>
#include <unknwn.h>
#include "dmp.h"

#define DEFINE_GUIDXXX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) EXTERN_C const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
DEFINE_GUIDXXX(IID_ITextDocument, 0x8CC497C0, 0xA1DF, 0x11CE, 0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D);

#define SB_SQLITE_VERSION 0
#define SB_GUI_VERSION    1
#define SB_CARET_POSITION 2
#define SB_TRANSACTION    3
#define SB_ELAPSED_TIME   4
#define SB_SELECTED_ROW   5
#define SB_RESULTSET      6
#define SB_AI_SETTINGS    7
#define SB_EXTENSIONS     8

#define EQ_PLAN     1   /* 0001 */
#define EQ_BATCH    2   /* 0010 */
#define EQ_CURRENT  4   /* 0100 */
#define EQ_EXPLAIN  8   /* 1000 */

#define SPLITTER_X  1
#define SPLITTER_Y  2
#define SPLITTER_C  3

const char *TYPES8[6] = {"current", "table", "view", "index", "trigger", "column"};
const TCHAR *TYPES16[6] = {TEXT("current"), TEXT("table"), TEXT("view"), TEXT("index"), TEXT("trigger"), TEXT("column")};
const TCHAR *TYPES16u[6] = {TEXT("CURRENT"), TEXT("TABLE"), TEXT("VIEW"), TEXT("INDEX"), TEXT("TRIGGER"), TEXT("COLUMN")};
const TCHAR *TYPES16p[6] = {TEXT(""), TEXT("Tables"), TEXT("Views"), TEXT("Indexes"), TEXT("Triggers"), TEXT("Columns")};
const TCHAR *transactionStates[] = {TEXT(""),TEXT(" TRN")};
const TCHAR* ADDON_EXTS16[3] = {TEXT("dll"), TEXT("vvp"), TEXT("cmp")};
COLORREF GRIDCOLORS[8]{0}; // 0 = current cell

// AutoComplete
const TCHAR *SQL_KEYWORDS[] = {TEXT("abort"), TEXT("action"), TEXT("add"), TEXT("after"), TEXT("all"), TEXT("alter"), TEXT("always"), TEXT("analyze"), TEXT("and"), TEXT("as"), TEXT("asc"), TEXT("attach"), TEXT("autoincrement"), TEXT("before"), TEXT("begin"), TEXT("between"), TEXT("by"), TEXT("cascade"), TEXT("case"), TEXT("cast"), TEXT("check"), TEXT("collate"), TEXT("column"), TEXT("commit"), TEXT("conflict"), TEXT("constraint"), TEXT("create"), TEXT("cross"), TEXT("current"), TEXT("current_date"), TEXT("current_time"), TEXT("current_timestamp"), TEXT("database"), TEXT("default"), TEXT("deferrable"), TEXT("deferred"), TEXT("delete"), TEXT("desc"), TEXT("detach"), TEXT("distinct"), TEXT("do"), TEXT("drop"), TEXT("each"), TEXT("else"), TEXT("end"), TEXT("escape"), TEXT("except"), TEXT("exclude"), TEXT("exclusive"), TEXT("exists"), TEXT("explain"), TEXT("fail"), TEXT("filter"), TEXT("first"), TEXT("following"), TEXT("for"), TEXT("foreign"), TEXT("from"), TEXT("full"), TEXT("generated"), TEXT("glob"), TEXT("group"), TEXT("groups"), TEXT("having"), TEXT("if"), TEXT("ignore"), TEXT("immediate"), TEXT("in"), TEXT("index"), TEXT("indexed"), TEXT("initially"), TEXT("inner"), TEXT("insert"), TEXT("instead"), TEXT("intersect"), TEXT("into"), TEXT("is"), TEXT("isnull"), TEXT("join"), TEXT("key"), TEXT("last"), TEXT("left"), TEXT("like"), TEXT("limit"), TEXT("match"), TEXT("natural"), TEXT("no"), TEXT("not"), TEXT("nothing"), TEXT("notnull"), TEXT("null"), TEXT("nulls"), TEXT("of"), TEXT("offset"), TEXT("on"), TEXT("or"), TEXT("order"), TEXT("others"), TEXT("outer"), TEXT("over"), TEXT("partition"), TEXT("plan"), TEXT("pragma"), TEXT("preceding"), TEXT("primary"), TEXT("query"), TEXT("raise"), TEXT("range"), TEXT("recursive"), TEXT("references"), TEXT("regexp"), TEXT("reindex"), TEXT("release"), TEXT("rename"), TEXT("replace"), TEXT("restrict"), TEXT("right"), TEXT("rollback"), TEXT("row"), TEXT("rows"), TEXT("savepoint"), TEXT("select"), TEXT("set"), TEXT("table"), TEXT("temp"), TEXT("temporary"), TEXT("then"), TEXT("ties"), TEXT("to"), TEXT("transaction"), TEXT("trigger"), TEXT("unbounded"), TEXT("union"), TEXT("unique"), TEXT("update"), TEXT("using"), TEXT("vacuum"), TEXT("values"), TEXT("view"), TEXT("virtual"), TEXT("when"), TEXT("where"), TEXT("window"), TEXT("with"), TEXT("without"), TEXT("returning"), TEXT("materialized"), 0};
const TCHAR *TEMPLATES[] = {TEXT("select * from"), TEXT("select count(*) from"), TEXT("insert into"), TEXT("delete from"), TEXT("inner join"), TEXT("primary key"), TEXT("foreign key"), TEXT("case when  then  else  end"), TEXT("create table"), TEXT("create temp table"), TEXT("if not exists"), TEXT("update or ignore"), TEXT("update or replace"), 0};
TCHAR* PRAGMAS[MAX_ENTITY_COUNT] = {0};
TCHAR* FUNCTIONS[MAX_ENTITY_COUNT] = {0};
TCHAR* MODULES[MAX_ENTITY_COUNT] = {0};
TCHAR* TABLES[MAX_ENTITY_COUNT] = {0};

sqlite3* db;
HWND hMainWnd = 0, hToolbarWnd, hEditorSearchWnd, hStatusWnd, hTreeWnd, hTreeSearchWnd, hSchemaWnd, hEditorWnd, hTabWnd, hMainTabWnd, hLoggerWnd = 0,
hTooltipWnd = 0, hAutoComplete, hAutoCompleteHelp, hDragWnd = 0, hDialog, hSortingResultWnd, hAiChatWnd;
HMENU hMainMenu, hSchemaMenu, hDbMenu, hEditorMenu, hResultMenu, hModifierMenu, hTabResultMenu, hCliMenu, hPreviewMenu, hAiMenu, hAiChatMenu;
HWND hDialogs[MAX_DIALOG_COUNT]{0};
HWND hEditors[MAX_DIALOG_COUNT + MAX_TAB_COUNT]{0};

TPlugin plugins[MAX_PLUGIN_COUNT + 1] = {0};

HTREEITEM treeItems[5]; // 0 - current
TCHAR treeEditName[255];
HMENU treeMenus[IDC_MENU_TEMP + 1]{0};

TCHAR searchString[255]{0};
TCHAR resultSearchString[255]{0};
DWORD doubleClickTick = 0; // issue #68

HFONT hFont = 0; // TreeView/ListView/Richedit
HFONT hMenuFont = 0;
HACCEL hAccel = LoadAccelerators(0, MAKEINTRESOURCE(IDA_ACCEL));
HACCEL hDlgAccel = LoadAccelerators(0, MAKEINTRESOURCE(IDA_ACCEL2));
int editorNumWidth = 50;
HBRUSH hEditorNumBrush = CreateSolidBrush(RGB(245, 245, 250));

HICON hIcons[2] = {LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_LOGO)), LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_LOGO2))};
HIMAGELIST hIconsImageList = ImageList_LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_ICONS), 0, 0, RGB(255,255,255));

struct TEditorTab {
	int id; // unique tab identificator
	HWND hEditorWnd;
	HWND hAIWnd;
	HWND hTabWnd; // Use WMU_GET_CURRENT_RESULTSET to get a current ListView for the tab. ListView.USERDATA = resultNo
	HWND hQueryListWnd;
	TCHAR* tabTooltips[MAX_RESULT_COUNT];
	TCHAR queryElapsedTimes[MAX_RESULT_COUNT][64];
	BOOL isPinned[MAX_RESULT_COUNT];

	sqlite3* db;
	HANDLE thread;
	bool isPlan;
	bool isExplain;
	bool isBatch;

	int aiSid;
	bool aiNeedStop;
};
TEditorTab tabs[MAX_TAB_COUNT] = {0};

struct TCliTab {
	HWND hEditorWnd;
	HWND hResultWnd;

	sqlite3* db;
	HANDLE thread;
	bool isPlan;
};
TCliTab cli{0};

// storage for cell who triggered context menu; IDM_RESULT_COPY_CELL, IDM_RESULT_COPY_ROW
struct ListViewCell {
	HWND hListWnd;
	int iItem;
	int iSubItem;
};
ListViewCell currCell;

int currParenthesisPos[] = {-1, -1};

TCHAR APP_PATH[MAX_PATH]{0};
TCHAR TMP_PATH[MAX_PATH]{0};

int executeCLIQuery(bool isPlan = false);
int executeEditorQuery(char flags = 0, int vkKey = 0);
void suggestCLIQuery(int key);
void loadCLIResults(int cnt);

bool openConnection(sqlite3** _db, const char* path8, bool isReadOnly = false);
bool openDb(const TCHAR* path, bool isReadOnly = false, bool isReloadExtensions = false);
bool closeDb(bool isReloadExtensions = false);
bool closeConnections();
bool attachOdbcDb(sqlite3* conn, const TCHAR* alias);
bool isDbBusy();
void enableMainMenu();
void disableMainMenu();
void reloadPlugins(int type = 0);
void updatePlugins(int type = 0);
void updateExecuteMenu(bool isEnable);
void updateRecentList();
void updateTables(const TCHAR* schema16);
void updateAttachedRecentList(bool isODBC = false);
bool updateReferences();
void updateCustomFunction(bool unreg, sqlite3* conn = NULL);
void updateTree(int type = 0, TCHAR* select = NULL);
void updateTransactionState();
void switchSchemaTo(TCHAR* schema);
void reloadMeta(TCHAR** arr, const char* query8);
bool loadExtensions(sqlite3* db, sqlite3* conn);
int enableDbObject(const char* name8, int type);
int disableDbObject(const char* name8, int type);
void applyDbPrefs(sqlite3* _db = NULL);
bool isObjectPinned(const TCHAR* name16);
void saveQuery(const char* storage, const char* query);
unsigned int __stdcall checkUpdate (void* data);
bool ListView_DrillDown(HWND hListWnd, int rowNo, int colNo);
bool ListView_DrillUp(HWND hListWnd);
void createResultControls(HWND hTabWnd, int resultNo);
HWND createResultList(HWND hParentWnd, int resultNo);
bool saveResultToTable(sqlite3* db, const TCHAR* table16, int tabNo, int resultNo, int searchNext = LVNI_ALL);
bool startHttpServer();
bool stopHttpServer();
HFONT loadFont();
TCHAR* getHelp(TCHAR* word, TCHAR* altword = 0);

WNDPROC cbOldResultTabFilterEdit;
LRESULT CALLBACK cbNewEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewTree(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewTreeItemEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewTreeSearch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewSchema(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewEditorSearch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewEditorSearchString(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewMainTab(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewMainTabRename(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewStatusBar(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewResultHeader(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewResultTab(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewResultTabFilterEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewAutoComplete(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewAutoCompleteHelp(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewAiChat(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbNewListView(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cbMainWindow (HWND, UINT, WPARAM, LPARAM);

bool isCipherSupport = GetProcAddress(GetModuleHandle(TEXT("sqlite3.dll")), "sqlite3mc_config") != NULL;
bool isInjaSupport = false;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// setlocale(LC_ALL, "");

	GetModuleFileName(0, APP_PATH, MAX_PATH);
	PathRemoveFileSpec(APP_PATH);

	GetTempPath(MAX_PATH, TMP_PATH);
	_tcscat(TMP_PATH, TEXT("sqlite-gui"));
	CreateDirectory(TMP_PATH, NULL);

	SetProcessDPIAware();

	TCHAR prefPath16[MAX_PATH + 1]{0};

	int nArgs = 0;
	TCHAR** args = CommandLineToArgvW(GetCommandLine(), &nArgs);

	int dbArgNo = 0;
	int tableArgNo = 0;
	int profileArgNo = 0;
	int exportArgNo = 0;
	int importArgNo = 0;
	int readOnlyArgNo = 0;
	for (int argNo = 1; argNo < nArgs; argNo++) {
		if (_tcscmp(args[argNo], TEXT("--profile")) == 0)
			profileArgNo = argNo;

		if (_tcscmp(args[argNo], TEXT("--csv-import")) == 0)
			importArgNo = argNo;

		if (_tcscmp(args[argNo], TEXT("--csv-export")) == 0)
			exportArgNo = argNo;

		if (_tcscmp(args[argNo], TEXT("--readonly")) == 0)
			readOnlyArgNo = argNo;
	}

	for (int argNo = 1; argNo < nArgs && !dbArgNo; argNo++) {
		if (argNo == readOnlyArgNo) {
			argNo += 0;
		} else if (argNo == profileArgNo) {
			argNo += 1;
		} else if (argNo == importArgNo || argNo == exportArgNo) {
			argNo += 2;
		} else {
			dbArgNo = argNo;
		}
	}

	tableArgNo = dbArgNo && (dbArgNo + 1 < nArgs) && args[dbArgNo + 1][0] != TEXT('-') ? dbArgNo + 1 : 0;

	if (profileArgNo && profileArgNo < nArgs) {
		_tcscpy(prefPath16, args[profileArgNo + 1]);
	} else {
		_tcscpy(prefPath16, APP_PATH);
		_tcscat(prefPath16, TEXT("\\prefs.sqlite"));
	}
	char* prefPath8 = utils::utf16to8(prefPath16);

	bool isFirstRun = !utils::isFileExists(prefPath16);
	if (!prefs::load(prefPath8)) {
		MessageBox(0, TEXT("Settings loading failed"), TEXT("Error"), MB_OK);
		return EXIT_FAILURE;
	}
	delete [] prefPath8;

	GRIDCOLORS[0] = prefs::get("color-current-cell");
	GRIDCOLORS[SQLITE_NULL] = prefs::get("color-null");
	GRIDCOLORS[SQLITE_BLOB] = prefs::get("color-blob");
	GRIDCOLORS[SQLITE_FLOAT] = prefs::get("color-real");
	GRIDCOLORS[SQLITE_INTEGER] = prefs::get("color-integer");
	GRIDCOLORS[SQLITE_TEXT] = prefs::get("color-text");

	hEditorMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_EDITOR));
	hEditorMenu = GetSubMenu(hEditorMenu, 0);

	hResultMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_RESULT));
	hResultMenu = GetSubMenu(hResultMenu, 0);
	int menuPos = Menu_GetItemPositionByName(hResultMenu, TEXT("Column modifier"));
	if (menuPos != -1) {
		hModifierMenu = GetSubMenu(hResultMenu, menuPos);
	} else {
		MessageBox(hMainWnd, TEXT("Result menu bug"), 0, MB_OK);
	}

	hTabResultMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_TAB_RESULT));
	hTabResultMenu = GetSubMenu(hTabResultMenu, 0);

	hCliMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_CLI));
	hCliMenu = GetSubMenu(hCliMenu, 0);

	hPreviewMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_PREVIEW));
	hPreviewMenu = GetSubMenu(hPreviewMenu, 0);

	hAiMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_AI));
	hAiMenu = GetSubMenu(hAiMenu, 0);

	hAiChatMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_AI_CHAT));
	hAiChatMenu = GetSubMenu(hAiChatMenu, 0);

	hFont = loadFont();

	/*
	GetStockObject(DEFAULT_GUI_FONT) is obsolete
	https://stackoverflow.com/questions/40340580/using-default-gui-font-in-high-dpi-windows-application
	*/
	LOGFONT lf = {0};
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
	hMenuFont = CreateFontIndirect(&lf);

	LoadLibrary(TEXT("msftedit.dll"));

	if (dbArgNo && (tableArgNo || exportArgNo || importArgNo)) {
		if (openDb(args[dbArgNo], readOnlyArgNo)) {
			if (tableArgNo)
				DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_EDITDATA), hMainWnd, (DLGPROC)dialogs::cbDlgEditData, (LPARAM)args[tableArgNo]);

			if (importArgNo && (importArgNo + 2 < nArgs)) {
				TCHAR err16[1024];
				int rowCount = tools::importCSV(args[importArgNo + 1], args[importArgNo + 2], err16);
				if (rowCount != -1)
					_tprintf(TEXT("Done. Imported %i rows."), rowCount);
				else
					_tprintf(TEXT("%ls"), err16);
			}

			if (exportArgNo && (exportArgNo + 2 < nArgs)) {
				TCHAR err16[1024];
				int rowCount = tools::exportCSV(args[exportArgNo + 1], args[exportArgNo + 2], err16);
				if (rowCount != -1)
					_tprintf(TEXT("Done. Exported %i rows."), rowCount);
				else
					_tprintf(TEXT("%ls"), err16);
			}
		} else {
			_tprintf(TEXT("Error to open database: %ls\n"), args[dbArgNo]);
		}

		closeDb();
		sqlite3_shutdown();
		LocalFree(args);
		return 0;
	}

	MSG msg;
	WNDCLASSEX wc;

	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("sqlite-gui-class");
	wc.lpfnWndProc = cbMainWindow;
	wc.style = CS_DBLCLKS;
	wc.cbSize = sizeof (WNDCLASSEX);
	wc.hIcon = hIcons[0];
	wc.hIconSm = hIcons[0];
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.lpszMenuName = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH) COLOR_WINDOW;

	if (!RegisterClassEx (&wc))
		return EXIT_FAILURE;

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_DATE_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icex);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput{0};
	ULONG_PTR gdiplusToken = 0;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	if (prefs::get("backup-prefs")) {
		_sntprintf(prefPath16, MAX_PATH, TEXT("%ls/prefs.backup"), APP_PATH);
		DeleteFile(prefPath16);
		if (!prefs::backup())
			MessageBox(0, TEXT("Settings back up failed"), TEXT("Error"), MB_OK);
	}

	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);
	hMainWnd = CreateWindowEx (0, TEXT("sqlite-gui-class"), TEXT("sqlite-gui"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CLAMP(prefs::get("x"), 0, sw - 10), CLAMP(prefs::get("y"), 0, sh - 10), CLAMP(prefs::get("width"), 400, sw), CLAMP(prefs::get("height"), 300, sh),
		HWND_DESKTOP, 0, hInstance, NULL);

	hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_MAIN));
	ModifyMenu(hMainMenu, 4, MF_BYPOSITION | MFT_RIGHTJUSTIFY, 0, TEXT("?"));
	SetMenu(hMainWnd, hMainMenu);

	TBBUTTON tbButtons [ ] = {
		{0, IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Open")},
		{1, IDM_CLOSE, TBSTATE_INDETERMINATE, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Close")},
		{-1, IDM_SPAN, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0L, 0},
		{2, IDM_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Save")},
		{3, IDM_PLAN, TBSTATE_INDETERMINATE, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Plan")},
		{6, IDM_AI_EXPLAIN, TBSTATE_INDETERMINATE, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Explain")},
		{4, IDM_EXECUTE, TBSTATE_INDETERMINATE, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Execute")},
		{5, IDM_INTERRUPT, TBSTATE_HIDDEN, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Interrupt")},
		{-1, IDM_SPAN2, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0L, 0},
		{6, IDM_AI_CHAT, TBSTATE_ENABLED, TBSTYLE_CHECK | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("AI Chat")}

	};

	hToolbarWnd = CreateToolbarEx (hMainWnd, WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_CUSTOMERASE, IDC_TOOLBAR, 0, NULL, 0,
		tbButtons, sizeof(tbButtons)/sizeof(tbButtons[0]), 0, 0, 0, 0, sizeof (TBBUTTON));

	int iSize = GetSystemMetrics(SM_CXSMICON) <= 16 ? 16 : 24;
	int idc = iSize == 16 ? IDB_TOOLBAR16 : IDB_TOOLBAR24;
	HIMAGELIST tbImages = ImageList_LoadBitmap(GetModuleHandle (0), MAKEINTRESOURCE(idc), iSize, 0, RGB(255, 255, 255));
	SendMessage(hToolbarWnd, TB_SETIMAGELIST, 0, (LPARAM)tbImages);

	float z = utils::getWndScale(hMainWnd).x;
	hStatusWnd = CreateStatusWindow(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP | SBARS_TOOLTIPS, NULL, hMainWnd, IDC_STATUSBAR);
	int sizes[9] = {(int)(z * 80), (int)(z * 150), (int)(z * 215), (int)(z * 251), (int)(z * 365), (int)(z * 412), (int)(z * 482), (int)(z * 527), -1};
	SendMessage(hStatusWnd, SB_SETPARTS, 9, (LPARAM)&sizes);
	SetProp(hStatusWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hStatusWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewStatusBar));

	SendMessage(hStatusWnd, SB_SETTIPTEXT, SB_SQLITE_VERSION, (LPARAM)TEXT("SQLite library version"));
	SendMessage(hStatusWnd, SB_SETTIPTEXT, SB_GUI_VERSION, (LPARAM)TEXT("sqlite-gui version"));
	SendMessage(hStatusWnd, SB_SETTIPTEXT, SB_CARET_POSITION, (LPARAM)TEXT("Caret position in editor"));
	SendMessage(hStatusWnd, SB_SETTIPTEXT, SB_TRANSACTION, (LPARAM)TEXT("Current transaction status"));
	SendMessage(hStatusWnd, SB_SETTIPTEXT, SB_ELAPSED_TIME, (LPARAM)TEXT("Time spent on query execution"));
	SendMessage(hStatusWnd, SB_SETTIPTEXT, SB_SELECTED_ROW , (LPARAM)TEXT("Row number in resultset"));
	SendMessage(hStatusWnd, SB_SETTIPTEXT, SB_RESULTSET, (LPARAM)TEXT("Filtered/Total row numbers of resultset"));
	SendMessage(hStatusWnd, SB_SETTIPTEXT, SB_AI_SETTINGS, (LPARAM)TEXT("AI Settings"));
	SendMessage(hStatusWnd, SB_SETTIPTEXT, SB_EXTENSIONS, (LPARAM)TEXT("Loaded SQLite extensions"));

	char version8[32];
	sprintf(version8, " SQLite: %s", sqlite3_libversion());
	TCHAR* version16 = utils::utf8to16(version8);
	SendMessage(hStatusWnd, SB_SETTEXT, SB_SQLITE_VERSION, (LPARAM)version16);
	delete [] version16;
	TCHAR guiVersion[32]{0};
	_sntprintf(guiVersion, 31, TEXT(" GUI: %ls"), TEXT(GUI_VERSION));
	SendMessage(hStatusWnd, SB_SETTEXT, SB_GUI_VERSION, (LPARAM)guiVersion);

	hSchemaWnd = CreateWindowEx(0, WC_STATIC, TEXT("main"), WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | WS_BORDER | SS_NOTIFY, 0, 0, 100, 100, hMainWnd, (HMENU)IDC_SCHEMA, hInstance,  NULL);
	SetProp(hSchemaWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hSchemaWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewSchema));
	SendMessage(hSchemaWnd, WM_SETFONT, (LPARAM)hMenuFont, 0);

	hTreeWnd = CreateWindowEx(0, WC_TREEVIEW, NULL, WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | WS_DISABLED | TVS_EDITLABELS, 0, 0, 100, 100, hMainWnd, (HMENU)IDC_TREE, hInstance,  NULL);
	SetProp(hTreeWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hTreeWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewTree));
	DragAcceptFiles(hTreeWnd, TRUE);
	hTreeSearchWnd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("RICHEDIT50W"), NULL, WS_CHILD, 0, 0, 100, 100, hMainWnd, (HMENU)IDC_TREE_SEARCH, hInstance,  NULL);
	SendMessage(hTreeSearchWnd, WM_SETFONT, (LPARAM)hFont, 0);
	SetProp(hTreeSearchWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hTreeSearchWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewTreeSearch));
	SendMessage(hTreeSearchWnd, EM_SETEVENTMASK, 0, ENM_CHANGE);

	hMainTabWnd = CreateWindowEx(0, WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | SS_NOTIFY, 100, 0, 100, 100, hMainWnd, (HMENU)IDC_MAINTAB, hInstance,  NULL);
	createTooltip(hMainWnd);

	int span = 4 * z;
	int editH = 22 * z;
	hEditorSearchWnd = CreateWindowEx(0, WC_STATIC, NULL, WS_CHILD | SS_NOTIFY, 0, 0, 100, editH + 2 * span + 2 /* Bottom border */, hMainWnd, (HMENU)IDC_EDITOR_SEARCH, hInstance,  NULL);
	SetProp(hEditorSearchWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hEditorSearchWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewEditorSearch));
	HWND hSearchEdit = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_VISIBLE | WS_CHILD, 0, span, 200 * z, editH, hEditorSearchWnd, (HMENU)IDC_EDITOR_SEARCH_STRING, hInstance,  NULL);
	Edit_SetCueBannerText(hSearchEdit, TEXT("Search in editor"));
	SetProp(hSearchEdit, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hSearchEdit, GWLP_WNDPROC, (LONG_PTR)&cbNewEditorSearchString));
	SendMessage(hEditorSearchWnd, WM_SETFONT, (LPARAM)hMenuFont, 0);
	CreateWindowEx(0, WC_BUTTON, TEXT("\U000025B2"), WS_VISIBLE | WS_CHILD, 200 * z + span, span, editH, editH, hEditorSearchWnd, (HMENU)IDC_EDITOR_SEARCH_PREV, hInstance,  NULL);
	CreateWindowEx(0, WC_BUTTON, TEXT("\U000025BC"), WS_VISIBLE | WS_CHILD, 200 * z + span + editH + span, span, editH, editH, hEditorSearchWnd, (HMENU)IDC_EDITOR_SEARCH_NEXT, hInstance,  NULL);
	CreateWindowEx(0, WC_BUTTON, TEXT("Case sensitive"), WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 200 * z + span + editH + span + editH + 4 * span, span, 100 * z, editH, hEditorSearchWnd, (HMENU)IDC_DLG_CASE_SENSITIVE, hInstance,  NULL);
	CreateWindowEx(0, WC_STATIC, TEXT("x"), WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | SS_NOTIFY, span, 0, editH, editH, hEditorSearchWnd, (HMENU)IDC_EDITOR_SEARCH_CLOSE, hInstance,  NULL);
	EnumChildWindows(hEditorSearchWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETMENUFONT);

	SendMessage(hMainTabWnd, WM_SETFONT, (LPARAM)hMenuFont, 0);
	SetProp(hMainTabWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hMainTabWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewMainTab));
	for (int i = 0; i < (prefs::get("restore-editor") ? prefs::get("editor-tab-count") : 1); i++) {
		SendMessage(hMainTabWnd, WMU_TAB_ADD, 0, 0);
	}
	SendMessage(hMainTabWnd, WMU_TAB_SET_CURRENT, 0, 0);

	CreateWindow(WC_STATIC, NULL, WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)IDC_FUNCTION_CODES, GetModuleHandle(0),  NULL);

	cli.hEditorWnd = CreateWindowEx(0, TEXT("RICHEDIT50W"), NULL, WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP | ES_NOHIDESEL, 0, 0, 100, 100, hMainWnd, (HMENU)IDC_EDITOR, GetModuleHandle(0),  NULL);
	SetProp(cli.hEditorWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(cli.hEditorWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewEditor));
	SendMessage(cli.hEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_DROPFILES);
	SetWindowLongPtr(cli.hEditorWnd, GWLP_USERDATA, 0);
	SendMessage(cli.hEditorWnd, EM_EXLIMITTEXT, 0, 32767 * 10);
	if (prefs::get("word-wrap"))
		toggleWordWrap(cli.hEditorWnd);
	cli.hResultWnd = CreateWindowEx(WS_EX_STATICEDGE, TEXT("RICHEDIT50W"), NULL, WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP | ES_READONLY, 0, 0, 100, 100, hMainWnd, (HMENU)IDC_CLI_RESULT, GetModuleHandle(0),  NULL);
	SendMessage(cli.hResultWnd, EM_SETBKGNDCOLOR, 0, RGB(0, 0, 0));
	SendMessage(cli.hResultWnd, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(10, 10));
	CreateWindow(WC_LISTBOX, NULL, WS_CHILD, 0, 0, 150, 200, cli.hResultWnd, (HMENU)IDC_CLI_RAWDATA, GetModuleHandle(0), NULL);
	DragAcceptFiles(cli.hEditorWnd, true);

	hDbMenu = GetSubMenu(hMainMenu, 0);
	if (!isCipherSupport)
		RemoveMenu(GetSubMenu(hMainMenu, 0), IDM_ENCRYPTION, MF_BYCOMMAND);
	updateRecentList();
	updateAttachedRecentList();
	updateAttachedRecentList(true);

	HMENU hMenu;
	int idcs [] = {IDC_MENU_TABLEVIEW, IDC_MENU_INDEXTRIGGER, IDC_MENU_TABLE, IDC_MENU_VIEW, IDC_MENU_INDEX, IDC_MENU_TRIGGER, IDC_MENU_COLUMN, IDC_MENU_DISABLED, IDC_MENU_TEMP};
	for (int idc : idcs) {
		hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(idc));
		treeMenus[idc] = GetSubMenu(hMenu, 0);
	}

	if (dbArgNo) {
		openDb(args[dbArgNo], readOnlyArgNo);
	} else if (isFirstRun) {
		utils::alignDialog(hMainWnd, 0);
		PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_DEMODB_BOOKSTORE, 0), 0);
	} else if (prefs::get("restore-db")) {
		int recent = GetMenuItemID(hDbMenu, 4);
		if (recent && !(GetMenuState(hDbMenu, IDM_RECENT, MF_BYCOMMAND) & (MF_DISABLED | MF_GRAYED)))
			PostMessage(hMainWnd, WM_COMMAND, recent, 0);
	}

	if (prefs::get("restore-editor") && !isFirstRun) {
		char* text8 = prefs::get("editor-cli-text", "");
		TCHAR* text16 = utils::utf8to16(text8);
		SetWindowText(cli.hEditorWnd, text16);
		delete [] text8;
		delete [] text16;

		int tabCount = prefs::get("editor-tab-count");

		for (int tabNo = 0; tabNo < tabCount; tabNo++) {
			char key[60] = {0};
			sprintf(key, "editor-text-%i", tabNo);
			char* text8 = prefs::get(key, "");
			TCHAR* text16 = utils::utf8to16(text8);
			SetWindowText(tabs[tabNo].hEditorWnd, text16);
			delete [] text8;
			delete [] text16;

			TCHAR def[64] = {0};
			_sntprintf(def, 63, TEXT("Editor #%i"), tabNo + 1);
			sprintf(key, "editor-name-%i", tabNo);
			text8 = prefs::get(key, "");
			text16 = utils::utf8to16(text8);
			SendMessage(hMainTabWnd, WMU_TAB_SET_TEXT, tabNo, (LPARAM)(_tcslen(text16) ? text16: def));
			delete [] text8;
			delete [] text16;
		}

		// Restore pinned resultsets
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select id, tab_no, caption, query, elapsed_time from pinned_results order by id", -1, &stmt, 0)) {
			while (SQLITE_ROW == sqlite3_step(stmt)) {
				int tabNo = sqlite3_column_int(stmt, 1);
				if (tabNo >= tabCount)
					continue;

				TEditorTab* tab = &tabs[tabNo];
				int resultNo = TabCtrl_GetItemCount(tab->hTabWnd);

				TCITEM tci;
				tci.mask = TCIF_TEXT | TCIF_IMAGE;
				tci.iImage = 0;
				tci.pszText = utils::utf8to16((const char*)sqlite3_column_text(stmt, 2));
				tci.cchTextMax = _tcslen(tci.pszText) + 1;
				TabCtrl_InsertItem(tab->hTabWnd, resultNo, &tci);

				ListBox_AddString(tab->hQueryListWnd, tci.pszText);

				createResultControls(tab->hTabWnd, resultNo);
				tab->isPinned[resultNo] = TRUE;
				tab->tabTooltips[resultNo] = utils::utf8to16((const char*)sqlite3_column_text(stmt, 3));

				TCHAR* elapsedTime16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 4));
				_sntprintf(tab->queryElapsedTimes[resultNo], 63, TEXT("%ls"), elapsedTime16);
				delete [] elapsedTime16;

				HWND hRowsWnd = GetDlgItem(tab->hTabWnd, IDC_TAB_ROWS + resultNo);
				HWND hMessageWnd = GetDlgItem(tab->hTabWnd, IDC_TAB_MESSAGE + resultNo);
				HWND hPreviewWnd = GetDlgItem(tab->hTabWnd, IDC_TAB_PREVIEW + resultNo);

				int tableId = sqlite3_column_int(stmt, 0);
				char query8[128];
				snprintf(query8, 127, "select * from \"$pinned_result%i\"", tableId);

				sqlite3_stmt* stmt2;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, query8, -1, &stmt2, 0)) {
					SetWindowLongPtr(tab->hTabWnd, GWLP_USERDATA, (LONG_PTR)hRowsWnd);
					SetWindowLongPtr(hMessageWnd, GWLP_USERDATA, (LONG_PTR)-1);
					ListView_SetData(hRowsWnd, stmt2, false);
				} else {
					SetWindowLongPtr(tab->hTabWnd, GWLP_USERDATA, (LONG_PTR)hMessageWnd);
					SetWindowLongPtr(hRowsWnd, GWLP_USERDATA, (LONG_PTR)-1);
					SetWindowLongPtr(hPreviewWnd, GWLP_USERDATA, (LONG_PTR)-1);

					SetWindowText(hMessageWnd, TEXT("Can't load the pinned resultset"));
				}
				sqlite3_finalize(stmt2);
			}
		}
		sqlite3_finalize(stmt);

		for (int tabNo = 0; tabNo < prefs::get("editor-tab-count"); tabNo++) {
			HWND hTabWnd = tabs[tabNo].hTabWnd;
			EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETFONT);
			EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_RESIZETAB);
		}

		SendMessage(hMainTabWnd, WMU_TAB_SET_CURRENT, prefs::get("editor-tab-current"), 0);
	}

	hAiChatWnd = CreateWindowEx(0, WC_STATIC, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_NOTIFY, 0, 0, 0, 0, hMainWnd, (HMENU)0, GetModuleHandle(0), NULL);
	CreateWindowEx(0, TEXT("RICHEDIT50W"), NULL, WS_VISIBLE | WS_CHILD | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL, 0, 0, 0, 0, hAiChatWnd, (HMENU)IDC_AI_CHAT_MESSAGES, GetModuleHandle(0), NULL);
	CreateWindowEx(0, TEXT("RICHEDIT50W"), NULL, WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | ES_NOHIDESEL | WS_VSCROLL, 0, 0, 0, 0, hAiChatWnd, (HMENU)IDC_AI_CHAT_INPUT, GetModuleHandle(0), NULL);
	SetProp(hAiChatWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hAiChatWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewAiChat));
	PostMessage(hAiChatWnd, WMU_INIT_AI_CHAT, 0, 0);

	hAutoComplete = CreateWindowEx(WS_EX_TOPMOST, WC_LISTBOX, NULL, WS_POPUP | WS_BORDER | WS_VSCROLL, 0, 0, 170, 200, hMainWnd, (HMENU)0, GetModuleHandle(0), NULL);
	SetProp(hAutoComplete, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hAutoComplete, GWLP_WNDPROC, (LONG_PTR)&cbNewAutoComplete));

	hAutoCompleteHelp = CreateWindowEx(WS_EX_TOPMOST, WC_STATIC, NULL, WS_POPUP | WS_BORDER | SS_LEFT, 0, 0, 170, 200, hMainWnd, (HMENU)0, GetModuleHandle(0), NULL);
	SetProp(hAutoCompleteHelp, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hAutoCompleteHelp, GWLP_WNDPROC, (LONG_PTR)&cbNewAutoCompleteHelp));

	SendMessage(hMainWnd, WMU_SET_THEME, 0, 0);
	setEditorFont(cli.hResultWnd);

	ShowWindow (hMainWnd, prefs::get("maximized") == 1 ? SW_MAXIMIZE : SW_SHOW);
	SetFocus(hEditorWnd);

	if (prefs::get("check-update")) {
		// Use thread to prevent GUI blocking on startup
		HANDLE hThread = (HANDLE)_beginthreadex(0, 0, &checkUpdate, 0, 0, 0);
		CloseHandle(hThread);
	}

	reloadPlugins();
	PostMessage(hMainWnd, WMU_UPDATE_AI_UI, 0, 0);

	// https://docs.microsoft.com/en-us/windows/win32/dlgbox/using-dialog-boxes
	while (GetMessage(&msg, NULL, 0, 0)) {
		bool isDialogMessage = false;
		HWND hDlg = 0;
		for (int i = 0; i < MAX_DIALOG_COUNT && !isDialogMessage; i++) {
			isDialogMessage = hDialogs[i] && IsWindow(hDialogs[i]) && IsDialogMessage(hDialogs[i], &msg);
			hDlg = hDialogs[i];
		}

		if (isDialogMessage && TranslateAccelerator(hDlg, hDlgAccel, &msg))
			continue;

		if (!isDialogMessage && TranslateAccelerator(hMainWnd, hAccel, &msg))
			continue;

		if (!isDialogMessage) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);

	// Preventing buffer clearing on exit
	TCHAR* txt = utils::getClipboardText();
	utils::setClipboardText(txt);
	delete [] txt;
	LocalFree(args);

	return msg.wParam;
}

LRESULT CALLBACK cbMainWindow (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE: {
			if (!closeDb())
				return 0;

			prefs::setSyncMode(0);

			if (prefs::get("restore-editor")) {
				int tabCurrent = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
				prefs::set("editor-tab-current", tabCurrent);

				int len = GetWindowTextLength(cli.hEditorWnd);
				TCHAR* text16 = new TCHAR[len + 1]{0};
				GetWindowText(cli.hEditorWnd, text16, len + 1);
				char* text8 = utils::utf16to8(text16);
				prefs::set("editor-cli-text", text8);
				delete [] text8;
				delete [] text16;

				int tabCount = SendMessage(hMainTabWnd, WMU_TAB_GET_COUNT, 0, 0);
				prefs::set("editor-tab-count", tabCount);

				sqlite3_stmt* stmt;
				char query8[MAX_TEXT_LENGTH + 1]{0};
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
					"select group_concat('drop table \"' || name || '\";', ' ') from sqlite_master where type = 'table' and name like '$pinned_result%'",
					-1, &stmt, 0)) {
					if (SQLITE_ROW == sqlite3_step(stmt))
						snprintf(query8, MAX_TEXT_LENGTH, "%s", (const char*)sqlite3_column_text(stmt, 0));
				}
				sqlite3_finalize(stmt);
				sqlite3_exec(prefs::db, query8, 0, 0, 0);
				sqlite3_exec(prefs::db, "delete from pinned_results", 0, 0, 0);

				for (int tabNo = 0; tabNo < tabCount; tabNo++) {
					HWND hEditorWnd = tabs[tabNo].hEditorWnd;
					int len = GetWindowTextLength(hEditorWnd);
					TCHAR* text16 = new TCHAR[len + 1]{0};
					GetWindowText(hEditorWnd, text16, len + 1);
					char key[80] = {0};
					sprintf(key, "editor-text-%i", tabNo);
					char* text8 = utils::utf16to8(text16);
					prefs::set(key, text8);
					delete [] text8;
					delete [] text16;

					TCHAR name16[80] = {0};
					SendMessage(hMainTabWnd, WMU_TAB_GET_TEXT, tabNo, (LPARAM)name16);
					sprintf(key, "editor-name-%i", tabNo);
					char* name8 = utils::utf16to8(name16);
					prefs::set(key, name8);
					delete [] name8;

					// Save pinned resultsets
					for (int resultNo = 0; resultNo < MAX_RESULT_COUNT; resultNo++) {
						HWND hRowsWnd = GetDlgItem(tabs[tabNo].hTabWnd, IDC_TAB_ROWS + resultNo);
						if (tabs[tabNo].isPinned[resultNo] && GetWindowLongPtr(hRowsWnd, GWLP_USERDATA) != -1) {
							int id = 0;
							sqlite3_stmt* stmt;
							if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "insert into pinned_results (tab_no, caption, query, elapsed_time) values (?, ?, ?, ?) returning id", -1, &stmt, 0)) {
									sqlite3_bind_int(stmt, 1, tabNo);

									TCHAR buf16[255];
									TabCtrl_GetItemText(tabs[tabNo].hTabWnd, resultNo, buf16, 255);
									char* caption8 = utils::utf16to8(buf16);
									sqlite3_bind_text(stmt, 2, caption8, strlen(caption8), SQLITE_TRANSIENT);
									delete [] caption8;

									char* query8 = utils::utf16to8(tabs[tabNo].tabTooltips[resultNo]);
									sqlite3_bind_text(stmt, 3, query8, strlen(query8), SQLITE_TRANSIENT);
									delete [] query8;

									char* elapsedTime8 = utils::utf16to8(tabs[tabNo].queryElapsedTimes[resultNo]);
									sqlite3_bind_text(stmt, 4, elapsedTime8, strlen(elapsedTime8), SQLITE_TRANSIENT);
									delete [] elapsedTime8;

									if (SQLITE_ROW == sqlite3_step(stmt))
										id = sqlite3_column_int(stmt, 0);
							}
							sqlite3_finalize(stmt);

							if (id) {
								TCHAR table16[255];
								_sntprintf(table16, 255, TEXT("$pinned_result%i"), id);
								saveResultToTable(prefs::db, table16, tabNo, resultNo, LVNI_ALL);
							}
						}
					}

					// Close all plugins
					for (int resultNo = 0; resultNo < MAX_RESULT_COUNT; resultNo++) {
						HWND hPreviewWnd = GetDlgItem(tabs[tabNo].hTabWnd, IDC_TAB_PREVIEW + resultNo);
						SendMessage(hPreviewWnd, WM_DESTROY, 0, 0);
					}
				}
			}

			if (prefs::save()) {
				prefs::setSyncMode(1);
				sqlite3_close(prefs::db);
			} else {
				MessageBox(hWnd, TEXT("Settings saving failed"), TEXT("Error"), MB_OK);
			}

			// Clear temporary files e.g. generated by "Export to Excel" or BLOB viewer
			SHFILEOPSTRUCT fo = {0};
			fo.wFunc = FO_DELETE;
			fo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
			fo.pFrom = TMP_PATH;
			SHFileOperation(&fo);
			CreateDirectory(TMP_PATH, NULL); // another instance may want to use TMP-path

			sqlite3_shutdown();

			RemoveProp(hWnd, TEXT("LASTFOCUS"));
			RemoveProp(hWnd, TEXT("ACTIVATETIME"));
			RemoveProp(hWnd, TEXT("READONLYDB"));

			if (prefs::get("use-logger")) {
				hLoggerWnd = FindWindow(TEXT("logger-logger-class"), NULL);
				SendMessage(hLoggerWnd, WM_DESTROY, 0, 0);
			}

			PostQuitMessage (0);
		}
		break;

		case WM_ACTIVATE: {
			if (LOWORD(wParam) != WA_INACTIVE) {
				SetFocus((HWND)GetProp(hWnd, TEXT("LASTFOCUS")));
				SetProp(hWnd, TEXT("ACTIVATETIME"), UIntToPtr(GetTickCount()));
			} else
				SetProp(hWnd, TEXT("LASTFOCUS"), GetFocus());
		}
		break;

		case WM_SIZE: {
			SendMessage(hToolbarWnd, WM_SIZE, 0, 0);
			SendMessage(hStatusWnd, WM_SIZE, 0, 0);

			SendMessage(hWnd, WMU_UPDATE_SIZES, 0, 0);
		}
		break;

		case WM_LBUTTONDOWN: {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			int sx = prefs::get("splitter-position-x");
			int sy = prefs::get("splitter-position-y");
			int top = (int)(LONG_PTR)GetProp(hWnd, TEXT("NCTOP"));
			int cw = prefs::get("ai-chat-width");

			RECT rc;
			GetClientRect(hWnd, &rc);
			int w = rc.right;

			int splitter = (abs(x - sx) < 10) ? SPLITTER_X :
				(x > sx + 10) && (abs(y - top - sy) < 10) ? SPLITTER_Y :
				(abs(w - cw - x) < 10) ? SPLITTER_C :
				0;

			// Drag by tab nc-area
			if (!splitter && (x > sx + 10)) {
				POINT p{x, y};
				ClientToScreen(hMainWnd, &p);
				ScreenToClient(hTabWnd, &p);

				RECT rc = {0, 0, -10, 30};
				int tabCount = TabCtrl_GetItemCount(hTabWnd);
				if (tabCount != 0)
					TabCtrl_GetItemRect(hTabWnd, tabCount - 1, &rc);
				splitter = (p.x > rc.right + 10) && (p.y <= rc.bottom) && (p.y >= rc.top) ? SPLITTER_Y : 0;
			}

			SetProp(hWnd, TEXT("SPLITTERMOVE"), IntToPtr(splitter));

			if (splitter)
				SetCapture(hMainWnd);

			if (x > sx + 10 && x < w - cw - 10 && y > top + sy + 10) {
				POINT p {x, y};
				ClientToScreen(hWnd, &p);
				ScreenToClient(hTabWnd, &p);
				SendMessage(hTabWnd, WM_LBUTTONDOWN, wParam, MAKELPARAM(p.x, p.y));
			}
		}
		break;

		case WM_LBUTTONUP: {
			if (GetProp(hWnd, TEXT("SPLITTERMOVE"))) {
				SendMessage(hWnd, WMU_UPDATE_SIZES, 0, 0);
				ReleaseCapture();
			}

			if (hDragWnd) {
				ReleaseCapture();
				ShowCursor(true);

				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				POINT p{x, y};
				if (ChildWindowFromPointEx(hMainWnd, p, CWP_SKIPDISABLED | CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT) == hEditorWnd) {
					TCHAR schema16[256];
					GetWindowText(hSchemaWnd, schema16, 255);

					TCHAR name16[256];
					GetWindowText(hDragWnd, name16, 255);
					TCHAR* fullname16 = utils::getFullTableName(schema16, name16, true);

					if (!HIWORD(GetKeyState(VK_CONTROL)) || GetWindowLongPtr(hDragWnd, GWLP_USERDATA) == COLUMN) {
						SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)fullname16);
					} else {
						TCHAR res[MAX_TEXT_LENGTH + 1];
						_sntprintf(res, MAX_TEXT_LENGTH, fullname16);
						if (DLG_OK == DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_DROP), hWnd, (DLGPROC)&dialogs::cbDlgDrop, (LPARAM)res))
							SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)res);
					}

					delete [] fullname16;
				}

				DestroyWindow(hDragWnd);
				hDragWnd = 0;
			}

			RemoveProp(hWnd, TEXT("SPLITTERMOVE"));
		}
		break;

		case WM_MOUSEMOVE: {
			LONG x = GET_X_LPARAM(lParam);
			LONG y = GET_Y_LPARAM(lParam);
			if (hDragWnd) {
				SetWindowPos(hDragWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				POINT p{x, y};
				if (ChildWindowFromPointEx(hMainWnd, p, CWP_SKIPDISABLED | CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT) == hEditorWnd) {
					ClientToScreen(hMainWnd, &p);
					ScreenToClient(hEditorWnd, &p);
					int pos = SendMessage(hEditorWnd, EM_CHARFROMPOS, 0, (LPARAM)&p);
					SendMessage(hEditorWnd, EM_SETSEL, pos, pos);
					SetFocus(hEditorWnd);
				}
				InvalidateRect(hDragWnd, NULL, true);
				return true;
			}

			int splitter = wParam == MK_LBUTTON ? PtrToInt(GetProp(hWnd, TEXT("SPLITTERMOVE"))) : 0;

			RECT rc;
			GetClientRect(hWnd, &rc);
			int cw = prefs::get("ai-chat-width");
			int w = rc.right;
			bool isChatMaximized = prefs::get("ai-chat-maximized");

			if (splitter == SPLITTER_X && prefs::get("splitter-position-x") != x - 3) {
				prefs::set("splitter-position-x", x - 3);
				SendMessage(hWnd, WMU_UPDATE_SIZES, 0, 0);
			}

			int top = splitter == SPLITTER_Y ? (int)(LONG_PTR)GetProp(hWnd, TEXT("NCTOP")) : 0;
			if (splitter == SPLITTER_Y && prefs::get("splitter-position-y") != y - 3 - top) {
				prefs::set("splitter-position-y", y - 3 - top);
				SendMessage(hWnd, WMU_UPDATE_SIZES, 0, 0);
			}

			if (splitter == SPLITTER_C && !isChatMaximized && (w - prefs::get("ai-chat-width") != x - 3)  && (w - x - 3 > 50)) {
				prefs::set("ai-chat-width", w - x - 3);
				SendMessage(hWnd, WMU_UPDATE_SIZES, 0, 0);
			}

			if (!GetProp(hWnd, TEXT("ISMOUSEHOVER"))) {
				int spX = prefs::get("splitter-position-x");
				int spY = prefs::get("splitter-position-y");
				int top = (int)(LONG_PTR)GetProp(hWnd, TEXT("NCTOP"));

				if (((spX <= x) && (x < spX + 8)) || ((w - cw - 8 <= x) && (x < w - cw)) || ((spY + top <= y) && (y < spY + 8 + top)))  {
					TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, hWnd, 0};
					TrackMouseEvent(&tme);
					SetProp(hWnd, TEXT("ISMOUSEHOVER"), (HANDLE)1);
				}
			}
		}
		break;

		case WM_MOUSEWHEEL: {
			if (LOWORD(wParam) == MK_CONTROL) {
				HWND hFocusWnd = GetFocus();
				prefs::set("font-size", prefs::get("font-size") + (GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? 1: -1));
				SendMessage(hWnd, WMU_SET_THEME, 0, 0);
				SetFocus(hFocusWnd);
				return 1;
			}
		}
		break;

		case WM_SETCURSOR: {
			if (!GetProp(hWnd, TEXT("ISMOUSEHOVER")))
				return DefWindowProc(hWnd, msg, wParam, lParam);

			POINT p = {0};
			GetCursorPos(&p);
			ScreenToClient(hWnd, &p);

			int spX = prefs::get("splitter-position-x");
			int spY = prefs::get("splitter-position-y");
			int top = (int)(LONG_PTR)GetProp(hWnd, TEXT("NCTOP"));

			RECT rc;
			GetClientRect(hWnd, &rc);
			int chatW = prefs::get("ai-chat-width");
			int w = rc.right;

			TCHAR* idc = (spX <= p.x) && (p.x < spX + 8) ? IDC_SIZEWE :
				(w - chatW - 8 <= p.x) && (p.x < w - chatW) ? IDC_SIZEWE :
				(spY + top <= p.y) && (p.y < spY + 8 + top) ? IDC_SIZENS :
				IDC_ARROW;

			SetCursor(LoadCursor(0, idc));
			return TRUE;
		}
		break;

		case WM_MOUSELEAVE: {
			SetProp(hWnd, TEXT("ISMOUSEHOVER"), 0);
		}
		break;

		case WM_NCLBUTTONDBLCLK: {
			WINDOWPLACEMENT wp = {};
			wp.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hMainWnd, &wp);
			prefs::set("maximized", (int)(wp.showCmd != SW_SHOWMAXIMIZED));
			return DefWindowProc (hWnd, msg, wParam, lParam);
		}
		break;

		case WM_SYSCOMMAND: {
			if (wParam == SC_MAXIMIZE || wParam == SC_RESTORE)
				prefs::set("maximized", wParam == SC_MAXIMIZE);
			return DefWindowProc (hWnd, msg, wParam, lParam);
		}
		break;

		case WM_EXITSIZEMOVE: {
			RECT rc;
			GetWindowRect(hMainWnd, &rc);
			prefs::set("x", rc.left);
			prefs::set("y", rc.top);
			prefs::set("width", rc.right - rc.left);
			prefs::set("height", rc.bottom - rc.top);
			SendMessage(hWnd, WMU_UPDATE_SIZES, 0, 0);
		}
		break;

		// Prevent full redraw to reduce flickering
		case WM_ERASEBKGND: {
			HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

			RECT rc;
			GetClientRect(hWnd, &rc);
			int w = rc.right;
			int h = rc.bottom;
			int top = PtrToInt(GetProp(hWnd, TEXT("NCTOP")));
			int spX = prefs::get("splitter-position-x");
			int spY = prefs::get("splitter-position-y") + top;

			rc = (RECT){0, top, w, top + 3};
			FillRect((HDC)wParam, &rc, hBrush);

			rc = (RECT){spX, top + 3, spX + 5, h};
			FillRect((HDC)wParam, &rc, hBrush);

			rc = (RECT){spX, spY, w, spY + 10};
			FillRect((HDC)wParam, &rc, hBrush);

			bool isShowChat = prefs::get("ai-enable") && prefs::get("ai-show-chat");
			if (isShowChat) {
				int cw = prefs::get("ai-chat-width");
				rc = (RECT){w - cw - 10, 0, w - cw, h};
				FillRect((HDC)wParam, &rc, hBrush);
			}

			if (IsWindowVisible(hTreeSearchWnd)) {
				RECT rcSearch;
				GetWindowRect(hTreeSearchWnd, &rcSearch);
				InflateRect(&rcSearch, 5, 5);
				MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT) &rcSearch, 2);
				FillRect((HDC)wParam, &rcSearch, hBrush);
			}

			DeleteObject(hBrush);

			return 1;
		}
		break;

		case WMU_UNREGISTER_DIALOG: {
			for (int i = 0; i < MAX_DIALOG_COUNT; i++)
				hDialogs[i] = (HWND)wParam == hDialogs[i] ? 0 : hDialogs[i];

			HWND hNextWnd = hMainWnd;
			for (int i = 0; i < MAX_DIALOG_COUNT && hNextWnd == hMainWnd; i++)
				hNextWnd = hDialogs[i] != 0 ? hDialogs[i] : hNextWnd;

			SetActiveWindow(hNextWnd);
			if (hNextWnd == hMainWnd)
				SetFocus(hEditorWnd);
		}
		break;

		// wParam - type
		case WMU_OBJECT_CREATED: {
			// With hope that the last record is a new object
			int type = (int)wParam;
			sqlite3_stmt *stmt;
			sqlite3_prepare_v2(db, "select name from sqlite_master where type = ?1", -1, &stmt, 0);
			sqlite3_bind_text(stmt, 1, TYPES8[type], strlen(TYPES8[type]), SQLITE_TRANSIENT);
			while(SQLITE_ROW == sqlite3_step(stmt)) {
				TCHAR* name16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
				updateTree(type, name16);
				delete [] name16;
			}
			sqlite3_finalize(stmt);
		}
		break;

		// lParam = isDatabase encrypted
		case WMU_SET_ECRYPT_FLAG: {
			SendMessage(hMainWnd, WM_SETICON, 0, (LPARAM)hIcons[lParam]);
			SetProp(hMainWnd, TEXT("ISENCRYPT"), (HANDLE)lParam);
		}
		break;

		case WMU_UPDATE_EXTENSIONS_UI: {
			sqlite3_stmt* stmt;
			if (SQLITE_OK == sqlite3_prepare_v2(db, "select ' Loaded extensions: ' || group_concat(name, ', ') from temp.loaded_extensions order by name", -1, &stmt, 0)) {
				if (SQLITE_ROW == sqlite3_step(stmt)) {
					TCHAR* extensions = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
					SendMessage(hStatusWnd, SB_SETTEXT, SB_EXTENSIONS, (LPARAM)extensions);
					delete [] extensions;
				}
			}
			sqlite3_finalize(stmt);

			if (SQLITE_OK == sqlite3_prepare_v2(db, "select 1 from temp.loaded_extensions where lower(name) == 'odbc'", -1, &stmt, 0)) {
				HMENU hToolMenu = GetSubMenu(hMainMenu, Menu_GetItemPositionByName(hMainMenu, TEXT("Tools")));
				HMENU hImportMenu = GetSubMenu(hToolMenu, Menu_GetItemPositionByName(hToolMenu, TEXT("Import")));
				HMENU hExportMenu = GetSubMenu(hToolMenu, Menu_GetItemPositionByName(hToolMenu, TEXT("Export")));
				UINT state = SQLITE_ROW == sqlite3_step(stmt) ? MF_ENABLED : MF_DISABLED | MF_GRAYED;
				if (!Menu_SetItemState(hImportMenu, IDM_IMPORT_ODBC, state) || !Menu_SetItemState(hExportMenu, IDM_EXPORT_ODBC, state))
					MessageBox(hMainWnd, TEXT("Import-Export menu bug"), 0, MB_OK);

				int pos = Menu_GetItemPositionByName(hDbMenu, TEXT("Attach database by ODBC"));
				Menu_SetItemStateByPosition(hDbMenu, pos, state);
			}
			sqlite3_finalize(stmt);
		}
		break;

		case WMU_UPDATE_AI_UI: {
			bool isEnable = prefs::get("ai-enable");
			Menu_SetItemState(hEditorMenu, IDM_EDITOR_FIX_ERROR, isEnable ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
			SendMessage(hStatusWnd, SB_SETTEXT, SB_AI_SETTINGS, (LPARAM)(isEnable ? TEXT("  <AI> ") : 0));

			bool isShowChat = isEnable && prefs::get("ai-show-chat");
			ShowWindow(hAiChatWnd, isShowChat ? SW_SHOW : SW_HIDE);
			SendMessage(hMainWnd, WMU_UPDATE_TOOLBAR_BUTTONS, 0, 0);
		}
		break;

		case WMU_UPDATE_CARET_INFO: {
			TCHAR buf16[256];
			CHARRANGE cr{0};
			SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&cr);
			int x = cr.cpMin - SendMessage(hEditorWnd, EM_LINEINDEX, (WPARAM)-1, 0);
			_sntprintf(buf16, 255, TEXT(" Col: %i"), x + 1);
			SendMessage(hStatusWnd, SB_SETTEXT, SB_CARET_POSITION, (LPARAM)buf16);
		}
		break;

		case WMU_UPDATE_ROWNO: {
			HWND hListWnd = (HWND)SendMessage(hWnd, WMU_GET_CURRENT_RESULTSET, 0, 0);
			TCHAR buf16[256]{0};
			int pos = hListWnd && IsWindow(hListWnd) ? ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED) : -1;
			if (pos != - 1)
				_sntprintf(buf16, 255, TEXT(" %i"), pos + 1);
			SendMessage(hStatusWnd, SB_SETTEXT, SB_SELECTED_ROW, (LPARAM)buf16);
		}
		break;

		case WMU_GET_CURRENT_RESULTSET: {
			int resultNo = TabCtrl_GetCurSel(hTabWnd);
			return (LRESULT)GetDlgItem(hTabWnd, IDC_TAB_ROWS + resultNo);
		}
		break;

		case WM_CONTEXTMENU: {
			POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			bool isContextKey = p.x == -1 && p.y == -1;
			if ((HWND)wParam == hEditorWnd && !isContextKey)
				TrackPopupMenu(hEditorMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hMainWnd, NULL);

			if ((HWND)wParam == cli.hResultWnd && !isContextKey)
				TrackPopupMenu(hCliMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hMainWnd, NULL);

			if ((HWND)wParam == hTreeWnd) {
				// update selected item on right click
				if (!isContextKey) {
					POINT p2 = {0};
					GetCursorPos(&p2);
					ScreenToClient(hTreeWnd, &p2);
					TVHITTESTINFO thi = {p2,TVHT_ONITEM};
					HTREEITEM hItem = TreeView_HitTest(hTreeWnd, &thi);
					TreeView_SelectItem(hTreeWnd, hItem);
					if (!hItem)
						return 0;

					treeItems[0] = thi.hItem;
				}

				TCHAR name16[256] = {0};

				TVITEM ti;
				ti.hItem = treeItems[0];
				ti.mask = TVIF_PARAM | TVIF_TEXT;
				ti.pszText = name16;
				ti.cchTextMax = 256;
				TreeView_GetItem(hTreeWnd, &ti);

				if (isContextKey) {
					RECT rc = {0};
					//TreeView_GetItemRect(hTreeWnd, ti.hItem, &rc, TRUE); // is the macros problem?
					*(HTREEITEM*)&rc = ti.hItem;
					SendMessage(hTreeWnd, TVM_GETITEMRECT, FALSE, (LPARAM)&rc);
					p.x = rc.left + 10;
					p.y = rc.top + 10;
					ClientToScreen(hTreeWnd, &p);
				}

				int type = ti.lParam;
				if (type) {
					TCHAR schema16[256];
					GetWindowText(hSchemaWnd, schema16, 255);

					bool isMain = _tcscmp(schema16, TEXT("main")) == 0;
					BOOL isCut = TreeView_GetItemState(hTreeWnd, ti.hItem, TVIS_CUT) & TVIS_CUT;

					int idc = _tcscmp(schema16, TEXT("temp")) == 0 ? IDC_MENU_TEMP :
						type == TABLE ? IDC_MENU_TABLE :
						type == VIEW ? IDC_MENU_VIEW :
						type == INDEX && !isCut ? IDC_MENU_INDEX :
						type == TRIGGER && !isCut  ? IDC_MENU_TRIGGER :
						type == -TABLE || type == -VIEW ? IDC_MENU_TABLEVIEW :
						type == -INDEX || type == -TRIGGER ? IDC_MENU_INDEXTRIGGER :
						type == COLUMN && TreeView_GetParent(hTreeWnd, TreeView_GetParent(hTreeWnd, ti.hItem)) == treeItems[TABLE] ? IDC_MENU_COLUMN :
						isCut ? IDC_MENU_DISABLED :
						0;

					HMENU hMenu = treeMenus[idc];

					if (type == VIEW) {
						char* name8 = utils::utf16to8(name16);
						sqlite3_stmt *stmt;
						sqlite3_prepare_v2(db, "select 1 from sqlite_master where tbl_name = ?1 and type = 'trigger' and instr(lower(sql), 'instead of ') > 0", -1, &stmt, 0);
						sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
						bool hasEditTrigger = SQLITE_ROW == sqlite3_step(stmt);
						ModifyMenu(hMenu, IDM_EDIT_DATA, MF_BYCOMMAND | MF_STRING, IDM_EDIT_DATA, hasEditTrigger ? TEXT("Edit data") : TEXT("View data"));
						sqlite3_finalize(stmt);
						delete [] name8;
					}

					if (idc) {
						UINT state = MF_BYCOMMAND | (isMain ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

						if (idc == IDC_MENU_TABLE || idc == IDC_MENU_VIEW) {
							if (isMain) {
								bool isPinned = isObjectPinned(name16);
								ModifyMenu(hMenu, IDM_PIN_ON_TOP, MF_BYCOMMAND | MF_STRING | state, IDM_PIN_ON_TOP, isPinned ? TEXT("Unpin from top") : TEXT("Pin on top"));
							} else {
								ModifyMenu(hMenu, IDM_PIN_ON_TOP, MF_BYCOMMAND | MF_STRING | state, IDM_PIN_ON_TOP, TEXT("Pin on top"));
							}
						}

						if (idc == IDC_MENU_INDEX || idc == IDC_MENU_TRIGGER)
							EnableMenuItem(hMenu, IDM_DISABLE, state);

						if (idc == IDC_MENU_INDEXTRIGGER) {
							EnableMenuItem(hMenu, IDM_ENABLE_ALL, state);
							EnableMenuItem(hMenu, IDM_DISABLE_ALL, state);
						}
						ModifyMenu(hMenu, IDM_EDIT, MF_BYCOMMAND | MF_STRING, IDM_EDIT, isMain ? TEXT("Edit") : TEXT("View"));
						TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hMainWnd, NULL);
					}
				}
			}
		}
		break;

		case WM_COMMAND: {
			WORD cmd = LOWORD(wParam);

			if (cmd == IDM_EXIT)
				SendMessage(hMainWnd, WM_CLOSE, 0, 0);

			if (cmd == IDM_OPEN) {
				TCHAR path16[MAX_PATH]{0};
				bool rc = HIWORD(GetKeyState(VK_SHIFT)) ?
					DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_URI_DB_PATH), hMainWnd, (DLGPROC)dialogs::cbDlgUriDbPath, (LPARAM)&path16) :
					utils::openFile(path16, TEXT("Databases (*.sqlite, *.sqlite3, *.db, *.db3)\0*.sqlite;*.sqlite3;*.db;*.db3\0All\0*.*\0"), hWnd);

				if (rc)
					openDb(path16, HIWORD(GetKeyState(VK_CONTROL)));
			}

			if (cmd == IDM_SAVE_AS) {
				TCHAR path16[MAX_PATH + 1]{0};
				char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0), true);
				TCHAR* dbname16 = utils::utf8to16(dbname8);
				_sntprintf(path16, MAX_PATH, dbname16);
				delete [] dbname8;
				delete [] dbname16;

				if (utils::saveFile(path16, TEXT("Databases (*.sqlite, *.sqlite3, *.db, *.db3)\0*.sqlite;*.sqlite3;*.db;*.db3\0All\0*.*\0"), TEXT("sqlite"), hWnd)) {
					sqlite3_stmt *stmt;
					bool rc = SQLITE_OK == sqlite3_prepare_v2(db, "vacuum into ?1", -1, &stmt, 0);
					if (rc) {
						char* path8 = utils::utf16to8(path16);
						sqlite3_bind_text(stmt, 1, path8, strlen(path8), SQLITE_TRANSIENT);
						delete [] path8;
						rc = SQLITE_DONE == sqlite3_step(stmt);
					}
					sqlite3_finalize(stmt);

					if (!rc)
						showDbError(hMainWnd);
				}
			}

			if (cmd == IDM_CLOSE)
				closeDb();

			if (cmd == IDM_EXTENSIONS) {
				if (db && isDbBusy())
					return false;

				if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADDON_MANAGER), hMainWnd, (DLGPROC)dialogs::cbDlgAddonManager, ADDON_SQLITE_EXTENSION) && db) {
					TCHAR* dbpath16 = utils::utf8to16(sqlite3_db_filename(db, 0));
					bool isReadOnly = PtrToInt(GetProp(hMainWnd, TEXT("READONLYDB")));

					closeDb(true);
					updatePlugins(ADDON_SQLITE_EXTENSION);
					openDb(dbpath16, isReadOnly, true);
				}
			}

			if (cmd == IDM_VIEWER_PLUGINS) {
				if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADDON_MANAGER), hMainWnd, (DLGPROC)dialogs::cbDlgAddonManager, (LPARAM)ADDON_VALUE_VIEWER)) {

					// Close all opened plugins
					int tabCount = SendMessage(hMainTabWnd, WMU_TAB_GET_COUNT, 0, 0);
					for (int tabNo = 0; tabNo < tabCount; tabNo++) {
						HWND hTabWnd = tabs[tabNo].hTabWnd;
						int resultCount = TabCtrl_GetItemCount(hTabWnd);
						for (int resultNo = 0; resultNo < resultCount; resultNo++) {
							HWND hPreviewWnd = GetDlgItem(hTabWnd, IDC_TAB_PREVIEW + resultNo);
							SendMessage(hPreviewWnd, WMU_RESET_PREVIEW, 0, 0);
						}
					}

					for (int i = 0; i < MAX_DIALOG_COUNT; i++) {
						HWND hDlg = hDialogs[i];

						if (hDlg && GetDlgItem(hDlg, IDC_PREVIEW)) {
							SendMessage(hMainWnd, WMU_UNREGISTER_DIALOG, (WPARAM)hDlg, 0);
							DestroyWindow(hDlg);
						}
					}

					updatePlugins(ADDON_VALUE_VIEWER);
					reloadPlugins(ADDON_VALUE_VIEWER);
				}
			}

			if (cmd == IDM_MODIFIER_PLUGINS) {
				if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADDON_MANAGER), hMainWnd, (DLGPROC)dialogs::cbDlgAddonManager, (LPARAM)ADDON_COLUMN_MODIFIER)) {

					// Reset all plugins
					EnumChildWindows(hMainWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_RESET_MODIFIERS);

					updatePlugins(ADDON_COLUMN_MODIFIER);
					reloadPlugins(ADDON_COLUMN_MODIFIER);
				}
			}

			if (cmd == IDM_CUSTOM_FUNCTIONS) {
				if (isDbBusy())
					return false;

				updateCustomFunction(true);
				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_CUSTOM_FUNCTIONS), hMainWnd, (DLGPROC)dialogs::cbDlgCustomFunctions);
				updateCustomFunction(false);
				reloadMeta(FUNCTIONS, "select distinct name from pragma_function_list()");
				processHighlight(hEditorWnd, true, true, true);
				reloadPlugins(ADDON_COLUMN_MODIFIER);
			}

			if (cmd == IDM_REFERENCES) {
				if (isDbBusy())
					return false;

				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_REFERENCES), hMainWnd, (DLGPROC)dialogs::cbDlgReferences);
			}

			if (cmd == IDM_ENCRYPTION) {
				if (isDbBusy())
					return false;

				if (DLG_OK == DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ENCRYPTION), hMainWnd, (DLGPROC)dialogs::cbDlgEncryption))
					closeConnections();
			}

			if (cmd == IDM_ATTACH) {
				if (isDbBusy())
					return false;

				TCHAR path16[MAX_PATH]{0};
				bool rc = HIWORD(GetKeyState(VK_SHIFT)) ?
					DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_URI_DB_PATH), hMainWnd, (DLGPROC)dialogs::cbDlgUriDbPath, (LPARAM)&path16) :
					utils::openFile(path16, TEXT("Databases (*.sqlite, *.sqlite3, *.db, *.db3)\0*.sqlite;*.sqlite3;*.db;*.db3\0All\0*.*\0"), hWnd);

				if (rc) {
					char* path8 = utils::utf16to8(path16);
					if (attachDb(&db, path8)) {
						TCHAR* name16 = utils::getFileName(path16, true);
						switchSchemaTo(name16);
						delete [] name16;

						updateAttachedRecentList();
					}
					delete [] path8;
				}
			}

			if (cmd >= IDM_RECENT_ATTACHED && cmd <= IDM_RECENT_ATTACHED + prefs::get("recent-count")) {
				HMENU hMenu = GetSubMenu(hDbMenu, Menu_GetItemPositionByName(hDbMenu, TEXT("Attach database")));

				TCHAR path16[MAX_PATH];
				GetMenuString(hMenu, cmd, path16, MAX_PATH, MF_BYCOMMAND);
				char* path8 = utils::utf16to8(path16);
				if (attachDb(&db, path8)) {
					TCHAR* name16 = utils::getFileName(path16, true);
					switchSchemaTo(name16);
					delete [] name16;

					updateAttachedRecentList();
				}
				delete [] path8;
			}

			if (cmd == IDM_ATTACH_ODBC) {
				if (isDbBusy())
					return false;

				TCHAR alias16[2048] = {0};
				if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ATTACH_ODBC), hMainWnd, (DLGPROC)dialogs::cbDlgAttachODBC, (LPARAM)alias16)) {
					if (attachOdbcDb(db, alias16))
						switchSchemaTo(alias16);
				}
			}

			if (cmd >= IDM_RECENT_ATTACHED_ODBC && cmd <= IDM_RECENT_ATTACHED_ODBC + prefs::get("recent-count")) {
				HMENU hMenu = GetSubMenu(hDbMenu, Menu_GetItemPositionByName(hDbMenu, TEXT("Attach database by ODBC")));

				TCHAR alias16[2048];
				GetMenuString(hMenu, cmd, alias16, 2048, MF_BYCOMMAND);
				if (attachOdbcDb(db, alias16)) {
					switchSchemaTo(alias16);
				} else {
					if (IDOK == MessageBox(hWnd, TEXT("Can't connect with stored parameters.\nDo you want to edit them and try again?"), 0, MB_OKCANCEL | MB_ICONEXCLAMATION)) {
						if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ATTACH_ODBC), hMainWnd, (DLGPROC)dialogs::cbDlgAttachODBC, (LPARAM)alias16)) {
							if (attachOdbcDb(db, alias16))
								switchSchemaTo(alias16);
						}
					}
				}
			}

			if (cmd == IDM_SETTINGS && DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_SETTINGS), hMainWnd, (DLGPROC)&dialogs::cbDlgSettings, lParam)) {
				SendMessage(hMainWnd, WMU_SET_THEME, 0, 0);

				if (!prefs::get("use-highlight")) {
					for (int i = 0; i < MAX_TAB_COUNT; i++) {
						if (tabs[i].id)
							setEditorColor(tabs[i].hEditorWnd, RGB(0, 0, 0), true);
					}
				}

				SendMessage(hMainWnd, WMU_UPDATE_AI_UI, 0, 0);

				stopHttpServer();
				startHttpServer();

				GRIDCOLORS[0] = prefs::get("color-current-cell");
				GRIDCOLORS[SQLITE_TEXT] = prefs::get("color-text");
				GRIDCOLORS[SQLITE_NULL] = prefs::get("color-null");
				GRIDCOLORS[SQLITE_BLOB] = prefs::get("color-blob");
				GRIDCOLORS[SQLITE_INTEGER] = prefs::get("color-integer");
				GRIDCOLORS[SQLITE_FLOAT] = prefs::get("color-real");

				applyDbPrefs();
				SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);

				SetFocus(hMainWnd);
			}

			if (cmd == IDM_EXECUTE || cmd == IDM_PLAN || cmd == IDM_EXECUTE_BATCH || cmd == IDM_AI_EXPLAIN || cmd == IDM_EXECUTE_CURRENT_LINE) {
				bool isShowChat = prefs::get("ai-enable") && prefs::get("ai-show-chat");
				if (isShowChat && cmd == IDM_EXECUTE_CURRENT_LINE && ((GetFocus() == GetDlgItem(hAiChatWnd, IDC_AI_CHAT_MESSAGES) || GetFocus() == hAiChatWnd) || prefs::get("ai-chat-maximized"))) {
					SendMessage(hAiChatWnd, WM_COMMAND, IDM_AI_CHAT_EXECUTE, 0);
					return 0;
				}

				int currTabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);

				if (currTabNo != -1) {
					if (cmd == IDM_EXECUTE_CURRENT_LINE) {
						HWND hEditorWnd = tabs[currTabNo].hEditorWnd;

						int start, end;
						SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&start, (WPARAM)&end);
						int lineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, start, 0);
						int lineIdx = SendMessage(hEditorWnd, EM_LINEINDEX, lineNo, 0);
						int lineSize = SendMessage(hEditorWnd, EM_LINELENGTH, lineIdx, 0);
						SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)lineIdx, (LPARAM)lineIdx + lineSize);
						executeEditorQuery(EQ_CURRENT);
						SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)start, (LPARAM)end);
					} else {
						executeEditorQuery((cmd == IDM_PLAN ? EQ_PLAN : 0) | (cmd == IDM_EXECUTE_BATCH ? EQ_BATCH : 0)  | (cmd == IDM_AI_EXPLAIN ? EQ_EXPLAIN : 0));
					}
				} else {
					executeCLIQuery(cmd == IDM_PLAN);
				}
			}

			if (cmd == IDM_SHORTCUTS) {
				DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_SHORTCUTS), hMainWnd, (DLGPROC)dialogs::cbDlgShortcuts, 0);
				return true;
			}

			if (cmd == IDM_INTERRUPT) {
				int tabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
				tabs[tabNo].aiNeedStop = true;
				sqlite3_interrupt(tabNo >= 0 ? tabs[tabNo].db : cli.db);
			}

			if (cmd == IDM_AI_CHAT) {
				prefs::set("ai-show-chat", !prefs::get("ai-show-chat"));
				SendMessage(hMainWnd, WMU_UPDATE_AI_UI, 0, 0);
				SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);

				if (prefs::get("ai-enable") && prefs::get("ai-show-chat")) {
					SetFocus(GetDlgItem(hAiChatWnd, IDC_AI_CHAT_INPUT));
					SendMessage(hAiChatWnd, WMU_SET_THEME, 0, 0);
				} else {
					SetFocus(hEditorWnd);
				}
			}

			if (cmd == IDM_AI_CHAT_COPY || cmd == IDM_AI_CHAT_EXECUTE)
				SendMessage(hAiChatWnd, WM_COMMAND, cmd, 0);

			// By accelerator
			if (cmd == IDM_OPEN_EDITOR)
				SendMessage(hMainTabWnd, WMU_TAB_ADD, 0, 0);

			if (cmd == IDM_CLOSE_EDITOR) {
				int tabCount = SendMessage(hMainTabWnd, WMU_TAB_GET_COUNT, 0, 0);
				int currTab = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
				SendMessage(hMainTabWnd, WMU_TAB_DELETE, currTab, 0);
				SendMessage(hMainTabWnd, WMU_TAB_SET_CURRENT, currTab == tabCount - 1 ? currTab - 1 : currTab, 0);
			}

			if (cmd == IDM_PREV_EDITOR || cmd == IDM_NEXT_EDITOR) {
				int tabCount = SendMessage(hMainTabWnd, WMU_TAB_GET_COUNT, 0, 0);
				int currTab = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
				SendMessage(hMainTabWnd, WMU_TAB_SET_CURRENT, (currTab + (cmd == IDM_PREV_EDITOR ? - 1 : 1) + tabCount) % tabCount, 1);
			}

			if (cmd == IDM_NEXT_RESULT) {
				int pos = TabCtrl_GetCurSel(hTabWnd);
				if (pos != -1) {
					int count = TabCtrl_GetItemCount(hTabWnd);
					TabCtrl_SetCurFocus(hTabWnd, (pos + 1) % count);
				}
			}

			if (cmd == IDM_CHANGE_FOCUS) {
				HWND hWndList[] = {hTreeWnd, hEditorWnd, (HWND)SendMessage(hWnd, WMU_GET_CURRENT_RESULTSET, 0, 0)};
				HWND hFocus = GetFocus();
				int idx = hFocus == hTreeWnd ? 0 : hFocus == hEditorWnd ? 1 : 2;
				HWND hNextWnd = hWndList[(idx + 1 + 3) % 3];

				if (hNextWnd == hWndList[2] && !TabCtrl_GetItemCount(hTabWnd))
				hNextWnd = hFocus == hTreeWnd ? hEditorWnd : hTreeWnd;

				SetFocus(hNextWnd);
			}

			if (cmd == IDM_PROCESS_INDENT) {
				// Fix https://github.com/little-brother/sqlite-gui/issues/124#issuecomment-1200335003
				if (labs(PtrToUint(GetProp(hWnd, TEXT("ACTIVATETIME"))) - GetTickCount()) < 100)
					return 0;

				int start, end;
				HWND hEditorWnd = (HWND)lParam;
				SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

				SetWindowRedraw(hEditorWnd, FALSE);
				POINT scrollPos{0};
				SendMessage(hWnd, EM_GETSCROLLPOS, 0, (LPARAM)&scrollPos);

				int dir = HIWORD(GetKeyState(VK_SHIFT)) ? -1 : +1;
				int startLineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, start, 0);
				int endLineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, end - (start != end), 0);
				const TCHAR* indent = dialogs::INDENTS[prefs::get("editor-indent")];

				int shift0 = 0;

				// issue #128
				int singleLineIndentLen = _tcslen(indent);
				int singleLineSpaceCount = 0;

				for (int currLineNo = startLineNo; currLineNo < endLineNo + 1; currLineNo++) {
					int currLineIdx = SendMessage(hEditorWnd, EM_LINEINDEX, currLineNo, 0);
					int currLineSize = SendMessage(hEditorWnd, EM_LINELENGTH, currLineIdx, 0);

					int shift = 0;
					if (dir == -1 || currLineNo == startLineNo) {
						TCHAR currLine[currLineSize + 1]{0};
						currLine[0] = currLineSize;
						SendMessage(hEditorWnd, EM_GETLINE, currLineNo, (LPARAM)currLine);

						// Is the selection start inside or outside the text?
						if (currLineNo == startLineNo) {
							bool isOut = true;
							for (int pos = 0; pos < start - currLineIdx; pos++)
								isOut = isOut && (currLine[pos] == TEXT(' ') || currLine[pos] == TEXT('\t'));

							for (int pos = start - currLineIdx - 1; pos >= 0 && currLine[pos] == TEXT(' '); pos--)
								singleLineSpaceCount++;

							if (!isOut) {
								if (dir == 1) {
									// https://github.com/little-brother/sqlite-gui/issues/128#issuecomment-1221265601
									if (start == end && indent[0] != TEXT('\t')) {
										int reducedIndentLen = (start - currLineIdx) % singleLineIndentLen;
										for (int i = 0; i < 4 - reducedIndentLen; i++)
											SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)TEXT(" "));
									} else {
										SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)indent);
									}
								}

								SendMessage(hWnd, EM_SETSCROLLPOS, 0, (LPARAM)&scrollPos);
								SetWindowRedraw(hEditorWnd, true);
								InvalidateRect(hEditorWnd, NULL, true);
								return true;
							}
						}

						shift = 0;
						if (dir == -1) {
							for (int i = 0; (i < currLineSize) && (i < (int)_tcslen(indent)) && (currLine[i] == TEXT(' ')); i++)
								shift++;

							shift = currLine[0] == TEXT('\t') ? 1 : shift;
						}

						if (currLineNo == startLineNo)
							shift0 = shift;
					}

					SendMessage(hEditorWnd, EM_SETSEL, currLineIdx, currLineIdx + shift);

					// issue #128
					if (start == end && indent[0] != TEXT('\t')) {
						TCHAR singleLineIndent[singleLineIndentLen + 1] = {0};
						for (int pos = 0; pos < singleLineIndentLen - singleLineSpaceCount % singleLineIndentLen; pos++)
							singleLineIndent[pos] = TEXT(' ');

						singleLineIndentLen = _tcslen(singleLineIndent);
						SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)singleLineIndent);
					} else {
						SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)(dir == 1 ? indent : TEXT("")));
					}
				}

				if (start == end) {
					shift0 = dir == 1 ? singleLineIndentLen : start != SendMessage(hEditorWnd, EM_LINEINDEX, startLineNo, 0) ? -shift0 : 0;
					SendMessage(hEditorWnd, EM_SETSEL, start + shift0, start + shift0);
				} else {
					start = SendMessage(hEditorWnd, EM_LINEINDEX, startLineNo, 0);
					end = SendMessage(hEditorWnd, EM_LINEINDEX, endLineNo, 0);
					end += SendMessage(hEditorWnd, EM_LINELENGTH, end, 0);
					PostMessage(hEditorWnd, EM_SETSEL, start, end); // Selection will be dropped for dialog editor if use SendMessage
				}

				SendMessage(hWnd, EM_SETSCROLLPOS, 0, (LPARAM)&scrollPos);
				SetWindowRedraw(hEditorWnd, true);
				InvalidateRect(hEditorWnd, NULL, true);

				return true;
			}

			if (cmd == IDM_ESCAPE) {
				HWND hActiveWnd = GetActiveWindow();
				if (hActiveWnd != hMainWnd)
					return SendMessage(hActiveWnd, WM_CLOSE, 0, 0);

				if (hDragWnd) {
					DestroyWindow(hDragWnd);
					hDragWnd = 0;
					ReleaseCapture();
					ShowCursor(true);
					return true;
				}

				if (!SendMessage(hToolbarWnd, TB_ISBUTTONHIDDEN, IDM_INTERRUPT, 0))
					return SendMessage(hWnd, WM_COMMAND, IDM_INTERRUPT, 0);

				HWND hFocusWnd = GetFocus();
				int focusId = GetDlgCtrlID(hFocusWnd);
				if (focusId == IDC_EDITOR_SEARCH_STRING || focusId == IDC_TREE_SEARCH)
					return SendMessage(hFocusWnd, WM_KEYDOWN, VK_ESCAPE, 0);

				int exitByEscape = prefs::get("exit-by-escape");
				if((exitByEscape == 1 || (exitByEscape == 2 && MessageBox(hMainWnd, TEXT("Are you sure you want to close the app?"), TEXT("Exit confirmation"), MB_YESNO) == IDYES)) &&
					!IsWindowVisible(hAutoComplete) && !GetDlgItem(hMainTabWnd, IDC_TAB_EDIT) && !TreeView_GetEditControl(hTreeWnd)) {
					SendMessage(hMainWnd, WM_CLOSE, 0, 0);
				}
			}

			if (cmd == IDM_QUERY_DATA || cmd == IDM_EDIT_DATA || cmd == IDM_ERASE_DATA ||
				cmd == IDM_ADD || cmd == IDM_VIEW || cmd == IDM_EDIT || cmd == IDM_DELETE ||
				cmd == IDM_PIN_ON_TOP || cmd == IDM_DUPLICATE ||
				cmd == IDM_DIAGRAM || cmd == IDM_GENERATE || cmd == IDM_TRANSFORM ||
				cmd == IDM_ADD_COLUMN || cmd == IDM_ADD_INDEX) {
				TCHAR schema16[256];
				GetWindowText(hSchemaWnd, schema16, 255);

				if (cmd == IDM_EDIT_DATA && _tcscmp(schema16, TEXT("temp")) == 0) {
					SendMessage(hMainWnd, WM_COMMAND, IDM_TEMP_EXPLAIN, 0);
					return 0;
				}

				TCHAR name16[256];
				TV_ITEM tv;
				tv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
				tv.hItem = treeItems[0];
				tv.pszText = name16;
				tv.cchTextMax = 255;

				if(!TreeView_GetItem(hTreeWnd, &tv))
					return 0;

				TCHAR* fullname16 = utils::getFullTableName(schema16, name16, true);
				TCHAR* ufullname16 = _tcsdup(fullname16);
				_tcsupr(ufullname16);

				int type = abs(tv.lParam);
				bool isMain = _tcscmp(schema16, TEXT("main")) == 0;

				if (cmd == IDM_QUERY_DATA) {
					if (SendMessage(hMainTabWnd, WMU_TAB_GET_COUNT, 0, 0) >= MAX_TAB_COUNT)
						return MessageBox(hMainWnd, TEXT("Unable to show data. Too many open tabs."), NULL, MB_OK | MB_ICONWARNING);

					int len = 256 + _tcslen(fullname16);
					TCHAR query[len + 1]{0};

					int tabNo = SendMessage(hMainTabWnd, WMU_TAB_ADD, 0, 0);
					SendMessage(hMainTabWnd, WMU_TAB_SET_CURRENT, tabNo, 0);
					SendMessage(hMainTabWnd, WMU_TAB_SET_TEXT, tabNo, (WPARAM)name16);

					_sntprintf(query, len, TEXT("select * from %ls;\n"), fullname16);
					len = _tcslen(query);
					SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (LPARAM)query);
					SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
					SetFocus(hEditorWnd);

					executeEditorQuery();
					TabCtrl_SetCurFocus(hTabWnd, TabCtrl_GetItemCount(hTabWnd) - 1);
				}

				if (cmd == IDM_EDIT_DATA)
					openDialog(IDD_EDITDATA, (DLGPROC)&dialogs::cbDlgEditData, (LPARAM)fullname16);

				if (cmd == IDM_ERASE_DATA) {
					int len = 256 + _tcslen(fullname16);
					TCHAR query16[len + 1];
					_sntprintf(query16, len, TEXT("Are you sure you want to delete all data from %ls?"), ufullname16);
					if (MessageBox(hMainWnd, query16, TEXT("Delete confirmation"), MB_OKCANCEL) == IDOK) {
						_sntprintf(query16, len, TEXT("delete from %ls;"), fullname16);
						char* query8 = utils::utf16to8(query16);
						if (SQLITE_OK != sqlite3_exec(db, query8, NULL, 0 , 0))
							showDbError(hMainWnd);
						delete [] query8;
					}
				}

				if (cmd == IDM_DIAGRAM)
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_DATABASE_DIAGRAM), hMainWnd, (DLGPROC)tools::cbDlgDatabaseDiagram, (LPARAM)name16);

				if (cmd == IDM_GENERATE)
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_GENERATE_DATA), hMainWnd, (DLGPROC)tools::cbDlgDataGenerator, (LPARAM)name16);

				if (cmd == IDM_TRANSFORM) {
					if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_TRANSFORM_DATA), hMainWnd, (DLGPROC)tools::cbDlgTransformData, (LPARAM)ufullname16)){
						updateTree();
					}
				}

				if (cmd == IDM_ADD && type == TABLE) {
					_sntprintf(name16, 255, schema16);
					if (DLG_OK == DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADD_TABLE), hMainWnd, (DLGPROC)dialogs::cbDlgAddTable, (LPARAM)name16)) {
						updateTables(schema16);
						updateReferences();
						updateTree(TABLE, name16);
					}
				}

				if ((cmd == IDM_ADD && type != TABLE) || ((cmd == IDM_VIEW || cmd == IDM_EDIT) && type != COLUMN)) {
					int len = _tcslen(fullname16) + 2;
					TCHAR buf16[len + 1];
					_sntprintf(buf16, len, TEXT(" %ls"), fullname16);
					int action = cmd == IDM_ADD ? 0 : cmd == IDM_VIEW || (cmd == IDM_EDIT && !isMain) ? 1 : 2;
					buf16[0] = MAKEWORD(action, type);
					openDialog(IDD_ADDVIEWEDIT, (DLGPROC)dialogs::cbDlgAddViewEdit, (LPARAM)buf16);
					return true;
				}

				if (cmd == IDM_DUPLICATE && type == TABLE) {
					int len = 256 + _tcslen(fullname16);
					TCHAR query16[len + 1];
					TCHAR dupname16[len + 1];

					bool rc = false;
					for (int i = 0; i < 10; i++) {
						TCHAR num[10] = {0};
						if (i > 0)
							_sntprintf(num, 9, TEXT("%i"), i + 1);

						_sntprintf(query16, len, TEXT("create table \"%ls\".\"%ls_copy%ls\" as select * from %ls"), schema16, name16, num, fullname16);
						_sntprintf(dupname16, len, TEXT("%ls_copy%ls"), name16, num);
						char* query8 = utils::utf16to8(query16);
						rc = SQLITE_OK == sqlite3_exec(db, query8, NULL, 0 , 0);
						delete [] query8;

						if (rc)
							break;
					}

					if (rc) {
						updateTree(TABLE, dupname16);
					} else {
						showDbError(hMainWnd);
					}
				}

				if (cmd == IDM_DELETE) {
					BOOL isCut = (type == INDEX || type == TRIGGER) && TreeView_GetItemState(hTreeWnd, treeItems[0], TVIS_CUT) & TVIS_CUT;

					TCHAR msg16[512];
					if (type == COLUMN) {
						_tcstok(name16, TEXT(":"));
						TCHAR* uname16 = _tcsdup(name16);
						_tcsupr(uname16);
						_sntprintf(msg16, 511, TEXT("Are you sure you want to delete the %ls %ls?"), TYPES16[type], uname16);
						free(uname16);
					} else {
						_sntprintf(msg16, 511, TEXT("Are you sure you want to delete the %ls %ls?"), TYPES16[type], ufullname16);
					}

					if (IDOK == MessageBox(hMainWnd, msg16, TEXT("Delete confirmation"), MB_OKCANCEL)) {
						if (type == COLUMN) {
							TCHAR tblname16[255];
							HTREEITEM hParent = TreeView_GetParent(hTreeWnd, treeItems[0]);
							TVITEM ti;
							ti.hItem = hParent;
							ti.mask = TVIF_PARAM | TVIF_TEXT;
							ti.pszText = tblname16;
							ti.cchTextMax = 255;
							TreeView_GetItem(hTreeWnd, &ti);
							if (ti.lParam != TABLE)
								return 0;

							delete [] fullname16;
							fullname16 = utils::getFullTableName(schema16, tblname16, false);

							TCHAR query16[1024];
							_sntprintf(query16, 1023, TEXT("alter table %ls drop column \"%ls\""), fullname16, name16);
							char* query8 = utils::utf16to8(query16);
							int rc = sqlite3_exec(db, query8, NULL, 0 , 0);
							delete [] query8;

							if (SQLITE_OK == rc) {
								TreeView_DeleteItem(hTreeWnd, tv.hItem);
								TreeView_SelectItem(hTreeWnd, hParent);
								SetFocus(hTreeWnd);
							} else {
								showDbError(hMainWnd);
							}
						} else {
							char query8[512 + _tcslen(fullname16) + MAX_PATH];
							if (isCut && isMain) {
								char* name8 = utils::utf16to8(name16);
								sprintf(query8, "delete from disabled where name = \"%s\" and type = \"%s\" and dbpath = \"%s\"", name8, TYPES8[type], sqlite3_db_filename(db, 0));
								delete [] name8;
							} else {
								char* fullname8 = utils::utf16to8(fullname16);
 								sprintf(query8, "drop %s %s", TYPES8[type], fullname8);
 								delete [] fullname8;
							}

							if (SQLITE_OK == sqlite3_exec(isCut ? prefs::db : db, query8, 0, 0, 0)) {
								HTREEITEM hItem = TreeView_GetNextItem(hTreeWnd, treeItems[0], TVGN_PREVIOUS);
								if (!hItem)
									hItem = TreeView_GetNextItem(hTreeWnd, treeItems[0], TVGN_NEXT);
								if (!hItem)
									hItem = TreeView_GetNextItem(hTreeWnd, treeItems[0], TVGN_PARENT);

								TreeView_DeleteItem(hTreeWnd, treeItems[0]);
								TreeView_SelectItem(hTreeWnd, hItem);
								treeItems[0] = hItem;
							} else {
								showDbError(hWnd);
							}
						}
					}
				}

				if (cmd == IDM_PIN_ON_TOP) {
					bool isPinned = isObjectPinned(name16);
					sqlite3_stmt* stmt;
					bool rc = SQLITE_OK == sqlite3_prepare_v2(prefs::db, isPinned ? "delete from pinned where dbname = ?1 and name = ?2" : "insert into pinned (dbname, name) values (?1, ?2)", -1, &stmt, 0);
					if (rc) {
						char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0), false);
						char* name8 = utils::utf16to8(name16);

						sqlite3_bind_text(stmt, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt, 2, name8, strlen(name8), SQLITE_TRANSIENT);
						rc = SQLITE_DONE == sqlite3_step(stmt);

						delete [] dbname8;
						delete [] name8;
					}
					sqlite3_finalize(stmt);

					if (rc)
						updateTree(abs(type), name16);
				}

				if (cmd == IDM_ADD_COLUMN) {
					if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADD_COLUMN), hMainWnd, (DLGPROC)dialogs::cbDlgAddColumn, (LPARAM)fullname16))
						updateTree(TABLE, name16);
					SetFocus(hTreeWnd);
				}

				if (cmd == IDM_ADD_INDEX) {
					if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADD_INDEX), hMainWnd, (DLGPROC)dialogs::cbDlgAddIndex, (LPARAM)fullname16))
						updateTree(INDEX);
				}

				delete [] fullname16;
				free(ufullname16);

				if (cmd != IDM_EDIT_DATA && cmd != IDM_EDIT && cmd != IDM_VIEW)
					SetFocus(hTreeWnd);
			}

			if (cmd == IDM_RENAME)
				(void)TreeView_EditLabel(hTreeWnd, treeItems[0]);

			if (cmd == IDM_REFRESH) {
				TV_ITEM tv{0};
				tv.mask = TVIF_PARAM | TVIF_TEXT;
				tv.hItem = treeItems[0];
				TreeView_GetItem(hTreeWnd, &tv);
				updateTree(abs(tv.lParam));
			}

			if (cmd == IDM_ENABLE || cmd == IDM_DISABLE || cmd == IDM_ENABLE_ALL || cmd == IDM_DISABLE_ALL) {
				TCHAR name16[256] = {0};
				TV_ITEM tv;
				tv.mask = TVIF_PARAM | TVIF_TEXT;
				tv.hItem = treeItems[0];
				tv.cchTextMax = 255;
				tv.pszText = name16;
				if(!TreeView_GetItem(hTreeWnd, &tv) || !tv.lParam)
					return 0;

				int type = abs(tv.lParam);
				bool rc = true;

				if (cmd == IDM_ENABLE) {
					char* name8 = utils::utf16to8(name16);
					rc = enableDbObject(name8, type);
					delete [] name8;
				}

				if (cmd == IDM_DISABLE) {
					char* name8 = utils::utf16to8(name16);
					rc = disableDbObject(name8, type);
					delete [] name8;
				}

				if (cmd == IDM_ENABLE_ALL) {
					sqlite3_stmt *stmt;
					sqlite3_prepare_v2(prefs::db, "select name from disabled where type = ?1 and dbpath = ?2", -1, &stmt, 0);
					sqlite3_bind_text(stmt, 1, TYPES8[type], strlen(TYPES8[type]), SQLITE_TRANSIENT);
					const char* dbpath = sqlite3_db_filename(db, 0);
					sqlite3_bind_text(stmt, 2, dbpath, strlen(dbpath), SQLITE_TRANSIENT);
					while (SQLITE_ROW == sqlite3_step(stmt))
						enableDbObject((const char*)sqlite3_column_text(stmt, 0), type);
					sqlite3_finalize(stmt);
				}

				// Uses names8 list to prevent database lock
				if (cmd == IDM_DISABLE_ALL) {
					char names8[MAX_TEXT_LENGTH]{0};

					sqlite3_stmt *stmt;
					sqlite3_prepare_v2(db, "select name from sqlite_master where type = ?1 and sql is not null", -1, &stmt, 0);
					sqlite3_bind_text(stmt, 1, TYPES8[type], strlen(TYPES8[type]), SQLITE_TRANSIENT);
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						strcat(names8, (const char*)sqlite3_column_text(stmt, 0));
						strcat(names8, ";");
					}
					sqlite3_finalize(stmt);

					char* name8;
					name8 = strtok (names8, ";");
					while (name8 != NULL) {
						disableDbObject(name8, type);
						name8 = strtok (NULL, ";");
					}
				}

				if (rc)
					updateTree(type, name16);
				SetFocus(hTreeWnd);
			}

			if (cmd >= IDM_RECENT && cmd <= IDM_RECENT + prefs::get("recent-count")) {
				TCHAR name16[MAX_PATH];
				GetMenuString(hDbMenu, cmd, name16, MAX_PATH, MF_BYCOMMAND);
				openDb(name16, HIWORD(GetKeyState(VK_CONTROL)));
			}

			if (cmd == IDM_SAVE && IDYES == MessageBox(0, TEXT("Save query?"), TEXT("Confirmation"), MB_YESNO)) {
				CHARRANGE range;
				SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&range);

				bool isSelection = range.cpMin != range.cpMax;
				int size =  isSelection ? range.cpMax - range.cpMin + 1 : GetWindowTextLength(hEditorWnd);
				if (size > 0) {
					TCHAR text16[size + 1]{0};
					if (SendMessage(hEditorWnd, isSelection ? EM_GETSELTEXT : WM_GETTEXT, size + 1, (LPARAM)text16)) {
						TCHAR* ttext16 = utils::trim(text16);
						char* text8 = utils::utf16to8(ttext16);
						saveQuery("gists", text8);
						delete [] ttext16;
						delete [] text8;
					}
				}
				SetFocus(hEditorWnd);
			}

			if (cmd == IDM_TAB_RESULT_PIN || cmd == IDM_TAB_RESULT_COPY_QUERY) {
				ULONG_PTR data = Menu_GetData(hTabResultMenu);
				int tabNo = LOWORD(data);
				int resultNo = HIWORD(data);

				if (tabNo == -1 || resultNo >= MAX_RESULT_COUNT)
					return false;

				if (cmd == IDM_TAB_RESULT_PIN) {
					bool isPinned = tabs[tabNo].isPinned[resultNo];
					isPinned = !isPinned;
					tabs[tabNo].isPinned[resultNo] = isPinned;

					TCITEM ti = {0};
					ti.mask = TCIF_IMAGE;
					ti.iImage = isPinned ? 0 : -1;
					TabCtrl_SetItem(tabs[tabNo].hTabWnd, resultNo, &ti);
					InvalidateRect(tabs[tabNo].hTabWnd, NULL, TRUE);
				}

				if (cmd == IDM_TAB_RESULT_COPY_QUERY) {
					TCHAR* query16 = tabs[tabNo].tabTooltips[resultNo];
					utils::setClipboardText(query16);
				}
			}

			if (cmd == IDM_RESULT_FILTERS) {
				HWND hListWnd = (HWND)SendMessage(hWnd, WMU_GET_CURRENT_RESULTSET, 0, 0);
				HWND hHeader = ListView_GetHeader(hListWnd);
				bool isShowFilters = GetWindowLongPtr(hHeader, GWL_STYLE) & HDS_FILTERBAR;
				isShowFilters = (isShowFilters + 1) % 2;
				prefs::set("show-filters", isShowFilters);

				for (int colNo = 1; !isShowFilters && colNo < Header_GetItemCount(hHeader); colNo++)
					SetDlgItemText(hHeader, IDC_HEADER_EDIT + colNo, TEXT(""));

				SendMessage(hListWnd, WMU_UPDATE_RESULTSET, 0, 0);
			}

			if (cmd == IDM_RESULT_PREVIEW) {
				bool isShowPreview = prefs::get("show-preview");
				isShowPreview = (isShowPreview + 1) % 2;
				prefs::set("show-preview", isShowPreview);

				EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_UPDATETAB);
				EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_RESIZETAB);

				HWND hListWnd = (HWND)SendMessage(hWnd, WMU_GET_CURRENT_RESULTSET, 0, 0);
				int rowNo = (int)(LONG_PTR)GetProp(hListWnd, TEXT("CURRENTROW"));
				int colNo = (int)(LONG_PTR)GetProp(hListWnd, TEXT("CURRENTCOLUMN"));
				if (isShowPreview) {
					SendMessage(hTabWnd, WMU_SET_CURRENT_CELL, -1, -1); // force update preview
					SendMessage(hTabWnd, WMU_SET_CURRENT_CELL, rowNo, colNo);
				} else {
					RECT rc;
					ListView_GetSubItemRect(hListWnd, rowNo, colNo, LVIR_BOUNDS, &rc);
					InvalidateRect(hListWnd, &rc, TRUE);
				}
				InvalidateRect(hTabWnd, NULL, TRUE);
			}

			if (cmd == IDM_RESULT_TRANSPOSE) {
				HWND hListWnd = (HWND)SendMessage(hWnd, WMU_GET_CURRENT_RESULTSET, 0, 0);
				HWND hParentWnd = GetParent(hListWnd);
				int resultNo = GetWindowLongPtr(hListWnd, GWLP_USERDATA);
				if (GetProp(hListWnd, TEXT("ISTRANSPOSED"))) {
					HWND hOriginalListWnd = GetDlgItem(hListWnd, IDC_TAB_ROWS + resultNo);
					SetParent(hOriginalListWnd, hParentWnd);
					DestroyWindow(hListWnd);
					ShowWindow(hOriginalListWnd, SW_SHOW);
				} else {
					HWND hTransposedWnd = createResultList(hParentWnd, resultNo);
					SetProp(hTransposedWnd, TEXT("ISTRANSPOSED"), IntToPtr(1));
					SetParent(hListWnd, hTransposedWnd);
					ShowWindow(hListWnd, SW_HIDE);

					sqlite3_exec(db, "drop table temp.transposed", 0, 0, 0);

					HWND hHeader = ListView_GetHeader(hListWnd);
					int colCount = ListView_GetColumnCount(hListWnd);
					int rowCount = ListView_GetItemCount(hListWnd);

					char create8[colCount * 255 + 1024]{0};
					char insert8[colCount * 255 + 1024]{0};
					sprintf(create8, "create table temp.transposed (name, ");
					sprintf(insert8, "insert into temp.transposed (name, ");
					for (int rowNo = 1; rowNo <= rowCount; rowNo++) {
						char buf8[255];
						sprintf(buf8, "R%i%s", rowNo, rowNo < rowCount ? ", " : ")");
						strcat(create8, buf8);
						strcat(insert8, buf8);
					}

					strcat(insert8, " values (");
					for (int rowNo = 0; rowNo <= rowCount; rowNo++)
						strcat(insert8, rowNo < rowCount ? "?, " : "?)");

					sqlite3_exec(db, create8, 0, 0, 0);

					TCHAR*** cache = (TCHAR***)GetProp(hListWnd, TEXT("CACHE"));
					int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
					byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
					unsigned char** blobs = (unsigned char**)GetProp(hListWnd, TEXT("BLOBS"));

					sqlite3_stmt* stmt;
					sqlite3_prepare_v2(db, insert8, -1, &stmt, 0);
					for (int colNo = 1; colNo < colCount; colNo++) {
						TCHAR colname16[256];
						Header_GetItemText(hHeader, colNo, colname16, 255);
						char* colname8 = utils::utf16to8(colname16);
						sqlite3_bind_text(stmt, 1, colname8, strlen(colname8), SQLITE_TRANSIENT);
						delete [] colname8;

						for (int rowNo = 0; rowNo < rowCount; rowNo++) {
							int no = colNo + resultset[rowNo] * (colCount - 1);
							const unsigned char* blob = datatypes && blobs && datatypes[no] == SQLITE_BLOB ? blobs[no] : 0;
							if (!blob) {
								char* value8 = utils::utf16to8(cache[resultset[rowNo]][colNo]);
								dbutils::bind_variant(stmt, rowNo + 2, value8, datatypes && datatypes[no] == SQLITE_TEXT);
								delete [] value8;
							} else {
								sqlite3_bind_blob(stmt, rowNo + 2, blob + 4, utils::getBlobSize(blob) - 4, SQLITE_TRANSIENT);
							}
						}

						sqlite3_step(stmt);
						sqlite3_reset(stmt);
					}
					sqlite3_finalize(stmt);

					sqlite3_prepare_v2(db, "select * from temp.transposed", -1, &stmt, 0);
					ListView_SetData(hTransposedWnd, stmt);
					sqlite3_finalize(stmt);

					LVCOLUMN lvc = {mask: LVCF_FMT, fmt: LVCFMT_LEFT};
					for (int colNo = 1; colNo <= rowCount + 1; colNo++)
						ListView_SetColumn(hTransposedWnd, colNo, &lvc);
				}

				SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);
			}

			if (cmd >= IDM_RESULT_MODIFIER && cmd <= IDM_RESULT_MODIFIER + 100) {
				HWND hListWnd = (HWND)SendMessage(hWnd, WMU_GET_CURRENT_RESULTSET, 0, 0);
				int colNo = currCell.iSubItem;

				int* modifiers = (int*)GetProp(hListWnd, TEXT("MODIFIERS"));
				int* modifierRows = (int*)GetProp(hListWnd, TEXT("MODIFIER_ROWS"));
				if (!modifiers || !modifierRows) {
					MessageBox(hWnd, TEXT("Can't set modifier for the column: no modifiers"), TEXT("Internal error"), MB_OK);
					return false;
				}

				SendMessage(hListWnd, WMU_RESET_MODIFIER, colNo, 0);

				bool rc = false;
				TCHAR err16[MAX_MODIFIER_ERROR_LEN + 1] {0};
				int modifierNo = Menu_GetItemData(hModifierMenu, cmd);
				if ((modifierNo > 0 && modifierNo < MAX_PLUGIN_COUNT && plugins[modifierNo].hModule) || modifierNo <= 0) {
					rc = true;
					modifierRows[colNo] = currCell.iItem;
					if (modifierNo > 0 && plugins[modifierNo].activate)
						rc = plugins[modifierNo].activate(hListWnd, colNo, err16);

					InvalidateRect(hListWnd, NULL, TRUE);
				}

				modifiers[colNo] = rc ? modifierNo : 0;
				if (!rc && _tcslen(err16))
					MessageBox(hWnd, err16, plugins[modifierNo].name, MB_OK);
			}

			if (cmd == IDM_RESULT_CHART || cmd == IDM_RESULT_VALUE_FILTER || cmd == IDM_RESULT_COPY_CELL || cmd == IDM_RESULT_COPY_ROW || cmd == IDM_RESULT_EXPORT)
				onListViewMenu(currCell.hListWnd, currCell.iItem, currCell.iSubItem, cmd);

			if (cmd == IDM_RESULT_AS_TABLE) {
				HWND hListWnd = currCell.hListWnd;
				TCHAR table16[255];
				_tcscpy(table16, TEXT("result"));
				if (DLG_CANCEL == DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_TABLENAME), hMainWnd, (DLGPROC)&dialogs::cbDlgTableName, (LPARAM)table16))
					return false;

				int tabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
				int resultNo = GetWindowLongPtr(hListWnd, GWLP_USERDATA);
				int searchNext = ListView_GetSelectedCount(hListWnd) < 2 ? LVNI_ALL : LVNI_SELECTED;
				if (saveResultToTable(db, table16, tabNo, resultNo, searchNext)) {
					MessageBeep(0);
					updateTree(TABLE, table16);
					SetFocus(hTreeWnd);
				}
				return true;
			}

			if (cmd == IDM_RESULT_EXCEL) {
				HWND hListWnd = currCell.hListWnd;
				int currTabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
				HWND hCurrListWnd = (HWND)GetWindowLongPtr(tabs[currTabNo].hTabWnd, GWLP_USERDATA);
				int currResultNo = GetWindowLongPtr(hCurrListWnd, GWLP_USERDATA);

				TCHAR path16[MAX_PATH + 1];
				SYSTEMTIME st;
				GetLocalTime(&st);
				_sntprintf(path16, MAX_PATH, TEXT("%ls\\Query-result-%.2u-%.2u.xlsx"), TMP_PATH, st.wHour, st.wMinute);

				int searchNext = ListView_GetSelectedCount(hListWnd) < 2 ? LVNI_ALL : LVNI_SELECTED;
				sqlite3_exec(db, "drop table temp.excel;", 0, 0, 0);
				saveResultToTable(db, TEXT("temp.excel"), currTabNo, currResultNo, searchNext);
				if (tools::exportExcel(path16, TEXT("select * from temp.excel")))
					ShellExecute(0, TEXT("open"), path16, NULL, NULL, 0);
				else
					MessageBox(hMainWnd, TEXT("Can't export data to Excel"), NULL, MB_OK);

				return true;
			}

			if (cmd >= IDM_RESULT_COMPARE && cmd <= IDM_RESULT_COMPARE + MAX_COMPARE_RESULT) {
				int currTabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
				HWND hCurrListWnd = (HWND)SendMessage(hWnd, WMU_GET_CURRENT_RESULTSET, 0, 0);
				int currResultNo = GetWindowLongPtr(hCurrListWnd, GWLP_USERDATA);

				HMENU hCompareMenu = GetSubMenu(hResultMenu, GetMenuItemCount(hResultMenu) - 1);

				ULONG_PTR data = Menu_GetItemData(hCompareMenu, cmd);
				int tabNo = LOWORD(data);
				int resultNo = HIWORD(data);

				TCHAR title[1024]{0};
				TCHAR buf[256];
				SendMessage(hMainTabWnd, WMU_TAB_GET_TEXT, currTabNo, (LPARAM)buf);
				_tcscat(title, buf);
				_tcscat(title, TEXT(" > "));
				TabCtrl_GetItemText(tabs[currTabNo].hTabWnd, currResultNo, buf, 255);
				_tcscat(title, buf);
				_tcscat(title, TEXT("   vs   "));
				SendMessage(hMainTabWnd, WMU_TAB_GET_TEXT, tabNo, (LPARAM)buf);
				_tcscat(title, buf);
				_tcscat(title, TEXT(" > "));
				TabCtrl_GetItemText(tabs[tabNo].hTabWnd, resultNo, buf, 255);
				_tcscat(title, buf);

				sqlite3_exec(db, "drop table temp.result1; drop table temp.result2;", 0, 0, 0);
				saveResultToTable(db, TEXT("temp.result1"), currTabNo, currResultNo);
				saveResultToTable(db, TEXT("temp.result2"), tabNo, resultNo);
				openDialog(IDD_RESULTS_COMPARISON, (DLGPROC)&dialogs::cbDlgResultsComparison, (LPARAM)title);
				return true;
			}

			if (cmd >= IDM_SCHEMA && cmd <= IDM_SCHEMA + MAX_SCHEMA_COUNT) {
				TCHAR schema16[256];
				GetMenuString(hSchemaMenu, cmd, schema16, 255, MF_BYCOMMAND);
				switchSchemaTo(schema16);
			}

			if (cmd == IDM_AI_HELP)
				ShellExecute(0, 0, TEXT("https://github.com/little-brother/sqlite-gui/wiki#ai-assistance"), 0, 0, SW_SHOW);

			if (cmd == IDM_AI_DB_ACCESS)
				prefs::set("ai-has-db-access", !prefs::get("ai-has-db-access"));

			if (cmd == IDM_AI_EXPLAIN_ERRORS)
				prefs::set("ai-explain-query-error", !prefs::get("ai-explain-query-error"));

			if (cmd == IDM_AI_DO_FORMAT)
				prefs::set("ai-do-format", !prefs::get("ai-do-format"));

			if (cmd == IDM_AI_PROCESS_NOSQL)
				prefs::set("ai-process-nosql", !prefs::get("ai-process-nosql"));

			if (cmd == IDM_EDITOR_COMMENT)
				toggleComment(hEditorWnd);

			if (cmd == IDM_EDITOR_COMPARE) {
				TDlgParam dp = {
					0,
					utils::getClipboardText(),
					TEXT("Editor"),
					TEXT("Clipboard"),
				};

				CHARRANGE range;
				SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&range);
				bool isSelection = range.cpMin != range.cpMax;
				int len = isSelection ? range.cpMax - range.cpMin + 1 : GetWindowTextLength(hEditorWnd);
				dp.s1 = new TCHAR[len + 1]{0};
				SendMessage(hEditorWnd, isSelection ? EM_GETSELTEXT : WM_GETTEXT, len + 1, (LPARAM)dp.s1);

				DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_TEXT_COMPARISON), hWnd, (DLGPROC)tools::cbDlgTextComparison, (LPARAM)&dp);

				delete [] dp.s1;
				delete [] dp.s2;

				SetFocus(hEditorWnd);
				return true;
			}

			if (cmd == IDM_EDITOR_FIX_ERROR) {
				if (!prefs::get("ai-enable"))
					return 0;

				TCHAR* query16 = getEditorText(hEditorWnd);
				TCHAR* err16 = dbutils::getQueryError(db, query16);

				if (!err16) {
					delete [] query16;
					showPopup(TEXT("No errors"), hEditorWnd);
					return 0;
				}

				int sid = aiGetSid();
				TCHAR* fixed16 = aiRequest(TEXT("fix-error-prompt"), query16, err16, sid);
				delete [] err16;

				for (int attemptNo = 0; attemptNo < prefs::get("ai-fix-attempt-count") && err16; attemptNo++) {
					err16 = dbutils::getQueryError(db, fixed16);
					if (err16 == 0)
						break;

					fixed16 = aiRequest(TEXT("fix-error-next-prompt"), fixed16, err16, sid);
				}

				if (fixed16) {
					char* query8 = utils::utf16to8(query16);
					char* fixed8 = utils::utf16to8(fixed16);
					char* cmp8 = new char[MAX_TEXT_LENGTH] {0};

					struct TDiffData {
						char* query8;
						char* fixed8;
						char* cmp8;
					};

					auto cbDiff = [](void* ref, dmp_operation_t op, const void* data8, uint32_t len8) {
						if (op == DMP_DIFF_EQUAL)
							return 0;

						TDiffData* dd = (TDiffData*)ref;
						char* query8 = (char*)dd->query8;
						char* fixed8 = (char*)dd->fixed8;
						char* cmp8 = (char*)dd->cmp8;
						char* txt8 = (char*)data8;

						if (strlen(cmp8) > MAX_TEXT_LENGTH + 100)
							return 0;

						bool isSkip = true;
						for (int i = 0; i < (int)len8; i++)
							isSkip = isSkip && strchr(" \r\n", txt8[i]);
						if (isSkip)
							return 0;

						if (cmp8[0])
							strcat(cmp8, "\n");

						int len = len8 + 64;
						char* buf8 = new char[len + 1] {0};
						char* src8 = op == DMP_DIFF_DELETE ? query8 : fixed8;

						int pos = 0;
						int clen = 16;

						buf8[pos] = '>';
						pos++;

						buf8[pos] = '>';
						pos++;

						buf8[pos] = ' ';
						pos++;

						int shift = txt8 - src8 > clen ? txt8 - src8 - clen : 0;
						for (int i = 0; i < MIN(txt8 - src8, clen) ; i++) {
							buf8[pos] = strchr(" \r\n", src8[i + shift]) ? ' ' : src8[i + shift];
							pos++;
						}

						buf8[pos] = '(';
						pos++;

						buf8[pos] = op == DMP_DIFF_DELETE ? '-' : '+';
						pos++;

						for (int i = 0; i < (int)len8; i++) {
							buf8[pos] = txt8[i];
							pos++;
						}

						buf8[pos] = ')';
						pos++;

						for (int i = (int)len8; i < (clen + (int)len8) && txt8[i]; i++) {
							buf8[pos] = strchr(" \r\n", txt8[i]) ? ' ' : txt8[i];
							pos++;
						}

						strcat(cmp8, buf8);
						delete [] buf8;

						return 0;
					};

					dmp_diff* diff;
					dmp_diff_from_strs(&diff, NULL, query8, fixed8);
					TDiffData dd {query8, fixed8, cmp8};
					dmp_diff_foreach(diff, cbDiff, &dd);
					dmp_diff_free(diff);

					TCHAR* cmp16 = utils::utf8to16(cmp8);
					int len = _tcslen(cmp16) + 256;
					TCHAR* msg16 = new TCHAR[len + 1] {0};
					_sntprintf(msg16, len, TEXT("Changes\n\n%ls\n\nCtrl + Z to revert changes"), cmp16);
					showPopup(msg16, hEditorWnd);

					delete [] msg16;
					delete [] cmp16;

					delete [] cmp8;
					delete [] query8;
					delete [] fixed8;

					SendMessage(hEditorWnd, WM_SETREDRAW, FALSE, 0);
					int sx = GetScrollPos(hEditorWnd, SB_HORZ);
					int sy = GetScrollPos(hEditorWnd, SB_VERT);
					setEditorText(hEditorWnd, fixed16);
					SetScrollPos(hEditorWnd, SB_HORZ, sx, FALSE);
					SetScrollPos(hEditorWnd, SB_VERT, sy, FALSE);
					SendMessage(hEditorWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hEditorWnd, NULL, FALSE);

					delete [] fixed16;
				}

				if (err16)
					delete [] err16;

				delete [] query16;
			}

			if (cmd == IDM_EDITOR_CUT)
				SendMessage(hEditorWnd, WM_CUT, 0, 0);

			if (cmd == IDM_EDITOR_COPY)
				SendMessage(hEditorWnd, WM_COPY, 0, 0);

			if (cmd == IDM_EDITOR_PASTE)
				pasteText (hEditorWnd, true);

			if (cmd == IDM_EDITOR_DELETE)
				SendMessage (hEditorWnd, EM_REPLACESEL, TRUE, 0);

			if (cmd == IDM_EDITOR_FIND) {
				DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_FIND), hWnd, (DLGPROC)dialogs::cbDlgFind, (LPARAM)hEditorWnd);
				SetFocus(hEditorWnd);
				return true;
			}

			if (cmd == IDM_EDITOR_FORMAT) {
				TCHAR* text16 = getEditorText(hEditorWnd);
				TCHAR* formatted16 = prefs::get("ai-enable") && prefs::get("ai-do-format") ? aiRequest(TEXT("format-prompt"), text16) : formatQuery(text16);
				if (formatted16) {
					setEditorText(hEditorWnd, formatted16);
					processHighlight(hEditorWnd, true, true, true);
					delete [] formatted16;
				} else {
					MessageBox(hWnd, TEXT(AI_ERROR_MESSAGE), 0, MB_OK);
				}
				delete [] text16;
			}

			if (cmd == IDM_CLI_COPY)
				SendMessage(cli.hResultWnd, WM_COPY, 0, 0);

			if (cmd == IDM_CLI_CUT) {
				CHARRANGE range;
				SendMessage(cli.hResultWnd, EM_EXGETSEL, 0, (LPARAM)&range);
				int size =  range.cpMax - range.cpMin;
				if (size > 1) {
					TCHAR buf16[size + 1]{0};
					SendMessage(cli.hResultWnd, EM_GETSELTEXT, size + 1, (LPARAM)buf16);
					utils::setClipboardText(buf16);
				}

				SendMessage(cli.hResultWnd, EM_REPLACESEL, 0, 0);
			}

			if (cmd == IDM_CLI_CLEAR_ALL)
				SetWindowText(cli.hResultWnd, 0);

			if (cmd == IDM_ABOUT) {
				OSVERSIONINFO vi = {0};
				vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

				NTSTATUS (WINAPI *pRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation) = NULL;
				HINSTANCE hNTdllDll = LoadLibrary(TEXT("ntdll.dll"));

				if (hNTdllDll != NULL) {
					pRtlGetVersion = (NTSTATUS (WINAPI *)(PRTL_OSVERSIONINFOW))
					GetProcAddress (hNTdllDll, "RtlGetVersion");

					if (pRtlGetVersion != NULL)
						pRtlGetVersion((PRTL_OSVERSIONINFOW)&vi);

					FreeLibrary(hNTdllDll);
				}

				if (pRtlGetVersion == NULL)
					GetVersionEx(&vi);

				BOOL isWin64 = FALSE;
				#if defined(_WIN64)
					isWin64 = TRUE;
				#else
					isWin64 = IsWow64Process(GetCurrentProcess(), &isWin64) && isWin64;
				#endif

				TCHAR buf[1024];
				_sntprintf(buf, 1023, TEXT("SQLite database editor\n\nGUI version: %ls\nArchitecture: %ls\nSQLite version: %hs\nEncryption support: %ls\nBuild date: %ls\nGCC: %i.%i.%i\n\nOS: %i.%i.%i %ls %ls"),
					TEXT(GUI_VERSION),
					GUI_PLATFORM == 32 ? TEXT("x86") : TEXT("x86-64"),
					sqlite3_libversion(),
					isCipherSupport ? TEXT("Yes") : TEXT("No"),
					TEXT(__DATE__),
					__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__,
					vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber, vi.szCSDVersion,
					isWin64 ? TEXT("x64") : TEXT("x32")
				);

				MessageBox(hMainWnd, buf, TEXT("About sqlite-gui"), MB_OK);
			}

			if (cmd == IDM_TIPS || cmd == IDM_HOTKEYS) {
				TCHAR info16[MAX_TEXT_LENGTH];
				LoadString(GetModuleHandle(0), cmd == IDM_TIPS ? IDS_TIPS : IDS_HOTKEYS, info16, MAX_TEXT_LENGTH);

				TDlgParam dp = {
						info16,
						cmd == IDM_TIPS ? TEXT("Tips") : TEXT("Shortcuts")
				};

				DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_INFO), hWnd, (DLGPROC)dialogs::cbDlgInfo, (LPARAM)&dp);
				return true;
			}

			if (cmd == IDM_TEMP_EXPLAIN) {
				TCHAR buf[MAX_TEXT_LENGTH];
				LoadString(GetModuleHandle(NULL), IDS_TEMP_EXPLAIN, buf, MAX_TEXT_LENGTH);
				MessageBox(hMainWnd, buf, TEXT("Temp schema explanation"), MB_OK | MB_ICONINFORMATION);
			}

			if (cmd == IDM_HOMEPAGE)
				ShellExecute(0, 0, TEXT("https://github.com/little-brother/sqlite-gui"), 0, 0, SW_SHOW);

			if (cmd == IDM_WIKI)
				ShellExecute(0, 0, TEXT("https://github.com/little-brother/sqlite-gui/wiki"), 0, 0, SW_SHOW);

			if (cmd == IDM_DEMODB_BOOKSTORE || cmd == IDM_DEMODB_CHINOOK || cmd == IDM_DEMODB_NORTHWIND || cmd == IDM_DEMODB_WORLD) {
				TCHAR name16[64];
				_sntprintf(name16, 63, TEXT("%ls"),
					cmd == IDM_DEMODB_BOOKSTORE ? TEXT("bookstore") :
					cmd == IDM_DEMODB_CHINOOK ? TEXT("chinook") :
					cmd == IDM_DEMODB_NORTHWIND ? TEXT("northwind") :
					cmd == IDM_DEMODB_WORLD ? TEXT("world") :
					TEXT(""));

				TCHAR path16[MAX_PATH + 1];
				_sntprintf(path16, MAX_PATH, TEXT("%ls\\examples\\%ls.sqlite"), APP_PATH, name16);
				if (utils::isFileExists(path16))
					return openDb(path16);

				TCHAR url16[1024];
				_sntprintf(url16, 1023, TEXT("https://github.com/little-brother/sqlite-example-databases/raw/main/%ls/%ls.sqlite"), name16, name16);

				TCHAR msg16[1024];
				_sntprintf(msg16, 1023, TEXT("Download and open a demo database %ls.sqlite?\nThe database will be placed to %ls"), name16, path16);
				if (MessageBox(hWnd, msg16, TEXT("Confirmation"), MB_OKCANCEL) != IDOK)
					return false;

				if (utils::downloadFile(url16, path16))
					return openDb(path16);
			}

			if (cmd == IDM_SQLITE_HOMEPAGE)
				ShellExecute(0, 0, TEXT("https://www.sqlite.org/index.html"), 0, 0, SW_SHOW);

			if (cmd == IDM_TUTORIAL1)
				ShellExecute(0, 0, TEXT("https://www.sqlitetutorial.net/"), 0, 0, SW_SHOW);

			if (cmd == IDM_TUTORIAL2)
				ShellExecute(0, 0, TEXT("https://www.tutorialspoint.com/sqlite/"), 0, 0, SW_SHOW);

			if (cmd == IDM_HISTORY || cmd == IDM_GISTS) {
				DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_QUERYLIST), hMainWnd, (DLGPROC)dialogs::cbDlgQueryList, (LPARAM)cmd);
				SetFocus(hEditorWnd);
			}

			if (cmd == IDM_DATABASE_DIAGRAM)
				openDialog(IDD_TOOL_DATABASE_DIAGRAM, (DLGPROC)tools::cbDlgDatabaseDiagram, 0);

			if (cmd == IDM_COMPARE_DATABASE)
				openDialog(IDD_TOOL_COMPARE_DATABASE, (DLGPROC)tools::cbDlgCompareDatabase, 0);

			if (cmd == IDM_DATABASE_SEARCH)
				openDialog(IDD_TOOL_DATABASE_SEARCH, (DLGPROC)tools::cbDlgDatabaseSearch, 0);

			if (cmd == IDM_IMPORT_SQL) {
				TCHAR path16[MAX_PATH];
				if(utils::openFile(path16, TEXT("*.sql\0*.sql\0All\0*.*\0"), hWnd) && tools::importSqlFile(path16)) {
					updateTree();
					MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);
				}
			}

			if (cmd == IDM_IMPORT_CSV) {
				TCHAR path16[MAX_PATH];
				if(utils::openFile(path16, TEXT("CSV files\0*.csv\0All\0*.*\0"), hWnd)) {
					int rc = DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_IMPORT_CSV), hMainWnd, (DLGPROC)&tools::cbDlgImportCSV, (LPARAM)path16);
					if (rc != -1) {
						TCHAR msg16[256];
						_sntprintf(msg16, 255, TEXT("Done.\nImported %i rows."), rc);
						MessageBox(hMainWnd, msg16, TEXT("Info"), MB_OK);
						updateTree(TABLE, path16);
						SetFocus(hTreeWnd);
					}
				}
			}

			if (cmd == IDM_IMPORT_JSON) {
				TCHAR path16[MAX_PATH];
				if(utils::openFile(path16, TEXT("Json files\0*.json\0All\0*.*\0"), hWnd)) {
					if (DLG_OK == DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_IMPORT_JSON), hMainWnd, (DLGPROC)&tools::cbDlgImportJSON, (LPARAM)path16)) {
						MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);
						updateTree(TABLE, path16);
						SetFocus(hTreeWnd);
					}
				}
			}

			if (cmd == IDM_IMPORT_GOOGLE_SHEETS) {
				char* googleApiKey = prefs::get("google-api-key", "");
				bool isKeyEmpty = strlen(googleApiKey) == 0;
				delete [] googleApiKey;
				if (isKeyEmpty) {
					MessageBox(hMainWnd, TEXT("Google API key is not set in Settings.\nCheck Wiki to find out how to get the key."), TEXT(" Error"), MB_ICONERROR);
					SendMessage(hMainWnd, WM_COMMAND, IDM_SETTINGS, 2);
					return 0;
				}

				TCHAR tablename16[1024]{0};
				if (DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_IMPORT_SHEET), hMainWnd, (DLGPROC)tools::cbDlgImportSheet, (LPARAM)tablename16) == DLG_OK) {
					updateTree(TABLE, tablename16);
					MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);
					SetFocus(hTreeWnd);
				}
			}

			if (cmd == IDM_IMPORT_ODBC || cmd == IDM_EXPORT_ODBC) {
				bool isExport = cmd == IDM_EXPORT_ODBC;
				DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_EXPORT_IMPORT_ODBC), hMainWnd, (DLGPROC)&tools::cbDlgExportImportODBC, isExport);
				if (!isExport)
					updateTree();
			}

			if ((cmd == IDM_EXPORT_CSV) && (DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_EXPORT_CSV), hMainWnd, (DLGPROC)tools::cbDlgExportCSV) == DLG_OK))
				MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);

			if ((cmd == IDM_EXPORT_JSON) && (DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_EXPORT_JSON), hMainWnd, (DLGPROC)tools::cbDlgExportJSON) == DLG_OK))
				MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);

			if ((cmd == IDM_EXPORT_EXCEL) && (DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_EXPORT_JSON), hMainWnd, (DLGPROC)tools::cbDlgExportExcel) == DLG_OK))
				MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);

			if ((cmd == IDM_EXPORT_SQL) && (DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_EXPORT_SQL), hMainWnd, (DLGPROC)tools::cbDlgExportSQL) == DLG_OK))
				MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);

			if (cmd == IDM_GENERATE_DATA)
				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_GENERATE_DATA), hMainWnd, (DLGPROC)tools::cbDlgDataGenerator);

			if (cmd == IDM_TRANSFORM_DATA) {
				if (DLG_OK == DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_TRANSFORM_DATA), hMainWnd, (DLGPROC)tools::cbDlgTransformData))
					updateTree();
			}

			if (cmd == IDM_LOCATE_FILE) {
				const char* dbname8 = sqlite3_db_filename(db, 0);
				TCHAR* dbname16 = utils::utf8to16(dbname8);
				TCHAR path16[MAX_PATH + 20];
				_sntprintf(path16, MAX_PATH + 20, TEXT("/select,%ls"), dbname16);
				delete [] dbname16;
				ShellExecute(0, 0, TEXT("explorer"), path16, 0, SW_SHOWNORMAL);
			}

			if (cmd == IDM_STATISTICS)
				openDialog(IDD_TOOL_STATISTICS, (DLGPROC)tools::cbDlgStatistics, 0);

			if (cmd == IDM_COMPARE_TEXTS) {
				TDlgParam dp = {0, 0, TEXT("Text #1"), TEXT("Text #2")};
				openDialog(IDD_TOOL_TEXT_COMPARISON, (DLGPROC)tools::cbDlgTextComparison, (LPARAM)&dp);
			}

			if (cmd == IDM_FOREIGN_KEY_CHECK)
				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_FOREIGN_KEY_CHECK), hWnd, (DLGPROC)tools::cbDlgForeignKeyCheck);

			if (cmd == IDM_DESKTOP_SHORTCUT && DLG_OK == DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_DESKTOP_SHORTCUT), hMainWnd, (DLGPROC)tools::cbDlgDesktopShortcut))
				MessageBox(hWnd, TEXT("Done!"), TEXT("Link creation"), MB_OK);

			if (cmd == IDM_CHECK_INTEGRITY) {
				sqlite3_stmt* stmt;
				int rc = SQLITE_OK == sqlite3_prepare_v2(db, "select integrity_check = 'ok' from pragma_integrity_check;", -1, &stmt, NULL) && SQLITE_ROW == sqlite3_step(stmt);
				if (rc) {
					MessageBox(hMainWnd, sqlite3_column_int(stmt, 0) == 1 ? TEXT("Ok") : TEXT("Fail"), TEXT("Check result"), MB_OK);
				} else {
					showDbError(hMainWnd);
				}
				sqlite3_finalize(stmt);
			}

			if (cmd == IDM_VACUUM && IDYES == MessageBox(0, TEXT("The operation may take some time. Continue?"), TEXT("Confirmation"), MB_YESNO)) {
				if (SQLITE_OK == sqlite3_exec(db, "vacuum;", 0, 0, 0))
					MessageBox(hMainWnd, TEXT("Done"), TEXT("Vacuum database"), MB_OK);
				else
					showDbError(hMainWnd);
			}

			if (cmd == IDM_REINDEX && IDYES == MessageBox(0, TEXT("The operation may take some time. Continue?"), TEXT("Confirmation"), MB_YESNO)) {
				if (tools::reindexDatabase())
					MessageBox(hMainWnd, TEXT("Done"), TEXT("Reindex database"), MB_OK);
				else
					showDbError(hMainWnd);
			}

			// Notifies via WM_COMMAND
			if (LOWORD(wParam) == IDC_EDITOR && HIWORD(wParam) == EN_CHANGE)
				SendMessage((HWND)lParam, WMU_TEXT_CHANGED, 0, 0);

			if (LOWORD(wParam) == IDC_TREE_SEARCH && HIWORD(wParam) == EN_CHANGE)
				updateTree(0);

			if (LOWORD(wParam) == IDC_EDITOR && HIWORD(wParam) == EN_KILLFOCUS && IsWindowVisible(hTooltipWnd))
				hideTooltip();

			if (cmd == IDC_SCHEMA && HIWORD(wParam) == STN_CLICKED) {
				TCHAR schema[256] = {0};
				GetWindowText(hSchemaWnd, schema, 255);
				DestroyMenu(hSchemaMenu);
				hSchemaMenu = CreatePopupMenu();

				RECT rc = {0};

				int count = 0;
				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db,
					"select name, case when name = 'main' then 1 when name = 'temp' then 2 when name == 'temp2' then 3 else 4 end from (select name from pragma_database_list union select 'temp') order by 2, 1", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						if (count == 3)
							Menu_InsertItem(hSchemaMenu, 3, IDM_SCHEMA, 0, NULL);
						TCHAR* name16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
						Menu_InsertItem(hSchemaMenu, count + 1, IDM_SCHEMA + count, _tcscmp(name16, schema) == 0 ? MF_CHECKED : 0,  name16);
						delete [] name16;
						count++;
					}
				}

				sqlite3_finalize(stmt);

				GetWindowRect(hSchemaWnd, &rc);
				TrackPopupMenu(hSchemaMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_CENTERALIGN, (rc.right + rc.left)/2, rc.bottom, 0, hMainWnd, NULL);
			}


			if (wParam == IDCANCEL)
				SendMessage(hMainWnd, WM_CLOSE, 0, 0);
		}
		break;

		case WM_NOTIFY: {
			NMHDR* pHdr = (LPNMHDR)lParam;
			MSGFILTER* pF = (MSGFILTER*)lParam;
			TCHAR wndClass[256];
			GetClassName(pHdr->hwndFrom, wndClass, 256);

			if (pHdr->hwndFrom == hTabWnd && pHdr->code == TCN_SELCHANGE) {
				EnumChildWindows(hMainWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_UPDATETAB);
				SendMessage(hMainWnd, WMU_UPDATE_ROWNO, 0, 0);
			}

			if ((HWND)GetParent(pHdr->hwndFrom) == hTabWnd) {
				int idc = GetDlgCtrlID(pHdr->hwndFrom);
				if (idc >= IDC_TAB_ROWS && idc < IDC_TAB_ROWS + MAX_RESULT_COUNT) {
					if (pHdr->code == (DWORD)NM_CLICK && HIWORD(GetKeyState(VK_MENU))) {
						NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
						return ListView_ShowRef(pHdr->hwndFrom, ia->iItem, ia->iSubItem);
					}

					if (pHdr->code == (DWORD)NM_DBLCLK) {
						NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
						currCell = {ia->hdr.hwndFrom, ia->iItem, ia->iSubItem};
						if (ia->iItem == -1)
							return 0;

						if (HIWORD(GetKeyState(VK_MENU))) {
							ListView_DrillDown(ia->hdr.hwndFrom, ia->iItem, ia->iSubItem);
						} else {
							DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hMainWnd, (DLGPROC)&dialogs::cbDlgRow, MAKELPARAM(ROW_VIEW, 1));
						}
					}

					if (pHdr->code == (DWORD)LVN_KEYDOWN) {
						NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;
						bool isCtrl = HIWORD(GetKeyState(VK_CONTROL));
						HWND hListWnd = pHdr->hwndFrom;

						if (kd->wVKey == VK_F4) {
							int rowNo = currCell.iItem;
							int colNo = currCell.iSubItem;
							if (rowNo == -1 || colNo < 1 || ListView_GetSelectedCount(hListWnd) > 1)
								return 0;

							TValue value = {0};
							if (ListView_GetItemValue(hListWnd, rowNo, colNo, &value))
								openDialog(IDD_VALUE_VIEWER, (DLGPROC)dialogs::cbDlgValueViewer, (LPARAM)&value);
						}


						if (isCtrl && kd->wVKey == 0x43) {// Ctrl + C
							currCell = {hListWnd, 0, 0};
							PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_RESULT_COPY_ROW, 0), 0);
						}

						if (isCtrl && kd->wVKey == 0x41) // Ctrl + A
							ListView_SetItemState(kd->hdr.hwndFrom, -1, LVIS_SELECTED, LVIS_SELECTED);

						if (kd->wVKey == VK_RETURN) {
							currCell = {hListWnd, ListView_GetNextItem(pHdr->hwndFrom, -1, LVNI_SELECTED), 0};
							if (currCell.iItem != -1)
								DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hMainWnd, (DLGPROC)&dialogs::cbDlgRow, MAKELPARAM(ROW_VIEW, 1));
							SetFocus(hListWnd);
						}

						if (isCtrl && kd->wVKey == 0x46) { // Ctrl + F
							DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_RESULT_FIND), hMainWnd, (DLGPROC)&dialogs::cbDlgResultFind, (LPARAM)hListWnd);
							SetFocus(hListWnd);
						}

						if (kd->wVKey == VK_F3)
							SendMessage(hListWnd, WMU_RESULT_SEARCH, 0, 0);

						if (kd->wVKey == VK_BACK) {
							ListView_DrillUp(hListWnd);
							return 0;
						}

						if (kd->wVKey == 0x49) { // Ctrl + I
							for (int rowNo = 0; rowNo < ListView_GetItemCount(hListWnd); rowNo++) {
								BOOL isSelected = ListView_GetItemState(hListWnd, rowNo, LVIS_SELECTED) & LVIS_SELECTED;
								ListView_SetItemState(hListWnd, rowNo, isSelected ? 0 : LVIS_SELECTED, LVIS_SELECTED);
							}
						}

						if (kd->wVKey == 0x50) { // Ctrl + P
							SendMessage(hWnd, WM_COMMAND, IDM_RESULT_PREVIEW, 0);
							return 0;
						}

						bool isNum = kd->wVKey >= 0x31 && kd->wVKey <= 0x39;
						bool isNumPad = kd->wVKey >= 0x61 && kd->wVKey <= 0x69;
						if ((isNum || isNumPad) && GetKeyState(VK_CONTROL)) // Ctrl + 1-9
							return ListView_Sort(hListWnd, kd->wVKey - (isNum ? 0x31 : 0x61) + 1 );
					}

					if (pHdr->code == (DWORD)LVN_ITEMCHANGED)
						SendMessage(hMainWnd, WMU_UPDATE_ROWNO, 0, 0);

					if (pHdr->code == LVN_COLUMNCLICK) {
						NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
						return ListView_Sort(pHdr->hwndFrom, pLV->iSubItem);
					}
				}
			}

			if (pHdr->hwndFrom == hEditorWnd && pHdr->code == WM_RBUTTONDOWN) {
				POINT p;
				GetCursorPos(&p);
				TrackPopupMenu(hEditorMenu, 0, p.x, p.y, 0, hMainWnd, NULL);
			}

			if (pF->nmhdr.hwndFrom == hEditorWnd && pF->msg == WM_KEYDOWN && pF->wParam == VK_SPACE && HIWORD(GetKeyState(VK_CONTROL))) { // Ctrl + Space
				return SendMessage(hMainWnd, WMU_SHOW_INFO, 0, 0);
			}

			// Query shortcut Alt + F1, Ctrl + 0, Ctrl + 1, ...
			if (pF->nmhdr.hwndFrom == hEditorWnd && (
				(pF->msg == WM_KEYDOWN && pF->wParam >= 0x30 && pF->wParam <= 0x39 && HIWORD(GetKeyState(VK_CONTROL))) ||
				(pF->msg == WM_SYSKEYDOWN && pF->wParam == VK_F1 && HIWORD(GetKeyState(VK_MENU)))
				)) {
				if (-1 != SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0))
					executeEditorQuery(0, pF->wParam);
				return true;
			}

			if (pF->nmhdr.hwndFrom == hEditorWnd && pF->msg == WM_KEYDOWN && pF->wParam == 0x44 && HIWORD(GetKeyState(VK_CONTROL))) // Ctrl + D
				SendMessage(hMainWnd, WM_COMMAND, IDM_EDITOR_COMPARE, 0);

			if (pF->nmhdr.hwndFrom == hEditorWnd && pF->msg == WM_KEYUP && pF->wParam == VK_CONTROL && IsWindowVisible(hTooltipWnd))
				hideTooltip();

			if (pHdr->hwndFrom == hEditorWnd && pHdr->code == EN_SELCHANGE) {
				SendMessage(hMainWnd, WMU_UPDATE_CARET_INFO, 0, 0);
				SendMessage(pHdr->hwndFrom, WMU_SELECTION_CHANGED, wParam, lParam);
				return true;
			}

			if ((pHdr->idFrom == IDC_EDITOR || pHdr->idFrom == IDC_CLI_EDITOR) && pHdr->code == EN_DROPFILES) {
				TCHAR path16[MAX_PATH];
				HDROP drop = (HDROP)((ENDROPFILES*)lParam)->hDrop;
				DragQueryFileW(drop, 0, path16, MAX_PATH);
				DragFinish(drop);

				FILE* f = _tfopen(path16, TEXT("r, ccs=UTF-8"));
				if (f == NULL) {
					MessageBox(hWnd, TEXT("Error to open file"), NULL, MB_OK);
					return 0;
				}

				fseek(f, 0L, SEEK_END);
				long size = ftell(f);
				rewind(f);

				if (size > 0) {
					TCHAR data16[size + 1]{0};
					fread(data16, size, sizeof(TCHAR), f);

					int pos = ((ENDROPFILES*)lParam)->cp;
					SendMessage(pHdr->hwndFrom, EM_SETSEL, pos, pos);
					SendMessage(pHdr->hwndFrom, EM_REPLACESEL, true, (LPARAM)data16);
				} else {
					MessageBox(0, TEXT("Only text files are supported"), 0, MB_OK);
				}

				fclose(f);
				return 0;
			}

			if (pHdr->hwndFrom == hTreeWnd && (pHdr->code == (DWORD)NM_DBLCLK || pHdr->code == (DWORD)NM_RETURN)) {
				HTREEITEM hParent = TreeView_GetParent(hTreeWnd, treeItems[0]);
				if (hParent != NULL) {
					int cmd = hParent == treeItems[TABLE] || hParent == treeItems[VIEW] ? IDM_EDIT_DATA :
						TreeView_GetItemState(hTreeWnd, treeItems[0], TVIS_CUT) & TVIS_CUT ? IDM_VIEW : IDM_EDIT;
					PostMessage(hMainWnd, WM_COMMAND, cmd, 0);
				}
				return 1;
			}

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == (DWORD)TVN_KEYDOWN) {
				NMTVKEYDOWN* pKd = (LPNMTVKEYDOWN) lParam;
				bool isCtrl = HIWORD(GetKeyState(VK_CONTROL));
				bool isShift = HIWORD(GetKeyState(VK_SHIFT));

				if ((pKd->wVKey == VK_DELETE)) {
					TVITEM ti{0};
					ti.hItem = treeItems[0];
					ti.mask = TVIF_PARAM;
					TreeView_GetItem(hTreeWnd, &ti);
					if (ti.lParam == TABLE || ti.lParam == VIEW || ti.lParam == TRIGGER || ti.lParam == INDEX)
						PostMessage(hMainWnd, WM_COMMAND, IDM_DELETE, 0);
				}

				if (isCtrl && pKd->wVKey == 0x43) {// Ctrl + C
					TCHAR name16[256] = {0};
					TV_ITEM tv;
					tv.mask = TVIF_TEXT | TVIF_PARAM;
					tv.hItem = treeItems[0];
					tv.pszText = name16;
					tv.cchTextMax = 256;

					if(!TreeView_GetItem(hTreeWnd, &tv))
						return 0;

					if (tv.lParam == COLUMN)
						_tcstok(name16, TEXT(":"));

					if (isShift && tv.lParam == TABLE) {
						TCHAR* ddl = getDDL(TEXT("main"), name16, tv.lParam);
						if (ddl != NULL) {
							utils::setClipboardText(ddl);
							delete [] ddl;
						}
					} else {
						utils::setClipboardText(name16);
					}
				}

				if (isCtrl && pKd->wVKey == 0x46) {// Ctrl + F
					ShowWindow(hTreeSearchWnd, SW_SHOW);
					SetFocus(hTreeSearchWnd);
					SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);
				}

				return 1;
			}

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == TVN_SELCHANGED)
				treeItems[0] = TreeView_GetSelection(hTreeWnd);

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == TVN_ITEMEXPANDING)
				TreeView_SelectItem(hTreeWnd, ((LPNMTREEVIEW)lParam)->itemNew.hItem);

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == TVN_BEGINLABELEDIT) {
				const NMTVDISPINFO * pMi = (LPNMTVDISPINFO)lParam;
				HTREEITEM hParentItem = TreeView_GetParent(hTreeWnd, pMi->item.hItem);
				if (!hParentItem)
					return true;

				if (TreeView_GetItemParam(hTreeWnd, hParentItem) == VIEW)
					return true;

				if (pMi->item.state & TVIS_CUT)
					return true;

				HWND hEdit = TreeView_GetEditControl(hTreeWnd);
				SetProp(hEdit, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)&cbNewTreeItemEdit));

				_tcscpy(treeEditName, pMi->item.pszText);
				SetWindowText(hEdit, _tcstok(pMi->item.pszText, TEXT(":")));
			}

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == TVN_ENDLABELEDIT) {
				NMTVDISPINFO * pMi = (LPNMTVDISPINFO)lParam;
				int type = pMi->item.lParam;

				if (pMi->item.pszText == NULL || _tcslen(pMi->item.pszText) == 0)
					return false;

				TCHAR schema16[256];
				GetWindowText(hSchemaWnd, schema16, 255);

				TCHAR query16[MAX_TEXT_LENGTH + 1];
				if (type == COLUMN) {
					TCHAR tblname16[255];
					HTREEITEM hParent = TreeView_GetParent(hTreeWnd, treeItems[0]);
					TVITEM ti;
					ti.hItem = hParent;
					ti.mask = TVIF_PARAM | TVIF_TEXT;
					ti.pszText = tblname16;
					ti.cchTextMax = 255;
					TreeView_GetItem(hTreeWnd, &ti);
					if (ti.lParam != TABLE)
						return 0;

					TCHAR* fullname16 = utils::getFullTableName(schema16, tblname16, false);

					TCHAR* colDesc16 = _tcstok(treeEditName, TEXT(":"));
					colDesc16 = colDesc16 ? _tcstok (NULL, TEXT(":")) : 0;

					TCHAR* qOldName16 = utils::double_quote(treeEditName);
					TCHAR* qNewName16 = utils::double_quote(pMi->item.pszText);
					_sntprintf(query16, MAX_TEXT_LENGTH, TEXT("alter table %ls rename column %ls to %ls"), fullname16, qOldName16, qNewName16);
					delete [] qOldName16;
					delete [] qNewName16;
					if (_tcscmp(treeEditName, pMi->item.pszText)) {
						char* query8 = utils::utf16to8(query16);
						int rc = sqlite3_exec(db, query8, NULL, 0 , 0);
						delete [] query8;

						if (SQLITE_OK != rc) {
							showDbError(hMainWnd);
							return false;
						}
					}
					delete [] fullname16;

					TCHAR item16[1024] = {0};
					_sntprintf(item16, 1023, TEXT("%ls:%ls"), pMi->item.pszText, colDesc16);
					pMi->item.pszText = item16;
					pMi->item.cchTextMax = _tcslen(item16) + 1;
					return true;
				}

				if (type == TABLE) {
					TCHAR* qOldName16 = utils::double_quote(treeEditName);
					TCHAR* qNewName16 = utils::double_quote(pMi->item.pszText);
					_sntprintf(query16, MAX_TEXT_LENGTH, TEXT("alter table \"%ls\".%ls rename to %ls"), schema16, qOldName16, qNewName16);
					delete [] qOldName16;
					delete [] qNewName16;
				} else if (type > 1) {
					TCHAR* fullname16 = utils::getFullTableName(schema16, treeEditName, false);
					const TCHAR* ddl = getDDL(schema16, treeEditName, pMi->item.lParam);
					if (ddl) {
						TCHAR* newDdl = utils::replace(ddl, treeEditName, pMi->item.pszText, _tcslen(TEXT("create ")) + _tcslen(TYPES16u[type]));
						_sntprintf(query16, MAX_TEXT_LENGTH,TEXT("drop %ls %ls; %ls;"), TYPES16[type], fullname16, newDdl);
						delete [] ddl;
						delete [] newDdl;
					}
					delete [] fullname16;
				}

				if (!_tcscmp(treeEditName, pMi->item.pszText))
					return false;

				char* query8 = utils::utf16to8(query16);
				int rc = SQLITE_OK == sqlite3_exec(db, query8, NULL, 0 , 0);
				if (!rc)
					showDbError(hMainWnd);
				delete [] query8;
				return rc;
			}

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == (UINT)TVN_BEGINDRAG) {
				NMTREEVIEW* lpnmtv = (LPNMTREEVIEW)lParam;

				TCHAR buf[255];
				TV_ITEM tv{0};
				tv.mask = TVIF_TEXT | TVIF_PARAM;
				tv.pszText = buf;
				tv.cchTextMax = 255;
				tv.hItem = lpnmtv->itemNew.hItem;
				TreeView_GetItem(hTreeWnd, &tv);

				int len = _tcslen(buf);
				_tcstok(buf, TEXT(":"));

				if (tv.lParam < 0)
					return true;

				RECT rc{0};
				*(HTREEITEM*)&rc = tv.hItem;
				SendMessage(hTreeWnd, TVM_GETITEMRECT, true, (LPARAM)&rc);

				TreeView_Select(hTreeWnd, tv.hItem, TVGN_CARET);
				SetCapture(hMainWnd);
				ShowCursor(false);

				hDragWnd = CreateWindowEx(WS_EX_TOPMOST, WC_STATIC, buf, WS_CHILD | SS_LEFT | WS_VISIBLE,
					rc.left, rc.top, (rc.right - rc.left) * _tcslen(buf) / len, rc.bottom - rc.top, hMainWnd, (HMENU)0, GetModuleHandle(0), NULL);;
				SetWindowLongPtr(hDragWnd, GWLP_USERDATA, tv.lParam);
				SendMessage(hDragWnd, WM_SETFONT, (LPARAM)hFont, true);
				InvalidateRect(hDragWnd, NULL, true);
			}

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == (UINT)NM_CUSTOMDRAW) {
				LPNMTVCUSTOMDRAW pCustomDraw = (LPNMTVCUSTOMDRAW)lParam;
				if (pCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
					return CDRF_NOTIFYITEMDRAW;

				if (pCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
					UINT state = TreeView_GetItemState(pHdr->hwndFrom, (HTREEITEM)pCustomDraw->nmcd.dwItemSpec, TVIS_CUT);
					UINT type = TreeView_GetItemParam(pHdr->hwndFrom, (HTREEITEM)pCustomDraw->nmcd.dwItemSpec);
					pCustomDraw->clrText = pCustomDraw->clrTextBk != RGB(255, 255, 255) ? RGB(255, 255, 255) :
						state & TVIS_CUT ? RGB(200, 200, 200) :
						state & TVIS_BOLD && type == COLUMN ? RGB(200, 0, 200) :
						state & TVIS_BOLD ? RGB(0, 0, 200) :
						RGB(0, 0, 0);
				}

				return CDRF_DODEFAULT;
			}

			if (pHdr->hwndFrom == hToolbarWnd && pHdr->code == (UINT)NM_CUSTOMDRAW) {
				LPNMTBCUSTOMDRAW pCustomDraw = (LPNMTBCUSTOMDRAW)lParam;
				if (pCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
					return CDRF_NOTIFYITEMDRAW;

				if (pCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT && pCustomDraw->nmcd.dwItemSpec == IDM_SPAN2)
						return CDRF_SKIPDEFAULT;
			}

			if (pHdr->code == TTN_GETDISPINFO) {
				LPTOOLTIPTEXT pTtt = (LPTOOLTIPTEXT) lParam;
				bool isToolTip = pTtt->hdr.hwndFrom == (HWND)SendMessage(hToolbarWnd, TB_GETTOOLTIPS, 0, 0);
				bool isTabTip = pTtt->hdr.hwndFrom == (HWND)SendMessage(hTabWnd, TCM_GETTOOLTIPS, 0, 0) && (pTtt->hdr.idFrom < MAX_RESULT_COUNT);
				if (isToolTip || isTabTip) {
					SendMessage(pTtt->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, MAX_TEXT_LENGTH);
					if (isTabTip) {
						int tabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
						pTtt->lpszText = tabs[tabNo].tabTooltips[pTtt->hdr.idFrom];
					}
					else {
						pTtt->hinst = GetModuleHandle(0);
						pTtt->lpszText = (LPWSTR)pTtt->hdr.idFrom;
					}
				}
			}

			if (pHdr->hwndFrom == hToolbarWnd && pHdr->code == (UINT)NM_CUSTOMDRAW) {
				NMTBCUSTOMDRAW *pCustomDraw = (NMTBCUSTOMDRAW*)lParam;
				if (pCustomDraw->nmcd.dwDrawStage == CDDS_PREERASE) {
					HDC hDC = pCustomDraw->nmcd.hdc;
					RECT rect;
					GetClientRect(hToolbarWnd, &rect);
					FillRect(hDC, &rect, GetSysColorBrush(COLOR_3DFACE));
					return CDRF_SKIPDEFAULT ;
				}
			}

			if (pHdr->code == (DWORD)NM_RCLICK && pHdr->idFrom == IDC_TAB) {
				POINT p;
				GetCursorPos(&p);

				int tabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
				TCHITTESTINFO thi{p, TCHT_ONITEM};
				ScreenToClient(pHdr->hwndFrom, &(thi.pt));
				int resultNo = TabCtrl_HitTest(pHdr->hwndFrom, &thi);
				Menu_SetData(hTabResultMenu, MAKELPARAM(tabNo, resultNo));

				//Menu_SetItemState(hTabResultMenu, IDM_TAB_RESULT_PIN, tabs[tabNo].isPinned[resultNo] ? MF_CHECKED: MF_UNCHECKED);
				Menu_SetItemText(hTabResultMenu, IDM_TAB_RESULT_PIN, tabs[tabNo].isPinned[resultNo] ? TEXT("Unpin result") : TEXT("Pin result"));

				TrackPopupMenu(hTabResultMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hMainWnd, NULL);
			}

			if (pHdr->code == (DWORD)NM_RCLICK && !_tcscmp(wndClass, WC_LISTVIEW)) {
				NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
				currCell = {ia->hdr.hwndFrom, ia->iItem, ia->iSubItem};

				POINT p;
				GetCursorPos(&p);

				HMENU hCompareMenu = GetSubMenu(hResultMenu, GetMenuItemCount(hResultMenu) - 1);
				int cnt = GetMenuItemCount(hCompareMenu);
				for (int i = 0; i < cnt; i++)
					DeleteMenu(hCompareMenu, 0, MF_BYPOSITION);

				int currColCount = ListView_GetColumnCount(ia->hdr.hwndFrom);
				int menuNo = 0;

				for (int tabNo = 0; tabNo < MAX_TAB_COUNT && menuNo < MAX_COMPARE_RESULT; tabNo++) {
					TEditorTab tab = tabs[tabNo];
					if (!tab.hEditorWnd)
						continue;

					TCHAR tabname16[80] = {0};
					SendMessage(hMainTabWnd, WMU_TAB_GET_TEXT, tabNo, (LPARAM)tabname16);

					for (int resultNo = 0; resultNo < TabCtrl_GetItemCount(tab.hTabWnd) && menuNo < MAX_COMPARE_RESULT; resultNo++) {
						HWND hListWnd = GetDlgItem(tab.hTabWnd, IDC_TAB_ROWS + resultNo);
						if (!hListWnd)
							continue;

						if (hListWnd == ia->hdr.hwndFrom)
							continue;

						if (currColCount != ListView_GetColumnCount(hListWnd))
							continue;

						TCHAR resname16[80];
						TabCtrl_GetItemText(tab.hTabWnd, resultNo, resname16, 80);

						TCHAR menu16[256];
						_sntprintf(menu16, 255, TEXT("%ls > %ls"), tabname16, resname16);

						AppendMenu(hCompareMenu, MF_STRING, IDM_RESULT_COMPARE + menuNo, menu16);

						MENUITEMINFO mii{0};
						mii.cbSize = sizeof(MENUITEMINFO);
						mii.fMask = MIIM_DATA;
						mii.dwItemData = MAKELPARAM(tabNo, resultNo);
						SetMenuItemInfo(hCompareMenu, IDM_RESULT_COMPARE + menuNo, false, &mii);
						menuNo++;
					}
				}

				if (GetMenuItemCount(hCompareMenu) == 0)
					AppendMenu(hCompareMenu, MF_STRING | MF_DISABLED | MF_GRAYED, 0, TEXT("No similar results"));

				HWND hListWnd = (HWND)SendMessage(hWnd, WMU_GET_CURRENT_RESULTSET, 0, 0);
				HWND hHeader = ListView_GetHeader(hListWnd);
				Menu_SetItemState(hResultMenu, IDM_RESULT_PREVIEW, prefs::get("show-preview") ? MF_CHECKED : MF_UNCHECKED);
				bool isShowFilters = GetWindowLongPtr(hHeader, GWL_STYLE) & HDS_FILTERBAR;
				Menu_SetItemState(hResultMenu, IDM_RESULT_FILTERS, isShowFilters ? MF_CHECKED : MF_UNCHECKED);
				bool isTransposed = GetProp(hListWnd, TEXT("ISTRANSPOSED"));
				UINT tState = (isTransposed ? MF_CHECKED : MF_UNCHECKED) | (!isTransposed && *(int*)GetProp(hListWnd, TEXT("TOTALROWCOUNT")) > MAX_TRANSPOSE_ROWS ? MF_DISABLED | MF_GRAYED : 0);
				Menu_SetItemState(hResultMenu, IDM_RESULT_TRANSPOSE, tState);

				int* modifiers = (int*)GetProp(hListWnd, TEXT("MODIFIERS"));
				if (modifiers) {
					int itemCount = GetMenuItemCount(hModifierMenu);
					for (int itemNo = 0; itemNo < itemCount; itemNo++) {
						int id = GetMenuItemID(hModifierMenu, itemNo);
						int modifierNo = Menu_GetItemData(hModifierMenu, id);
						Menu_SetItemState(hModifierMenu, id, modifierNo == modifiers[currCell.iSubItem] ? MF_CHECKED : MF_UNCHECKED);
					}

					if (itemCount == 1)
						Menu_SetItemState(hModifierMenu, IDM_RESULT_MODIFIER, MF_UNCHECKED | MF_DISABLED | MF_GRAYED);
				}

				TrackPopupMenu(hResultMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hMainWnd, NULL);
			}

			if (pHdr->code == EN_MSGFILTER && wParam == IDC_EDITOR)
				return processEditorEvents(pF);

			if (pHdr->code == NM_TAB_ADD && pHdr->hwndFrom == hMainTabWnd) {
				ShowWindow(hEditorWnd, SW_HIDE);
				ShowWindow(hTabWnd, SW_HIDE);
				int tabNo = wParam;
				int id = 0;
				for (int i = 0; i < tabNo; i++)
					id = id < tabs[i].id ? tabs[i].id : id;

				hEditorWnd = CreateWindowEx(0, TEXT("RICHEDIT50W"), NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP | ES_NOHIDESEL, 100, 0, 100, 100, hMainWnd, (HMENU)IDC_EDITOR, GetModuleHandle(0),  NULL);
				SetProp(hEditorWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hEditorWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewEditor));
				SendMessage(hEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_DROPFILES);
				SendMessage(hEditorWnd, EM_EXLIMITTEXT, 0, 32767 * 10);
				if (prefs::get("word-wrap"))
					toggleWordWrap(hEditorWnd);

				//HWND hAIWnd = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | TCS_TOOLTIPS, 100, 100, 100, 100, hMainWnd, (HMENU)IDC_AI, GetModuleHandle(0), NULL);

				hTabWnd = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | TCS_TOOLTIPS, 100, 100, 100, 100, hMainWnd, (HMENU)IDC_TAB, GetModuleHandle(0), NULL);
				SetProp(hTabWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hTabWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewResultTab));
				TabCtrl_SetImageList(hTabWnd, hIconsImageList);
				SendMessage(hTabWnd, WM_SETFONT, (LPARAM)hMenuFont, 0);
				SetWindowTheme(hTabWnd, TEXT(" "), TEXT(" "));
				DragAcceptFiles(hEditorWnd, true);

				tabs[tabNo].hEditorWnd = hEditorWnd;
				tabs[tabNo].hTabWnd = hTabWnd;
				tabs[tabNo].hQueryListWnd = CreateWindow(WC_LISTBOX, NULL, WS_CHILD | SS_LEFT | WS_BORDER, 20, 20, 100, 100, hEditorWnd, (HMENU)IDC_QUERYLIST, GetModuleHandle(0), NULL);
				tabs[tabNo].id = id + 1;

				SendMessage(hEditorWnd, EM_SETWORDWRAPMODE, WBF_WORDWRAP, 0);
				SendMessage(hMainTabWnd, WMU_TAB_SET_CURRENT, tabNo, 0);
			}

			if (pHdr->code == NM_TAB_CHANGE && pHdr->hwndFrom == hMainTabWnd) {
				int tabNo = wParam;
				ShowWindow(hEditorWnd, SW_HIDE);
				ShowWindow(hTabWnd, SW_HIDE);
				ShowWindow(cli.hResultWnd, tabNo == -1 ? SW_SHOW : SW_HIDE);

				hEditorWnd = tabNo != -1 ? tabs[tabNo].hEditorWnd : cli.hEditorWnd;
				hTabWnd = tabNo != - 1 ? tabs[tabNo].hTabWnd : 0;
				int resultNo = TabCtrl_GetCurSel(hTabWnd);
				SendMessage(hStatusWnd, SB_SETTEXT, SB_ELAPSED_TIME, tabNo != -1 && resultNo != -1 ? (LPARAM)tabs[tabNo].queryElapsedTimes[resultNo] : 0);
				SendMessage(hWnd, WMU_UPDATE_ROWNO, 0, 0);

				ShowWindow(hEditorWnd, SW_SHOW);
				ShowWindow(hTabWnd, SW_SHOW);
				setEditorFont(hEditorWnd);
				SetFocus(hEditorWnd);
				SendMessage(hWnd, WMU_UPDATE_SIZES, 0, 0);
				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_UPDATETAB);

				SendMessage(hMainWnd, WMU_UPDATE_SB_RESULTSET, 0, 0);

				bool isRunning = tabNo >=0 ? tabs[tabNo].thread : cli.thread;
				updateExecuteMenu(!isRunning && db != NULL);
				SendMessage(hMainWnd, WMU_UPDATE_TOOLBAR_BUTTONS, 0, 0);
				updateTransactionState();

				TCHAR schema16[256];
				GetWindowText(hSchemaWnd, schema16, 255);
				if (_tcscmp(schema16, TEXT("temp")) == 0)
					updateTree();
			}

			if (pHdr->code == NM_TAB_DELETE && pHdr->hwndFrom == hMainTabWnd) {
				int tabNo = wParam;
				TEditorTab* tab = &tabs[tabNo];
				DestroyWindow(tab->hEditorWnd);
				DestroyWindow(tab->hTabWnd);
				DestroyWindow(tab->hQueryListWnd);
				sqlite3_close(tab->db);

				for (int i = tabNo; i < MAX_TAB_COUNT - 1; i++)
					tabs[i] = tabs[i + 1];

				tabs[MAX_TAB_COUNT - 2]	= {0};

				tabs[SendMessage(hMainTabWnd, WMU_TAB_GET_COUNT, 0, 0)] = {0};
			}

			if (pHdr->code == NM_TAB_REQUEST_DELETE && pHdr->hwndFrom == hMainTabWnd) {
				bool isRunning = tabs[wParam].thread != 0;
				if (isRunning)
					MessageBox(hMainWnd, TEXT("The tab is busy"), TEXT("Info"), MB_OK);
				return isRunning;
			}

			if (pHdr->idFrom == IDC_STATUSBAR && (pHdr->code == NM_CLICK || pHdr->code == NM_RCLICK)) {
				bool isEnable = prefs::get("ai-enable");
				NMMOUSE* pm = (NMMOUSE*)lParam;
				int id = pm->dwItemSpec;
				if (id != SB_AI_SETTINGS || !isEnable)
					return 0;

				RECT rc, rc2;
				GetWindowRect(pHdr->hwndFrom, &rc);
				SendMessage(pHdr->hwndFrom, SB_GETRECT, id, (LPARAM)&rc2);

				Menu_SetItemState(hAiMenu, IDM_AI_DB_ACCESS, (prefs::get("ai-has-db-access") ? MF_CHECKED : 0) | MF_ENABLED);
				Menu_SetItemState(hAiMenu, IDM_AI_EXPLAIN_ERRORS, (prefs::get("ai-explain-query-error") ? MF_CHECKED : 0) | MF_ENABLED);
				Menu_SetItemState(hAiMenu, IDM_AI_DO_FORMAT, (prefs::get("ai-do-format") ? MF_CHECKED : 0) | MF_ENABLED);
				Menu_SetItemState(hAiMenu, IDM_AI_PROCESS_NOSQL, (prefs::get("ai-process-nosql") ? MF_CHECKED : 0) | MF_ENABLED);

				POINT p = {rc.left + rc2.left, rc.top};
				TrackPopupMenu(hAiMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
			}
		}
		break;

		case WM_PARENTNOTIFY: {
			if (LOWORD(wParam) == WM_LBUTTONDOWN && IsWindowVisible(hAutoComplete) && (GetFocus() != hAutoComplete))
				ShowWindow(hAutoComplete, SW_HIDE);
		}
		break;

		case WMU_SHOW_INFO: {
			TCHAR result16[MAX_TEXT_LENGTH]{0};
			hideTooltip();

			TCHAR* text = getCurrentText(hEditorWnd);

			// Is keyword/table/function?
			if (_tcslen(text) && _tcslen(result16) == 0) {
				TCHAR* help16 = getHelp(text);
				if (help16) {
					_tcsncpy(result16, help16, MAX_TEXT_LENGTH);
					delete [] help16;
				}
			}

			// Is function argument?
			if (_tcslen(result16) == 0) {
				int crStart, crEnd;
				SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&crStart, (WPARAM)&crEnd);
				SetWindowRedraw(hEditorWnd, false);

				bool isSearch = true;
				int pCounter = 1;
				int aCounter = 0;
				int crLineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, -1, 0);
				int currLineNo = crLineNo;
				while (isSearch && currLineNo >= 0) {
					int currLineIdx = SendMessage(hEditorWnd, EM_LINEINDEX, currLineNo, 0);
					int currLineSize = SendMessage(hEditorWnd, EM_LINELENGTH, currLineIdx, 0);
					TCHAR currLine[currLineSize + 1]{0};
					currLine[0] = currLineSize;
					SendMessage(hEditorWnd, EM_GETLINE, currLineNo, (LPARAM)currLine);

					int pos = currLineNo == crLineNo ? crStart - currLineIdx - 1: currLineSize - 1;
					while (isSearch && pos >= 0) {
						TCHAR c = currLine[pos];

						if (_tcschr(TEXT(",();"), c) != 0 && !RichEdit_GetTextColor(hEditorWnd, currLineIdx + pos)) {
							if (c == TEXT(',') && pCounter == 1)
								aCounter++;

							if (c == TEXT(')'))
								pCounter++;

							if (c == TEXT('(')) {
								pCounter--;
								if (pCounter == 0)
									isSearch = false;

								if (pCounter == 0 && pos > 0) {
									int cCounter = 0;
									pos--;
									int posStart = pos, posEnd = pos;
									while (pos >= 0) {
										bool isChar = _istalnum(currLine[pos]) || (currLine[pos] == TEXT('_'));
										cCounter += isChar;

										if (isChar && cCounter == 1)
											posEnd = pos;

										if ((!isChar || pos == 0) && cCounter > 0) {
											posStart = pos + (pos > 0);
											break;
										}

										pos--;
									}

									if (posStart < posEnd) {
										TCHAR fname16[posEnd - posStart + 2]{0};
										_tcsncpy(fname16, currLine + posStart, posEnd - posStart + 1);

										sqlite3_stmt *stmt;
										char query8[] = "with " \
											"t (fname, args, nargs) as (select coalesce(max(brief), ?1), max(args), max(nargs) from help where word = ?1), " \
											"split(arg, str, no) as ( " \
											"select '', args || '|', 0 from t union all " \
											"select substr(str, 0, instr(str, '|')), substr(str, instr(str, '|') + 1), no + 1 from split where str <> '')," \
											"t2(arg) as (select coalesce(max(arg), (?2 + 1) || ' argument') from split where no = ?2 + 1)"
											"select case when nargs = -1 or ?2 < nargs or nargs is null then (select arg from t2) when nargs = 0 then 'The function takes no arguments' else 'The maximum arguments count is exceeded by ' || (?2 + 1) || ' argument' end " \
											"|| char(10, 10) || fname from t";

										if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, query8, -1, &stmt, 0)) {
											char* fname8 = utils::utf16to8(fname16);
											sqlite3_bind_text(stmt, 1, fname8, strlen(fname8), SQLITE_TRANSIENT);
											sqlite3_bind_int(stmt, 2, aCounter);
											delete [] fname8;

											if (SQLITE_ROW == sqlite3_step(stmt)) {
												TCHAR* res16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
												_tcscpy(result16, res16);
												delete [] res16;
											}
										}

										sqlite3_finalize(stmt);
									}
								}
							}

							if (c == TEXT(';'))
								isSearch = false;
						}

						pos--;
					}
					currLineNo--;
				}

				SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)crStart, (WPARAM)crEnd);
				SetWindowRedraw(hEditorWnd, true);
			}

			delete [] text;

			if (_tcslen(result16) > 0) {
				POINT p{0};
				GetCaretPos(&p);
				ClientToScreen(hEditorWnd, &p);
				ShowWindow(hAutoComplete, SW_HIDE);
				showTooltip(p.x, p.y + 20, result16);
			}
			return true;
		}
		break;

		case WMU_APPEND_TEXT: {
			TCHAR* buf = (TCHAR*)wParam;
			int crPos = 0;
			SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&crPos, (LPARAM)&crPos);
			int lineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, crPos, 0);
			int lineIdx = SendMessage(hEditorWnd, EM_LINEINDEX, lineNo, 0);
			int lineSize = SendMessage(hEditorWnd, EM_LINELENGTH, lineIdx, 0);
			if (lineSize > 0 && crPos <= lineIdx + lineSize) {
				lineIdx = SendMessage(hEditorWnd, EM_LINEINDEX, lineNo + 1, 0);
				SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)lineIdx, (LPARAM)lineIdx);
			}

			SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (LPARAM)buf);
			SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (LPARAM)(buf[_tcslen(buf) - 1] != TEXT(';') ? TEXT(";\n") : TEXT("\n")));
			delete [] buf;
		}
		break;

		case WMU_OPEN_NEW_TAB: {
			int tabNo = SendMessage(hMainTabWnd, WMU_TAB_ADD, 0, 0);
			SendMessage(hMainTabWnd, WMU_TAB_SET_CURRENT, tabNo, 0);
		}
		break;

		case WMU_UPDATE_SIZES: {
			RECT rc, rcStatus, rcToolbar, rcEditorSearch = {0};
			GetClientRect(hWnd, &rc);
			GetClientRect(hEditorSearchWnd, &rcEditorSearch);
			GetClientRect(hStatusWnd, &rcStatus);
			GetClientRect(hToolbarWnd, &rcToolbar);

			if (rc.right == 0 && rc.bottom == 0)
				return 0;

			int top = rcToolbar.bottom + 3;
			SetProp(hWnd, TEXT("NCTOP"), IntToPtr(top));
			int h = rc.bottom - rcStatus.bottom - rcToolbar.bottom - 3;

			float z = utils::getWndScale(hTabWnd).x;
			int splitterX = CLAMP(prefs::get("splitter-position-x"), 50 * z, rc.right - 50 * z);
			int splitterY = CLAMP(prefs::get("splitter-position-y"), 50 * z + rcToolbar.bottom, h - 100 * z);
			prefs::set("splitter-position-y", splitterY);

			int tabH = 19 * z;
			int editorSearchH = IsWindowVisible(hEditorSearchWnd) ? rcEditorSearch.bottom + 2: 0;
			int treeSearchH = IsWindowVisible(hTreeSearchWnd) ? utils::getEditHeightByFont(hTreeSearchWnd) + 2 * GetSystemMetrics(SM_CYFIXEDFRAME): 0;
			int splitterW = 5;

			bool isShowChat = prefs::get("ai-enable") && prefs::get("ai-show-chat");
			bool isChatMaximized = prefs::get("ai-chat-maximized");
			int chatW = isShowChat ? (isChatMaximized ? rc.right - splitterX : MIN(rc.right - splitterX, prefs::get("ai-chat-width") + splitterW)) : 0;

			RECT rcEditor;
			GetClientRect(hEditorWnd, &rcEditor);

			LONG flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_DEFERERASE;

			SetWindowPos(hSchemaWnd, 0, 0, top + 2, splitterX, tabH, flags);
			SetWindowPos(hTreeWnd, 0, 0, top + tabH + 2, splitterX, h - tabH - 2 - treeSearchH - (treeSearchH > 0), flags);
			SetWindowPos(hTreeSearchWnd, 0, 0, top + h - treeSearchH, splitterX, treeSearchH, flags);
			SetWindowPos(hMainTabWnd, 0, splitterX + splitterW, top + 2, rc.right - splitterX - splitterW - chatW, tabH, flags);
			SetWindowPos(hEditorWnd, 0, splitterX + splitterW, top + tabH + 2, rc.right - splitterX - splitterW - chatW, splitterY - tabH + 2 - editorSearchH, flags);
			SetWindowPos(hEditorSearchWnd, 0, splitterX + splitterW, top + splitterY - rcEditorSearch.bottom + 2, rc.right - splitterW - chatW, rcEditorSearch.bottom, flags);
			SetWindowPos(cli.hResultWnd, 0, splitterX + 3, top + splitterY + 7, rc.right - splitterX - splitterW - chatW, h - splitterY - 10, flags);
			SetWindowPos(hAiChatWnd, 0, rc.right - chatW + splitterW, top + 2, chatW - splitterW, h - 2, flags);

			setEditorPadding(hEditorWnd);

			int dy = MAX(abs(rcEditor.bottom - (splitterY - tabH + 2)) + 10, 20);
			rcEditor = (RECT){0, rcEditor.bottom - dy, rcEditor.right, rcEditor.bottom + dy};
			InvalidateRect(hEditorWnd, &rcEditor, FALSE);
			InvalidateRect(hSchemaWnd, NULL, TRUE);
			InvalidateRect(hTreeSearchWnd, NULL, TRUE);
			InvalidateRect(hAiChatWnd, NULL, TRUE);

			SetWindowPos(hTabWnd, 0, splitterX + 3, top + splitterY + splitterW, rc.right - splitterX - chatW, h - splitterY - 3, flags);
			EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_RESIZETAB);

			SendMessage(hMainWnd, WMU_UPDATE_TOOLBAR_BUTTONS, 0, 0);
		}
		break;

		case WMU_UPDATE_TOOLBAR_BUTTONS: {
			bool isEnable = prefs::get("ai-enable");
			bool isShowChat = isEnable && prefs::get("ai-show-chat");
			bool isChatMaximized = isEnable && isShowChat && prefs::get("ai-chat-maximized");
			int tabCurrent = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);

			if (isChatMaximized) {
				Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, TBSTATE_HIDDEN);
				Toolbar_SetButtonState(hToolbarWnd, IDM_SPAN, TBSTATE_HIDDEN);
				Toolbar_SetButtonState(hToolbarWnd, IDM_PLAN, TBSTATE_HIDDEN);
				Toolbar_SetButtonState(hToolbarWnd, IDM_AI_EXPLAIN, TBSTATE_HIDDEN);
				Toolbar_SetButtonState(hToolbarWnd, IDM_EXECUTE, TBSTATE_HIDDEN);
				Toolbar_SetButtonState(hToolbarWnd, IDM_INTERRUPT, TBSTATE_HIDDEN);
			} else {
				bool isRunning = tabCurrent == -1 ? cli.thread : tabs[tabCurrent].thread;
				BYTE state = isRunning ? TBSTATE_INDETERMINATE : TBSTATE_ENABLED;

				Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, TBSTATE_ENABLED);
				Toolbar_SetButtonState(hToolbarWnd, IDM_SPAN, TBSTATE_ENABLED);
				Toolbar_SetButtonState(hToolbarWnd, IDM_PLAN, state);
				Toolbar_SetButtonState(hToolbarWnd, IDM_AI_EXPLAIN, isEnable ? (tabCurrent != -1 ? state : TBSTATE_INDETERMINATE) : TBSTATE_HIDDEN);
				Toolbar_SetButtonState(hToolbarWnd, IDM_EXECUTE, state);
				Toolbar_SetButtonState(hToolbarWnd, IDM_INTERRUPT, isRunning ? TBSTATE_ENABLED : TBSTATE_HIDDEN);
			}

			Toolbar_SetButtonState(hToolbarWnd, IDM_AI_CHAT, !isEnable ? TBSTATE_HIDDEN : (isShowChat ? TBSTATE_CHECKED : 0) | TBSTATE_ENABLED);

			if (isEnable) {
				RECT rcToolbar;
				GetClientRect(hToolbarWnd, &rcToolbar);

				RECT rcSpan, rcChat;
				SendMessage(hToolbarWnd, TB_GETRECT, IDM_SPAN2, (LPARAM)&rcSpan);
				SendMessage(hToolbarWnd, TB_GETRECT, IDM_AI_CHAT, (LPARAM)&rcChat);

				TBBUTTONINFO bi = {0};
				bi.cbSize = sizeof(TBBUTTONINFO);
				bi.dwMask = TBIF_SIZE;
				bi.cx = rcToolbar.right - (rcChat.right - rcChat.left) - rcSpan.left;
				SendMessage(hToolbarWnd, TB_SETBUTTONINFO, IDM_SPAN2, (LPARAM)&bi);
			}
		}
		break;

		case WMU_SET_THEME: {
			DeleteObject(hFont);
			hFont = loadFont();
			editorNumWidth = utils::getTextSize(hFont, TEXT("0000")).cx + 10;

			setEditorFont(hEditorWnd);
			SendMessage(hTreeWnd, WM_SETFONT, (LPARAM)hFont, FALSE);
			SendMessage(hTreeSearchWnd, WM_SETFONT, (LPARAM)hFont, FALSE);
			SendMessage(hAutoComplete, WM_SETFONT, (LPARAM)hFont, FALSE);
			SendMessage(hAutoCompleteHelp, WM_SETFONT, (LPARAM)hFont, FALSE);
			SendMessage(hTooltipWnd, WM_SETFONT, (LPARAM)hFont, FALSE);

			int w = utils::getTextSize(hFont, TEXT("0123456789012345678901")).cx + 10;
			SetWindowPos(hAutoComplete, 0, 0, 0, w, 170, SWP_NOMOVE | SWP_NOZORDER);
			ShowWindow(hAutoComplete, SW_HIDE);

			EnumChildWindows(hMainWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SET_THEME);

			for (int i = 0; i < MAX_DIALOG_COUNT; i++) {
				HWND hDlg = hDialogs[i];
				if (hDlg)
					setEditorFont(GetDlgItem(hDlg, IDC_DLG_EDITOR));
			}

			SendMessage(hAiChatWnd, WMU_SET_THEME, 0, 0);
		}
		break;

		case WMU_UPDATE_SB_RESULTSET: {
			HWND hListWnd = (HWND)SendMessage(hWnd, WMU_GET_CURRENT_RESULTSET, 0, 0);
			int* pTotalRowCount = (int*)GetProp(hListWnd, TEXT("TOTALROWCOUNT"));
			int* pRowCount = (int*)GetProp(hListWnd, TEXT("ROWCOUNT"));

			TCHAR buf[64] {0};
			if (pTotalRowCount && pRowCount) {
				if (*pRowCount != *pTotalRowCount)
					_sntprintf(buf, 63, TEXT(" %i/%i"), *pRowCount, *pTotalRowCount);
				else
					_sntprintf(buf, 63, TEXT(" %i"), *pTotalRowCount);
			}

			SendMessage(hStatusWnd, SB_SETTEXT, SB_RESULTSET, (LPARAM)buf);
		}
		break;


		default:
			return DefWindowProc (hWnd, msg, wParam, lParam);
	}

	return 0;
}


LRESULT CALLBACK cbNewListView(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		// https://devblogs.microsoft.com/oldnewthing/20070626-00/?p=26263
		case WM_GETDLGCODE: {
			if (lParam && ((MSG*)lParam)->message == WM_KEYDOWN && (wParam == VK_RETURN))
			   return DLGC_WANTMESSAGE;
		}
		break;

		case WM_SYSKEYUP: {
			// Quick reference tooltip
			if (wParam == VK_MENU && IsWindowVisible(hTooltipWnd))
				hideTooltip();
		}
		break;

		case WM_XBUTTONDOWN: {
			ListView_DrillUp(hWnd);
		}
		break;

		case WM_LBUTTONDOWN: {
			LVHITTESTINFO hi{0};
			hi.pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			ListView_SubItemHitTest(hWnd, &hi);
			SendMessage(GetParent(hWnd), WMU_SET_CURRENT_CELL, hi.iItem, hi.iSubItem);
		}
		break;

		case WM_HSCROLL: {
			if (LOWORD(wParam) == SB_ENDSCROLL)
				InvalidateRect(hWnd, 0, FALSE);
		}
		break;

		case WM_DESTROY : {
			SendMessage(hWnd, WMU_RESET_MODIFIER, -1, 0);
			SendMessage(hWnd, WMU_RESET_CACHE, 0, 0);

			int* pOrderBy = (int*)GetProp(hWnd, TEXT("ORDERBY"));
			if (pOrderBy)
				delete pOrderBy;

			int* pRowCount = (int*)GetProp(hWnd, TEXT("ROWCOUNT"));
			if (pRowCount)
				delete pRowCount;

			int* pTotalRowCount = (int*)GetProp(hWnd, TEXT("TOTALROWCOUNT"));
			if (pTotalRowCount)
				delete pTotalRowCount;

			RemoveProp(hWnd, TEXT("DATATYPES"));
			RemoveProp(hWnd, TEXT("BLOBS"));
			RemoveProp(hWnd, TEXT("CACHE"));
			RemoveProp(hWnd, TEXT("RESULTSET"));
			RemoveProp(hWnd, TEXT("ROWCOUNT"));
			RemoveProp(hWnd, TEXT("TOTALROWCOUNT"));
			RemoveProp(hWnd, TEXT("VALUECOUNT"));
			RemoveProp(hWnd, TEXT("CURRENTROW"));
			RemoveProp(hWnd, TEXT("CURRENTCOLUMN"));
			RemoveProp(hWnd, TEXT("ORDERBY"));
		}
		break;

		case WM_COMMAND: {
			// Edit data
			if (HIWORD(wParam) == CBN_KILLFOCUS && LOWORD(wParam) == IDC_DLG_VALUE_SELECTOR)
				DestroyWindow((HWND)lParam);
		}
		break;

		case WM_NOTIFY: {
			NMHDR* pHdr = (LPNMHDR)lParam;
			HWND hHeader = ListView_GetHeader(hWnd);

			// Prevent zero-width column resizing
			if ((pHdr->code == HDN_BEGINTRACK || pHdr->code == HDN_DIVIDERDBLCLICK) && ListView_GetColumnWidth(hWnd, ((LPNMHEADER)lParam)->iItem) == 0)
				return true;

			// issue #68: prevent sizing column smaller than caption
			if (pHdr->code == HDN_ITEMCHANGED && pHdr->hwndFrom == hHeader && (GetTickCount() - doubleClickTick < 200))
				InvalidateRect(pHdr->hwndFrom, NULL, false);

			if (pHdr->code == HDN_ITEMCHANGING && pHdr->hwndFrom == hHeader && (GetTickCount() - doubleClickTick < 100)) {
				NMHEADER* nmh = (LPNMHEADER) lParam;
				if (nmh->pitem && nmh->pitem->mask & HDI_WIDTH) {
					int w = ListView_GetColumnWidth(hWnd, nmh->iItem);
					if (nmh->pitem->cxy == w)
						return false;

					TCHAR colname16[256];
					Header_GetItemText(hHeader, nmh->iItem, colname16, 255);
					w = utils::getTextSize(hFont, colname16).cx;
					nmh->pitem->cxy = MAX(w + 12, nmh->pitem->cxy);
				}
			}

			if (pHdr->code == HDN_DIVIDERDBLCLICK && pHdr->hwndFrom == ListView_GetHeader(hWnd))
				doubleClickTick = GetTickCount();
			/* issue #68 end */

			if (pHdr->code == (UINT)NM_CUSTOMDRAW && pHdr->hwndFrom == hHeader) {
				HWND hRefList = GetDlgItem(hWnd, IDC_REFLIST);
				if (!hRefList)
					return 0;

				LPNMCUSTOMDRAW pCustomDraw = (LPNMCUSTOMDRAW)lParam;
				if (pCustomDraw->dwDrawStage == CDDS_PREPAINT)
					return CDRF_NOTIFYITEMDRAW;

				if (pCustomDraw->dwDrawStage == CDDS_ITEMPREPAINT)
					return CDRF_NOTIFYPOSTPAINT;

				if (pCustomDraw->dwDrawStage == CDDS_ITEMPOSTPAINT) {
					if (ListBox_GetTextLen(hRefList, pCustomDraw->dwItemSpec) == 0)
						return CDRF_DODEFAULT;

					RECT rc = pCustomDraw->rc;
					HDC hDC = pCustomDraw->hdc;

					COLORREF color = RGB(128, 128, 250);
					HPEN hPen = CreatePen(PS_SOLID, 1, color);
					HBRUSH hBrush = CreateSolidBrush(color);

					HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
					HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
					int r = 4;
					Ellipse(hDC, rc.right - 3, rc.top + 2, rc.right - 3 - r, rc.top + 2 + r);
					SelectObject(hDC, hOldPen);
					SelectObject(hDC, hOldBrush);
					DeleteObject(hPen);
					DeleteObject(hBrush);
				}

				return CDRF_DODEFAULT;
			}
		}
		break;

		// wParam = searchString or 0
		case WMU_RESULT_SEARCH: {
			if (wParam)
				_tcsncpy(resultSearchString, (TCHAR*)wParam, 255);

			bool isVirtual = GetWindowLong(hWnd, GWL_STYLE) & LVS_OWNERDATA;
			if (_tcslen(resultSearchString) == 0 || !isVirtual)
				return 0;

			TCHAR*** cache = (TCHAR***)GetProp(hWnd, TEXT("CACHE"));
			int* resultset = (int*)GetProp(hWnd, TEXT("RESULTSET"));

			bool isBackward = HIWORD(GetKeyState(VK_SHIFT));

			int rowNo = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
			int colNo = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN")) + 1;
			int colCount = Header_GetItemCount(ListView_GetHeader(hWnd));
			int rowCount = *(int*)(LONG_PTR)GetProp(hWnd, TEXT("ROWCOUNT"));

			bool isCaseSensitive = prefs::get("case-sensitive");
			bool isFound = false;
			do {
				for (;!isFound && colNo < colCount; colNo++)
					isFound = isCaseSensitive ? _tcsstr(cache[resultset[rowNo]][colNo], resultSearchString) != 0 : utils::hasString(cache[resultset[rowNo]][colNo], resultSearchString);

				colNo = isFound ? colNo - 1 : 1;
				rowNo += isFound ? 0 : isBackward ? -1 : 1;
			} while (!isFound && (isBackward ? rowNo > 0 : rowNo < rowCount));

			if (isFound) {
				ListView_SetItemState(hWnd, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
				ListView_SetItemState(hWnd, rowNo, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				ListView_EnsureVisible(hWnd, rowNo, FALSE);
				SendMessage(GetParent(hWnd), WMU_SET_CURRENT_CELL, rowNo, colNo);
			} else {
				MessageBeep(0);
			}
		}
		break;

		case WMU_UPDATE_RESULTSET: {
			bool isVirtual = GetWindowLong(hWnd, GWL_STYLE) & LVS_OWNERDATA;
			if (!isVirtual)
				return 0;

			HWND hListWnd = hWnd;
			HWND hHeader = ListView_GetHeader(hListWnd);

			ListView_SetItemCount(hListWnd, 0);
			TCHAR*** cache = (TCHAR***)GetProp(hWnd, TEXT("CACHE"));
			int* pTotalRowCount = (int*)GetProp(hWnd, TEXT("TOTALROWCOUNT"));
			int* pRowCount = (int*)GetProp(hWnd, TEXT("ROWCOUNT"));
			int* pOrderBy = (int*)GetProp(hWnd, TEXT("ORDERBY"));
			int* resultset = (int*)GetProp(hWnd, TEXT("RESULTSET"));

			if (!cache || *pTotalRowCount == 0) {
				SendMessage(hWnd, WMU_SET_HEADER_FILTERS, 0, 0);
				return 1;
			}

			int colCount = Header_GetItemCount(hHeader);
			if (colCount == 0)
				return 1;

			if (resultset)
				free(resultset);

			BOOL* bResultset = (BOOL*)calloc(*pTotalRowCount, sizeof(BOOL));
			for (int rowNo = 0; rowNo < *pTotalRowCount; rowNo++)
				bResultset[rowNo] = TRUE;

			for (int colNo = 0; colNo < colCount; colNo++) {
				HWND hEdit = GetDlgItem(hHeader, IDC_HEADER_EDIT + colNo);
				TCHAR filter[MAX_TEXT_LENGTH + 1];
				GetWindowText(hEdit, filter, MAX_TEXT_LENGTH);
				int len = _tcslen(filter);
				if (len == 0)
					continue;

				for (int rowNo = 0; rowNo < *pTotalRowCount; rowNo++) {
					if (!bResultset[rowNo])
						continue;

					TCHAR* value = ListView_GetItemValueText(hListWnd, rowNo, colNo, true); //cache[rowNo][colNo];
					if (len > 1 && (filter[0] == TEXT('<') || filter[0] == TEXT('>')) && utils::isNumber(filter + 1, NULL)) {
						TCHAR* end = 0;
						double df = _tcstod(filter + 1, &end);
						double dv = _tcstod(value, &end);
						bResultset[rowNo] = (filter[0] == TEXT('<') && dv < df) || (filter[0] == TEXT('>') && dv > df);
					} else {
						bResultset[rowNo] = len == 1 ? utils::hasString(value, filter) :
							filter[0] == TEXT('=') ? _tcsicmp(value, filter + 1) == 0 :
							filter[0] == TEXT('!') ? _tcsstr(value, filter + 1) == 0 :
							filter[0] == TEXT('<') ? _tcscmp(value, filter + 1) < 0 :
							filter[0] == TEXT('>') ? _tcscmp(value, filter + 1) > 0 :
							utils::hasString(value, filter);
					}
				}
			}

			int rowCount = 0;
			resultset = (int*)calloc(*pTotalRowCount, sizeof(int));
			for (int rowNo = 0; rowNo < *pTotalRowCount; rowNo++) {
				if (!bResultset[rowNo])
					continue;

				resultset[rowCount] = rowNo;
				rowCount++;
			}
			free(bResultset);

			if (rowCount > 0) {
				if (rowCount > *pTotalRowCount)
					MessageBeep(0);
				resultset = (int*)realloc(resultset, rowCount * sizeof(int));
				SetProp(hWnd, TEXT("RESULTSET"), (HANDLE)resultset);
				int orderBy = *pOrderBy;

				if (orderBy) {
					int colNo = abs(*pOrderBy);
					BOOL isBackward = orderBy < 0;

					BOOL isNum = TRUE;
					for (int i = 0; i < *pTotalRowCount && i <= 5; i++)
						isNum = isNum && utils::isNumber(cache[i][colNo], NULL);

					if (isNum) {
						double* nums = (double*)calloc(*pTotalRowCount, sizeof(double));
						for (int i = 0; i < rowCount; i++)
							nums[resultset[i]] = _tcstod(cache[resultset[i]][colNo], NULL);

						utils::mergeSort(resultset, (void*)nums, 0, rowCount - 1, isBackward, isNum);
						free(nums);
					} else {
						TCHAR** strings = (TCHAR**)calloc(*pTotalRowCount, sizeof(TCHAR*));
						for (int i = 0; i < rowCount; i++)
							strings[resultset[i]] = cache[resultset[i]][colNo];
						utils::mergeSort(resultset, (void*)strings, 0, rowCount - 1, isBackward, isNum);
						free(strings);
					}
				}
			} else {
				SetProp(hWnd, TEXT("RESULTSET"), (HANDLE)0);
				free(resultset);
			}

			*pRowCount = rowCount;
			ListView_SetItemCount(hListWnd, rowCount);
			InvalidateRect(hListWnd, NULL, TRUE);

			SendMessage(hMainWnd, WMU_UPDATE_SB_RESULTSET, 0, 0);
			SendMessage(hListWnd, WMU_SET_HEADER_FILTERS, 0, 0);
		}
		break;

		case WMU_SET_HEADER_FILTERS: {
			HWND hHeader = ListView_GetHeader(hWnd);
			int isShowFilters = prefs::get("show-filters") || GetDlgCtrlID(hWnd) == IDC_DLG_ROWS;
			int colCount = Header_GetItemCount(hHeader);

			SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);
			LONG_PTR styles = GetWindowLongPtr(hHeader, GWL_STYLE);
			styles = isShowFilters ? styles | HDS_FILTERBAR : styles & (~HDS_FILTERBAR);
			SetWindowLongPtr(hHeader, GWL_STYLE, styles);

			for (int colNo = 1; colNo < colCount; colNo++)
				ShowWindow(GetDlgItem(hHeader, IDC_HEADER_EDIT + colNo), isShowFilters ? SW_SHOW : SW_HIDE);

			SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);
			if (isShowFilters)
				SendMessage(hWnd, WMU_UPDATE_FILTER_SIZE, 0, 0);

			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;

		case WMU_UPDATE_FILTER_SIZE: {
			bool isVisible = ShowWindow(hWnd, SW_SHOW);

			HWND hHeader = ListView_GetHeader(hWnd);
			int colCount = Header_GetItemCount(hHeader);
			SendMessage(hHeader, WM_SIZE, 0, 0);
			for (int colNo = 1; colNo < colCount; colNo++) {
				RECT rc;
				Header_GetItemRect(hHeader, colNo, &rc);
				int h2 = round((rc.bottom - rc.top) / 2);
				SetWindowPos(GetDlgItem(hHeader, IDC_HEADER_EDIT + colNo), 0, rc.left, h2, rc.right - rc.left, h2 + 1, SWP_NOZORDER | SWP_NOACTIVATE);
			}

			if (!isVisible)
				ShowWindow(hWnd, SW_HIDE);
		}
		break;

		case WMU_AUTO_COLUMN_SIZE: {
			HWND hHeader = ListView_GetHeader(hWnd);
			int colCount = Header_GetItemCount(hHeader);

			bool isVirtual = GetWindowLong(hWnd, GWL_STYLE) & LVS_OWNERDATA;
			SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);
			EnableWindow(hHeader, FALSE);

			int w = ListView_GetColumnWidth(hWnd, colCount - 1);
			if (w == 0)
				colCount--; // Ignore the last column in Edit data dialog

			int maxWidth = prefs::get("max-column-width");
			for (int colNo = 1; colNo < colCount; colNo++) {
				int w1;
				// LVSCW_AUTOSIZE_USEHEADER ignores last column
				if (colNo != colCount - 1) {
					ListView_SetColumnWidth(hWnd, colNo, LVSCW_AUTOSIZE_USEHEADER);
					w1 = ListView_GetColumnWidth(hWnd, colNo);
				} else {
					TCHAR* buf = new TCHAR[MAX_TEXT_LENGTH + 1];
					Header_GetItemText(hHeader, colNo, buf, MAX_TEXT_LENGTH);
					w1 = ListView_GetStringWidth(hWnd, buf) + 12;
					delete [] buf;
				}

				int w2 = w1;
				if (!isVirtual) {
					ListView_SetColumnWidth(hWnd, colNo, LVSCW_AUTOSIZE);
					w2 = ListView_GetColumnWidth(hWnd, colNo);
				} else {
					int w = 0;
					TCHAR*** cache = (TCHAR***)GetProp(hWnd, TEXT("CACHE"));
					int* pTotalRowCount = (int*)GetProp(hWnd, TEXT("TOTALROWCOUNT"));
					for (int rowNo = 0; rowNo < MIN(*pTotalRowCount, 200); rowNo++)
						w = MAX(ListView_GetStringWidth(hWnd, cache[rowNo][colNo]), w);

					w += 12; // 2 * indent
					ListView_SetColumnWidth(hWnd, colNo, w);
					w2 = w;
				}

				int w = MAX(w1, w2);
				w = w > maxWidth ? maxWidth : w > 0 && w < 40 ? 40 : w;
				ListView_SetColumnWidth(hWnd, colNo, w);
			}
			EnableWindow(hHeader, TRUE);
			SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(hWnd, NULL, TRUE);

			PostMessage(hWnd, WMU_UPDATE_FILTER_SIZE, 0, 0);
		}
		break;

		// Add empty row
		case WMU_ADD_EMPTY_ROW: {
			TCHAR*** cache = (TCHAR***)GetProp(hWnd, TEXT("CACHE"));
			int* pTotalRowCount = (int*)GetProp(hWnd, TEXT("TOTALROWCOUNT"));
			int* pRowCount = (int*)GetProp(hWnd, TEXT("ROWCOUNT"));
			int* resultset = (int*)GetProp(hWnd, TEXT("RESULTSET"));
			byte* datatypes = (byte*)GetProp(hWnd, TEXT("DATATYPES"));
			int vCount = (int)(LONG_PTR)GetProp(hWnd, TEXT("VALUECOUNT"));
			unsigned char** blobs = (unsigned char**)GetProp(hWnd, TEXT("BLOBS"));

			HWND hHeader = ListView_GetHeader(hWnd);
			int colCount = Header_GetItemCount(hHeader) - 1;

			*pRowCount = *pRowCount + 1;
			*pTotalRowCount = *pTotalRowCount + 1;
			cache = (TCHAR***)realloc(cache, *pTotalRowCount * sizeof(TCHAR**));
			SetProp(hWnd, TEXT("CACHE"), cache);

			vCount = (colCount + 1) * (*pTotalRowCount);
			SetProp(hWnd, TEXT("VALUECOUNT"), IntToPtr(vCount));
			datatypes = (byte*)realloc(datatypes, vCount * sizeof(byte));
			SetProp(hWnd, TEXT("DATATYPES"), datatypes);
			blobs = (unsigned char**)realloc(blobs, vCount * sizeof(unsigned char*));
			SetProp(hWnd, TEXT("BLOBS"), blobs);
			resultset = (int*)realloc(resultset, *pTotalRowCount * sizeof(int));
			SetProp(hWnd, TEXT("RESULTSET"), resultset);

			int rowNo = *pTotalRowCount - 1;
			resultset[rowNo] = rowNo;
			cache[rowNo] = (TCHAR**)calloc (colCount + 1, sizeof (TCHAR*));
			for (int colNo = 0; colNo < colCount + 1; colNo++) {
				int idx = colNo + rowNo * (colCount + 1);
				cache[rowNo][colNo] = (TCHAR*)calloc (1, sizeof (TCHAR));
				blobs[idx] = 0;
				datatypes[idx] = SQLITE_NULL;
			}

			ListView_SetItemCount(hWnd, *pTotalRowCount);
		}
		break;

		case WMU_RESET_CACHE: {
			TCHAR*** cache = (TCHAR***)GetProp(hWnd, TEXT("CACHE"));
			int* pTotalRowCount = (int*)GetProp(hWnd, TEXT("TOTALROWCOUNT"));
			int colCount = Header_GetItemCount(ListView_GetHeader(hWnd));
			if (colCount > 0 && cache != 0) {
				for (int rowNo = 0; rowNo < *pTotalRowCount; rowNo++) {
					if (cache[rowNo]) {
						for (int colNo = 0; colNo < colCount; colNo++)
							if (cache[rowNo][colNo])
								free(cache[rowNo][colNo]);

						free(cache[rowNo]);
					}
					cache[rowNo] = 0;
				}
				free(cache);
			}

			int* resultset = (int*)GetProp(hWnd, TEXT("RESULTSET"));
			if (resultset) {
				SetProp(hWnd, TEXT("RESULTSET"), 0);
				free(resultset);
			}

			byte* datatypes = (byte*)GetProp(hWnd, TEXT("DATATYPES"));
			if (datatypes) {
				SetProp(hWnd, TEXT("DATATYPES"), 0);
				free(datatypes);
			}

			int* modifiers = (int*)GetProp(hWnd, TEXT("MODIFIERS"));
			if (modifiers) {
				SetProp(hWnd, TEXT("MODIFIERS"), 0);
				free(modifiers);
			}

			int* modifierRows = (int*)GetProp(hWnd, TEXT("MODIFIER_ROWS"));
			if (modifierRows) {
				SetProp(hWnd, TEXT("MODIFIER_ROWS"), 0);
				free(modifierRows);
			}

			TCHAR* modifierBuffer = (TCHAR*)GetProp(hWnd, TEXT("MODIFIERBUFFER"));
			if (modifierBuffer) {
				SetProp(hWnd, TEXT("MODIFIERBUFFER"), 0);
				delete [] modifierBuffer;
			}

			int vCount = (int)(LONG_PTR)GetProp(hWnd, TEXT("VALUECOUNT"));
			unsigned char** blobs = (unsigned char**)GetProp(hWnd, TEXT("BLOBS"));
			if (blobs) {
				SetProp(hWnd, TEXT("BLOBS"), 0);

				for (int i = 0; i < vCount; i++)
					if (blobs[i])
						delete [] blobs[i];
				free(blobs);
			}
		}
		break;

		// wParam = colNo, -1 = all
		case WMU_RESET_MODIFIER: {
			int* modifiers = (int*)GetProp(hWnd, TEXT("MODIFIERS"));
			if (modifiers) {
				int _colNo = (int)wParam;
				int colCount = ListView_GetColumnCount(hWnd);
				for (int colNo = 0; colNo < colCount; colNo++) {
					if (_colNo != colNo && _colNo != -1)
						continue;

					int modifierNo = modifiers[colNo];
					if (modifierNo > 0 && modifierNo < MAX_PLUGIN_COUNT && plugins[modifierNo].hModule && plugins[modifierNo].deactivate)
						plugins[modifierNo].deactivate(hWnd, colNo);
					modifiers[colNo] = 0;
				}
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (processEditKeys(hWnd, msg, wParam, lParam))
		return 0;

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg){
		case WM_GETDLGCODE: {
			return (DLGC_WANTTAB | CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam));
		}
		break;

		case WM_PAINT: {
			if (GetProp(hWnd, TEXT("NOREDRAW")))
				return false;

			bool isTabEditor = false;
			 for (int i = 0; !isTabEditor && i < MAX_TAB_COUNT; i++)
				isTabEditor = tabs[i].hEditorWnd == hWnd;

			if (isTabEditor) {
				RECT rc;
				GetClientRect(hWnd, &rc);
				rc.left = editorNumWidth + 1;
				InvalidateRect(hWnd, &rc, FALSE);

				rc.left = 0;
				rc.right = editorNumWidth;
				ValidateRect(hWnd, &rc);
			}

			LRESULT res = CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);

			if (isTabEditor)
				SendMessage(hWnd, WM_PRINTCLIENT, 0, 0);

            return res;
		}
		break;

		case WM_PRINTCLIENT: {
			HideCaret(hWnd);
			HDC hDC = GetDC(hWnd);

			RECT rc;
			GetClientRect(hWnd, &rc);
			rc.right = editorNumWidth;

			HDC hTmpDC = CreateCompatibleDC(hDC);
			HBITMAP hTmpBmp = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
			HGDIOBJ hPrevBmp = SelectObject(hTmpDC, hTmpBmp);
			HGDIOBJ hPrevFont = SelectObject(hTmpDC, hFont);

			FillRect(hTmpDC, &rc, hEditorNumBrush);
			SetBkMode(hTmpDC, TRANSPARENT);
			SetTextColor(hTmpDC, RGB(128, 128, 128));

			int lineCount = SendMessage(hWnd, EM_GETLINECOUNT, 0, 0);
			int lineNo = SendMessage(hWnd, EM_GETFIRSTVISIBLELINE, 0, 0);

			// If word-wrap mode is ON then line numbering should skip some numbers
			bool isBreak = false;
			int breakCount = 0;
			while (lineNo < lineCount) {
				int chrIdx = SendMessage(hWnd, EM_LINEINDEX, lineNo, 0);
				int len = SendMessage(hWnd, EM_LINELENGTH, chrIdx, 0);

				POINTL p;
				SendMessage(hWnd, EM_POSFROMCHAR, (WPARAM)&p, chrIdx);
				if (p.y > rc.bottom)
					break;

				if (!isBreak) {
					TCHAR num16[32];
					_sntprintf(num16, 32, TEXT("%i"), lineNo + breakCount + 1);
					RECT rc2 = {0, p.y, editorNumWidth - 10, p.y + 100};
					DrawText(hTmpDC, num16, -1, &rc2, DT_RIGHT | DT_TOP);
				}

				isBreak = lineNo != SendMessage(hWnd, EM_LINEFROMCHAR, chrIdx + len, 0);
				breakCount += isBreak;

				lineNo++;
			}

			BitBlt(hDC, 0, 0, rc.right, rc.bottom, hTmpDC, 0, 0, SRCCOPY);

			SelectObject(hDC, hPrevFont);
			SelectObject(hDC, hPrevBmp);
			DeleteObject(hTmpBmp);
			DeleteDC(hTmpDC);
			ReleaseDC(hWnd, hDC);
			ShowCaret(hWnd);

			return 0;
		}
		break;

		case WM_MOUSEWHEEL: {
			if (LOWORD(wParam) == MK_CONTROL) {
				SendMessage(hMainWnd, WM_MOUSEWHEEL, wParam, lParam);
				return 1;
			}
		}
		break;

		case WM_KILLFOCUS: {
			if ((HWND)wParam != hAutoComplete && (HWND)wParam != hWnd /* SetFocus */)
				ShowWindow(hAutoComplete, SW_HIDE);
		}
		break;

		case WM_DESTROY: {
			RemoveProp(hWnd, EDITOR_HIGHLIGHT);
			RemoveProp(hWnd, EDITOR_PARENTHESIS);
			RemoveProp(hWnd, EDITOR_OCCURRENCE);
			RemoveProp(hWnd, EDITOR_HASOCCURRENCE);
			RemoveProp(hWnd, EDITOR_SELECTION_START);

			KillTimer(hWnd, IDT_HIGHLIGHT);

			RemoveProp(hWnd, TEXT("LASTCLICKTIME"));
		}
		break;

		case WM_TIMER: {
			if (wParam == IDT_HIGHLIGHT) {
				KillTimer(hWnd, IDT_HIGHLIGHT);

				processHighlight(hWnd, GetProp(hWnd, EDITOR_HIGHLIGHT), GetProp(hWnd, EDITOR_PARENTHESIS), GetProp(hWnd, EDITOR_OCCURRENCE));
				SetProp(hWnd, EDITOR_HIGHLIGHT, 0);
				SetProp(hWnd, EDITOR_PARENTHESIS, 0);
				SetProp(hWnd, EDITOR_OCCURRENCE, 0);
			}
		}
		break;

		case WMU_HIGHLIGHT: {
			KillTimer(hWnd, IDT_HIGHLIGHT);
			SetTimer(hWnd, IDT_HIGHLIGHT, prefs::get("highlight-delay"), NULL);
		}
		break;

		case WMU_TEXT_CHANGED: {
			if (prefs::get("use-highlight") == 0)
				return 0;

			SetProp(hWnd, EDITOR_HIGHLIGHT, (HANDLE)1);
			SendMessage(hWnd, WMU_HIGHLIGHT, 0, 0);
		}
		break;

		case WMU_SELECTION_CHANGED: {
			SELCHANGE *pSc = (SELCHANGE*)lParam;
			bool isSelectionEmpty = pSc->seltyp == SEL_EMPTY;
			if (isSelectionEmpty)
				SetProp(hWnd, EDITOR_SELECTION_START, LongToPtr(pSc->chrg.cpMin));

			bool isRequireParenthesis = GetProp(hWnd, EDITOR_PARENTHESIS) == 0 && isSelectionEmpty;
			bool isRequireOccurrence = GetProp(hWnd, EDITOR_HASOCCURRENCE) || !isSelectionEmpty;

			if (isRequireParenthesis)
				SetProp(hWnd, EDITOR_PARENTHESIS, (HANDLE)1);

			if (isRequireOccurrence)
				SetProp(hWnd, EDITOR_OCCURRENCE, (HANDLE)1);

			if (isRequireParenthesis || isRequireOccurrence) {
				SetProp(hWnd, EDITOR_HIGHLIGHT, 0);
				SendMessage(hWnd, WMU_HIGHLIGHT, 0, 0);
			}
		}
		break;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewTree(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_DROPFILES) {
		TCHAR path16[MAX_PATH]{0};
		HDROP drop = (HDROP)wParam;

		DragQueryFileW(drop, 0, path16, MAX_PATH);
		TCHAR ext16[32]{0};
		_tsplitpath(path16, NULL, NULL, NULL, ext16);

		if (dbutils::isSQLiteDatabase(path16)) {
			openDb(path16, HIWORD(GetKeyState(VK_CONTROL)));
		} else if (_tcscmp(ext16, TEXT(".sql")) == 0) {
			TCHAR msg16[1024];
			_stprintf(msg16, 1023, TEXT("Are you sure you want to execute \"%ls\"?"), path16);
			if (IDOK == MessageBox(hWnd, msg16, TEXT("Confirmation"), MB_OK) && tools::importSqlFile(path16)) {
				updateTree();
				MessageBox(hWnd, TEXT("Done"), TEXT("Info"), MB_OK);
			}
		} else if (_tcscmp(ext16, TEXT(".csv")) == 0) {
			int rc = DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_IMPORT_CSV), hWnd, (DLGPROC)tools::cbDlgImportCSV, (LPARAM)path16);
			if (rc != -1) {
				TCHAR msg16[256];
				_sntprintf(msg16, 255, TEXT("Done.\nImported %i rows."), rc);
				MessageBox(hWnd, msg16, TEXT("Info"), MB_OK);
				updateTree(TABLE);
			}
		}

		DragFinish(drop);
		return 0;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewTreeItemEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_GETDLGCODE)
		return (DLGC_WANTALLKEYS | CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam));

	if ((msg == WM_KEYUP) && (wParam == VK_ESCAPE)) {
		DestroyWindow(hWnd);
		return 0;
	}

	if (processEditKeys(hWnd, msg, wParam, lParam))
		return 0;

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewTreeSearch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_CHAR && wParam == VK_ESCAPE)
		return 0;

	if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
		SetFocus(hTreeWnd);
		SetWindowText(hWnd, NULL);
		ShowWindow(hWnd, SW_HIDE);
		SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);
		return 0;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewResultTabFilterEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg){
		case WM_PAINT: {
			cbOldResultTabFilterEdit(hWnd, msg, wParam, lParam);

			RECT rc;
			GetClientRect(hWnd, &rc);

			HDC hDC = GetWindowDC(hWnd);
			HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
			HPEN oldPen = (HPEN)SelectObject(hDC, hPen);
			MoveToEx(hDC, 1, 0, 0);
			LineTo(hDC, rc.right - 1, 0);
			LineTo(hDC, rc.right - 1, rc.bottom - 1);

			SelectObject(hDC, oldPen);
			DeleteObject(hPen);
			ReleaseDC(hWnd, hDC);

			return 0;
		}
		break;

		// Prevent beep
		case WM_CHAR: {
			if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == VK_TAB)
				return 0;
		}
		break;

		case WM_KEYDOWN: {
			if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == VK_TAB) {
				HWND hHeader = GetParent(hWnd);
				HWND hListWnd = GetParent(hHeader);

				if (wParam == VK_RETURN)
					SendMessage(hListWnd, WMU_UPDATE_RESULTSET, 0, 0);

				return 0;
			}
		}
		break;
	}

	if (processEditKeys(hWnd, msg, wParam, lParam))
		return 0;

	return CallWindowProc(cbOldResultTabFilterEdit, hWnd, msg, wParam, lParam);
}

int CALLBACK cbListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	int* pOrderBy = (int*)lParamSort;
	int colNo = abs(*pOrderBy);
	int order = *pOrderBy > 0 ? 1 : -1;

	TCHAR buf[256]{0};
	TCHAR buf2[256]{0};

	ListView_GetItemText(hSortingResultWnd, lParam1, colNo, buf, 255);
	ListView_GetItemText(hSortingResultWnd, lParam2, colNo, buf2, 255);

	double num, num2;
	bool isNum = utils::isNumber(buf, &num) && utils::isNumber(buf2, &num2);

	return order * (isNum ? (num > num2 ? 1 : -1) : _tcscoll(buf, buf2));
}

void onCLIQueryEnd(const TCHAR* sql16, TCHAR* result16, bool isSave, int elapsed) {
	if (_tcslen(result16) > 0) {
		_tcscat(result16, TEXT("\n\n============================================================\n\n"));
		SendMessage(cli.hResultWnd, EM_SETSEL, 0, 0);
		SendMessage(cli.hResultWnd, EM_REPLACESEL, 0, (LPARAM)result16);

		if (sql16 && _tcslen(sql16) > 0) {
			SendMessage(cli.hResultWnd, EM_SETSEL, 0, 0);
			SendMessage(cli.hResultWnd, EM_REPLACESEL, 0, (LPARAM)TEXT("\n\n"));
			SendMessage(cli.hResultWnd, EM_SETSEL, 0, 0);
			SendMessage(cli.hResultWnd, EM_REPLACESEL, 0, (LPARAM)sql16);
		}
	}

	if (isSave) {
		sqlite3_stmt* stmt;
		if(SQLITE_OK == sqlite3_prepare_v2(prefs::db, "insert into cli (time, dbname, query, elapsed, result) values (strftime('%s', 'now'), ?1, ?2, ?3, ?4)", -1, &stmt, 0)) {
			char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));
			sqlite3_bind_text(stmt, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
			delete [] dbname8;

			char* sql8 = utils::utf16to8(sql16);
			sqlite3_bind_text(stmt, 2, sql8, strlen(sql8), SQLITE_TRANSIENT);
			delete [] sql8;

			sqlite3_bind_int(stmt, 3, elapsed);

			char* result8 = utils::utf16to8(result16);
			sqlite3_bind_text(stmt, 4, result8, strlen(result8), SQLITE_TRANSIENT);
			delete [] result8;

			if (SQLITE_DONE != sqlite3_step(stmt))
				showDbError(hMainWnd);
		}
		sqlite3_finalize(stmt);
	}

	// Scroll to top
	int cnt = SendMessage(cli.hResultWnd, EM_GETLINECOUNT, 0, 0);
	SendMessage(cli.hResultWnd, EM_LINESCROLL, 0, -cnt);
}

TCHAR* parseInja(TCHAR* buf) {
	TCHAR* res = 0;

	if (isInjaSupport && (
		(_tcsstr(buf, TEXT("{{")) && _tcsstr(buf, TEXT("}}"))) ||
		(_tcsstr(buf, TEXT("{%")) && _tcsstr(buf, TEXT("%}")))
		)) {
		sqlite3_stmt *stmt;
		bool rc = SQLITE_OK == sqlite3_prepare_v2(db, "select inja(?1)", -1, &stmt, 0);
		if (rc) {
			char* buf8 = utils::utf16to8(buf);
			sqlite3_bind_text(stmt, 1, buf8, strlen(buf8), SQLITE_TRANSIENT);
			delete [] buf8;
			rc = SQLITE_ROW == sqlite3_step(stmt);
			if (rc)
				res = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
		}
		sqlite3_finalize(stmt);

		if (!rc)
			showDbError(hMainWnd);
	} else {
		res = new TCHAR[_tcslen(buf) + 1] {0};
		_tcscpy(res, buf);
	}

	return res;
}

unsigned int __stdcall processCliQuery (void* data) {
	updateExecuteMenu(false);
	SendMessage(hMainWnd, WMU_UPDATE_TOOLBAR_BUTTONS, 0, 0);
	SendMessage(hMainTabWnd, WMU_TAB_SET_STYLE, -1, 1);

	float elapsed = 0;
	if (!cli.db)
		openConnection(&cli.db, sqlite3_db_filename(db, 0));

	int size = GetWindowTextLength(cli.hEditorWnd);
	TCHAR sql16[size + 1] = {0};
	GetWindowText(cli.hEditorWnd, sql16, size + 1);
	char *sql8 = utils::utf16to8(sql16);

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(cli.db, sql8, -1, &stmt, 0);
	if (rc == SQLITE_OK && cli.isPlan && !sqlite3_stmt_isexplain(stmt)) {
		sqlite3_finalize(stmt);
		char* new8 = new char[strlen(sql8) + strlen("explain query plan ") + 1]{0};
		sprintf(new8, prefs::get("extended-query-plan") ? "explain %s" : "explain query plan %s", sql8);
		delete [] sql8;
		sql8 = new8;
		rc = sqlite3_prepare_v2(cli.db, sql8, -1, &stmt, 0);
	}

	if (sqlite3_bind_parameter_count(stmt))
		DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_BIND_PARAMETERS), hMainWnd, (DLGPROC)&dialogs::cbDlgBindParameters, (LPARAM)stmt);

	TCHAR result16[MAX_TEXT_LENGTH + 1]{0};
	if (sqlite3_bind_parameter_count(stmt)) {
		_tcscat(result16, TEXT("EXECUTE: "));
		TCHAR* esql16 = utils::utf8to16(sqlite3_expanded_sql(stmt));
		_tcscat(result16, esql16);
		_tcscat(result16, TEXT("\n"));
		delete [] esql16;
	}

	if (rc == SQLITE_OK) {
		DWORD tStart = GetTickCount();
		rc = sqlite3_step(stmt);
		int rowCount = 0;
		int colCount = sqlite3_column_count(stmt);

		if (colCount && (rc == SQLITE_ROW || rc == SQLITE_DONE || rc == SQLITE_OK)) {
			HWND hListWnd = GetDlgItem(cli.hResultWnd, IDC_CLI_RAWDATA);
			ListBox_ResetContent(hListWnd);

			size_t maxWidths[colCount]{0};

			for (int i = 0; i < colCount; i++) {
				TCHAR* name16 = utils::utf8to16(sqlite3_column_name(stmt, i));
				ListBox_AddString(hListWnd, name16);
				maxWidths[i] = _tcslen(name16);
			}

			int rowLimit = prefs::get("cli-row-limit");
			while (rc == SQLITE_ROW && (rowCount < rowLimit || rowLimit <= 0)) {
				for (int i = 0; i < colCount; i++) {
					if (sqlite3_column_type(stmt, i) != SQLITE_BLOB) {
						TCHAR* value16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, i));
						TCHAR* tvalue16 = utils::replaceAll(value16, TEXT("\r\n"), TEXT(" "));
						TCHAR trimmed16[256]{0};
						_tcsncpy(trimmed16, tvalue16, 255);
						delete [] tvalue16;
						delete [] value16;
						if (maxWidths[i] < _tcslen(trimmed16))
							maxWidths[i] = _tcslen(trimmed16);

						ListBox_AddString(hListWnd, trimmed16);
					} else {
						if (maxWidths[i] < 6)
							maxWidths[i] = 6;
						ListBox_AddString(hListWnd, TEXT("(BLOB)"));
					}
				}

				rowCount++;
				rc = sqlite3_step(stmt);
			}

			int w = (colCount - 1) * 3;
			for (int i = 0; i < colCount; i++)
				w += maxWidths[i];

			TCHAR line16[w + 1]{0};
			for (int i = 0; i < w; i++)
				line16[i] = TEXT('-');
			_tcscat(result16, line16);
			_tcscat(result16, TEXT("\n"));

			for (int i = 0; i < colCount; i++) {
				TCHAR name16[255]{0};
				ListBox_GetText(hListWnd, i, name16);
				int len = maxWidths[i] + 10;
				TCHAR buf16[len + 1]{0};
				_sntprintf(buf16, len, TEXT("%-*s"), maxWidths[i], name16);
				_tcscat(result16, buf16);
				_tcscat(result16, (i < colCount - 1) ? TEXT(" | ") : TEXT("\n"));
			}
			_tcscat(result16, line16);
			_tcscat(result16, TEXT("\n"));

			for (int rowNo = 0; rowNo < rowCount; rowNo++) {
				for (int i = 0; i < colCount; i++) {
					int len = maxWidths[i] + 10;
					TCHAR value16[len + 1]{0};
					ListBox_GetText(hListWnd, i + (rowNo + 1) * colCount, value16);
					TCHAR buf16[len + 1]{0};
					_sntprintf(buf16, len, TEXT("%-*s"), maxWidths[i], value16);
					_tcscat(result16, buf16);
					_tcscat(result16, (i < colCount - 1) ? TEXT(" | ") : TEXT("\n"));
				}
			}

			if (rc == SQLITE_ROW) {
				_tcscat(result16, TEXT("... more rows available\n"));
				rc = SQLITE_DONE;
			}

			_tcscat(result16, rowCount ? line16 : TEXT("No rows"));

			DestroyWindow(hListWnd);
		}

		if (rc == SQLITE_DONE) {
			TCHAR total16[256];
			elapsed = (GetTickCount() - tStart) / 1000.0;
			_sntprintf(total16, 255, TEXT("%lsDone. Elapsed time: %.2fs"), colCount > 0 ? TEXT("\n") : TEXT(""), elapsed);
			_tcscat(result16, total16);
		}

		rc = SQLITE_OK;
	}
	sqlite3_finalize(stmt);

	if (rc != SQLITE_OK) {
		const char* msg8 = sqlite3_errmsg(cli.db);
		TCHAR* msg16 = utils::utf8to16(msg8);
		_sntprintf(result16, MAX_TEXT_LENGTH, TEXT("Error: %ls"), msg16);
		delete [] msg16;
	}

	onCLIQueryEnd(sql16, result16, rc == SQLITE_OK, elapsed);
	SetWindowText(cli.hEditorWnd, NULL);
	delete [] sql8;

	RedrawWindow(cli.hResultWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
	SetWindowLongPtr(cli.hEditorWnd, GWLP_USERDATA, 0);

	CloseHandle(cli.thread);
	cli.thread = 0;

	updateExecuteMenu(true);
	SendMessage(hMainWnd, WMU_UPDATE_TOOLBAR_BUTTONS, 0, 0);
	SendMessage(hMainTabWnd, WMU_TAB_SET_STYLE, -1, 0);
	updateTransactionState();
	return 1;
}

int executeCLIQuery(bool isPlan) {
	int size = GetWindowTextLength(cli.hEditorWnd);
	if (size == 0 || cli.thread)
		return 0;

	setEditorColor(cli.hResultWnd, RGB(0, 196, 0), true);

	TCHAR sql16[size + 1] = {0};
	GetWindowText(cli.hEditorWnd, sql16, size + 1);
	if (sql16[0] == TEXT('.')) {
		int nArgs = 0;
		TCHAR** args = CommandLineToArgvW(sql16, &nArgs);
		bool isSave = true;
		TCHAR result16[MAX_TEXT_LENGTH + 1]{0};
		if (_tcscmp(args[0], TEXT(".clear")) == 0) {
			SetWindowText(cli.hResultWnd, NULL);
		} else if (_tcscmp(args[0], TEXT(".last")) == 0) {
			int cnt = _ttoi(args[1]);
			loadCLIResults(cnt);
			isSave = false;
		} else if (_tcscmp(args[0], TEXT(".set")) == 0 && nArgs >= 3) {
			TCHAR* name16 = args[1];
			TCHAR* value16 = args[2];
			char* name8 = utils::utf16to8(name16);
			char* value8 = utils::utf16to8(value16);

			_sntprintf(result16, MAX_TEXT_LENGTH, prefs::set(name8, _ttoi(value16)) || prefs::set(name8, value8, true) ? TEXT("Done: %ls = %ls") : TEXT("Error: invalid name %ls for value %ls"), name16, value16);

			delete [] name8;
			delete [] value8;
		} else if (_tcscmp(args[0], TEXT(".set")) == 0 && nArgs < 3) {
			isSave = false;
			_tcscat(result16, TEXT("Error: incorrect input\nUsage: .set <name> <value>"));
		} else if (_tcscmp(args[0], TEXT(".get")) == 0) {
			sqlite3_stmt* stmt;
			if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select name, coalesce(value, '') from prefs where name = coalesce(trim(?1), name) and name not like 'editor-%' order by 1", -1, &stmt, 0)) {
				if (nArgs > 1) {
					char* name8 = utils::utf16to8(args[1]);
					sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
					delete [] name8;
				}

				int paramNo = 0;
				while(SQLITE_ROW == sqlite3_step(stmt)) {
					const char* name8 = (const char*)sqlite3_column_text(stmt, 0);
					const char* value8 = (const char*)sqlite3_column_text(stmt, 1);
					int value = prefs::get(name8);
					char buf8[strlen(name8) + strlen(value8) + 10];
					if (value == -1)
						sprintf(buf8, "%s: %s", name8, value8);
					else
						sprintf(buf8, "%s: %i", name8, value);

					TCHAR* buf16 = utils::utf8to16(buf8);
					if (paramNo > 0)
						_tcscat(result16, TEXT("\n"));
					_tcscat(result16, buf16);
					delete [] buf16;

					paramNo++;
				}
			}
			sqlite3_finalize(stmt);
			isSave = _tcslen(sql16) > 5;
		} else if (_tcscmp(args[0], TEXT(".import-csv")) == 0 && nArgs == 3) {
			TCHAR err16[1024];
			int rowCount = tools::importCSV(args[1], args[2], err16);
			if (rowCount != -1)	{
				_sntprintf(result16, MAX_TEXT_LENGTH, TEXT("Done. Imported %i rows."), rowCount);
			} else {
				isSave = false;
				_sntprintf(result16, MAX_TEXT_LENGTH, err16);
			}
		} else if (_tcscmp(args[0], TEXT(".import-csv")) == 0 && nArgs != 3) {
			isSave = false;
			_tcscat(result16, TEXT("Error: incorrect input\nUsage: .import-csv <path> <table>\nUse quotes to mask spaces e.g.\n.import-csv \"D:\\my data\\my file.csv\" \"my table\""));
		} else if (_tcscmp(args[0], TEXT(".export-csv")) == 0 && nArgs == 3) {
			TCHAR err16[1024];
			int rowCount = tools::exportCSV(args[1], args[2], err16);
			if (rowCount != -1)	{
				_sntprintf(result16, MAX_TEXT_LENGTH, TEXT("Done. Exported %i rows."), rowCount);
			} else {
				isSave = false;
				_sntprintf(result16, MAX_TEXT_LENGTH, err16);
			}
		} else if (_tcscmp(args[0], TEXT(".export-csv")) == 0 && nArgs != 3) {
			isSave = false;
			_tcscat(result16, TEXT("Error: incorrect input\nUsage: .export-csv <path> <query>\nUse quotes to mask spaces e.g.\n.export-csv \"D:\\my data\\my file.csv\" \"select * from t\""));
		} else {
			isSave = false;
			LoadString(GetModuleHandle(NULL), IDS_CLI_HELP, result16, MAX_TEXT_LENGTH);
		}

		onCLIQueryEnd(sql16, result16, isSave, 0);
		SetWindowText(cli.hEditorWnd, NULL);

		LocalFree(args);
		return 0;
	}

	TCHAR* inja16 = parseInja(sql16);
	if (!inja16)
		return 0;

	if (_tcscmp(inja16, sql16) == 0) {
		cli.isPlan = isPlan;
		cli.thread = (HANDLE)_beginthreadex(0, 0, &processCliQuery, 0, 0, 0);
	} else {
		int len = _tcslen(inja16) + 255;
		TCHAR* res16 =  new TCHAR[len + 1];
		_stprintf(res16, len, TEXT("Inja script result\n------------------------------------------------------------\n%ls"), inja16);

		if (prefs::get("cli-preserve-inja")) {
			onCLIQueryEnd(TEXT(""), res16, false, 0);
		} else {
			onCLIQueryEnd(sql16, res16, true, 0);
			SetWindowText(cli.hEditorWnd, NULL);
		}

		delete [] res16;
	}
	delete [] inja16;
	return 1;
}

static int cbProcessEditorQueryBatch(void *queryNo, int count, char **data, char **columns) {
	(*(int*)queryNo)++;
	return 0;
}

int getPinnedResultCount(int tabNo) {
	if (tabNo == -1 || tabNo >= MAX_TAB_COUNT)
		return 0;

	int cnt = 0;
	for (int resultNo = 0; resultNo < MAX_RESULT_COUNT; resultNo++)
		cnt += tabs[tabNo].isPinned[resultNo];

	return cnt;
}

unsigned int __stdcall processEditorQuery (void* data) {
	TEditorTab* tab = (TEditorTab*)data;
	updateExecuteMenu(false);
	SendMessage(hMainWnd, WMU_UPDATE_TOOLBAR_BUTTONS, 0, 0);

	if (!tab->db)
		openConnection(&tab->db, sqlite3_db_filename(db, 0));

	// reset meta-data cache
	// If a table was deleted in an another tab then the table can still exist in this
	sqlite3_exec(tab->db, "select 1 from sqlite_master limit 0", 0, 0, 0);

	auto getTabNo = [](int id) {
		for (int i = 0; i < MAX_TAB_COUNT; i++)
			if (tabs[i].id == id)
				return i;

		return -1;
	};

	TEditorTab _tab = *tab;
	SendMessage(hMainTabWnd, WMU_TAB_SET_STYLE, getTabNo(_tab.id), 1);

	auto detectDataChanges = [](const char* sql8) {
		char* query8 = _strdup(sql8);
		strlwr(query8);

		auto hasString = [](const char* text, const char* word) {
			bool res = false;
			int tlen = strlen(text);
			int wlen = strlen(word);
			for (int pos = 0; pos < tlen - wlen && !res; pos++)
				res = strstr(text + pos, word) != NULL && (pos == tlen - wlen - 1 || !isalnum(text[pos + wlen]));

			return res;
		};

		bool res = (hasString(query8, "update") && hasString(query8, "set")) ||
			(hasString(query8, "insert") && hasString(query8, "into")) ||
			(hasString(query8, "delete") && hasString(query8, "from"));
		free(query8);
		return res;
	};

	auto detectMetaChanges = [](const char* sql8) {
		char* query8 = _strdup(sql8);
		strlwr(query8);
		bool res = strstr(query8, "create table") || strstr(query8, "create virtual table") || strstr(query8, "create view") ||
			strstr(query8, "create index") || strstr(query8, "create trigger") ||
			strstr(query8, "drop table") || strstr(query8, "drop view") || strstr(query8, "alter table") ||
			strstr(query8, "drop index") || strstr(query8, "drop trigger");
		free(query8);
		return res;
	};
	bool isMetaChanged = false;

	int tabNo = getTabNo(_tab.id);
	int pinCount = getPinnedResultCount(tabNo);

	if (_tab.isBatch) {
		int resultNo = pinCount;

		TCITEM tci;
		tci.mask = TCIF_TEXT | TCIF_IMAGE;
		tci.iImage = -1;
		tci.pszText = TEXT("Batch");
		tci.cchTextMax = 6;
		TabCtrl_InsertItem(_tab.hTabWnd, resultNo, &tci);

		DWORD tBatchStart = GetTickCount();
		HWND hResultWnd = GetDlgItem(_tab.hTabWnd, IDC_TAB_MESSAGE + resultNo);

		if (prefs::get("synchronous-off"))
			sqlite3_exec(_tab.db, "pragma synchronous = 0", NULL, 0, NULL);

		bool isTrn = sqlite3_get_autocommit(_tab.db);
		if (isTrn)
			sqlite3_exec(_tab.db, "begin", NULL, 0, NULL);

		int len = ListBox_GetTextLen(_tab.hQueryListWnd, 0) + 1;
		TCHAR query16[len + 1]{0};
		ListBox_GetText(_tab.hQueryListWnd, 0, query16);

		char* query8 = utils::utf16to8(query16);
		char *err8 = NULL;
		int queryNo = 0;
		if (SQLITE_OK != sqlite3_exec(_tab.db, query8, cbProcessEditorQueryBatch, &queryNo, &err8)) {
			char* qstart8 = query8;
			while (queryNo > 0) {
				const char* tail8 = 0;
				sqlite3_stmt *stmt;
				sqlite3_prepare(_tab.db, qstart8, -1, &stmt, &tail8);
				qstart8 = (char*)tail8;
				sqlite3_finalize(stmt);
				queryNo--;
			}

			TCHAR* err16 = utils::utf8to16(err8);
			int len = _tcslen(err16) + 128;
			TCHAR text16[len + 1];
			int lineNo = SendMessage(_tab.hEditorWnd, EM_LINEFROMCHAR, strlen(query8) - strlen(qstart8), 0);
			_sntprintf(text16, len, TEXT("Error\n%ls\nat line %i"), err16, lineNo + 1);
			SetWindowText(hResultWnd, text16);
			delete [] err16;
			sqlite3_free(err8);
		}
		isMetaChanged = detectMetaChanges(query8);
		delete [] query8;

		if (prefs::get("synchronous-off"))
			sqlite3_exec(_tab.db, "pragma synchronous = 1", NULL, 0, NULL);

		if (isTrn)
			sqlite3_exec(_tab.db, err8 == 0 ? "commit" : "rollback", NULL, 0, NULL);

		int tabNo = getTabNo(_tab.id);
		tab = &tabs[tabNo];
		if (tab->tabTooltips[resultNo] != NULL)
			delete [] tab->tabTooltips[resultNo];
		tab->tabTooltips[resultNo] = utils::replaceAll(TEXT("Batch"), TEXT(" "), TEXT(" "));

		_sntprintf(tab->queryElapsedTimes[resultNo], 63, TEXT(" Elapsed: %.2fs"), (GetTickCount() - tBatchStart) / 1000.0);

		TabCtrl_SetCurSel(tab->hTabWnd, resultNo);
		ShowWindow(hResultWnd, SW_SHOW);
		SetWindowLongPtr(tab->hTabWnd, GWLP_USERDATA, (LONG_PTR)hResultWnd); // Also see ACTION_UPDATETAB
		if (hTabWnd == tab->hTabWnd) {
			SendMessage(hStatusWnd, SB_SETTEXT, SB_ELAPSED_TIME, (LPARAM)tab->queryElapsedTimes[resultNo]);
			SendMessage(hMainWnd, WMU_UPDATE_SB_RESULTSET, 0, 0);
		}
		cbEnumChildren(hResultWnd, ACTION_RESIZETAB);

		MessageBeep(0);
	} else {
		bool isExplain = _tab.isExplain;
		bool isAiEnable = prefs::get("ai-enable");

		for (int queryNo = 0; queryNo < ListBox_GetCount(_tab.hQueryListWnd); queryNo++) {
			int resultNo = queryNo + pinCount;

			int size = ListBox_GetTextLen(_tab.hQueryListWnd, queryNo) + 1;
			TCHAR* query16 = new TCHAR[size] {0};
			ListBox_GetText(_tab.hQueryListWnd, queryNo, query16);

			char* sql8 = utils::utf16to8(query16);
			bool isSql = !isAiEnable || !prefs::get("ai-process-nosql") || dbutils::isSqlQuery(sql8);

			int rc = 0;
			sqlite3_stmt *stmt = 0;
			TCHAR* aiResult16 = 0;
			if (!isExplain) {
				if (isSql) {
					rc = sqlite3_prepare_v2(_tab.db, sql8, -1, &stmt, 0);
					if (rc == SQLITE_OK && _tab.isPlan && !sqlite3_stmt_isexplain(stmt)) {
						sqlite3_finalize(stmt);
						char* new8 = new char[strlen(sql8) + strlen("explain query plan ") + 1]{0};
						sprintf(new8, prefs::get("extended-query-plan") ? "explain %s" : "explain query plan %s", sql8);
						delete [] sql8;
						sql8 = new8;
						rc = sqlite3_prepare_v2(_tab.db, sql8, -1, &stmt, 0);
					}

					if (sqlite3_bind_parameter_count(stmt))
						DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_BIND_PARAMETERS), hMainWnd, (DLGPROC)&dialogs::cbDlgBindParameters, (LPARAM)stmt);
				} else {
					int sid = aiGetSid();
					tab->aiSid = sid;

					aiResult16 = aiRequest(TEXT("execute-prompt"), query16, 0, sid);
					if (aiResult16) {
						if (_tcsstr(aiResult16, TEXT("QUERY:"))) {
							char* query8 = utils::utf16to8(aiResult16 + 6);
							rc = sqlite3_prepare_v2(_tab.db, query8, -1, &stmt, 0);
							delete [] query8;
						}
					} else {
						aiResult16 = utils::utf8to16(tab->aiNeedStop ? AI_TERMINATE_MESSAGE : AI_ERROR_MESSAGE);
					}
				}
			}

			int colCount = rc == SQLITE_OK ? sqlite3_column_count(stmt) : 0;
			DWORD tStart, tEnd;
			tStart = GetTickCount();

			HWND hResultWnd = 0;
			HWND hRowsWnd = GetDlgItem(_tab.hTabWnd, IDC_TAB_ROWS + resultNo);
			HWND hMessageWnd = GetDlgItem(_tab.hTabWnd, IDC_TAB_MESSAGE + resultNo);
			HWND hPreviewWnd = GetDlgItem(_tab.hTabWnd, IDC_TAB_PREVIEW + resultNo);
			int rowCount = 0;
			if (rc == SQLITE_OK && colCount > 0) {
				hResultWnd = hRowsWnd;
				rowCount = ListView_SetData(hResultWnd, stmt, true);
				rc = sqlite3_errcode(_tab.db);
				if (rc != SQLITE_OK && rc != SQLITE_DONE && rowCount == 0)
					hResultWnd = 0;
			}

			if (hResultWnd == 0) {
				hResultWnd = hMessageWnd;
				if (isExplain) {
					TCHAR* res16 = aiRequest(isSql ? TEXT("explain-sql-prompt") : TEXT("explain-nosql-prompt"), query16);
					if (res16) {
						int len = _tcslen(res16) + 64;
						TCHAR* result16 = new TCHAR[len] {0};
						_sntprintf(result16, len, TEXT("Explanation\n%ls"), res16);
						SetWindowText(hResultWnd, result16);
						delete [] res16;
						delete [] result16;
					} else {
						SetWindowText(hResultWnd, TEXT(AI_ERROR_MESSAGE));
					}
				} else {
					if (rc == SQLITE_OK && SQLITE_DONE == sqlite3_step(stmt)) {
						TCHAR text[64];
						if (detectDataChanges(sql8))
							_sntprintf(text, 63, TEXT("%i rows have been changed"), sqlite3_changes(_tab.db));
						else
							_sntprintf(text, 63, TEXT("Done"));
						SetWindowText(hResultWnd, text);
						isMetaChanged = isMetaChanged || detectMetaChanges(sql8);
					} else {
						if (isSql) {
							TCHAR* err16 = utils::utf8to16((char*)sqlite3_errmsg(_tab.db));

							TCHAR* exp16 = 0;
							if (prefs::get("ai-enable") && prefs::get("ai-explain-query-error"))
								exp16 = aiRequest(TEXT("explain-sql-error-prompt"), query16, err16);

							int len = _tcslen(err16) + (exp16 ? _tcslen(exp16) : 0) + 128;
							TCHAR* result16 = new TCHAR[len + 1];
							int lineNo = ListBox_GetItemData(_tab.hQueryListWnd, resultNo);

							if (exp16) {
								_sntprintf(result16, len, TEXT("Error\n%ls\nat line %i\n\nExplanation\n%ls"), err16, lineNo + 1, exp16);
							} else {
								_sntprintf(result16, len, TEXT("Error\n%ls\nat line %i"), err16, lineNo + 1);
							}
							SetWindowText(hResultWnd, result16);

							delete [] err16;
							delete [] result16;
							if (exp16)
								delete [] exp16;
						} else {
							SetWindowText(hResultWnd, aiResult16);
						}
					}
				}
			}

			if (hResultWnd == hRowsWnd) {
				SetWindowLongPtr(hMessageWnd, GWLP_USERDATA, -1);
				ShowWindow(hRowsWnd, resultNo == 0 ? SW_SHOW : SW_HIDE);
				ShowWindow(hPreviewWnd, resultNo == 0 && prefs::get("show-preview") ? SW_SHOW : SW_HIDE);
			} else {
				SetWindowLongPtr(hRowsWnd, GWLP_USERDATA, -1);
				SetWindowLongPtr(hPreviewWnd, GWLP_USERDATA, -1);

				ShowWindow(hMessageWnd, resultNo == 0 ? SW_SHOW : SW_HIDE);
			}

			EnumChildWindows(tab->hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_RESIZETAB);

			if (rc == SQLITE_OK || rc == SQLITE_DONE)
				saveQuery("history", sql8);

			if (tab->tabTooltips[resultNo] != NULL)
				delete [] tab->tabTooltips[resultNo];

			TCHAR* exp_query16 = utils::utf8to16(stmt ? sqlite3_expanded_sql(stmt) : sql8);
			tab->tabTooltips[resultNo] = utils::replaceAll(exp_query16, TEXT("\t"), TEXT("    "));

			delete [] exp_query16;
			delete [] sql8;
			if (stmt)
				sqlite3_finalize(stmt);

			if (aiResult16)
				delete [] aiResult16;

			delete [] query16;

			int tabNo = getTabNo(_tab.id);
			tab = &tabs[tabNo];

			tEnd = GetTickCount();
			_sntprintf(tab->queryElapsedTimes[resultNo], 63, TEXT(" Elapsed: %.2fs"), (tEnd - tStart) / 1000.0);

			TCHAR caption[128];
			TCHAR resname[64]{0};
			if (_tcsncmp(tab->tabTooltips[resultNo], TEXT("--"), 2) == 0)
				_sntscanf(tab->tabTooltips[resultNo], _tcslen(tab->tabTooltips[resultNo]), TEXT("-- %63[^\n]"), resname);
			if (_tcsncmp(tab->tabTooltips[resultNo], TEXT("explain query plan --"), 21) == 0)
				_sntscanf(tab->tabTooltips[resultNo], _tcslen(tab->tabTooltips[resultNo]), TEXT("explain query plan -- %63[^\n]"), resname);
			if (_tcslen(resname) == 0)
				_sntprintf(resname, 63, TEXT("Result #%i"), resultNo + 1);

			if (rowCount < 0)
				_sntprintf(caption, 127, TEXT("%ls (Cut to %i rows)"), resname, -rowCount);
			if (rowCount == 0)
				_sntprintf(caption, 127, TEXT("%ls"), resname);
			if (rowCount > 0)
				_sntprintf(caption, 127, rowCount > 0 ? TEXT("%ls (%i rows)") : TEXT("%ls"), resname, rowCount);

			TCITEM tci;
			tci.mask = TCIF_TEXT | TCIF_IMAGE;
			tci.iImage = -1;
			tci.pszText = caption;
			tci.cchTextMax = _tcslen(caption);
			TabCtrl_InsertItem(tab->hTabWnd, resultNo, &tci);

			if (queryNo == 0) {
				TabCtrl_SetCurSel(tab->hTabWnd, resultNo);
				ShowWindow(hResultWnd, SW_SHOW);
				SetWindowLongPtr(tab->hTabWnd, GWLP_USERDATA, (LONG_PTR)hResultWnd); // Also see ACTION_UPDATETAB
				if (hTabWnd == tab->hTabWnd) {
					SendMessage(hStatusWnd, SB_SETTEXT, SB_ELAPSED_TIME, (LPARAM)tab->queryElapsedTimes[resultNo]);
					SendMessage(hMainWnd, WMU_UPDATE_SB_RESULTSET, 0, 0);
				}
			}
			cbEnumChildren(hResultWnd, ACTION_RESIZETAB);

			if (prefs::get("beep-query-duration") < (int)(tEnd - tStart))
				MessageBeep(0);
		}
	}

	tab = &tabs[tabNo];

	if (hTabWnd == tab->hTabWnd) {
		RedrawWindow(tab->hTabWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
		updateTransactionState();
		updateExecuteMenu(true);
	}

	CloseHandle(tab->thread);
	tab->thread = 0;
	tab->isPlan = 0;
	tab->isBatch = 0;
	tab->aiNeedStop = 0;
	tab->aiSid = 0;
	updateTransactionState();

	EnumChildWindows(tab->hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_UPDATETAB);
	SendMessage(hMainTabWnd, WMU_TAB_SET_STYLE, getTabNo(_tab.id), 0);
	if (isMetaChanged) {
		updateTree();
		updateReferences();
	}

	SendMessage(hMainWnd, WMU_UPDATE_TOOLBAR_BUTTONS, 0, 0);

	return 1;
}

int executeEditorQuery(char flags, int vkKey) {
	bool onlyCurrent = flags & EQ_CURRENT;

	int tabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
	TEditorTab* tab = &tabs[tabNo];
	tab->isPlan = flags & EQ_PLAN;
	tab->isExplain = flags & EQ_EXPLAIN;
	tab->isBatch = flags & EQ_BATCH;

	if (tab->thread) {
		MessageBox(hMainWnd, TEXT("The tab is busy"), TEXT("Error"), MB_OK);
		return false;
	}

	CHARRANGE range;
	SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&range);

	bool isSelection = range.cpMin != range.cpMax;
	int size =  isSelection ? range.cpMax - range.cpMin + 1 : GetWindowTextLength(hEditorWnd);
	if (size <= 0)
		return 0;

	if (vkKey)
		size = MAX(size, MAX_TEXT_LENGTH);

	TCHAR* buf = new TCHAR[size + 1]{0};
	if (vkKey == 0) {
		if (!SendMessage(hEditorWnd, isSelection ? EM_GETSELTEXT : WM_GETTEXT, size + 1, (LPARAM)buf)) {
			delete [] buf;
			return 0;
		}

		if (isSelection)
			onlyCurrent = false;
	} else {
		TCHAR* replacement16 = getCurrentText(hEditorWnd);
		sqlite3_stmt *stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select replace(query, '$SUB$', trim(?4)) from shortcuts where key = ?1 and ctrl = ?2 and alt = ?3 and length(query) > 0", -1, &stmt, 0)) {
			sqlite3_bind_int(stmt, 1, vkKey);
			sqlite3_bind_int(stmt, 2, HIWORD(GetKeyState(VK_CONTROL)) > 0);
			sqlite3_bind_int(stmt, 3, HIWORD(GetKeyState(VK_MENU)) > 0);
			char* replacement8 = utils::utf16to8(replacement16);
			sqlite3_bind_text(stmt, 4, replacement8, strlen(replacement8), SQLITE_TRANSIENT);
			delete [] replacement8;

			if(SQLITE_ROW == sqlite3_step(stmt)) {
				TCHAR* query16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
				_tcscpy(buf, query16);
				delete [] query16;
			}
		} else {
			showDbError(hMainWnd);
		}
		sqlite3_finalize(stmt);
		delete [] replacement16;

		if (_tcslen(buf) == 0) {
			delete [] buf;
			return 0;
		}
	}

	// Sometimes EM_GETSELTEXT returns a text without new line as \r without \n
	for (int i = 0; isSelection && _tcschr(buf, TEXT('\r')) && (i < size + 1); i++)
		buf[i] = buf[i] == TEXT('\r') && buf[i + 1] != TEXT('\n') ? TEXT('\n') : buf[i];

	TCHAR* inja = parseInja(buf);
	if (!inja)
		return false;

	delete [] buf;
	buf = inja;
	size = _tcslen(buf);

	TCHAR bufcopy[size + 1] {0};
	_tcscpy(bufcopy, buf);

	ListBox_ResetContent(tab->hQueryListWnd);

	int start = 0;
	char quote = 0;
	int inComment = 0; // 0 - plain text, 1 - inside "--"-comment, 2 - inside "/* */"-comment
	bool isEmpty = true;

	for (int i = 0; i < size; i++) {
		if (buf[i] == TEXT('"') || buf[i] == TEXT('\''))
			quote = (quote == 0) && (buf[i] != quote) ? buf[i] : 0;

		if (inComment == 1 && buf[i] == TEXT('\n'))
			inComment = 0;

		if (inComment == 2 && buf[i] == TEXT('*') && buf[i + 1] == TEXT('/')) {
			inComment = 0;
			i++;
			continue;
		}

		if ((quote != 0 || inComment > 0) && (i != size - 1))
			continue;

		inComment = buf[i] == TEXT('-') && buf[i + 1] == TEXT('-') ? 1 : buf[i] == TEXT('/') && buf[i + 1] == TEXT('*') ? 2 : 0;
		if (inComment > 0 && (i != size - 1))
			continue;

		isEmpty = isEmpty && (_tcschr(TEXT(" \r\n\t;"), buf[i]) || inComment);

		if (buf[i] == ';' || (i == size - 1)) {
			TCHAR query16[i - start + 2]{0};
			_tcsncpy(query16, buf + start, i - start + 1);

			char* query8 = utils::utf16to8(query16);
			if (sqlite3_complete(query8) || (i == size - 1)) {
				if (query16[_tcslen(query16) - 1] == TEXT(';')) // remove last ;
					query16[_tcslen(query16) - 1] = 0;

				TCHAR* trimmed16 = utils::trim(query16);
				if(!isEmpty && _tcslen(trimmed16) > 1 && (!onlyCurrent || (onlyCurrent && i >= MIN(range.cpMin, range.cpMax)))) {
					int idx = ListBox_AddString(tab->hQueryListWnd, trimmed16);

					TCHAR ignores[] = TEXT(" \r\n\t;");
					int lineStart = start;
					while (buf[lineStart] && _tcschr(ignores, buf[lineStart]) != 0)
						lineStart++;

					int lineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, lineStart, 0) + (isSelection ? SendMessage(hEditorWnd, EM_LINEFROMCHAR, range.cpMin, 0) : 0);
					ListBox_SetItemData(tab->hQueryListWnd, idx, lineNo);

					if (onlyCurrent)
						break;
				}
				start = i + 1;
			}

			isEmpty = true; // reset
		}
	}

	int pinCount = getPinnedResultCount(tabNo);
	int queryCount = ListBox_GetCount(tab->hQueryListWnd);
	if (!tab->isBatch && (pinCount + queryCount >= MAX_RESULT_COUNT)) {
		if (IDYES != MessageBox(hMainWnd, TEXT("Max number of results (32) will be exceeded.\nDo you want to execute queries as batch?"), NULL, MB_YESNO | MB_ICONQUESTION)) {
			delete [] buf;
			return false;
		}
		tab->isBatch = true;
	}

	if (tab->isBatch) {
		ListBox_ResetContent(tab->hQueryListWnd);
		ListBox_AddString(tab->hQueryListWnd, bufcopy);
	}

	if (pinCount != 0) {
		HWND hTabWnd = tab->hTabWnd;
		int resultCount = TabCtrl_GetItemCount(hTabWnd);

		int pinNo = 0;
		for (int resultNo = 0; resultNo < resultCount; resultNo++) {
			HWND hRowsWnd = GetDlgItem(hTabWnd, IDC_TAB_ROWS + resultNo);
			HWND hMessageWnd = GetDlgItem(hTabWnd, IDC_TAB_MESSAGE + resultNo);
			HWND hPreviewWnd = GetDlgItem(hTabWnd, IDC_TAB_PREVIEW + resultNo);
			if (tab->isPinned[resultNo]) {
				if (pinNo != resultNo) {
					bool isError = GetWindowLongPtr(GetDlgItem(hTabWnd, IDC_TAB_ROWS + resultNo), GWLP_USERDATA) != resultNo;
					SetWindowLongPtr(hRowsWnd, GWLP_USERDATA, isError ? -1 : pinNo);
					SetWindowLongPtr(hMessageWnd, GWLP_USERDATA, isError ? pinNo : -1);
					SetWindowLongPtr(hPreviewWnd, GWLP_USERDATA, pinNo);

					SetWindowLongPtr(hRowsWnd, GWLP_ID, IDC_TAB_ROWS + pinNo);
					SetWindowLongPtr(hMessageWnd, GWLP_ID, IDC_TAB_MESSAGE + pinNo);
					SetWindowLongPtr(hPreviewWnd, GWLP_ID, IDC_TAB_PREVIEW + pinNo);

					_tcscpy(tab->queryElapsedTimes[pinNo], tab->queryElapsedTimes[resultNo]);

					if (tab->tabTooltips[pinNo])
						delete [] tab->tabTooltips[pinNo];
					tab->tabTooltips[pinNo] = tab->tabTooltips[resultNo];
					tab->tabTooltips[resultNo] = 0;

					tab->isPinned[pinNo] = true;
				}

				pinNo++;
			} else {
				DestroyWindow(hRowsWnd);
				DestroyWindow(hMessageWnd);
				DestroyWindow(hPreviewWnd);

				TabCtrl_DeleteItem(hTabWnd, resultNo);
			}
		}

		for (int resultNo = pinCount; resultNo < MAX_RESULT_COUNT; resultNo++) {
			tab->isPinned[resultNo] = false;
			TabCtrl_DeleteItem(hTabWnd, pinCount);
		}
	} else {
		TabCtrl_DeleteAllItems(tab->hTabWnd);
		EnumChildWindows(tab->hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_DESTROY);
	}

	if (tab->isBatch) {
		int resultNo = pinCount;
		HWND hMessageWnd = CreateWindow(WC_STATIC, TEXT("Done"), WS_CHILD | SS_LEFT, 20, 20, 100, 100, tab->hTabWnd, (HMENU)IntToPtr(IDC_TAB_MESSAGE + resultNo), GetModuleHandle(0), NULL);
		SendMessage(hMessageWnd, WM_SETFONT, (LPARAM)hFont, false);
		SetWindowLongPtr(hMessageWnd, GWLP_USERDATA, resultNo);
	} else {
		for (int queryNo = 0; queryNo < queryCount; queryNo++) {
			int resultNo = queryNo + pinCount;
			createResultControls(tab->hTabWnd, resultNo);
		}
		EnumChildWindows(tab->hTabWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETFONT);
	}

	tab->thread = (HANDLE)_beginthreadex(0, 0, &processEditorQuery, tab, 0, 0);
	delete [] buf;
	return 1;
}

void updateRecentList() {
	// 4 = New/Open, Save as, Close and separator
	// 7 = Exit, 3 * separator, Settings, Extensions, Custom functions
	int cnt = GetMenuItemCount(hDbMenu) - 7;
	for (int i = 0; i < cnt; i++)
		RemoveMenu(hDbMenu, IDM_RECENT + i, MF_BYCOMMAND);

	cnt = 0;
	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select path from recents order by time desc limit 100", -1, &stmt, 0)) {
		while(SQLITE_ROW == sqlite3_step(stmt) && cnt < prefs::get("recent-count")) {
			TCHAR* path16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
			if (utils::isFileExists(path16)) {
				Menu_InsertItem(hDbMenu, 4 + cnt, IDM_RECENT + cnt, MF_ENABLED, path16);
				cnt++;
			}
			delete [] path16;
		}
	}
	sqlite3_finalize(stmt);

	if (cnt == 0)
		Menu_InsertItem(hDbMenu, 4 + cnt, IDM_RECENT, MF_GRAYED | MF_DISABLED,  TEXT("No recents"));
};

void updateTables(const TCHAR* schema16) {
	int tblNo = 0;
	for (tblNo = 0; (tblNo < MAX_ENTITY_COUNT) && (TABLES[tblNo] != 0); tblNo++);

	char* schema8 = utils::utf16to8(schema16);
	char query8[strlen(schema8) + 512];
	sprintf(query8,
		"select iif(?1 = 'main', name, ?1 || '.' || name) "\
		"from (select name from \"%s\".sqlite_master where lower(type) in ('table', 'view') union "\
		"select 'sqlite_master' union " \
		"select 'sqlite_sequence') order by name = 'sqlite_master' and name = 'sqlite_sequence'",
		schema8);

	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
		sqlite3_bind_text(stmt, 1, schema8, -1, SQLITE_TRANSIENT);
		while((tblNo < MAX_ENTITY_COUNT) && (SQLITE_ROW == sqlite3_step(stmt))) {
			TABLES[tblNo] = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
			tblNo++;
		}
	}
	sqlite3_finalize(stmt);

	delete [] schema8;
}

void updateAttachedRecentList(bool isODBC) {
	HMENU hMenu = GetSubMenu(hDbMenu, Menu_GetItemPositionByName(hDbMenu, isODBC ? TEXT("Attach database by ODBC") : TEXT("Attach database")));
	if (!hMenu) {
		MessageBox(hMainWnd, TEXT("Attach-database menu bug"), 0, MB_OK);
		return;
	}

	int idc = isODBC ? IDM_RECENT_ATTACHED_ODBC : IDM_RECENT_ATTACHED;
	int cnt = GetMenuItemCount(hMenu);
	for (int i = 0; i < cnt - 2; i++)
		RemoveMenu(hMenu, idc + i, MF_BYCOMMAND);

	cnt = 0;
	int keepCount = 2; // Choose and separator
	sqlite3_stmt* stmt;
	char query8[256];
	sprintf(query8, "select coalesce(%s, '') from %s order by time desc limit 100", isODBC ? "alias" : "path", isODBC ? "odbc_recents" : "attached_recents");
	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, query8, -1, &stmt, 0)) {
		while(SQLITE_ROW == sqlite3_step(stmt) && cnt < prefs::get("recent-count")) {
			const char* path8 = (const char*)sqlite3_column_text(stmt, 0);
			if (!isODBC && !utils::isFileExists(path8))
				continue;

			TCHAR* path16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
			Menu_InsertItem(hMenu, keepCount + cnt, idc + cnt, MF_ENABLED, path16);
			delete [] path16;

			cnt++;
		}
	}
	sqlite3_finalize(stmt);

	if (cnt == 0)
		Menu_InsertItem(hMenu, keepCount + cnt, idc, MF_GRAYED | MF_DISABLED,  TEXT("No recents"));
};

void suggestCLIQuery(int key) {
	sqlite3_stmt *stmt;
	char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));

	int size = GetWindowTextLength(cli.hEditorWnd);
	TCHAR sql16[size + 1] = {0};
	GetWindowText(cli.hEditorWnd, sql16, size + 1);
	char *sql8 = utils::utf16to8(sql16);

	int rc = SQLITE_OK == sqlite3_prepare_v2(prefs::db,
		key == VK_UP ? "select time, query from cli where dbname = ?1 and time < coalesce(?2, time + 1) and query <> ?3 order by time desc limit 1" :
		key == VK_DOWN ? "select time, query from cli where dbname = ?1 and time > coalesce(?2, time - 1) and query <> ?3 order by time asc limit 1" :
		key == VK_TAB ? "select time, query from cli where dbname = ?1 and query like '%' || ?2 || '%'  and query <> ?3 limit 1" : "", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, sql8, strlen(sql8), SQLITE_TRANSIENT);

	if (rc && (key == VK_UP || key == VK_DOWN)) {
		int time = GetWindowLongPtr(cli.hEditorWnd, GWLP_USERDATA);
		if (time)
			sqlite3_bind_int(stmt, 2, time);
		else
			sqlite3_bind_null(stmt, 2);

		if (SQLITE_ROW == sqlite3_step(stmt)) {
			SetWindowLongPtr(cli.hEditorWnd, GWLP_USERDATA, sqlite3_column_int(stmt, 0));
			TCHAR* sql16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
			SetWindowText(cli.hEditorWnd, sql16);
			delete [] sql16;
		}
	}

	if (rc && (key == VK_TAB)) {
		int size = GetWindowTextLength(cli.hEditorWnd);
		TCHAR text16[size + 1] = {0};
		GetWindowText(cli.hEditorWnd, text16, size + 1);
		char *text8 = utils::utf16to8(text16);
		sqlite3_bind_text(stmt, 2, text8, strlen(text8), SQLITE_TRANSIENT);
		delete [] text8;

		if (SQLITE_ROW == sqlite3_step(stmt)) {
			TCHAR* sql16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
			SetWindowText(cli.hEditorWnd, sql16);

			delete [] sql16;
		}
	}

	sqlite3_finalize(stmt);
	delete [] dbname8;
	delete [] sql8;
}

void loadCLIResults(int cnt) {
	sqlite3_stmt *stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
		"with t as ("\
		"select query || char(10) || char(10) || result a from cli order by time desc limit ?2) " \
		"select group_concat(a, '') from t", -1, &stmt, 0)) {
		char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));
		sqlite3_bind_text(stmt, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 2, cnt);
		delete [] dbname8;
		if (SQLITE_ROW == sqlite3_step(stmt)) {
			TCHAR* text16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
			SetWindowText(cli.hResultWnd, text16);
			delete [] text16;
		} else {
			showDbError(hMainWnd);
		}
	}
	sqlite3_finalize(stmt);

	setEditorColor(cli.hResultWnd, RGB(0, 196, 0), true);
	RedrawWindow(cli.hResultWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
}

bool updateReferences() {
	char query8[1024] = {0};
	sqlite3_stmt *stmt, *stmt2;
	bool rc = SQLITE_OK == sqlite3_prepare_v2(db,
		"select m.tbl_name, fk.\"from\", fk.\"table\", fk.\"to\" " \
		"from sqlite_master m, pragma_foreign_key_list(m.tbl_name) fk " \
		"where m.type = 'table' ",
		-1, &stmt, 0);
	bool rc2 = SQLITE_OK == sqlite3_prepare_v2(prefs::db,
		"insert or ignore into refs (dbname, schema, tblname, colname, refname, query) " \
		"select ?1, ?2, ?3, ?4, upper(?5), ?6", -1, &stmt2, 0);

	sqlite3_exec(prefs::db, "begin;", 0, 0, 0);
	char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));
	char schema8[] = "main";
	while (rc && rc2 && (SQLITE_ROW == sqlite3_step(stmt))) {
		const char* tblname8 = (const char*)sqlite3_column_text(stmt, 0);
		const char* colname8 = (const char*)sqlite3_column_text(stmt, 1);
		const char* refname8 = (const char*)sqlite3_column_text(stmt, 2);
		sprintf(query8, "select * from \"main\".\"%s\" t where t.\"%s\" = ?1", (char*)sqlite3_column_text(stmt, 2), (char*)sqlite3_column_text(stmt, 3));
		sqlite3_bind_text(stmt2, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt2, 2, schema8, strlen(schema8), SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt2, 3, tblname8, strlen(tblname8), SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt2, 4, colname8, strlen(colname8), SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt2, 5, refname8, strlen(refname8), SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt2, 6, query8, strlen(query8), SQLITE_TRANSIENT);
		sqlite3_step(stmt2);
		sqlite3_reset(stmt2);
	}
	sqlite3_finalize(stmt);
	sqlite3_finalize(stmt2);

	sqlite3_exec(prefs::db, "commit;", 0, 0, 0);

	delete [] dbname8;
	return true;
}

void updateCustomFunction(bool unreg, sqlite3* conn) {
	/*
		Custom functions use one userDefinedFunction-function to implementation.
		Each function has own userdata that contains a pointer on a running code.
		Pointers/code strings are stored as properties of hCodesWnd.
	*/
	HWND hCodesWnd = GetDlgItem(hMainWnd, IDC_FUNCTION_CODES);

	auto update = [hCodesWnd](bool unreg, sqlite3* conn, const char* name8, const char* code8) {
		sqlite3_stmt* stmt;
		int nargs = SQLITE_OK == sqlite3_prepare_v2(db, code8, -1, &stmt, 0) ? sqlite3_bind_parameter_count(stmt) : -1;
		sqlite3_finalize(stmt);

		if (unreg) {
			char* stored8 = (char*)GetPropA(hCodesWnd, name8);
			if (stored8)
				free(stored8);

			sqlite3_create_function(conn, name8, nargs, SQLITE_UTF8, 0, 0, 0, 0);
		} else {
			char* stored8 = (char*)GetPropA(hCodesWnd, name8);
			if (!stored8)
				stored8 = strdup(code8);
			sqlite3_create_function(conn, name8, nargs, SQLITE_UTF8, (void*)stored8, dbutils::userDefinedFunction, 0, 0);
		}
	};

	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select name, code from functions", -1, &stmt, 0)) {
		while (SQLITE_ROW == sqlite3_step(stmt)) {
			const char* name8 = (const char*)sqlite3_column_text(stmt, 0);
			const char* code8 = (const char*)sqlite3_column_text(stmt, 1);

			if (conn == 0) {
				if (db)
					update(unreg, db, name8, code8);

				for (int i = 0; i < MAX_TAB_COUNT; i++) {
					TEditorTab* tab = &tabs[i];
					if (tab->id && tab->db)
						update(unreg, tab->db, name8, code8);
				}

				if (cli.db)
					update(unreg, cli.db, name8, code8);
			} else {
				update(unreg, conn, name8, code8);
			}
		}
	}
	sqlite3_finalize(stmt);
}

bool attachDb(sqlite3** _db, const char* path8, const char* alias8) {
	auto runQuery = [](sqlite3* conn, const char* query8, bool skip = false) {
		bool rc = !skip ? SQLITE_OK == sqlite3_exec(conn, query8, 0, 0, 0) : true;
		if (!rc || conn != db)
			return rc;

		for (int tabNo = 0; tabNo < MAX_TAB_COUNT; tabNo++)	{
			if (tabs[tabNo].id && tabs[tabNo].db)
				sqlite3_exec(tabs[tabNo].db, query8, 0, 0, 0);
		}

		if (cli.db)
			sqlite3_exec(cli.db, query8, 0, 0 , 0);

		return true;
	};

	char attach8[2 * strlen(path8) + 2048]{0};
	bool rc = dialogs::getAttachString(path8, alias8, 0, attach8) && runQuery(*_db, attach8);
	if (*_db != db)
		return rc;

	// Second attempt
	if (!rc && isCipherSupport) {
		if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ENCRYPTION), hMainWnd, (DLGPROC)dialogs::cbDlgEncryption, (LPARAM)path8)) {
			rc = dialogs::getAttachString(path8, alias8, 0, attach8) && runQuery(db, attach8, true);
		}
	}

	if (!rc) {
		showDbError(hMainWnd);
		return false;
	}

	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "replace into attached_recents (path, time) values (?1, ?2)", -1, &stmt, 0)) {
		sqlite3_bind_text(stmt, 1, path8, -1,  SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 2, std::time(0));
		sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	TCHAR* alias16;
	if (alias8) {
		alias16 = utils::utf8to16(alias8);
	} else {
		char* alias8 = utils::getFileName(path8, true);
		alias16 = utils::utf8to16(alias8);
		delete [] alias8;
	}
	updateTables(alias16);
	delete [] alias16;

	return true;
}

bool attachOdbcDb(sqlite3* conn, const TCHAR* alias) {
	bool res = true;
	char* alias8 = utils::utf16to8(alias);

	char attach8[strlen(alias8) + 256]{0};
	sprintf(attach8, "attach database 'file:%s?mode=memory&cache=shared' as '%s'", alias8, alias8);

	if (conn != db) {
		delete [] alias8;
		sqlite3_exec(conn, attach8, 0, 0, 0);
		return true;
	}

	char connectionString8[2048] = {0};
	sqlite3_stmt* stmt;

	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select iif(driver is not null, 'Driver={' || driver || '};', '') || coalesce(options, '') from odbc_recents where alias = ?1", -1, &stmt, 0)) {
		sqlite3_bind_text(stmt, 1, alias8, -1,  SQLITE_TRANSIENT);
		if (SQLITE_ROW == sqlite3_step(stmt))
			snprintf(connectionString8, 2048, "%s", (const char*)sqlite3_column_text(stmt, 0));
	}
	sqlite3_finalize(stmt);

	sqlite3_exec(db, "drop table temp.\"$TABLES\"", 0, 0, 0);
	if (SQLITE_OK == sqlite3_prepare_v2(db, "select odbc_read(?1, '$TABLES', 'temp.\"$TABLES\"')", -1, &stmt, 0)) {
		sqlite3_bind_text(stmt, 1, connectionString8, -1,  SQLITE_TRANSIENT);
		sqlite3_step(stmt);
		res = SQLITE_OK == sqlite3_exec(db, "select * from temp.\"$TABLES\"", 0, 0, 0);
	}
	sqlite3_finalize(stmt);

	if (!res || strlen(connectionString8) == 0) {
		delete [] alias8;
		return false;
	}

	sqlite3_exec(db, attach8, 0, 0, 0);
	for (int tabNo = 0; tabNo < MAX_TAB_COUNT; tabNo++)	{
		if (tabs[tabNo].id && tabs[tabNo].db)
			sqlite3_exec(tabs[tabNo].db, attach8, 0, 0, 0);
	}

	if (cli.db)
		sqlite3_exec(cli.db, attach8, 0, 0, 0);

	if (SQLITE_OK == sqlite3_prepare_v2(db, "select 'create virtual table ' || quote(?2) || '.' || quote(table_name) ||' using odbc_table(' || quote(?1) || ', ' || quote(table_name) || ')' from temp.\"$TABLES\"", -1, &stmt, 0)) {
		sqlite3_bind_text(stmt, 1, connectionString8, -1,  SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, alias8, -1,  SQLITE_TRANSIENT);

		while (SQLITE_ROW == sqlite3_step(stmt))
			sqlite3_exec(db, (const char*)sqlite3_column_text(stmt, 0), 0, 0, 0);
	}
	sqlite3_finalize(stmt);

	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "update odbc_recents set time = ?2 where alias = ?1", -1, &stmt, 0)) {
		sqlite3_bind_text(stmt, 1, alias8, -1,  SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 2, std::time(0));
		sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	delete [] alias8;

	updateTables(alias);

	return true;
}

bool openConnection(sqlite3** _db, const char* path8, bool isReadOnly) {
	int mode = (db == *_db && isReadOnly) || (db != *_db && sqlite3_db_readonly(db, 0)) ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
	SendMessage(hMainWnd, WMU_SET_ECRYPT_FLAG, 0, 0);

	sqlite3_open_v2(path8 && strlen(path8) > 0 ? path8 : "file::memory:?cache=shared", _db, mode | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_URI, NULL);
	sqlite3_enable_load_extension(*_db, true);

	loadExtensions(db, *_db);

	// load cipher
	if (SQLITE_OK != sqlite3_exec(*_db, "select 1 from sqlite_master limit 1", 0, 0, 0)) {
		bool rc = false;
		if (isCipherSupport) {
			sqlite3_stmt *stmt;
			if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
				"select param, value " \
				"from (select param, value, no from main.encryption where dbpath = ?1 union select param, value, no from temp.encryption where dbpath = ?1) " \
				"order by no", -1, &stmt, 0)) {
				sqlite3_bind_text(stmt, 1, path8, strlen(path8), SQLITE_TRANSIENT);
				while (SQLITE_ROW == sqlite3_step(stmt)) {
					char query8[1024];
					const char* param8 = (const char*)sqlite3_column_text(stmt, 0);
					const char* value8 = (const char*)sqlite3_column_text(stmt, 1);
					sprintf(query8, utils::isNumber(value8, 0) ? "pragma %s = %s" : "pragma %s = '%s'", param8, value8);
					sqlite3_exec(*_db, query8, 0, 0, 0);

					//if (strcmp(param8, "key") == 0)
					//	sprintf(cipherKey, value8);
				}
				rc = SQLITE_OK == sqlite3_exec(db, "select 1 from sqlite_master limit 1", 0, 0, 0);
			}
			sqlite3_finalize(stmt);

			if (!rc)
				rc = DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ENCRYPTION), hMainWnd, (DLGPROC)&dialogs::cbDlgEncryption, (LPARAM) *_db);

			if (rc)
				SendMessage(hMainWnd, WMU_SET_ECRYPT_FLAG, 0, 1);
		}

		if (!rc) {
			MessageBox(hMainWnd, TEXT("Unable to open a new connection to the database"), TEXT("Error"), MB_OK | MB_ICONSTOP);
			sqlite3_close_v2(*_db);
			*_db = 0;
			return false;
		}

		loadExtensions(db, *_db);
	}

	// attach databases
	if (*_db != db) {
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db, "select name, file from pragma_database_list t where seq > 0 and name <> 'temp' and name <> 'temp2' and name is not null", -1, &stmt, 0)) {
			while (SQLITE_ROW == sqlite3_step(stmt)) {
				const char* name8 = (const char*)sqlite3_column_text(stmt, 0);
				const char* file8 = (const char*)sqlite3_column_text(stmt, 1);
				if (strlen(file8)) {
					attachDb(_db, file8);
				} else {
					TCHAR* alias16 = utils::utf8to16(name8);
					attachOdbcDb(*_db, alias16);
					delete [] alias16;
				}
			}
		}
		sqlite3_finalize(stmt);
	}

	sqlite3_exec(*_db, "attach database 'file::memory:?cache=shared' as temp2", NULL, NULL, NULL);

	applyDbPrefs(*_db);

	char* startup8 = prefs::get("startup", "");
	if (strlen(startup8) > 0)
		sqlite3_exec(*_db, startup8, 0, 0, 0);
	delete [] startup8;

	updateCustomFunction(false, *_db);

	return true;
}


bool openDb(const TCHAR* path, bool isReadOnly, bool isReloadExtensions) {
	if (!closeDb())
		return false;

	ULONG attrs = GetFileAttributes(path);
	bool isOpenReadOnly = ((attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_READONLY)) || isReadOnly;

	SetWindowText(hMainWnd, TEXT("Loading database..."));
	char* path8 = utils::utf16to8(path);
	if (!openConnection(&db, path8, isOpenReadOnly)) {
		delete [] path8;
		SetWindowText(hMainWnd, TEXT("No database selected"));
		return false;
	}

	updateReferences();

	updateTables(TEXT("main"));

	if (!IsWindow(hMainWnd)) {
		delete [] path8;
		return true;
	}
	SetWindowText(hMainWnd, path);
	SetWindowText(hSchemaWnd, TEXT("main"));

	TCHAR prev[MAX_PATH];
	GetMenuString(hDbMenu, 4, prev, MAX_PATH, MF_BYPOSITION); // Open, Save as, Close, Separator

	if (_tcscmp(prev, path) != 0) {
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "replace into recents (path, time) values (?1, ?2)", -1, &stmt, 0)) {
			sqlite3_bind_text(stmt, 1, path8, strlen(path8),  SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, std::time(0));
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);

		updateRecentList();
	}
	delete [] path8;

	int len = _tcslen(path) + 30;
	TCHAR title[len + 1]{0};
	_sntprintf(title, len, TEXT("%ls%ls"), path, isOpenReadOnly ? TEXT(" (read only)") : TEXT(""));
	SetWindowText(hMainWnd, title);
	SetProp(hMainWnd, TEXT("READONLYDB"), IntToPtr(isOpenReadOnly));

	updateTree();
	EnableWindow(hTreeWnd, true);
	updateTransactionState();
	enableMainMenu();

	SendMessage(hMainWnd, WMU_UPDATE_EXTENSIONS_UI, 0, 0);

	reloadMeta(PRAGMAS, "select name from pragma_pragma_list() order by 1");
	reloadMeta(FUNCTIONS, "select distinct name from pragma_function_list() order by 1");
	reloadMeta(MODULES, "select name from pragma_module_list where name not like 'pragma_%' order by 1");

	// restore cli
	if(prefs::get("restore-editor"))
		loadCLIResults(30);

	processHighlight(hEditorWnd, true, true, true);

	startHttpServer();

	// define auxilary function
	sqlite3_create_function(db, "rownum", 1, SQLITE_UTF8, 0, dbutils::rownum, 0, 0);
	sqlite3_create_function(db, "md5", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, 0, dbutils::md5, 0, 0);
	sqlite3_create_function(db, "tosize", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, 0, dbutils::tosize, 0, 0);
	sqlite3_create_function(db, "dq", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, 0, dbutils::double_quote, 0, 0);

	return true;
}

bool closeConnections () {
	for (int i = 0; i < MAX_TAB_COUNT; i++) {
		TEditorTab* tab = &tabs[i];
		if (tab->id && tab->db) {
			sqlite3_exec(tab->db, "detach database temp2", 0, 0, 0);
			if (SQLITE_OK != sqlite3_close(tab->db))
				showDbError(hMainWnd);
			tab->db = 0;
		}
	}

	if (cli.db) {
		sqlite3_exec(cli.db, "detach database temp2", 0, 0, 0);
		if (SQLITE_OK != sqlite3_close(cli.db))
			showDbError(hMainWnd);
		cli.db = 0;
	}

	return true;
}

bool closeDb(bool isReloadExtensions) {
	if (!db)
		return true;

	if (isDbBusy())
		return false;

	int nDlgOpen = 0;
	for (int i = 0; i < MAX_DIALOG_COUNT; i++)
		nDlgOpen += hDialogs[i] && IsWindow(hDialogs[i]);

	if (!isReloadExtensions && nDlgOpen > 0) {
		MessageBox(hMainWnd, nDlgOpen == 1 ?
			TEXT("There is an open dialog. You should close it.") :
			TEXT("There are open dialogs. You should close them."),
			TEXT("Info"), MB_OK | MB_ICONWARNING);

		for (int i = 0; i < MAX_DIALOG_COUNT; i++) {
			if (hDialogs[i] && IsWindow(hDialogs[i])) {
				ShowWindow(hDialogs[i], SW_NORMAL);
				SetForegroundWindow(hDialogs[i]);
			}
		}

		return false;
	}

	closeConnections();
	stopHttpServer();

	if (db && prefs::get("retain-passphrase") == 0) {
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "delete from temp.encryption where dbpath = ?1", -1, &stmt, 0)) {
			const char* dbpath = sqlite3_db_filename(db, 0);
			sqlite3_bind_text(stmt, 1, dbpath, strlen(dbpath), SQLITE_TRANSIENT);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	sqlite3_exec(db, "detach database temp2", 0, 0, 0);
	if(SQLITE_OK != sqlite3_close(db))
		showDbError(hMainWnd);
	updateCustomFunction(true);
	db = 0;

	SetWindowText(hMainWnd, TEXT("No database selected"));
	SetWindowText(hSchemaWnd, TEXT(""));
	EnableWindow(hTreeWnd, false);
	TreeView_DeleteAllItems(hTreeWnd);
	SendMessage(hMainWnd, WMU_SET_ECRYPT_FLAG, 0, 0);
	SendMessage(hStatusWnd, SB_SETTEXT, SB_EXTENSIONS, (LPARAM)0);

	for (int i = 0; (i < MAX_ENTITY_COUNT) && (TABLES[i] != 0); i++) {
		delete [] TABLES[i];
		TABLES[i] = 0;
	}

	disableMainMenu();
	updateExecuteMenu(false);

	return true;
}

bool isDbBusy() {
	bool isRunning = false;
	for (int i = 0; i < MAX_TAB_COUNT; i++)
		isRunning = tabs[i].thread != 0 || isRunning;

	isRunning = cli.thread != 0 || isRunning;

	if (isRunning)
		MessageBox(hMainWnd, TEXT("There are executing queries. You should stop them."), TEXT("Info"), MB_OK | MB_ICONWARNING);

	return isRunning;
}

int enableDbObject(const char* name8, int type) {
	bool rc = true;
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(prefs::db, "select sql, rowid from disabled where name = ?1 and type = ?2 and dbpath = ?3", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, TYPES8[type], strlen(TYPES8[type]), SQLITE_TRANSIENT);
	const char* dbpath = sqlite3_db_filename(db, 0);
	sqlite3_bind_text(stmt, 3, dbpath, strlen(dbpath), SQLITE_TRANSIENT);

	if (SQLITE_ROW == sqlite3_step(stmt) && SQLITE_OK == sqlite3_exec(db, (char*)sqlite3_column_text(stmt, 0), 0, 0, 0)) {
		char query8[128];
		sprintf(query8, "delete from disabled where rowid = %i", sqlite3_column_int(stmt, 1));
		sqlite3_exec(prefs::db, query8, 0, 0, 0);
	} else {
		showDbError(hMainWnd);
		rc = false;
	}
	sqlite3_finalize(stmt);

	return rc;
}

int disableDbObject(const char* name8, int type) {
	bool rc = true;

	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "select sql from sqlite_master where type = ?2 and name = coalesce(?1, name)", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, TYPES8[type], strlen(TYPES8[type]), SQLITE_TRANSIENT);
	rc = SQLITE_ROW == sqlite3_step(stmt);
	char* sql = 0;
	if (rc) {
		sql = new char[sqlite3_column_bytes(stmt, 0) + 1]{0};
		strcpy(sql, (const char*)sqlite3_column_text(stmt, 0));
	}
	sqlite3_finalize(stmt);

	if (!rc)
		return false;

	sqlite3_prepare_v2(prefs::db, "replace into disabled (name, type, dbpath, sql) select ?1, ?2, ?3, ?4", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, TYPES8[type], strlen(TYPES8[type]), SQLITE_TRANSIENT);
	const char* dbpath = sqlite3_db_filename(db, 0);
	sqlite3_bind_text(stmt, 3, dbpath, strlen(dbpath), SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 4, sql, strlen(sql), SQLITE_TRANSIENT);

	char query8[128];
	sprintf(query8, "drop %s \"%s\"", TYPES8[type], name8);
	if (SQLITE_DONE != sqlite3_step(stmt) || SQLITE_OK != sqlite3_exec(db, query8, 0, 0, 0)) {
		showDbError(hMainWnd);
		rc = false;
	}
	sqlite3_finalize(stmt);
	delete [] sql;
	return rc;
}

void applyDbPrefs(sqlite3* _db) {
	auto applyPragmas = [](sqlite3* db) {
		sqlite3_exec(db, prefs::get("use-foreign-keys") ? "pragma foreign_keys = 1" : "pragma foreign_keys = 0", 0, 0, 0);
		sqlite3_exec(db, prefs::get("use-legacy-rename") ? "pragma legacy_alter_table = 1" : "pragma legacy_alter_table = 0", 0, 0, 0);
	};

	if (_db)
		return applyPragmas (_db);

	applyPragmas(db);
	for (int i = 0; i < MAX_TAB_COUNT; i++) {
		TEditorTab* tab = &tabs[i];
		if (tab->id && tab->db)
			applyPragmas(tab->db);
	}
}

bool isObjectPinned(const TCHAR* name16) {
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(prefs::db, "select 1 from pinned where dbname = ?1 and name = ?2", -1, &stmt, 0);

	char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));
	char* name8 = utils::utf16to8(name16);
	sqlite3_bind_text(stmt, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, name8, strlen(name8), SQLITE_TRANSIENT);
	delete [] dbname8;
	delete [] name8;

	bool isPinned = SQLITE_ROW == sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return isPinned;
}

void setEditorFont(HWND hWnd) {
	if (!hWnd)
		return;

	SendMessage(hWnd, WM_SETFONT, (LPARAM)hFont, 0);
	setEditorPadding(hWnd);
}

void setEditorPadding(HWND hWnd) {
	HWND hParentWnd = GetAncestor(hWnd, GA_ROOT);
	POINTFLOAT s = hParentWnd == hMainWnd ? utils::getWndScale(hParentWnd) : utils::getDlgScale(hParentWnd);

	RECT rc;
	GetClientRect(hWnd, &rc);
	InflateRect(&rc, -5 * s.x, -5 * s.y);

	 // line numbering
	for (int i = 0; i < MAX_TAB_COUNT; i++) {
		if (tabs[i].hEditorWnd != hWnd)
			continue;

		rc.left += editorNumWidth;
	}

	SendMessage(hWnd, EM_SETRECT, 0, (LPARAM)&rc);
}

void setEditorColor(HWND hWnd, COLORREF color, bool noEffects) {
	CHARFORMAT cf = {0};
	cf.cbSize = sizeof(cf);
	cf.dwMask = noEffects ? CFM_COLOR | CFM_EFFECTS : CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = color;
	SendMessage(hWnd, EM_SETCHARFORMAT, SCF_ALL | SCF_DEFAULT, (LPARAM)&cf);
}

void updateExecuteMenu(bool isEnable) {
	int state = isEnable ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED;
	HMENU hMenu = GetSubMenu(hMainMenu, 1);
	EnableMenuItem(hMenu, IDM_PLAN, state);
	EnableMenuItem(hMenu, IDM_AI_EXPLAIN, state);
	EnableMenuItem(hMenu, IDM_EXECUTE, state);
	EnableMenuItem(hMenu, IDM_EXECUTE_BATCH, state);

	state = isEnable ? TBSTATE_ENABLED : TBSTATE_INDETERMINATE;
	Toolbar_SetButtonState(hToolbarWnd, IDM_PLAN, state);
	Toolbar_SetButtonState(hToolbarWnd, IDM_AI_EXPLAIN, state);
	Toolbar_SetButtonState(hToolbarWnd, IDM_EXECUTE, state);
}

void disableMainMenu() {
	HMENU hMenu;
	hMenu = GetSubMenu(hMainMenu, 0);
	EnableMenuItem(hMenu, IDM_CLOSE, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_SAVE_AS, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu, Menu_GetItemPositionByName(hMenu, TEXT("Attach database")), MF_BYPOSITION | MF_GRAYED);
	EnableMenuItem(hMenu, Menu_GetItemPositionByName(hMenu, TEXT("Attach database by ODBC")), MF_BYPOSITION | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_ATTACH, MF_BYPOSITION | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_REFERENCES, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_ENCRYPTION, MF_BYCOMMAND | MF_GRAYED);

	hMenu = GetSubMenu(hMainMenu, 1);
	EnableMenuItem(hMenu, IDM_PLAN, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_AI_EXPLAIN, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_EXECUTE, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_EXECUTE_BATCH, MF_BYCOMMAND | MF_GRAYED);

	hMenu = GetSubMenu(hMainMenu, 2);
	for (int i = 0; i < GetMenuItemCount(hMenu); i++)
		EnableMenuItem(hMenu, i, MF_BYPOSITION | MF_GRAYED);

	Toolbar_SetButtonState(hToolbarWnd, IDM_CLOSE, TBSTATE_INDETERMINATE);
	SendMessage(hMainWnd, WMU_UPDATE_TOOLBAR_BUTTONS, 0, 0);
}

void enableMainMenu() {
	HMENU hMenu;

	// Database
	hMenu = GetSubMenu(hMainMenu, 0);
	EnableMenuItem(hMenu, IDM_CLOSE, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_SAVE_AS, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, Menu_GetItemPositionByName(hMenu, TEXT("Attach database")), MF_BYPOSITION | MF_ENABLED);
	EnableMenuItem(hMenu, Menu_GetItemPositionByName(hMenu, TEXT("Attach database by ODBC")), MF_BYPOSITION | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_REFERENCES, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_ENCRYPTION, MF_BYCOMMAND | MF_ENABLED);

	// Query
	hMenu = GetSubMenu(hMainMenu, 1);
	EnableMenuItem(hMenu, IDM_PLAN, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_AI_EXPLAIN, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_EXECUTE, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_EXECUTE_BATCH, MF_BYCOMMAND | MF_ENABLED);

	// Tools
	hMenu = GetSubMenu(hMainMenu, 2);
	for (int i = 0; i < GetMenuItemCount(hMenu); i++)
		EnableMenuItem(hMenu, i, MF_BYPOSITION | MF_ENABLED);

	Toolbar_SetButtonState(hToolbarWnd, IDM_CLOSE, TBSTATE_ENABLED);
	SendMessage(hMainWnd, WMU_UPDATE_TOOLBAR_BUTTONS, 0, 0);
}

int pluginSorter (const void* a, const void* b) {
	return (*(TPlugin*)b).priority - (*(TPlugin*)a).priority;
}

// 0-index as None-plugin
void reloadPlugins(int type) {
	for (int i = 1; i < MAX_PLUGIN_COUNT; i++) {
		if (plugins[i].hModule && (type == 0 || plugins[i].type == type)) {
			FreeLibrary(plugins[i].hModule);
			plugins[i] = {0};
		}
	}

	auto getNextPluginNo = [] () {
		int res = 1;
		while (res <= MAX_PLUGIN_COUNT && plugins[res].hModule)
			res++;
		return res;
	};

	int nextPluginNo = getNextPluginNo();
	TCHAR pluginsDir16[MAX_PATH + 1]{0};
	_sntprintf(pluginsDir16, MAX_PATH, TEXT("%ls" PLUGIN_DIRECTORY), APP_PATH);

	for (int pluginType = 1; pluginType < 3; pluginType++) {
		if (type && pluginType != type)
				continue;

		const TCHAR* pluginExt16 = ADDON_EXTS16[pluginType];

		TCHAR searchPath16[MAX_PATH];
		_sntprintf(searchPath16, MAX_PATH, TEXT("%ls%ls\\*.*"), pluginsDir16, pluginExt16);

		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(searchPath16, &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
			continue;

		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare_v2(prefs::db, "select 1 from main.addons where name = ?1 and type = ?2 and enable = 0", -1, &stmt, 0)) {
			sqlite3_finalize(stmt);
			continue;
		}

		do {
			TCHAR filepath16[MAX_PATH + 1]{0};
			_sntprintf(filepath16, MAX_PATH, TEXT("%ls%ls\\%ls"), pluginsDir16, pluginExt16, ffd.cFileName);

			TCHAR ext16[32]{0};
			TCHAR name16[32]{0};
			_tsplitpath(filepath16, NULL, NULL, name16, ext16);

			if (_tcscmp(ext16 + 1 /* skip first dot .ext */, pluginExt16))
				continue;

			char* name8 = utils::utf16to8(name16);
			sqlite3_reset(stmt);
			sqlite3_bind_text(stmt, 1, name8, strlen(name8),  SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, pluginType);
			bool isEnable = SQLITE_ROW != sqlite3_step(stmt);
			delete [] name8;

			if (isEnable && nextPluginNo < MAX_PLUGIN_COUNT) {
				HMODULE hModule = LoadLibrary(filepath16);
				if (hModule) {

					pluginView view = (pluginView)GetProcAddress(hModule, "view");
					pluginClose close = (pluginClose)GetProcAddress(hModule, "close");
					pluginGetPriority getPriority = (pluginGetPriority)GetProcAddress(hModule, "getPriority");

					pluginActivate activate = (pluginActivate)GetProcAddress(hModule, "activate");
					pluginDeactivate deactivate = (pluginDeactivate)GetProcAddress(hModule, "deactivate");
					pluginSetText setText = (pluginSetText)GetProcAddress(hModule, "setText");
					pluginSetColor setColor = (pluginSetColor)GetProcAddress(hModule, "setColor");
					pluginRender render = (pluginRender)GetProcAddress(hModule, "render");

					if ((pluginType == ADDON_VALUE_VIEWER && view && close) ||
						(pluginType == ADDON_COLUMN_MODIFIER && (setText || render || setColor))) {
						plugins[nextPluginNo] = {
							hModule,
							view,
							close,
							activate,
							deactivate,
							setText,
							setColor,
							render,
							pluginType,
							getPriority != NULL ? getPriority() : 0
						};
						_tsplitpath(filepath16, NULL, NULL, plugins[nextPluginNo].name, NULL);

						nextPluginNo = getNextPluginNo();
					} else {
						FreeLibrary(hModule);
					}
				}
			}
		} while  (FindNextFile(hFind, &ffd) && nextPluginNo < MAX_PLUGIN_COUNT);

		FindClose(hFind);
		sqlite3_finalize(stmt);
	}

	qsort(plugins, MAX_PLUGIN_COUNT, sizeof(TPlugin), pluginSorter);

	if (type == 0 || type == ADDON_COLUMN_MODIFIER) {
		int cnt = GetMenuItemCount(hModifierMenu);
		for (int itemNo = 0; itemNo < cnt; itemNo++)
			DeleteMenu(hModifierMenu, 0, MF_BYPOSITION);

		Menu_InsertItem(hModifierMenu, 0, IDM_RESULT_MODIFIER + 0, MF_STRING, TEXT("None"));

		int itemNo = 1;
		bool isSeparator = false;
		for (int pluginNo = 1; pluginNo < MAX_PLUGIN_COUNT; pluginNo++) {
			if (plugins[pluginNo].hModule && plugins[pluginNo].type == ADDON_COLUMN_MODIFIER) {
				if (!isSeparator) {
					AppendMenu(hModifierMenu, MF_STRING, 0, 0);
					isSeparator = true;
				}
				AppendMenu(hModifierMenu, MF_STRING, IDM_RESULT_MODIFIER + itemNo, plugins[pluginNo].name);
				Menu_SetItemData(hModifierMenu, IDM_RESULT_MODIFIER + itemNo, pluginNo);
				itemNo++;
			}
		}

		isSeparator = false;
		sqlite3_stmt* stmt;

		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select id, name, code from functions f order by name", -1, &stmt, 0)) {
			while (SQLITE_ROW == sqlite3_step(stmt)) {
				sqlite3_stmt* stmt2;
				int nargs = SQLITE_OK == sqlite3_prepare_v2(prefs::db, (const char*)sqlite3_column_text(stmt, 2), -1, &stmt2, 0) ? sqlite3_bind_parameter_count(stmt2) : -1;
				sqlite3_finalize(stmt2);

				if (nargs != 1)
					continue;

				if (!isSeparator) {
					AppendMenu(hModifierMenu, MF_STRING, 0, 0);
					isSeparator = true;
				}
				TCHAR* name16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
				AppendMenu(hModifierMenu, MF_STRING, IDM_RESULT_MODIFIER + itemNo, name16);
				Menu_SetItemData(hModifierMenu, IDM_RESULT_MODIFIER + itemNo, -sqlite3_column_int(stmt, 0));
				delete [] name16;
				itemNo++;
			}
		}
		sqlite3_finalize(stmt);

		if (itemNo == 1)
			Menu_SetItemText(hModifierMenu, IDM_RESULT_MODIFIER + 0, TEXT("No plugin installed"));
	}
}

void updatePlugins(int type) {
	TCHAR tmpPath16[MAX_PATH + 1] = {0};
	GetTempPath(MAX_PATH, tmpPath16);
	_tcscat(tmpPath16, TEXT("sqlite-gui\\update\\*.*\0"));

	TCHAR path16[MAX_PATH + 1] = {0};
	if (type == ADDON_SQLITE_EXTENSION)
		_sntprintf(path16, MAX_PATH, TEXT("%ls" EXTENSION_DIRECTORY), APP_PATH);
	else
		_sntprintf(path16, MAX_PATH, TEXT("%ls" PLUGIN_DIRECTORY "%ls\\"), APP_PATH, ADDON_EXTS16[type]);

	SHFILEOPSTRUCT fo = {0};
	fo.wFunc = FO_MOVE;
	fo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_FILESONLY;
	fo.pFrom = tmpPath16;
	fo.pTo = path16;
	SHFileOperation(&fo);
}

int Toolbar_SetButtonState(HWND hToolbar, int id, byte state, LPARAM lParam) {
	TBBUTTONINFO tbi{0};
	tbi.cbSize = sizeof(TBBUTTONINFO);
	tbi.dwMask = TBIF_STATE | TBIF_LPARAM;
	tbi.fsState = state;
	tbi.lParam = lParam;

	return SendMessage(hToolbar, TB_SETBUTTONINFO, id, (LPARAM)&tbi);
}

DWORD_PTR Toolbar_GetButtonData(HWND hToolbar, int id) {
	TBBUTTONINFO tbi{0};
	tbi.cbSize = sizeof(TBBUTTONINFO);
	tbi.dwMask = TBIF_LPARAM;

	return SUCCEEDED(SendMessage(hToolbar, TB_GETBUTTONINFO, id, (LPARAM)&tbi)) ? tbi.lParam : 0;
}

bool CALLBACK cbEnumChildren (HWND hWnd, LPARAM action) {
	if (hWnd == NULL)
		return true;

	if (LOWORD(action) == ACTION_DESTROY) {
		int exceptId = HIWORD(action);
		if (exceptId && GetWindowLongPtr(hWnd, GWL_ID) == exceptId)
			return true;
		DestroyWindow(hWnd);
	}

	if (action == ACTION_SETFONT) {
		TCHAR wndClass[256];
		GetClassName(hWnd, wndClass, 256);

		if (_tcscmp(TEXT("RICHEDIT50W"), wndClass) == 0)
			setEditorFont(hWnd);
		else
			SendMessage(hWnd, WM_SETFONT, (LPARAM)hFont, true);
	}

	if (action == ACTION_SETMENUFONT) {
		SendMessage(hWnd, WM_SETFONT, (LPARAM)hMenuFont, true);
	}

	if (action == ACTION_SETPARENTFONT) {
		HWND hParentWnd = GetParent(hWnd);
		if (hParentWnd == HWND_DESKTOP)
			return true;

		HFONT hFont = (HFONT)SendMessage(hParentWnd, WM_GETFONT, 0, 0);
		SendMessage(hWnd, WM_SETFONT, (LPARAM)hFont, true);
	}

	if (action == ACTION_RESIZETAB && GetParent(hWnd) == hTabWnd) {

		RECT rect, rect2;
		GetClientRect(hTabWnd, &rect);
		TabCtrl_GetItemRect(hTabWnd, 0, &rect2);

		int id = (int)(GetDlgCtrlID(hWnd) / 100) * 100;
		bool showPreview = prefs::get("show-preview");
		int w = showPreview && id != IDC_TAB_MESSAGE ? prefs::get("preview-width") : -5;

		LONG flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_DEFERERASE;
		if (id == IDC_TAB_MESSAGE || id == IDC_TAB_ROWS)
			SetWindowPos(hWnd, 0, rect.left + 5, rect.top + rect2.bottom + 5, rect.right - rect.left - 10 - 5 - w, rect.bottom - rect.top - rect2.bottom - 7, flags);

		if (id == IDC_TAB_PREVIEW && showPreview)
			SetWindowPos(hWnd, 0, rect.right - rect.left - w - 5, rect.top + rect2.bottom + 5, w, rect.bottom - rect.top - rect2.bottom - 7, flags);
	}

	if (action == ACTION_UPDATETAB && GetParent(hWnd) == hTabWnd) {
		int resultNo = TabCtrl_GetCurSel(hTabWnd);
		if (GetWindowLongPtr(hWnd, GWLP_USERDATA) == resultNo) {
			int id = (int)(GetDlgCtrlID(hWnd) / 100) * 100;
			if (id == IDC_TAB_ROWS) {
				ShowWindow(hWnd, SW_SHOW);

				int tabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
				SendMessage(hStatusWnd, SB_SETTEXT, SB_ELAPSED_TIME, (LPARAM)tabs[tabNo].queryElapsedTimes[resultNo]);
				SendMessage(hMainWnd, WMU_UPDATE_SB_RESULTSET, 0, 0);
			}

			if (id == IDC_TAB_PREVIEW)
				ShowWindow(hWnd, prefs::get("show-preview") ? SW_SHOW : SW_HIDE);

			if (id == IDC_TAB_MESSAGE)
				ShowWindow(hWnd, SW_SHOW);
		} else {
			ShowWindow(hWnd, SW_HIDE);
		}
	}

	if (action == ACTION_SET_THEME) {
		TCHAR wndClass[256];
		GetClassName(hWnd, wndClass, 256);

		if (_tcscmp(WC_TABCONTROL, wndClass) == 0) {
			EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SET_THEME);
		}

		if (_tcscmp(WC_LISTVIEW, wndClass) == 0) {
			SendMessage(hWnd, WM_SETFONT, (LPARAM)hFont, true);
			EnumChildWindows(ListView_GetHeader(hWnd), (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETPARENTFONT);

			if (prefs::get("show-filters"))
				SendMessage(hWnd, WMU_UPDATE_FILTER_SIZE, 0, 0);
		}
	}

	if (action == ACTION_RESET_MODIFIERS && (int)(GetDlgCtrlID(hWnd)) == IDC_TAB_ROWS && GetWindowLong(hWnd, GWL_STYLE) & LVS_OWNERDATA) {
		SendMessage(hWnd, WMU_RESET_MODIFIER, -1, 0);
	}

	return true;
}

/*
	Recommended height of any edit control is the same to a combobox
	For scale 100% the height is 13 DLU, for 125% is 12, and for 150% is 11
	In resource.rc all edit control have a size for 100% scale and it should be fixed.
*/
bool CALLBACK cbEnumFixEditHeights (HWND hWnd, LPARAM height) {
	if (hWnd == NULL)
		return true;

	TCHAR wndClass[256];
	GetClassName(hWnd, wndClass, 256);
	if (_tcscmp(wndClass, WC_EDIT))
		return true;

	if (GetWindowLongPtr(hWnd, GWL_STYLE) & ES_MULTILINE)
		return true;

	RECT rc;
	GetClientRect(hWnd, &rc);
	SetWindowPos(hWnd, 0, 0, 0, rc.right, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	return true;
}

int TabCtrl_GetItemText(HWND hWnd, int iItem, TCHAR* pszText, int cchTextMax) {
	TCITEM ti;
	ti.pszText = pszText;
	ti.cchTextMax = cchTextMax;
	ti.mask = TCIF_TEXT;
	TabCtrl_GetItem(hWnd, iItem, &ti);

	return (int)_tcslen(pszText);
}

COLORREF RichEdit_GetTextColor (HWND hWnd, int pos) {
	CHARFORMAT cf = {0};
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;

	SendMessage(hWnd, EM_SETSEL, pos, pos + 1);
	SendMessage(hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	return cf.crTextColor;
}

HTREEITEM TreeView_AddItem (const TCHAR* caption, HTREEITEM parent = TVI_ROOT, int type = 0, int disabled = 0, int pinned = 0) {
	TVITEM tvi{0};
	tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
	tvi.pszText = (TCHAR*)caption;
	tvi.cchTextMax = _tcslen(caption) + 1;
	tvi.lParam = type;

	if (pinned && (type == TABLE || type == VIEW)) {
		tvi.stateMask = TVIS_BOLD;
		tvi.state = TVIS_BOLD;
	}

	if (disabled && (type == INDEX || type == TRIGGER)) {
		tvi.stateMask = TVIS_CUT;
		tvi.state = TVIS_CUT;
	}

	TVINSERTSTRUCT tvins{0};
	tvins.item = tvi;
	tvins.hInsertAfter = pinned ? (HTREEITEM)GetProp(hTreeWnd, TEXT("LASTPINNED")) : parent != TVI_ROOT || type == 0 ? TVI_LAST : abs(type) == 1 ? TVI_FIRST : treeItems[abs(type) - 1];
	//pinned && parent != TVI_ROOT ? TVI_FIRST : TVI_LAST;//parent != TVI_ROOT || type == 0 ? TVI_LAST : //abs(type) == 1 ? TVI_FIRST : treeItems[abs(type)];
	tvins.hParent = parent;
	return (HTREEITEM)SendMessage(hTreeWnd, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
}

LPARAM TreeView_GetItemParam (HWND hTreeWnd, HTREEITEM hItem) {
	TVITEM ti = {0};
	ti.hItem = hItem;
	ti.mask = TVIF_PARAM;
	TreeView_GetItem(hTreeWnd, &ti);

	return ti.lParam;
}

void updateTree(int type, TCHAR* select) {
	if (type < 0) {
		printf("(updateTree) Error: illegal value of type: %i\n", type);
		return;
	}

	TCHAR treeFilter16[256] = {0};
	if (IsWindowVisible(hTreeSearchWnd)) {
		GetWindowText(hTreeSearchWnd, treeFilter16, 255);
		_tcslwr(treeFilter16);
	}

	TCHAR selText[256] = {0};
	int selType = 0;
	if (type && select) {
		_sntprintf(selText, 255, TEXT("%ls"), select);
		selType = type;
	} else {
		TV_ITEM tv{0};
		tv.mask = TVIF_TEXT | TVIF_PARAM;
		tv.pszText = selText;
		tv.cchTextMax = 255;
		tv.hItem = TreeView_GetSelection(hTreeWnd);
		TreeView_GetItem(hTreeWnd, &tv);
		selType = type && abs(tv.lParam) == type ? type : abs(tv.lParam);
	}

	int yPos = GetScrollPos(hTreeWnd, SB_VERT);

	bool isExpanded = false;
	if (type) {
		TV_ITEM tv{0};
		tv.mask = TVIF_STATE;
		tv.hItem = treeItems[type];
		isExpanded = TreeView_GetItem(hTreeWnd, &tv) && (tv.state & TVIS_EXPANDED);
	}

	SetWindowRedraw(hTreeWnd, FALSE);

	TreeView_DeleteItem(hTreeWnd, type == 0 ? TVI_ROOT : treeItems[type]);
	if (type == 0) {
		for (int i = 1; i < 5; i++)
			treeItems[i] = TreeView_AddItem(TYPES16p[i], TVI_ROOT, -i);
	} else {
		treeItems[type] = TreeView_AddItem(TYPES16p[type], TVI_ROOT, -type);
	}

	char sql[2048];
	TCHAR schema16[256];
	GetWindowText(hSchemaWnd, schema16, 255);
	bool isMain = _tcscmp(schema16, TEXT("main")) == 0;

	TCHAR* pinned16 = 0;
	if (isMain && (type == 0 || type == TABLE || type == VIEW)) {
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(prefs::db, "select '~~~' || group_concat(name, '~~~') || '~~~' from pinned where dbname = ?1", -1, &stmt, 0);
		char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0), false);
		sqlite3_bind_text(stmt, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
		delete [] dbname8;

		if (SQLITE_ROW == sqlite3_step(stmt))
			pinned16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
		sqlite3_finalize(stmt);
	}

	auto isObjectPinned = [pinned16](TCHAR* name16) {
		if (!pinned16)
			return false;

		int len = _tcslen(name16) + 6 + 1;
		TCHAR qname16[len];
		_sntprintf(qname16, len, TEXT("~~~%ls~~~"), name16);
		return _tcsstr(pinned16, qname16) != 0;
	};
	SetProp(hTreeWnd, TEXT("LASTPINNED"), TVI_FIRST);

	char* schema8 = utils::utf16to8(schema16);
	sprintf(sql,
		"select t.name, t.type, " \
		"iif(t.type in ('table', 'view'), group_concat(c.name || iif(t.type = 'table', ': ' || c.type || iif(c.pk,' [pk]',''), ''), ','), null) columns " \
		"from \"%s\".sqlite_master t left join pragma_table_xinfo c on t.tbl_name = c.arg and c.schema = \"%s\" " \
		"where t.sql is not null and t.type = coalesce(?1, t.type) and t.name <> 'sqlite_sequence' " \
		"group by t.type, t.name order by t.type, t.name",
		schema8, schema8);

	sqlite3* conn = db;
	if (_tcscmp(schema16, TEXT("temp")) == 0) {
		int tabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
		conn = tabNo == -1 ? cli.db : tabs[tabNo].db;
	}

	auto insertTreeItems = [conn, type, isObjectPinned, schema8, treeFilter16](const char* sql, bool hasColumns) {
		auto getItemType = [](const char* type8) {
			return !strcmp(type8, "table") ? TABLE :
				!strcmp(type8, "view") ? VIEW :
				!strcmp(type8, "index") ? INDEX :
				!strcmp(type8, "trigger") ? TRIGGER :
				0;
		};

		sqlite3_stmt *stmt;

		int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, 0);
		if (type)
				sqlite3_bind_text(stmt, 1, TYPES8[type], strlen(TYPES8[type]), SQLITE_TRANSIENT);
		else
				sqlite3_bind_null(stmt, 1);

		int prevItemType = -1;
		while (rc == SQLITE_OK && SQLITE_ROW == sqlite3_step(stmt)) {
			int itemType = getItemType((char*)sqlite3_column_text(stmt, 1));
			if (itemType == 0)
				continue;

			if (itemType != prevItemType)
				SetProp(hTreeWnd, TEXT("LASTPINNED"), TVI_FIRST);

			const char* name8 = (const char*) sqlite3_column_text(stmt, 0);
			const char* type8 = (const char*) sqlite3_column_text(stmt, 1);
			TCHAR* name16 = utils::utf8to16(name8);
			TCHAR* columns16 = utils::utf8to16((char *) sqlite3_column_text(stmt, 2));

			bool isFilterColumn = false;
			bool pinned = (itemType == TABLE || itemType == VIEW) && isObjectPinned(name16);
			HTREEITEM hItem = TreeView_AddItem(name16, treeItems[itemType], itemType, 0, pinned);

			if (itemType == TABLE || itemType == VIEW) {
				if (hasColumns) {
					TCHAR* column16 = _tcstok (columns16, TEXT(","));
					while (column16 != NULL) {
						HTREEITEM hColumnItem = TreeView_AddItem(column16, hItem, COLUMN);
						TCHAR* colName16 = _tcsdup(column16);
						_tcslwr(colName16);
						if (treeFilter16[0]) {
							TCHAR* s = _tcsstr(colName16, treeFilter16);
							if (s != 0 && (_tcschr(s, TEXT(':')) != 0)) {
								TreeView_SetItemState(hTreeWnd, hColumnItem, TVIS_BOLD, TVIS_BOLD);
								isFilterColumn = true;
							}
						}
						free(colName16);
						column16 = _tcstok (NULL, TEXT(","));
					}
				} else {
					sqlite3_stmt *substmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, "select name || iif(?2 = 'table', ': ' || type || case when pk then ' [pk]' else '' end, '') from pragma_table_xinfo(?1) where schema = ?3 order by cid", -1, &substmt, 0)) {
						sqlite3_bind_text(substmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
						sqlite3_bind_text(substmt, 2, type8, strlen(type8), SQLITE_TRANSIENT);
						sqlite3_bind_text(substmt, 3, schema8, strlen(schema8), SQLITE_TRANSIENT);
						while(SQLITE_ROW == sqlite3_step(substmt)) {
							TCHAR* column16 = utils::utf8to16((char *) sqlite3_column_text(substmt, 0));
							HTREEITEM hColumnItem = TreeView_AddItem(column16, hItem, COLUMN, 0, true);
							TCHAR* colName16 = _tcsdup(column16);
							_tcslwr(colName16);
							if (treeFilter16[0]) {
								TCHAR* s = _tcsstr(colName16, treeFilter16);
								if (s != 0 && (_tcschr(s, TEXT(':')) != 0)) {
									TreeView_SetItemState(hTreeWnd, hColumnItem, TVIS_BOLD, TVIS_BOLD);
									isFilterColumn = true;
								}
							}
							free(colName16);
							delete [] column16;
						}
					}

					if (SQLITE_DONE != sqlite3_errcode(db)) {
						TCHAR* err16 = utils::utf8to16(sqlite3_errmsg(db));
						TCHAR msg16[256];
						_sntprintf(msg16, 255, TEXT("Error: %ls"), err16);
						delete [] err16;
						TreeView_AddItem(msg16, hItem, 0);
						TreeView_Expand(hTreeWnd, hItem, TVE_EXPAND);
					}
					sqlite3_finalize(substmt);
				}
			}

			if (treeFilter16[0]) {
				if (_tcsstr(name16, treeFilter16) == 0 && !isFilterColumn)
					TreeView_DeleteItem(hTreeWnd, hItem);

				if (isFilterColumn)
					TreeView_Expand(hTreeWnd, hItem, TVE_EXPAND);
			} else {
				if (pinned) {
					SetProp(hTreeWnd, TEXT("LASTPINNED"), hItem);
					prevItemType = itemType;
				}
			}

			delete [] name16;
			delete [] columns16;
		}
		sqlite3_finalize(stmt);
		RemoveProp(hTreeWnd, TEXT("LASTPINNED"));

		return SQLITE_OK != sqlite3_errcode(conn);
	};

	if (insertTreeItems(sql, true)) {
		sprintf(sql, "select name, type from \"%s\".sqlite_master where sql is not null and type = coalesce(?1, type) and name <> 'sqlite_sequence' order by 2, 1", schema8);
		insertTreeItems(sql, false);
	}

	if ((type == 0 || type == INDEX || type == TRIGGER) && (strcmp(schema8, "main") == 0)) {
		char sql[] = "select d.name, d.type, 1 disabled from disabled d where d.type = coalesce(?1, d.type) and d.dbpath = ?2 order by type, name";
		sqlite3_stmt *stmt;
		int rc = sqlite3_prepare_v2(prefs::db, sql, -1, &stmt, 0);
		if (type)
				sqlite3_bind_text(stmt, 1, TYPES8[type], strlen(TYPES8[type]), SQLITE_TRANSIENT);
		else
				sqlite3_bind_null(stmt, 1);

		const char* dbpath = sqlite3_db_filename(db, 0);
		sqlite3_bind_text(stmt, 2, dbpath, strlen(dbpath), SQLITE_TRANSIENT);

		while (rc == SQLITE_OK && SQLITE_ROW == sqlite3_step(stmt)) {
			TCHAR* name16 = utils::utf8to16((char *) sqlite3_column_text(stmt, 0));
			TCHAR* type16 = utils::utf8to16((char *) sqlite3_column_text(stmt, 1));
			BOOL disabled = sqlite3_column_int(stmt, 2);

			if (!_tcscmp(type16, TEXT("trigger")))
				TreeView_AddItem(name16, treeItems[TRIGGER], TRIGGER, disabled);

			if (!_tcscmp(type16, TEXT("index")))
				TreeView_AddItem(name16, treeItems[INDEX], INDEX, disabled);

			delete [] type16;
			delete [] name16;
		}
		sqlite3_finalize(stmt);
	}

	delete [] schema8;


	if (isExpanded)
		TreeView_Expand(hTreeWnd, treeItems[type], TVE_EXPAND);

	if (!type) {
		TreeView_Expand(hTreeWnd, treeItems[TABLE], TVE_EXPAND);
		TreeView_Expand(hTreeWnd, treeItems[VIEW], TVE_EXPAND);
	}

	if (selType && _tcslen(selText) > 0) {
		HTREEITEM hItem = TreeView_GetNextItem(hTreeWnd, treeItems[selType], TVGN_CHILD);
		while (hItem) {
			TCHAR buf[256] = {0};
			TV_ITEM tv{0};
			tv.mask = TVIF_TEXT;
			tv.pszText = buf;
			tv.cchTextMax = 255;
			tv.hItem = hItem;
			TreeView_GetItem(hTreeWnd, &tv);
			if (!_tcscmp(buf, selText)) {
				TreeView_SelectItem(hTreeWnd, hItem);
				hItem = 0;
			} else {
				hItem = TreeView_GetNextItem(hTreeWnd, hItem, TVGN_NEXT);
				if (!hItem)
					TreeView_SelectItem(hTreeWnd, treeItems[selType]);
			}
		}
	} else {
		TreeView_SelectItem(hTreeWnd, NULL);
	}

	SetWindowRedraw(hTreeWnd, TRUE);
	SetScrollPos(hTreeWnd, SB_VERT, yPos, TRUE);
	SendMessage(hTreeWnd, WM_SIZE, 0, 0);

	HTREEITEM hItem = TreeView_GetSelection(hTreeWnd);
	if (hItem)
		TreeView_EnsureVisible(hTreeWnd, hItem);

	InvalidateRect(hTreeWnd, 0, TRUE);
}

void updateTransactionState() {
	int tabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
	int state = tabNo >= 0 ? tabs[tabNo].db && sqlite3_get_autocommit(tabs[tabNo].db) == 0 : cli.db && sqlite3_get_autocommit(cli.db) == 0;
	SendMessage(hStatusWnd, SB_SETTEXT, SB_TRANSACTION, (LPARAM)(transactionStates[state]));
}

void switchSchemaTo(TCHAR* schema) {
	SetWindowText(hSchemaWnd, schema);
	updateTree();
	TreeView_SelectItem(hTreeWnd, treeItems[TABLE]);
}

void reloadMeta(TCHAR** arr, const char* query8) {
	for (int i = 0; arr[i] && i < MAX_ENTITY_COUNT; i++) {
		delete [] arr[i];
		arr[i] = 0;
	}

	sqlite3_stmt *stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
		int rowNo = 0;
		while (SQLITE_ROW == sqlite3_step(stmt)) {
			arr[rowNo] = utils::utf8to16((char *) sqlite3_column_text(stmt, 0));
			arr[rowNo + 1] = 0;
			rowNo++;
		}
	}
	sqlite3_finalize(stmt);
}

bool loadExtensions(sqlite3* db, sqlite3* conn) {
	if (!db)
		return false;

	if (db == conn) {
		sqlite3_exec(db, "create table if not exists temp.loaded_extensions (name, path)", 0, 0, 0);
		sqlite3_exec(db, "delete from temp.loaded_extensions", 0, 0, 0);

		sqlite3_stmt* stmt;
		if (SQLITE_OK != sqlite3_prepare_v2(db, "insert into temp.loaded_extensions (name, path) values (?1, ?2)", -1, &stmt, 0)) {
			sqlite3_finalize(stmt);
			return false;
		}

		sqlite3_stmt* stmt2;
		if (SQLITE_OK != sqlite3_prepare_v2(prefs::db, "select 1 from main.addons where name = ?1 and type = 0 and enable = 0", -1, &stmt2, 0)) {
			sqlite3_finalize(stmt);
			sqlite3_finalize(stmt2);
			return false;
		}

		TCHAR searchPath[MAX_PATH + 1]{0};
		_sntprintf(searchPath, MAX_PATH, TEXT("%ls" EXTENSION_DIRECTORY "*.dll"), APP_PATH);

		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(searchPath, &ffd);

		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				TCHAR path16[MAX_PATH + 1]{0};
				_sntprintf(path16, MAX_PATH, TEXT("%ls" EXTENSION_DIRECTORY "%ls"), APP_PATH, ffd.cFileName);
				char* path8 = utils::utf16to8(path16);
				PathRemoveExtension(ffd.cFileName);
				char* name8 = utils::utf16to8(ffd.cFileName);

				sqlite3_reset(stmt2);
				sqlite3_bind_text(stmt2, 1, name8, strlen(name8),  SQLITE_TRANSIENT);
				bool isEnable = SQLITE_ROW != sqlite3_step(stmt2);

				if (isEnable) {
					if (SQLITE_OK == sqlite3_load_extension(db, path8, NULL, NULL)) {
						sqlite3_reset(stmt);
						sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt, 2, path8, strlen(path8), SQLITE_TRANSIENT);
						sqlite3_step(stmt);
					} else {
						TCHAR msg16[MAX_PATH + 512];
						_sntprintf(msg16, MAX_PATH + 511, TEXT("Can't load %ls\nDo you want to delete this dll?"), path16);
						if (IDOK == MessageBox(hMainWnd, msg16, TEXT("Confirmation"), MB_OKCANCEL))
							DeleteFile(path16);
					}
				}

				delete [] path8;
				delete [] name8;
			} while (FindNextFile(hFind, &ffd));
		}
		FindClose(hFind);

		sqlite3_finalize(stmt);
		sqlite3_finalize(stmt2);

		if (SQLITE_OK == sqlite3_prepare_v2(db, "select 1 from pragma_function_list() where name = 'inja'", -1, &stmt, 0))
			isInjaSupport = SQLITE_ROW == sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	} else {
		sqlite3_stmt *stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db, "select path from temp.loaded_extensions", -1, &stmt, 0)) {
			while (SQLITE_ROW == sqlite3_step(stmt))
				sqlite3_load_extension(conn, (const char*)sqlite3_column_text(stmt, 0), NULL, NULL);
		}
		sqlite3_finalize(stmt);
	}

	return true;
}

bool showDbError(HWND hWnd) {
	TCHAR* err16 = utils::utf8to16(sqlite3_errmsg(db));
	MessageBox(hWnd, err16, NULL, 0);
	delete [] err16;
	return true;
}

/*
	mode:
	0 - single DDL
	2 - DDL + child DDL + drop
*/
TCHAR* getDDL(const TCHAR* schema16, const TCHAR* name16, int type, int mode) {
	TCHAR* res = 0;
	sqlite3_stmt *stmt;
	char sql8[512 + _tcslen(schema16) + _tcslen(name16)];
	char* schema8 = utils::utf16to8(schema16);
	char* qschema8 = utils::double_quote(schema8);
	sprintf(sql8,
		mode == 1 ? "select group_concat(iif(type = ?1 or type = 'table' or type = 'view', " \
		"iif(type = 'table', '/*\n  Warning: The table will be recreated and all data will be lost.\n  Uncomment drop-statement if you are sure.\n*/' || char(10) || '--', '') || 'drop '|| type || ' ' || dq(name) || ';' || " \
		"char(10) || char(10), '') || sql || ';', char(10) || char(10) || char(10)) " \
		"from %s.sqlite_master where iif(?1 = 'table' or ?1 = 'view', tbl_name, name) = ?2 order by iif(type = 'table' or type = 'view', 'a', type)" :
		mode == 2 ?	"select group_concat(sql || ';', char(10) || char(10) || char(10)) " \
		"from %s.sqlite_master where iif(?1 = 'table' or ?1 = 'view', tbl_name, name) = ?2 order by iif(type = 'table' or type = 'view', 'a', type)" :
		"select sql from %s.sqlite_master where type = coalesce(?1, type) and name = ?2", qschema8);
	delete [] schema8;
	delete [] qschema8;

	if (SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0)) {
		char* name8 = utils::utf16to8(name16);
		if (type > 0)
			sqlite3_bind_text(stmt, 1, TYPES8[type], -1,  SQLITE_TRANSIENT);
		else
			sqlite3_bind_null(stmt, 1);
		sqlite3_bind_text(stmt, 2, name8, -1,  SQLITE_TRANSIENT);
		delete [] name8;
		if (SQLITE_ROW == sqlite3_step(stmt))
			res = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
	}
	sqlite3_finalize(stmt);

	if (!res && _tcscmp(schema16, TEXT("main")) == 0) {
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(prefs::db, "select sql, rowid from disabled where name = ?1 and type = ?2 and dbpath = ?3", -1, &stmt, 0);
		char* name8 = utils::utf16to8(name16);
		sqlite3_bind_text(stmt, 1, name8, -1, SQLITE_TRANSIENT);
		delete [] name8;
		sqlite3_bind_text(stmt, 2, TYPES8[type], -1, SQLITE_TRANSIENT);
		const char* dbpath8 = sqlite3_db_filename(db, 0);
		sqlite3_bind_text(stmt, 3, dbpath8, -1, SQLITE_TRANSIENT);

		if (SQLITE_ROW == sqlite3_step(stmt)) {
			res = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
		}
		sqlite3_finalize(stmt);
	}

	return res;
}

// A ListView with one column has broken sort. So, the first column is a row number
// The first column stores row length in TCHAR without first column and \0. Used for data export.
// lParam of each row is a row number. Used by cbListComparator for sorting
int ListView_SetData(HWND hListWnd, sqlite3_stmt *stmt, bool isRef) {
	int colCount = sqlite3_column_count(stmt);
	HWND hHeader = ListView_GetHeader(hListWnd);
	bool isVirtual = GetWindowLong(hListWnd, GWL_STYLE) & LVS_OWNERDATA;

	sqlite3* stmtDb = sqlite3_db_handle(stmt);

	// Find references for each columns and store appropriate sql to row.
	// If there is no the reference then the inserted row is empty
	// Used to show tooltip by Alt + Click by cell
	// I don't want to tangle with memory managment. This listbox will be automatically destroyed when its parent is gone.
	if (isRef) {
		HWND hColumnsWnd = GetDlgItem(hListWnd, IDC_REFLIST);
		ListBox_AddString(hColumnsWnd, TEXT(""));
		char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));
		sqlite3_stmt *stmt2;
		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select refname || ':' || query from refs where dbname = ?1 and \"schema\" = ?2 and tblname = ?3 and colname = ?4", -1, &stmt2, 0)) {
			for (int i = 0; i < colCount; i++) {
				if (!sqlite3_column_origin_name(stmt, i)) {
					ListBox_AddString(hColumnsWnd, TEXT(""));
					continue;
				}

				sqlite3_bind_text(stmt2, 1, dbname8, -1,  SQLITE_TRANSIENT);
				sqlite3_bind_text(stmt2, 2, sqlite3_column_database_name(stmt, i), -1,  SQLITE_TRANSIENT);
				sqlite3_bind_text(stmt2, 3, sqlite3_column_table_name(stmt, i), -1,  SQLITE_TRANSIENT);
				sqlite3_bind_text(stmt2, 4, sqlite3_column_origin_name(stmt, i), -1,  SQLITE_TRANSIENT);
				if (SQLITE_ROW == sqlite3_step(stmt2)) {
					TCHAR* line16 = utils::utf8to16((char*)sqlite3_column_text(stmt2, 0));
					ListBox_AddString(hColumnsWnd, line16);
					delete [] line16;
				} else {
					ListBox_AddString(hColumnsWnd, TEXT(""));
				}
				sqlite3_reset(stmt2);
			}
		}
		sqlite3_finalize(stmt2);
		delete [] dbname8;
	}

	if (hHeader == NULL || Header_GetItemCount(hHeader) == 0) {
		for (int i = 0; i <= colCount; i++) {
			TCHAR* name16 = utils::utf8to16(i > 0 ? sqlite3_column_name(stmt, i - 1) : "row length");
			LVCOLUMN lvc;
			lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.iSubItem = i;
			lvc.pszText = name16;
			lvc.cchTextMax = _tcslen(name16) + 1;
			lvc.cx = i > 0 ? 100 : 0;
			ListView_InsertColumn(hListWnd, i, &lvc);
			delete [] name16;
		}
	}

	ListView_DeleteAllItems(hListWnd);

	bool isStopByLimit = false;
	int rowLimit = prefs::get("row-limit");
	int rowNo = 0;

	SendMessage(hListWnd, WMU_RESET_CACHE, 0, 0);

	int cacheSize = (rowLimit > 0 ? rowLimit : 100) * (colCount + 1);
	TCHAR*** cache = isVirtual ? (TCHAR***)calloc(cacheSize, sizeof(TCHAR**)) : 0;
	byte* datatypes = (byte*)calloc(cacheSize, sizeof(byte));
	unsigned char** blobs = isVirtual ? (unsigned char**)calloc(cacheSize, sizeof(unsigned char*)) : 0;

	while(sqlite3_step(stmt) == SQLITE_ROW) {
		if (rowLimit > 0 && rowNo == rowLimit) {
			isStopByLimit = true;
			break;
		}

		if (!isVirtual) {
			LVITEM lvi = {0};
			lvi.mask = 0;
			lvi.iSubItem = 0;
			lvi.iItem = rowNo;
			ListView_InsertItem(hListWnd, &lvi);
		} else {
			cache[rowNo] = (TCHAR**)calloc (colCount + 1, sizeof (TCHAR*));
		}

		int rowLength = 0;
		for (int i = 1; i <= colCount; i++) {
			int colType = sqlite3_column_type(stmt, i - 1);
			TCHAR* value16 = colType == SQLITE_BLOB ? utils::toBlobSize(sqlite3_column_bytes(stmt, i - 1)) :
				utils::utf8to16(colType == SQLITE_NULL ? "" : (const char*) sqlite3_column_text(stmt, i - 1));

			datatypes[i + rowNo * colCount] = colType;

			if (blobs && colType == SQLITE_BLOB) {
				int dataSize = sqlite3_column_bytes(stmt, i - 1);
				blobs[i + rowNo * colCount] = utils::toBlob(dataSize, (const unsigned char*)sqlite3_column_blob(stmt, i - 1));
			}

			rowLength += _tcslen(value16);

			if (!isVirtual) {
				LVITEM lvi = {0};
				lvi.iItem = rowNo;
				lvi.iSubItem = i;
				lvi.mask = LVIF_TEXT;
				lvi.pszText = value16;
				ListView_SetItem(hListWnd, &lvi);
			} else {
				cache[rowNo][i] = _tcsdup(value16); // malloc!
			}
			delete [] value16;
		}

		TCHAR rowLength16[64];
		_sntprintf(rowLength16, 63, TEXT("%i"), rowLength);
		if (!isVirtual) {
			LVITEM lvi = {0};
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.iItem = rowNo;
			lvi.pszText = rowLength16;
			lvi.cchTextMax = _tcslen(rowLength16) + 1;
			lvi.lParam = rowNo;
			ListView_SetItem(hListWnd, &lvi);
		} else {
			cache[rowNo][0] = _tcsdup(rowLength16);
		}

		if (cacheSize - rowNo * (colCount + 1) < 10) {
			cacheSize += 10000 * (colCount + 1);
			if (isVirtual) {
				cache = (TCHAR***)realloc(cache, cacheSize * sizeof(TCHAR**));
				blobs = (unsigned char**)realloc(blobs, cacheSize * sizeof(unsigned char*));
			}

			datatypes = (byte*)realloc(datatypes, cacheSize * sizeof(byte));
		}

		rowNo++;
	}

	int rowCount = rowNo;

	// Issue #145
	for (int colNo = 1; colNo <= colCount; colNo++) {
		int rCount = 0;
		int lCount = 0;
		for (int rowNo = 0; rowNo < rowCount; rowNo++) {
			int type = datatypes[colNo + rowNo * colCount];
			rCount += type == SQLITE_INTEGER || type == SQLITE_FLOAT;
			lCount += type == SQLITE_TEXT || type == SQLITE_BLOB;
		}

		LVCOLUMN lvc = {mask: LVCF_FMT};
		lvc.fmt = rCount >= lCount ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ListView_SetColumn(hListWnd, colNo, &lvc);
	}

	ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
	if ((WNDPROC)GetWindowLongPtr(hListWnd, GWLP_WNDPROC) != cbNewListView)
		SetProp(hListWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hListWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewListView));
	SetWindowTheme(hHeader, TEXT(" "), TEXT(" "));

	if (SQLITE_DONE != sqlite3_errcode(stmtDb) && !isStopByLimit) {
		char *err8 = (char*)sqlite3_errmsg(stmtDb);
		TCHAR* err16 = utils::utf8to16(err8);
		int len = _tcslen(err16) + 256;
		TCHAR* msg16 = new TCHAR[len + 1]{0};

		_sntprintf(msg16, len, TEXT("*** Terminate with error: %ls"), err16);
		delete [] err16;

		if (!isVirtual) {
			TCHAR rowLength16[64];
			_sntprintf(rowLength16, 63, TEXT("%i"), _tcslen(msg16));
			LVITEM lvi = {0};
			lvi.mask = LVIF_TEXT;
			lvi.iItem = rowNo;
			lvi.iSubItem = 0;
			lvi.pszText = rowLength16;
			lvi.cchTextMax = _tcslen(rowLength16) + 1;
			ListView_InsertItem(hListWnd, &lvi);

			lvi.iSubItem = 1;
			lvi.mask = LVIF_TEXT;
			lvi.pszText = msg16;
			lvi.cchTextMax = _tcslen(msg16) + 1;
			ListView_SetItem(hListWnd, &lvi);
		} else {
			cache[rowNo] = (TCHAR**)calloc (colCount + 1, sizeof (TCHAR*));
			cache[rowNo][1] = _tcsdup(msg16); // malloc!
		}
		delete [] msg16;

		datatypes[1 + rowNo * colCount] = SQLITE_TEXT;
		rowCount++;
	}

	int vCount = (colCount + 1) * rowCount;
	SetProp(hListWnd, TEXT("VALUECOUNT"), IntToPtr(vCount));
	datatypes = (byte*)realloc(datatypes, vCount * sizeof(byte));
	SetProp(hListWnd, TEXT("DATATYPES"), (HANDLE)datatypes);

	SetProp(hListWnd, TEXT("ORDERBY"), new int(0));

	if (isVirtual) {
		cache = (TCHAR***)realloc(cache, vCount * sizeof(TCHAR**));
		blobs = (unsigned char**)realloc(blobs, vCount * sizeof(unsigned char*));
		SetProp(hListWnd, TEXT("CACHE"), cache);
		SetProp(hListWnd, TEXT("BLOBS"), (HANDLE)blobs);
		SetProp(hListWnd, TEXT("ROWCOUNT"), new int(rowCount));
		SetProp(hListWnd, TEXT("TOTALROWCOUNT"), new int(rowCount));

		SetProp(hListWnd, TEXT("MODIFIERS"), (int*)calloc(colCount + 1, sizeof(int)));
		SetProp(hListWnd, TEXT("MODIFIER_ROWS"), (int*)calloc(colCount + 1, sizeof(int)));
		SetProp(hListWnd, TEXT("MODIFIERBUFFER"), new TCHAR[MAX_MODIFIER_OUTPUT_LEN + 1]);

		ListView_SetItemCount(hListWnd, 0);
		SendMessage(hListWnd, WMU_UPDATE_RESULTSET, 0, 0);
	}

	SendMessage(hListWnd, WMU_AUTO_COLUMN_SIZE, 0, 0);
	if (IsWindowVisible(hListWnd))
		ListView_EnsureVisible(hListWnd, 0, 0);

	return isStopByLimit ? -rowCount : rowCount;
}

int ListView_ShowRef(HWND hListWnd, int rowNo, int colNo) {
	if (IsWindowVisible(hTooltipWnd))
		hideTooltip();

	HWND hColumnsWnd = FindWindowEx(hListWnd, 0, WC_LISTBOX, NULL);
	if (!hColumnsWnd)
		return 0;

	TCHAR line16[MAX_TEXT_LENGTH];
	ListBox_GetText(hColumnsWnd, colNo, line16);
	if (!_tcslen(line16))
		return 0;

	TCHAR* refname16 = _tcstok(line16, TEXT(":"));
	TCHAR* query16 = _tcstok(0, TEXT(":"));

	TCHAR value16[256];
	ListView_GetItemText(hListWnd, rowNo, colNo, value16, 255);

	char* refname8 = utils::utf16to8(refname16);
	char* query8 = utils::utf16to8(query16);
	char* value8 = utils::utf16to8(value16);

	sqlite3_stmt *stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
		sqlite3_bind_text(stmt, 1, value8, -1, SQLITE_TRANSIENT);
		if (SQLITE_ROW == sqlite3_step(stmt)) {
			char* res8 = new char[MAX_TEXT_LENGTH + 1]{0};
			sprintf(res8, "%s\n", refname8);
			int colCount = sqlite3_column_count(stmt);
			for (int i = 0; i < colCount; i++) {
				if (i > 0)
					strcat(res8, "\n");

				const char* colname8 = (const char*)sqlite3_column_name(stmt, i);
				const char* value8 = (const char*)sqlite3_column_text(stmt, i);
				if (strlen(colname8) && colCount > 1) {
					strcat(res8, colname8);
					strcat(res8, ": ");
				}
				strcat(res8, value8 ? value8 : "(null)");
			}

			TCHAR* res16 = utils::utf8to16(res8);
			POINT p;
			GetCursorPos(&p);
			showTooltip(p.x, p.y + 10, res16);

			delete [] res16;
			delete [] res8;
		}
	} else {
		showDbError(0);
	}
	sqlite3_finalize(stmt);
	delete [] refname8;
	delete [] query8;
	delete [] value8;

	return 0;
}

bool ListView_DrillDown(HWND hListWnd, int rowNo, int colNo) {
	hideTooltip();

	HWND hColumnsWnd = FindWindowEx(hListWnd, 0, WC_LISTBOX, NULL);
	if (!hColumnsWnd)
		return 0;

	TCHAR line16[MAX_TEXT_LENGTH];
	ListBox_GetText(hColumnsWnd, colNo, line16);
	if (!_tcslen(line16))
		return 0;

	_tcstok(line16, TEXT(":"));
	TCHAR* query16 = _tcstok(0, TEXT(":"));

	HWND hParentWnd = GetParent(hListWnd);
	int resultNo = GetWindowLongPtr(hListWnd, GWLP_USERDATA);
	HWND hDrillWnd = createResultList(hParentWnd, resultNo);
	SetParent(hListWnd, hDrillWnd);
	ShowWindow(hListWnd, SW_HIDE);

	TCHAR value16[256];
	ListView_GetItemText(hListWnd, rowNo, colNo, value16, 255);

	char* query8 = utils::utf16to8(query16);
	char* value8 = utils::utf16to8(value16);

	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
		sqlite3_bind_text(stmt, 1, value8, -1, SQLITE_TRANSIENT);
		ListView_SetData(hDrillWnd, stmt, true);
	}
	sqlite3_finalize(stmt);
	delete [] query8;
	delete [] value8;

	SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);

	return 1;
}

bool ListView_DrillUp(HWND hListWnd) {
	HWND hDrillWnd = GetDlgItem(hListWnd, GetDlgCtrlID(hListWnd));
	if (!hDrillWnd)
		return 0;

	HWND hParentWnd = GetParent(hListWnd);
	SetParent(hDrillWnd, hParentWnd);
	ShowWindow(hDrillWnd, SW_SHOW);
	DestroyWindow(hListWnd);

	SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);
	return 1;
}

BOOL ListView_GetItemValue(HWND hListWnd, int rowNo, int colNo, TValue* cell, bool ignoreResultset) {
	TCHAR*** cache = (TCHAR***)GetProp(hListWnd, TEXT("CACHE"));
	int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
	byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
	unsigned char** blobs = (unsigned char**)GetProp(hListWnd, TEXT("BLOBS"));
	int colCount = ListView_GetColumnCount(hListWnd);

	if (!hListWnd || !cache || !datatypes || !blobs || colNo > colCount)
		return FALSE;

	if (!ignoreResultset && !resultset)
		rowNo = resultset[rowNo];

	int no = colNo + rowNo * (colCount - 1);
	int dataType = datatypes ? datatypes[no] : SQLITE_TEXT;
	const unsigned char* blob = blobs && dataType == SQLITE_BLOB ? blobs[no] : 0;

	cell->dataType = dataType;
	cell->dataLen = blob && dataType == SQLITE_BLOB ? utils::getBlobSize(blob) - 4 : _tcslen(cache[rowNo][colNo]);
	cell->data = blob && dataType == SQLITE_BLOB ? blob + 4 : (const unsigned char*)cache[rowNo][colNo];

	return TRUE;
}

TCHAR* ListView_GetItemValueText(HWND hListWnd, int rowNo, int colNo, bool ignoreResultset) {
	TCHAR* res16 = 0;
	TCHAR*** cache = (TCHAR***)GetProp(hListWnd, TEXT("CACHE"));
	int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));

	if (!cache) {
		MessageBox(hMainWnd, TEXT("ListView_GetValueText allowed only for a virtual listview"), NULL, MB_OK);
		return res16;
	}

	if (!ignoreResultset)
		rowNo = resultset[rowNo];

	res16 = cache[rowNo][colNo]; // default behaviour

	int* modifiers = (int*)GetProp(hListWnd, TEXT("MODIFIERS"));
	int modifierNo = modifiers ? modifiers[colNo] : 0;
	TValue value = {0};
	if (modifierNo && ListView_GetItemValue(hListWnd, rowNo, colNo, &value, ignoreResultset)) {
		TCHAR* buf16 = (TCHAR*)GetProp(hListWnd, TEXT("MODIFIERBUFFER"));
		ZeroMemory(buf16, (MAX_MODIFIER_OUTPUT_LEN + 1) * sizeof(TCHAR));

		if (modifierNo > 0 && plugins[modifierNo].hModule && plugins[modifierNo].type == ADDON_COLUMN_MODIFIER && plugins[modifierNo].setText) {
			if (plugins[modifierNo].setText(hListWnd, colNo, value.data, value.dataLen, value.dataType, buf16))
				res16 = buf16;
		}

		if (modifierNo < 0)	{
			modifierNo = abs(modifierNo);
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(prefs::db, "select code from functions where id = ?1", -1, &stmt, 0);
			sqlite3_bind_int(stmt, 1, abs(modifierNo));
			if (SQLITE_ROW == sqlite3_step(stmt)) {
				sqlite3_stmt* stmt2;
				if (SQLITE_OK == sqlite3_prepare_v2(db, (const char*)sqlite3_column_text(stmt, 0), -1, &stmt2, 0)) {
					if (value.dataType != SQLITE_BLOB) {
						char* value8 = utils::utf16to8(cache[rowNo][colNo]);
						dbutils::bind_variant(stmt2, 1, value8, false);
						delete [] value8;
					} else {
						sqlite3_bind_blob(stmt2, 1, value.data, value.dataLen, SQLITE_TRANSIENT);
					}

					if (SQLITE_ROW == sqlite3_step(stmt2)) {
						TCHAR* value16 = utils::utf8to16((const char*)sqlite3_column_text(stmt2, 0));
						_sntprintf(buf16, MAX_MODIFIER_OUTPUT_LEN, value16);
						delete [] value16;

						res16 = buf16;
					}
				}
				sqlite3_finalize(stmt2);
			}
			sqlite3_finalize(stmt);
		}
	}

	return res16;
}

LRESULT onListViewMenu(HWND hListWnd, int rowNo, int colNo, int cmd, bool ignoreLastColumn) {
	auto getRowLength = [hListWnd](int rowNo) {
		TCHAR rowLength[64 + 1]{0};
		LVITEM lvi = {0};
		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 0;
		lvi.pszText = rowLength;
		lvi.cchTextMax = 64;
		lvi.iItem = rowNo;
		ListView_GetItem(hListWnd, &lvi);
		return _ttoi(rowLength);
	};

	if (cmd == IDM_RESULT_CHART)
		return DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_CHART), hMainWnd, (DLGPROC)dialogs::cbDlgChart, (LPARAM)hListWnd);

	if (cmd == IDM_RESULT_COPY_CELL) {
		TCHAR* value16 = ListView_GetItemValueText(hListWnd, rowNo, colNo);
		utils::setClipboardText(value16);
		return true;
	}

	if (cmd == IDM_RESULT_VALUE_FILTER) {
		HWND hHeader = ListView_GetHeader(hListWnd);
		if ((GetWindowLongPtr(hHeader, GWL_STYLE) & HDS_FILTERBAR) == 0)
			SendMessage(hMainWnd, WM_COMMAND, IDM_RESULT_FILTERS, 0);

		TCHAR* value16 = ListView_GetItemValueText(hListWnd, rowNo, colNo);
		int len = _tcslen(value16) + 2;
		TCHAR buf16[len];
		_sntprintf(buf16, len, TEXT("=%ls"), value16);
		SetDlgItemText(hHeader, IDC_HEADER_EDIT + colNo, buf16);
		SendMessage(hListWnd, WMU_UPDATE_RESULTSET, 0, 0); // Resultset
		SendMessage(GetAncestor(hListWnd, GA_ROOT), WMU_UPDATE_DATA, 0, 0); // Edit data dialog
	}

	if (cmd != IDM_RESULT_VALUE_FILTER && cmd != IDM_RESULT_COPY_ROW && cmd != IDM_RESULT_EXPORT)
		return false;

	HWND hHeader =  ListView_GetHeader(hListWnd);
	int colCount = (int)SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0L) - ignoreLastColumn;
	int rowCount = ListView_GetSelectedCount(hListWnd);
	int searchNext = LVNI_SELECTED;
	if (cmd == IDM_RESULT_EXPORT && rowCount < 2) {
		rowCount = ListView_GetItemCount(hListWnd);
		searchNext = LVNI_ALL;
	}

	TCHAR path16[MAX_PATH + 1] {0};
	if (cmd == IDM_RESULT_EXPORT) {
		_sntprintf(path16, MAX_PATH, TEXT("result.csv"));
		if (!utils::saveFile(path16, TEXT("CSV files\0*.csv\0All\0*.*\0"), TEXT("csv"), GetParent(hListWnd)))
			return false;

		if (rowCount == prefs::get("row-limit")) {
			int tabNo = SendMessage(hMainTabWnd, WMU_TAB_GET_CURRENT, 0, 0);
			TCHAR err16[1024];
			if (tools::exportCSV(path16, *tabs[tabNo].tabTooltips, err16) == -1)
				MessageBox(hMainWnd, err16, NULL, MB_OK);
			else
				MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);
			return true;
		}
	}

	bool withHeader = cmd == IDM_RESULT_EXPORT || HIWORD(GetKeyState(VK_SHIFT));
	const TCHAR* delimiter16 = tools::DELIMITERS[prefs::get(cmd == IDM_RESULT_COPY_ROW ? "copy-to-clipboard-delimiter" : "csv-export-delimiter")];
	const TCHAR* newLine16 = cmd == IDM_RESULT_COPY_ROW || !prefs::get("csv-export-is-unix-line") ? TEXT("\r\n") : TEXT("\n");

	if (!colCount || !rowCount)
		return false;

	int* modifiers = (int*)GetProp(hListWnd, TEXT("MODIFIERS"));
	int bufSize = withHeader ? 255 * colCount : 0;
	int currRowNo = -1;
	while((currRowNo = ListView_GetNextItem(hListWnd, currRowNo, searchNext)) != -1) {
		bufSize += getRowLength(currRowNo);
		for (int colNo = 1; colNo < colCount; colNo++)
			bufSize += modifiers && modifiers[colNo] ? _tcslen(ListView_GetItemValueText(hListWnd, currRowNo, colNo)) : 0;
	}

	bufSize += (colCount /* delimiters*/ + 64 /* possible quotes */ + 3 /* new line */) * rowCount + 1 /* EOF */;
	bufSize *= 2; // single " is extended to "" => allocate additional memory
	TCHAR* res16 = new TCHAR[bufSize] {0};

	auto addValue = [](TCHAR* res16, TCHAR* buf16) {
		TCHAR* qvalue16 = utils::replaceAll(buf16, TEXT("\""), TEXT("\"\""));
		if (_tcschr(qvalue16, TEXT(',')) || _tcschr(qvalue16, TEXT('"')) || _tcschr(qvalue16, TEXT('\n'))) {
			int len = _tcslen(qvalue16) + 3;
			TCHAR val16[len + 1]{0};
			_sntprintf(val16, len, TEXT("\"%ls\""), qvalue16);
			_tcscat(res16, val16);
		} else {
			_tcscat(res16, buf16);
		}
		delete [] qvalue16;
	};

	if (withHeader) {
		for(int colNo = 1; colNo < colCount; colNo++) {
			TCHAR buf16[256]{0};
			Header_GetItemText(hHeader, colNo, buf16, 255);
			addValue(res16, buf16);
			_tcscat(res16, colNo != colCount - 1 ? delimiter16 : newLine16);
		}
	}

	currRowNo = -1;
	while((currRowNo = ListView_GetNextItem(hListWnd, currRowNo, searchNext)) != -1) {
		for(int colNo = 1; colNo < colCount; colNo++) {
			TCHAR* value16 = ListView_GetItemValueText(hListWnd, currRowNo, colNo);
			addValue(res16, value16);
			_tcscat(res16, colNo != colCount - 1 ? delimiter16 : newLine16);
		}
	}

	if (cmd == IDM_RESULT_COPY_ROW)
		utils::setClipboardText(res16);

	if (cmd == IDM_RESULT_EXPORT) {
		FILE* f = _tfopen(path16, TEXT("wb"));
		if (f != NULL) {
			char* res8 = utils::utf16to8(res16);
			fprintf(f, res8);
			delete [] res8;
		} else {
			MessageBox(0, TEXT("Error to open file"), NULL, MB_OK);
		}

		fclose(f);
	}

	delete [] res16;
	return true;
}

bool updateHighlighting(HWND hWnd) {
	int size = GetWindowTextLength(hWnd);
	TCHAR text[size + 1];
	GetWindowText(hWnd, text, size + 1);

	logger(LOGGER_HIGHLIGHT, text);

	CHARFORMAT2 cf2;
	ZeroMemory(&cf2, sizeof(CHARFORMAT2));
	cf2.cbSize = sizeof(CHARFORMAT2) ;
	SendMessage(hWnd, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM) &cf2);
	cf2.dwMask = CFM_BACKCOLOR | CFM_COLOR | CFM_BOLD;
	cf2.crBackColor = RGB(255, 255, 255);
	cf2.dwEffects = 0;
	cf2.crTextColor = RGB(0, 0, 0);

	int crPos;
	SendMessage(hWnd, EM_GETSEL, (WPARAM)&crPos, (WPARAM)&crPos);

	int pos = 0;
	int rCount = 0;
	int mode = 0; // 0 - casual, 1 - keyword, 2 - function, 3 - pragma, 4 - quotted string, 5 - single line comment, 6 - multiline comment
	COLORREF colors[7] = {
		RGB(0, 0, 0),
		(COLORREF)prefs::get("color-keyword"),
		(COLORREF)prefs::get("color-function"),
		(COLORREF)prefs::get("color-pragma"),
		(COLORREF)prefs::get("color-quoted"),
		(COLORREF)prefs::get("color-comment"),
		(COLORREF)prefs::get("color-comment")
	};
	TCHAR breakers[] = TEXT(" \"'`\n\t-;:(),=<>/");

	SendMessage(hWnd, EM_SETSEL, 0, -1);
	SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf2);

	while (pos < size) {
		mode =
			text[pos] == TEXT('/') && (pos < size - 1) && text[pos + 1] == TEXT('*') ? 6 :
			text[pos] == TEXT('-') && (pos < size - 1) && text[pos + 1] == TEXT('-') ? 5 :
			text[pos] == TEXT('"') || text[pos] == TEXT('\'') ? 4 :
			_tcschr(breakers, text[pos]) ? 0 :
			1;

		int start = pos;
		int prCount = rCount;

		switch (mode) {
			case 0: {
				rCount += text[pos] == TEXT('\r');
				pos++;
			}
			break;

			case 1:
			case 2:
			case 3: {
				do {
					rCount += text[pos] == TEXT('\r');
					pos++;
				} while (!_tcschr(breakers, text[pos]) && pos < size);

				TCHAR buf[pos - start + 1]{0};
				for(int i = 0; i < pos - start; i++)
					buf[i] = _totlower(text[start + i]);

				TCHAR* tbuf = utils::trim(buf);
				mode = 0;

				if (_tcslen(buf)) {
					for (int i = 0; !mode && SQL_KEYWORDS[i]; i++)
						mode = !_tcscmp(SQL_KEYWORDS[i], tbuf) ? 1 : 0;

					for (int i = 0; !mode && FUNCTIONS[i]; i++)
						mode = !_tcscmp(FUNCTIONS[i], tbuf) ? 2 : 0;

					for (int i = 0; !mode && PRAGMAS[i]; i++)
						mode = !_tcscmp(PRAGMAS[i], tbuf) || (_tcsncmp(tbuf, TEXT("pragma_"), 7) == 0 && _tcscmp(tbuf + 7, PRAGMAS[i]) == 0) ? 3 : 0;

					for (int i = 0; !mode && MODULES[i]; i++)
						mode = !_tcscmp(MODULES[i], tbuf) ? 3 : 0;
				}

				delete [] tbuf;
			}
			break;

			case 4: {
				TCHAR q = text[start];
				do {
					rCount += text[pos] == TEXT('\r');
					pos++;
				} while (!((text[pos] == q) && (text[pos - 1] != TEXT('\\'))) && pos < size);

				if (text[pos] == q)
					pos++;
			}
			break;

			case 5: {
				while (text[pos] != TEXT('\n') && pos < size) {
					rCount += text[pos] == TEXT('\r');
					pos++;
				}
			}
			break;

			case 6: {
				int start = pos;
				while (pos < size && !((pos > start + 1) && text[pos] == TEXT('*') && text[pos + 1] == TEXT('/'))) {
					rCount += text[pos] == TEXT('\r');
					pos++;
				}

				if (pos < size - 1 && text[pos] == TEXT('*') && text[pos + 1] == TEXT('/'))
					pos += 2;
			}
			break;
		}

		int from = start - prCount;
		int to = pos - rCount;
		from = from < 0 ? 0 : from;
		to = from > to ? from : to;

		if (mode) {
			cf2.crTextColor = colors[mode];
			cf2.dwEffects = mode == 1 || mode == 2 || mode == 3 ? CFM_BOLD : 0;

			SendMessage(hWnd, EM_SETSEL, from, to);
			SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf2);
		}
	}

	SendMessage(hWnd, EM_SETSEL, crPos, crPos);

	currParenthesisPos[0] = -1;
	currParenthesisPos[1] = -1;

	logger(LOGGER_HIGHLIGHT, 0);

	return true;
}

bool updateParenthesisHighlighting(HWND hWnd) {
	bool res = false;

	// crPos, crLine, etc -  where the cursor is standing
	int crPos;
	SendMessage(hWnd, EM_GETSEL, (WPARAM)&crPos, (WPARAM)&crPos);

	auto setChar = [hWnd](int pos, bool isSelected) {
		CHARFORMAT2 cf;
		ZeroMemory(&cf, sizeof(CHARFORMAT2));
		cf.cbSize = sizeof(CHARFORMAT2);
		cf.dwMask = CFM_BACKCOLOR | CFM_EFFECTS | CFM_BOLD;
		cf.dwEffects = isSelected ? CFM_BOLD : 0;
		cf.crBackColor = isSelected ? prefs::get("color-parenthesis") : RGB(255, 255, 255);
		SendMessage(hWnd, EM_SETSEL, pos, pos + 1);
		SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	};

	for (int i = 0;  i < 2 && currParenthesisPos[i] != -1; i++) {
		setChar(currParenthesisPos[i], false);
		currParenthesisPos[i] = -1;
	}

	SendMessage(hWnd, EM_SETSEL, crPos, crPos);
	int lineCount = SendMessage(hWnd, EM_GETLINECOUNT, 0, 0);
	int crLineNo = SendMessage(hWnd, EM_LINEFROMCHAR, -1, 0);
	int crLineSize = SendMessage(hWnd, EM_LINELENGTH, crPos, 0);
	int crLineIdx = SendMessage(hWnd, EM_LINEINDEX, -1, 0);
	TCHAR crLine[crLineSize + 1]{0};
	crLine[0] = crLineSize;
	SendMessage(hWnd, EM_GETLINE, crLineNo, (LPARAM)crLine);
	int pos = crPos - crLineIdx;

	logger(LOGGER_PARENTHESIS, crLine);

	TCHAR parentheses[] = TEXT("[]{}()");
	TCHAR* chr = _tcschr(parentheses, crLine[pos]);

	if (chr && chr[0] && !RichEdit_GetTextColor(hWnd, crLineIdx + pos)) {
		int dir = chr[0] == TEXT('[') || chr[0] == TEXT('{') || chr[0] == TEXT('(') ? +1 : -1;

		setChar(crLineIdx + pos, true);
		currParenthesisPos[0] = crLineIdx + pos;

		int counter = 1;
		int currLineNo = crLineNo;
		while (currLineNo >= 0 && currLineNo < lineCount && currParenthesisPos[1] == -1) {
			int currLineIdx = SendMessage(hWnd, EM_LINEINDEX, currLineNo, 0);
			int currLineSize = SendMessage(hWnd, EM_LINELENGTH, currLineIdx, 0);
			TCHAR currLine[currLineSize + 1]{0};
			currLine[0] = currLineSize;
			SendMessage(hWnd, EM_GETLINE, currLineNo, (LPARAM)currLine);

			pos = currLineNo == crLineNo ? pos + dir : dir > 0 ? 0 : currLineSize - 1;
			while (pos >= 0 && pos < currLineSize && currParenthesisPos[1] == -1) {
				TCHAR* chr2 = _tcschr(parentheses, currLine[pos]);
				if (chr2 && chr2[0] && !RichEdit_GetTextColor(hWnd, currLineIdx + pos)) {
					counter += chr == chr2 ? +1 : chr + dir == chr2 ? -1 : 0;
					if (counter == 0) {
						setChar(currLineIdx + pos, true);
						currParenthesisPos[1] = currLineIdx + pos;
					}
				}
				pos += dir;
			}
			currLineNo += dir;
		}

		res = true;
	}

	SendMessage(hWnd, EM_SETSEL, crPos, crPos);

	logger(LOGGER_PARENTHESIS, 0);

	return res;
}

bool updateOccurrenceHighlighting(HWND hWnd) {
	int min, max;
	SendMessage(hWnd, EM_GETSEL, (WPARAM)&min, (LPARAM)&max);

	int size = max - min;
	TCHAR selection[size + 1]{0};
	SendMessage(hWnd, EM_GETSELTEXT, size + 1, (LPARAM)selection);

	logger(LOGGER_OCCURRENCE, selection);

	bool isAlphaNum = true;
	for (int i = 0; isAlphaNum && i < size; i++)
		isAlphaNum = _istalnum(selection[i]) || selection[i] == TEXT('_');

	// Reset all
	CHARFORMAT2 cf2;
	ZeroMemory(&cf2, sizeof(CHARFORMAT2));
	cf2.cbSize = sizeof(CHARFORMAT2);
	cf2.dwMask = CFM_BACKCOLOR;
	cf2.crBackColor = RGB(255, 255, 255);

	SendMessage(hWnd, EM_SETSEL, 0, -1);
	SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf2);

	SetProp(hWnd, EDITOR_HASOCCURRENCE, 0);

	if (size > 1 && isAlphaNum) {
		cf2.crBackColor = RGB(225, 225, 255);

		int tsize = GetWindowTextLength(hWnd);
		TCHAR text[tsize + 1];
		GetWindowText(hWnd, text, tsize + 1);

		TCHAR breakers[] = TEXT(" \"'`\r\n\t-;:(),.=<>/");

		int nl = 0;
		for (int i = 0; i <= tsize - size; i++) {
			nl += text[i] == TEXT('\n');
			if (_tcsncmp(text + i, selection, size) == 0 &&
				(i == 0 || _tcschr(breakers, text[i - 1]) != NULL) &&
				_tcschr(breakers, text[i + size]) != NULL) {
				SendMessage(hWnd, EM_SETSEL, i - nl, i + size - nl);
				SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf2);
				i += size;
			}
		}

		SetProp(hWnd, EDITOR_HASOCCURRENCE, (HANDLE)1);
	}

	logger(LOGGER_OCCURRENCE, 0);

	return true;
}

void processHighlight(HWND hWnd, bool isRequireHighlight, bool isRequireParenthesisHighlight, bool isRequireOccurrenceHighlight) {
	if (!isRequireHighlight && !isRequireParenthesisHighlight && !isRequireOccurrenceHighlight)
		return;

	SetProp(hWnd, TEXT("NOREDRAW"), IntToPtr(1)); // Use instead WM_SETREDRAW to prevent flickering

	IUnknown *tr_code = NULL;
	ITextDocument *td_code;

	SendMessage(hWnd, EM_GETOLEINTERFACE, 0, (LPARAM)&tr_code);
	if(tr_code == (IRichEditOle*)NULL) {
		MessageBox(0, TEXT("Error when trying to get RichEdit OLE Object"), NULL, 0);
		return;
	}
	tr_code->QueryInterface(IID_ITextDocument,(void**)&td_code);

	POINT scrollPos{0};
	SendMessage(hWnd, EM_GETSCROLLPOS, 0, (LPARAM)&scrollPos);
	int min, max;
	SendMessage(hWnd, EM_GETSEL, (WPARAM)&min, (LPARAM)&max);

	long cnt = 0;
	SendMessage(hWnd, EM_SETEVENTMASK, 0, ENM_NONE);
	td_code->Undo(tomSuspend, NULL);
	td_code->Freeze(&cnt);

	bool rc = false;
	if (isRequireHighlight)
		rc = rc || updateHighlighting(hWnd);

	if (isRequireOccurrenceHighlight)
		rc = rc || updateOccurrenceHighlighting(hWnd);

	// Call before updateParenthesisHighlighting because under Win10 with
	// default msftedit.dll SendMessage(hEdit, EM_LINEINDEX, 0, 0) returns -1
	if (isRequireParenthesisHighlight)
		rc = rc || updateParenthesisHighlighting(hWnd);

	td_code->Unfreeze(&cnt);

	td_code->Undo(tomResume, NULL);
	td_code->Release();
	tr_code->Release();

	SendMessage(hWnd, EM_SETSCROLLPOS, 0, (LPARAM)&scrollPos);

	// The selection has a direction defined by min and max positions.
	// The direction must be restored for the selection to work properly.
	// https://docs.microsoft.com/en-us/windows/win32/controls/em-setsel
	// The start value is the anchor point of the selection, and the end value is the active end.
	// If the user uses the SHIFT key to adjust the size of the selection, the active end can move but the anchor point remains the same.
	if (min == max || (LONG_PTR)GetProp(hWnd, EDITOR_SELECTION_START) == min) {
		SendMessage(hWnd, EM_SETSEL, min, max);
	} else {
		SendMessage(hWnd, EM_SETSEL, max, min);
	}

	RemoveProp(hWnd, TEXT("NOREDRAW"));

	if (rc)
		InvalidateRect(hWnd, 0, TRUE);

	SendMessage(hWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE |  ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_DROPFILES);
}

int ListView_Sort(HWND hListWnd, int colNo) {
	hSortingResultWnd = hListWnd;
	int* pOrderBy = (int*)GetProp(hListWnd, TEXT("ORDERBY"));
	*pOrderBy = colNo == *pOrderBy || colNo == -*pOrderBy ? -*pOrderBy : colNo;

	bool isVirtual = GetWindowLong(hListWnd, GWL_STYLE) & LVS_OWNERDATA;
	if (isVirtual) {
		SendMessage(hListWnd, WMU_UPDATE_RESULTSET, 0, 0);
		return true;
	}

	ListView_SortItems (hListWnd, cbListComparator, pOrderBy);
	int rowCount = ListView_GetItemCount(hListWnd);

	int vCount = (int)(LONG_PTR)GetProp(hListWnd, TEXT("VALUECOUNT"));
	byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
	if (datatypes != NULL){
		int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd)) - 1;

		byte* datatypes2 = (byte*)calloc(vCount, sizeof(byte));
		for (int rowNo = 0; rowNo < rowCount; rowNo++) {
			LVITEM lvi = {0};
			lvi.mask = LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.iItem = rowNo;
			ListView_GetItem(hListWnd, &lvi);

			for (int colNo = 1; colNo <= colCount; colNo++)
				datatypes2[colNo + rowNo * colCount] = datatypes[colNo + lvi.lParam * colCount];
		}

		free(datatypes);
		SetProp(hListWnd, TEXT("DATATYPES"), (HANDLE)datatypes2);
	}

	for (int i = 0; i < rowCount; i++) {
		LVITEM lvi = {0};
		lvi.iItem = i;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.lParam = i;
		ListView_SetItem(hListWnd, &lvi);
	}

	return true;
}

int ListView_Reset(HWND hListWnd) {
	SendMessage(hListWnd, WMU_RESET_CACHE, 0, 0);

	ListView_DeleteAllItems(hListWnd);
	HWND hHeader = ListView_GetHeader(hListWnd);
	int cnt = Header_GetItemCount(hHeader);
	for (int i = 0; i < cnt; i++) {
		ListView_DeleteColumn(hListWnd, cnt - 1 - i);
		Header_DeleteItem(hHeader, cnt - 1 - i);
	}

	return true;
}

int ListView_GetColumnCount(HWND hWnd) {
	HWND hHeader = ListView_GetHeader(hWnd);
	return hHeader ? Header_GetItemCount(hHeader) : -1;
}

int Header_GetItemText(HWND hWnd, int i, TCHAR* pszText, int cchTextMax) {
	if (i < 0)
		return FALSE;

	TCHAR buf[cchTextMax]{0};

	HDITEM hdi {0};
	hdi.mask = HDI_TEXT;
	hdi.pszText = buf;
	hdi.cchTextMax = cchTextMax;
	int rc = Header_GetItem(hWnd, i, &hdi);

	_tcsncpy(pszText, buf, cchTextMax);
	return rc;
}

int Header_SetItemText(HWND hWnd, int i, TCHAR* pszText) {
	HDITEM hdi = {0};
	hdi.mask = HDI_TEXT;
	hdi.pszText = pszText;
	hdi.cchTextMax = _tcslen(pszText);

	return Header_SetItem(hWnd, i, &hdi);
}

int Menu_GetItemPositionByName(HMENU hMenu, const TCHAR* itemName) {
	TCHAR buf16[MAX_PATH + 1];
	for (int itemNo = 0; itemNo < GetMenuItemCount(hMenu); itemNo++) {
		GetMenuString(hMenu, itemNo, buf16, MAX_PATH + 1, MF_BYPOSITION);
		if (_tcscmp(buf16, itemName) == 0)
			return itemNo;
	}

	return -1;
}

BOOL Menu_SetItemText(HMENU hMenu, UINT wID, const TCHAR* caption) {
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STRING;
	mii.dwTypeData = (TCHAR*)caption;
	mii.cch = _tcslen(caption);

	return SetMenuItemInfo(hMenu, wID, FALSE, &mii);
}

BOOL Menu_SetItemData(HMENU hMenu, UINT wID, ULONG_PTR lParam) {
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_DATA;
	mii.dwItemData = lParam;

	return SetMenuItemInfo(hMenu, wID, FALSE, &mii);
}

ULONG_PTR Menu_GetItemData(HMENU hMenu, UINT wID) {
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_DATA;

	GetMenuItemInfo(hMenu, wID, FALSE, &mii);
	return mii.dwItemData;
}

BOOL Menu_SetItemState(HMENU hMenu, UINT wID, UINT fState) {
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fState = fState;

	return SetMenuItemInfo(hMenu, wID, FALSE, &mii);
}

BOOL Menu_SetItemStateByPosition(HMENU hMenu, UINT pos, UINT fState) {
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fState = fState;

	return SetMenuItemInfo(hMenu, pos, TRUE, &mii);
}

BOOL Menu_InsertItem(HMENU hMenu, UINT uPosition, UINT wID, UINT fState, const TCHAR* pszText) {
	MENUITEMINFO mi = {0};
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
	mi.fState = fState;
	mi.wID = wID;
	mi.dwTypeData = (TCHAR*)pszText;
	return InsertMenuItem(hMenu, uPosition, true, &mi);
}

BOOL Menu_SetData(HMENU hMenu, ULONG_PTR data) {
	MENUINFO mi = {0};
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;
	mi.dwMenuData = data;

	return SetMenuInfo(hMenu, &mi);
}

ULONG_PTR Menu_GetData(HMENU hMenu) {
	MENUINFO mi = {0};
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;
	GetMenuInfo(hMenu, &mi);

	return mi.dwMenuData;
}

bool doEditorSearch(HWND hWnd, bool isBackward) {
	int len = _tcslen(searchString);
	if (!len)
		return false;

	int crPos = LOWORD(SendMessage(hWnd, EM_GETSEL, 0, 0));
	FINDTEXT ft = {{crPos, GetWindowTextLength(hWnd)}, searchString};

	LONG mode = (prefs::get("case-sensitive") ? FR_MATCHCASE : 0) | (isBackward ? 0 : FR_DOWN);
	int pos = SendMessage(hWnd, EM_FINDTEXT, mode, (LPARAM)&ft);
	if (crPos == pos) {
		ft.chrg.cpMin = crPos + _tcslen(searchString);
		pos = SendMessage(hWnd, EM_FINDTEXT, mode, (LPARAM)&ft);
	}

	if (pos == -1 && crPos != 0) {
		ft.chrg.cpMin = 0;
		pos = SendMessage(hWnd, EM_FINDTEXT, mode, (LPARAM)&ft);
	}

	if (pos != -1) {
		SendMessage(hWnd, EM_SETSEL, pos, pos + len);
	} else {
		MessageBeep(0);
	}

	return pos != -1;
}

bool processEditorEvents(MSGFILTER* pF) {
	HWND hWnd = pF->nmhdr.hwndFrom;
	HWND hParentWnd = GetAncestor(hWnd, GA_ROOT);
	bool isControl = HIWORD(GetKeyState(VK_CONTROL));
	bool isShift = HIWORD(GetKeyState(VK_SHIFT));

	if (pF->msg == WM_CHAR || pF->msg == WM_KEYDOWN || pF->msg == WM_KEYUP) {
		int key = pF->wParam;
		int vkKey = MapVirtualKey(LOBYTE(HIWORD(pF->lParam)), MAPVK_VSC_TO_VK);
		bool isKeyDown = pF->lParam & (1U << 31);

		if (hWnd == cli.hEditorWnd && ((pF->msg == WM_KEYDOWN && key == VK_UP) || (pF->msg == WM_KEYDOWN && key == VK_DOWN) || (pF->msg == WM_KEYUP && key == VK_TAB)) && isControl) {
			suggestCLIQuery(key);
			return true;
		}

		if (hWnd != hEditorWnd && key == VK_RETURN && isControl) {
			PostMessage(GetParent(hWnd), WM_COMMAND, IDC_DLG_OK, 0);
			pF->wParam = 0;
			return true;
		}

		if (key == 0x56 && isControl && !isKeyDown) { // Ctrl + V Prevent pasting other than text
			pasteText(hWnd);
			pF->wParam = 0;
			return true;
		}

		if (key == 0x5A && isControl && isShift) { // Ctrl + Shift + Z
			if (isKeyDown)
				PostMessage(hWnd, EM_REDO, 0, 0);
			pF->wParam = 0;
			return true;
		}

		if (key == VK_F2 && isShift && isKeyDown) {
			toggleTextCase(hWnd);
			pF->wParam = 0;
			return true;
		}

		if (key == VK_OEM_2 && isControl) { // Ctrl + /
			if (isKeyDown)
				toggleComment(hWnd);
			pF->wParam = 0;
			return true;
		}

		if (key == VK_TAB && !isControl && !(IsWindowVisible(hAutoComplete) && prefs::get("autocomplete-by-tab"))) {
			if (isKeyDown)
				SendMessage(hMainWnd, WM_COMMAND, IDM_PROCESS_INDENT, (LPARAM)hWnd);
			pF->wParam = 0;
			return true;
		}

		if (key == 0x57 && isControl) { // Ctrl + W
			if (isKeyDown)
				toggleWordWrap(hWnd);

			pF->wParam = 0;
			return true;
		}

		if (key == 0x4A && isControl) { // Ctrl + J
			if (isKeyDown)
				SendMessage(hMainWnd, WM_COMMAND, IDM_EDITOR_FIX_ERROR, 0);

			pF->wParam = 0;
			return true;
		}

		if ((key == 0x46 || key == 0x52) && isControl) { // Ctrl + F or Ctrl + R
			HWND hParentWnd = GetAncestor(hWnd, GA_ROOT);
			if (hParentWnd == hMainWnd && key == 0x46) {
				HWND hEdit = GetDlgItem(hEditorSearchWnd, IDC_EDITOR_SEARCH_STRING);

				TCHAR* searchText =  getCurrentText(hEditorWnd);
				SetWindowText(hEdit, searchText);
				delete [] searchText;

				if (!IsWindowVisible(hEditorSearchWnd)) {
					ShowWindow(hEditorSearchWnd, SW_SHOW);
					SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);
				}

				SendMessage(hEdit, EM_SETSEL, 0, -1);
				SetFocus(hEdit);
			} else {
				SendMessage(hParentWnd, WM_COMMAND, IDM_EDITOR_FIND, 0);
			}

			pF->wParam = 0;
			return true;
		}

		if (key == 0x54 && isControl) { // Ctrl + T
			if (isKeyDown)
				SendMessage(GetAncestor(hWnd, GA_ROOT), WM_COMMAND, IDM_EDITOR_FORMAT, 0);
			pF->wParam = 0;
			return true;
		}

		if (vkKey == VK_F3 && !isKeyDown) {
			doEditorSearch(hWnd, isShift);
			pF->wParam = 0;
			return true;
		}

		if (pF->msg == WM_CHAR && _tcschr(TEXT("(){}\"'"), key)) {
			if (wrapSelectedText(hWnd, key)) {
				pF->wParam = 0;
				return true;
			}
		}

		bool res = processAutoComplete(hWnd, key, isKeyDown);
		if (res)
			pF->wParam = 0; // Stop propagation

		return res;
	}

	if (pF->msg == WM_LBUTTONDOWN && isControl && hParentWnd == hMainWnd) {
		SetFocus(hWnd);
		POINT p{GET_X_LPARAM(pF->lParam), GET_Y_LPARAM(pF->lParam)};
		int pos = SendMessage(hWnd, EM_CHARFROMPOS, 0, (LPARAM)&p);
		SendMessage(hWnd, EM_SETSEL, pos, pos);

		// Is table?
		bool isTable = false;
		TCHAR* text = getCurrentText(hWnd);
		if (!_tcslen(text)) {
			delete [] text;
			return false;
		}

		sqlite3_stmt *stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db, "select count(*) from pragma_table_info where schema = ?2 and arg = ?1 limit 1", -1, &stmt, 0)) {
			TCHAR* tablename16 = utils::getTableName(text, false);
			TCHAR* schema16 = utils::getTableName(text, true);
			char* tablename8 = utils::utf16to8(tablename16);
			char* schema8 = utils::utf16to8(schema16);

			sqlite3_bind_text(stmt, 1, tablename8, strlen(tablename8), SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, schema8, strlen(schema8), SQLITE_TRANSIENT);

			delete [] tablename8;
			delete [] schema8;
			delete [] tablename16;
			delete [] schema16;

			if (SQLITE_ROW == sqlite3_step(stmt))
				isTable = sqlite3_column_int(stmt, 0) > 0;
		}
		sqlite3_finalize(stmt);

		if (isTable)
			openDialog(IDD_EDITDATA, (DLGPROC)&dialogs::cbDlgEditData, (LPARAM)text);

		delete [] text;
		return true;
	}

	if (pF->msg == WM_LBUTTONDOWN && !isControl) {
		if (GetMessageTime() - PtrToLong(GetProp(hWnd, TEXT("LASTCLICKTIME"))) < (LONG)GetDoubleClickTime()) {
			int crPos;
			SendMessage(hWnd, EM_GETSEL, (WPARAM)&crPos, 0);
			int lineNo = SendMessage(hWnd, EM_LINEFROMCHAR, crPos, 0);
			int lineIdx = SendMessage(hWnd, EM_LINEINDEX, lineNo, 0);
			int lineSize = SendMessage(hWnd, EM_LINELENGTH, lineIdx, 0);
			SendMessage(hWnd, EM_SETSEL, (WPARAM)lineIdx, (LPARAM)lineIdx + lineSize);

			pF->wParam = 0;
			return true;
		}
	}

	if (pF->msg == WM_LBUTTONDBLCLK) {
		SetProp(hWnd, TEXT("LASTCLICKTIME"), LongToPtr(GetMessageTime()));	// triple click support

		POINT p{GET_X_LPARAM(pF->lParam), GET_Y_LPARAM(pF->lParam)};
		int crPos = SendMessage(hWnd, EM_CHARFROMPOS, 0, (WPARAM)&p);

		int lineNo = SendMessage(hWnd, EM_LINEFROMCHAR, crPos, 0);
		int lineIdx = SendMessage(hWnd, EM_LINEINDEX, lineNo, 0);
		int lineSize = SendMessage(hWnd, EM_LINELENGTH, lineIdx, 0);
		TCHAR line[lineSize + 1]{0};
		line[0] = lineSize;
		SendMessage(hWnd, EM_GETLINE, lineNo, (LPARAM)line);

		bool isQuoteString = isControl;

		int start, end;
		if (isQuoteString) {
			for (start = crPos - lineIdx; start > 0 && _tcschr(TEXT("'\"`["), line[start - 1]) == 0; start--);
			TCHAR q = start > 0 ? line[start - 1] : 0;
			if (q == TEXT('['))
				q = TEXT(']');
			for (end = crPos - lineIdx; q && end < lineSize && line[end] != q; end++);
			isQuoteString = q && start > 0 && line[end] == q;
		}

		if (!isQuoteString) {
			for (start = crPos - lineIdx; start > 0 && (_istalnum(line[start - 1]) || line[start - 1] == TEXT('_')); start--);
			for (end = crPos - lineIdx; end < lineSize && (_istalnum(line[end]) || line[end] == TEXT('_')); end++);
		}

		if (start != end)
			SendMessage(hWnd, EM_SETSEL, start + lineIdx, end + lineIdx);

		if (hParentWnd != hMainWnd)
			SetWindowLongPtr(hParentWnd, DWLP_MSGRESULT, 1);

		pF->wParam = 0;
		return true;
	}

	return 0;
}

bool processAutoComplete(HWND hEditorWnd, int key, bool isKeyDown) {
	if (!prefs::get("use-autocomplete"))
		return false;

	TCHAR breakers[] = TEXT(" \"'`\n\r\t+-;:(),=<>!");

	// https://stackoverflow.com/questions/8161741/handling-keyboard-input-in-win32-wm-char-or-wm-keydown-wm-keyup
	// Shift + 7 and Shift + 9 equals to up and down
	bool isByTab = prefs::get("autocomplete-by-tab");
	bool isNavKey = !HIWORD(GetKeyState(VK_SHIFT)) && (
		key == VK_ESCAPE ||
		key == VK_UP || key == VK_DOWN ||
		key == VK_HOME || key == VK_END ||
		key == VK_PRIOR || key == VK_NEXT ||
		(key == VK_RETURN && !isByTab) ||
		(key == VK_TAB && isByTab));
	bool isCtrlSpace = HIWORD(GetKeyState(VK_CONTROL)) && (key == VK_SPACE);

	if (IsWindowVisible(hAutoComplete) && isNavKey) {
		if (isKeyDown) {
			if (key == VK_ESCAPE)
				ShowWindow(hAutoComplete, SW_HIDE);

			int iCount = ListBox_GetCount(hAutoComplete);
			if (key == VK_UP || key == VK_DOWN) {
				int pos = ListBox_GetCurSel(hAutoComplete);
				ListBox_SetCurSel(hAutoComplete, (pos + (key == VK_UP ? -1 : 1) + iCount) % iCount);
			}

			if ((key == VK_RETURN && !isByTab) || (key == VK_TAB && isByTab)) {
				TCHAR buf[1024]{0}, ins[1031]{0};
				int idx = ListBox_GetCurSel(hAutoComplete);
				ListBox_GetText(hAutoComplete, idx, buf);
				SendMessage(hEditorWnd, WM_SETREDRAW, FALSE, 0);

				bool isSchemaTable = _tcschr(buf, TEXT('.')) != 0;
				bool isNeedQuotes = _istdigit(buf[0]); // add double quotes around the insertion
				for (int i = 0; !isNeedQuotes && (i < (int)_tcslen(buf)); i++)
					isNeedQuotes = !_istalnum(buf[i]) && (buf[i] != TEXT('_'));
				for (int i = 0; TEMPLATES[i] && isNeedQuotes; i++)
					isNeedQuotes = _tcsicmp(buf, TEMPLATES[i]) != 0;

				bool isFunc = false; // add parentheses at the end and place a cursor between them
				for (int i = 0; FUNCTIONS[i] && !isFunc; i++)
					isFunc = _tcsicmp(buf, FUNCTIONS[i]) == 0;

				bool isKeyword = false; // add extra space at the end
				for (int i = 0; SQL_KEYWORDS[i] && !isKeyword; i++)
					isKeyword = _tcsicmp(buf, SQL_KEYWORDS[i]) == 0;
				for (int i = 0; TEMPLATES[i] && !isKeyword; i++)
					isKeyword = _tcsicmp(buf, TEMPLATES[i]) == 0;

				if (isKeyword && isFunc) // ambiguous case e.g. "replace"
					isKeyword = false;

				int crStart, crEnd;
				SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&crStart, (LPARAM)&crEnd);

				int currLineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, -1, 0);
				int currLineSize = SendMessage(hEditorWnd, EM_LINELENGTH, crStart, 0);
				int currLineIdx = SendMessage(hEditorWnd, EM_LINEINDEX, -1, 0);
				TCHAR currLine[currLineSize + 1]{0};
				currLine[0]  = currLineSize;

				SendMessage(hEditorWnd, EM_GETLINE, currLineNo, (LPARAM)currLine);
				int pos = crStart - currLineIdx;
				int start = pos;
				while(start > 0 && !_tcschr(breakers, currLine[start - 1]) && currLine[start - 1] != TEXT('.'))
					start--;
				int end = pos;
				while(end < (int)_tcslen(currLine) && !_tcschr(breakers, currLine[end]) && currLine[end] != TEXT('.'))
					end++;

				int endLen = end - pos;
				bool isEnded = (endLen < (int)_tcslen(buf)) && (_tcsnicmp(currLine + pos, buf + _tcslen(buf) - endLen, endLen) == 0);
				SendMessage(hEditorWnd, EM_SETSEL, currLineIdx + start, currLineIdx + (!isEnded || isNeedQuotes ? pos : end));

				if (isFunc && currLine[end] == TEXT('(')) // disable to add () at the end if they are exist
					isFunc = false;

				if (isSchemaTable) {
					TCHAR* tablename16 = utils::getTableName(buf, false);
					TCHAR* schema16 = utils::getTableName(buf, true);
					TCHAR* table16 = utils::getFullTableName(schema16, tablename16);
					_sntprintf(ins, 1030, TEXT("%ls"), table16);
					delete [] table16;
					delete [] schema16;
					delete [] tablename16;
				} else if (isNeedQuotes)
					_sntprintf(ins, 1030, TEXT("\"%ls\""), buf);
				else if (isFunc)
					_sntprintf(ins, 1030, TEXT("%ls()"), buf);
				else if (isKeyword)
					_sntprintf(ins, 1030, TEXT("%ls "), buf);
				else
					_sntprintf(ins, 1030, TEXT("%ls"), buf);

				SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (LPARAM)ins);
				int crPos = currLineIdx + start + _tcslen(ins);
				if (isFunc)
					crPos = crPos - 1;
				SendMessage(hEditorWnd, EM_SETSEL, crPos, crPos);

				SendMessage(hEditorWnd, WM_SETREDRAW, TRUE, 0);
				ShowWindow(hAutoComplete, SW_HIDE);
			}

			if (key == VK_HOME || key == VK_END || key == VK_NEXT || key == VK_PRIOR) {
				int itemNo = 0;

				if (key == VK_HOME)
					itemNo = 0;

				if (key == VK_END)
					itemNo = iCount - 1;

				if (key == VK_NEXT || key == VK_PRIOR) {
					RECT rc;
					GetClientRect(hAutoComplete, &rc);
					int iPage = rc.bottom/ListBox_GetItemHeight(hAutoComplete, 0);
					int currNo = ListBox_GetCurSel(hAutoComplete);
					itemNo = key == VK_NEXT ? MIN(iCount - 1, currNo + iPage) : MAX(0, currNo - iPage);
				}

				ListBox_SetCurSel(hAutoComplete, itemNo);
				SendMessage(hAutoComplete, LB_SETTOPINDEX, itemNo, 0);
			}
		}

		if (!isKeyDown && ListBox_GetCount(hAutoComplete) == 1 && (key == VK_UP || key == VK_DOWN)) {
			ShowWindow(hAutoComplete, SW_HIDE);
			return false;
		}

		return true;
	}

	if (!isCtrlSpace && (!isKeyDown || isNavKey))
		return false;

	if (isCtrlSpace && !isKeyDown)
		return true;

	if (IsWindowVisible(hAutoComplete) && key == VK_CONTROL && isKeyDown)
		return true;

	if (isCtrlSpace && IsWindowVisible(hTooltipWnd))
		return true;

	int crStart, crEnd;
	SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&crStart, (WPARAM)&crEnd);
	if (!isCtrlSpace && crStart != crEnd) {
		ShowWindow(hAutoComplete, SW_HIDE);
		return false;
	}

	int crPos = crStart;

	int currLineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, -1, 0);
	int currLineLen = SendMessage(hEditorWnd, EM_LINELENGTH, crPos, 0);
	int currLineIdx = SendMessage(hEditorWnd, EM_LINEINDEX, -1, 0);
	TCHAR currLine[currLineLen + 1]{0};
	currLine[0] = currLineLen;
	SendMessage(hEditorWnd, EM_GETLINE, currLineNo, (LPARAM)currLine);
	size_t start = crPos - currLineIdx;
	size_t end = start;

	// Auto-complete reads the beginning of the word and ignores next chars
	while(start > 0 && !_tcschr(breakers, currLine[start - 1]))
		start--;

	TCHAR word[end - start + 1] = {0};
	for (size_t i = 0; i < end - start; i++)
		word[i] = currLine[start + i];

	// word2 is a full word. Auto-complete shouldn't show it.
	int end2 = end;
	while(end2 < (int)_tcslen(currLine) && !_tcschr(breakers, currLine[end2]))
		end2++;
	TCHAR word2[end2 - start + 1] = {0};
	for (size_t i = 0; i < end2 - start; i++)
		word2[i] = currLine[start + i];

	size_t wPos = crPos - currLineIdx - start; // Cursor position in the word

	ListBox_ResetContent(hAutoComplete);
	size_t wLen = _tcslen(word);

	const HWND hWnd = hAutoComplete;
	auto addString = [&word, &word2, wLen, hWnd, wPos, isCtrlSpace](const TCHAR* str, bool noEntryCheck = false) {
		if (_tcsnicmp(str, word, wLen) == 0 && _tcslen(str) == wLen)
			return 1;

		if (_tcsicmp(str, word2) == 0)
			return 1;

		if (_tcsstr(word2, str)) // disable shows existing words e.g. "table" in "tables"
			return 0;

		if (_tcsstr(str, word) == str && _tcscmp(word, word2)) {
			TCHAR* ending = word2 + MAX(wPos - 1, 0);
			bool isEnding = true;
			int eLen = _tcslen(ending);
			int sLen = _tcslen(str);
			for (int i = 0; i < eLen && isEnding; i++)
				isEnding = isEnding && (str[sLen - i] ==  ending[eLen - i]);

			if (!isEnding)
				return 0;
		}

		TCHAR* dotPos = _tcsrchr(word, TEXT('.'));
		if (dotPos && !_tcsicmp(str, dotPos + 1))
			return 1;

		if(!(noEntryCheck || (!_tcsnicmp(str, word, wLen) && _tcslen(str) != wLen && (wPos == wLen || isCtrlSpace))))
			return 0;

		TCHAR str2[_tcslen(str) + 1]{0};
		_tcscpy(str2, str);
		if (!noEntryCheck && wLen > 0 && word[0] == _totupper(word[0])) {
			if (wLen == 1 || (wLen > 1 && word[1] == _totupper(word[1])))
				_tcsupr(str2);
			else
				str2[0] = _totupper(str2[0]);
		}

		if (ListBox_FindStringExact(hWnd, -1, str2) != -1)
			return 0;

		ListBox_AddString(hWnd, str2);
		return 0;
	};

	int isExact = 0;
	// tablename. or aliasname.
	TCHAR* dotPos = _tcsrchr(word, TEXT('.'));
	if (dotPos && wPos == wLen) {
		TCHAR prefix[_tcslen(word) - _tcslen(dotPos) + 1]{0};
		_tcsncpy(prefix, word, _tcslen(word) - _tcslen(dotPos));
		size_t prefixLen = _tcslen(prefix);

		if (prefixLen > 0) {
			size_t tLen = GetWindowTextLength(hEditorWnd);
			TCHAR text[tLen + 1]{0};
			GetWindowText(hEditorWnd, text, tLen + 1);
			_tcslwr(text);

			for (int i = 0; TABLES[i]; i++)	{
				bool isSuitable = _tcslen(TABLES[i]) == prefixLen && !_tcsicmp(TABLES[i], prefix);

				size_t tbl_aLen = _tcslen(TABLES[i]) + prefixLen;
				TCHAR tbl_a[tbl_aLen + 1 + 1 + 2 + 4  /* as */] {0}; // +1 - space, +1 - \0, +2 for quotes
				TCHAR tbl_t[][20] = {
					TEXT("%ls %ls"), TEXT("\"%ls\" %ls"), TEXT("'%ls' %ls"), TEXT("[%ls] %ls"), TEXT("`%ls` %ls"),
					TEXT("%ls as %ls"), TEXT("\"%ls\" as %ls"), TEXT("'%ls' as %ls"), TEXT("[%ls] as %ls"), TEXT("`%ls` as %ls")
				};
				for (int j = 0; j < 10 && !isSuitable; j++) {
					_sntprintf(tbl_a, tbl_aLen + 4 + 4 /* as */, tbl_t[j], TABLES[i], prefix); // tablename alias
					_tcslwr(tbl_a);

					TCHAR* p = _tcsstr(text, tbl_a);
					isSuitable = p && // found
						(_tcslen(p) == tLen || _tcschr(breakers, (p - 1)[0])) && // not xxxtablename alias
						((p + tbl_aLen)[0] == 0 || _tcschr(breakers, (p + tbl_aLen + (j == 0 ? 1 : j < 5 ? 3 : j == 5 ? 4 : 6))[0])); // not tablename aliasxxx
				}

				if (isSuitable) {
					TCHAR query16[512] = {0};
					_sntprintf(query16, 511, TEXT("select name from pragma_table_info('%ls') where name like '%ls%%' and name <> '%ls' order by cid"), TABLES[i], dotPos + 1, dotPos + 1);
					char* query8 = utils::utf16to8(query16);

					sqlite3_stmt *stmt;
					sqlite3_prepare_v2(db, query8, -1, &stmt, 0);
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* column16 =  utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
						int pos = ListBox_AddString(hAutoComplete, column16);
						ListBox_SetItemData(hAutoComplete, pos, 0);
						delete [] column16;
					}
					sqlite3_finalize(stmt);

					delete [] query8;
					break;
				}
			}
		}

		sqlite3_stmt *stmt;
		TCHAR query16[512] = {0};
		_sntprintf(query16, 511, TEXT("select name from (select name from \"%ls\".sqlite_master where type in ('table', 'view') union select 'sqlite_master') where name like '%ls%%'"), prefix, dotPos + 1);
		char* query8 = utils::utf16to8(query16);
		int rc = sqlite3_prepare_v2(db, query8, -1, &stmt, 0);
		while ((rc == SQLITE_OK) && (SQLITE_ROW == sqlite3_step(stmt))) {
			TCHAR* name16 = utils::utf8to16((char *) sqlite3_column_text(stmt, 0));
			isExact += addString(name16, true);
			delete [] name16;
		}
		sqlite3_finalize(stmt);
		delete [] query8;
	}

	// Columns
	int tLen = GetWindowTextLength(hEditorWnd);
	if (tLen < MAX_TEXT_LENGTH) {
		TCHAR text[tLen + 1];
		GetWindowText(hEditorWnd, text, tLen + 1);
		_tcslwr(text);

		int cPos = crPos;
		// fix new line \r \n
		for (int i = 0; (i < cPos) && (i < tLen + 1); i++) {
			if (text[i] == TEXT('\r'))
				cPos = cPos + 1;
		}

		//                    qStart                                    qEnd
		//                    |                                         |
		// where discount > 0;   select id,   nam| from customers where ;
		//                                 |     |
		//                              tPos     cPos

		int tPos = cPos;
		while (tPos > 0 && _tcschr(TEXT("\"'`,\r\n\t()[]; "), text[tPos - 1]) == 0)
			tPos--;

		while (tPos > 0 && _istgraph(text[tPos - 1]) == 0)
			tPos--;

		TCHAR breakers[] = TEXT("\"'`,\r\n\t)] ");
		if (wLen || (cPos > 6 && _tcschr(TEXT(" \r\n;"), text[cPos]) != NULL && ((_tcschr(breakers, text[tPos]) != 0 && tPos != cPos) || text[tPos - 1] == TEXT('(')) && (
			utils::isStartBy(text, tPos - 1, TEXT(",")) ||
			utils::isStartBy(text, tPos - 1, TEXT("(")) ||
			utils::isStartBy(text, tPos - 1, TEXT(">")) ||
			utils::isStartBy(text, tPos - 1, TEXT("<")) ||
			utils::isStartBy(text, tPos - 1, TEXT("=")) ||
			(utils::isStartBy(text, tPos - 4, TEXT("when")) && _tcschr(breakers, text[tPos - 5]) != 0) ||
			(utils::isStartBy(text, tPos - 4, TEXT("else")) && _tcschr(breakers, text[tPos - 5]) != 0) ||
			(utils::isStartBy(text, tPos - 3, TEXT("and")) && _tcschr(breakers, text[tPos - 4]) != 0) ||
			(utils::isStartBy(text, tPos - 2, TEXT("or")) && _tcschr(breakers, text[tPos - 3]) != 0) ||
			(utils::isStartBy(text, tPos - 2, TEXT("on")) && _tcschr(breakers, text[tPos - 3]) != 0) ||
			(utils::isStartBy(text, tPos - 6, TEXT("select")) && (tPos < 7 || _tcschr(breakers, text[tPos - 7]) != 0)) ||
			(utils::isStartBy(text, tPos - 5, TEXT("where")) && _tcschr(breakers, text[tPos - 6]) != 0) ||
			(utils::isStartBy(text, tPos - 8, TEXT("order by"))&& _tcschr(breakers, text[tPos - 9]) != 0) ||
			(utils::isStartBy(text, tPos - 8, TEXT("group by"))&& _tcschr(breakers, text[tPos - 9]) != 0) ||
			(utils::isStartBy(text, tPos - 6, TEXT("having"))&& _tcschr(breakers, text[tPos - 7]) != 0)
		))) {
			int nParentheses = 0;
			int qStart = cPos;
			while (qStart > 0) {
				if (text[qStart] == TEXT(')'))
					nParentheses++;
				if (text[qStart] == TEXT('(') && nParentheses > 0)
					nParentheses--;

				if (nParentheses >= 0 && (utils::isStartBy(text, qStart, TEXT("select")) || text[qStart - 1] == TEXT(';')))
					break;

				qStart--;
			}

			nParentheses = 0;
			int qEnd = cPos;
			while (qEnd < tLen) {
				if (text[qEnd] == TEXT('('))
					nParentheses++;
				if (text[qEnd] == TEXT(')') && nParentheses > 0)
					nParentheses--;

				if (nParentheses >= 0 && (utils::isStartBy(text, qEnd, TEXT("select")) || text[qEnd] == TEXT(';')))
					break;

				qEnd++;
			}

			TCHAR query16[qEnd - qStart + 1]{0};
			_tcsncpy(query16, text + qStart, qEnd - qStart);
			_tcslwr(query16);

			if (_tcslen(query16) > 0) {
				auto hasWord = [](TCHAR* str, TCHAR* word) {
					bool res = false;

					TCHAR breakers[] = TEXT(" \"'`\n\t+-;:(),=<>/%");
					TCHAR* s = _tcsstr(str, word);
					int slen = _tcslen(str);
					int wlen = _tcslen(word);

					while (!res && s) {
						int l = slen - _tcslen(s);
						int r = l + wlen;
						res = (l == 0 || (l > 0 && _tcschr(breakers, str[l - 1]))) && (r == slen || (r < slen && _tcschr(breakers, str[r])));
						s = _tcsstr(s + 1, word);
					}

					return res;
				};

				// Each table is processed separately because pragma_table_xinfo-command can raise an error
				// hasWord-function is used instead of regexp \b<tbl_name>\b because tbl_name can contain special char for regexp e.g. $
				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select lower(tbl_name) from main.sqlite_master where type in ('table', 'view') and sql is not null order by tbl_name", -1, &stmt, 0)) {
					bool isNoCheck = wLen == 0;

					while (SQLITE_ROW == sqlite3_step(stmt)) {
						const char* tblName8 = (const char*) sqlite3_column_text(stmt, 0);
						TCHAR* tblName16 = utils::utf8to16(tblName8);
						if (hasWord(query16, tblName16)) {
							sqlite3_stmt *stmt2;
							if (SQLITE_OK == sqlite3_prepare_v2(db, "select name from pragma_table_xinfo where schema = 'main' and arg = ?1", -1, &stmt2, 0)) {
								sqlite3_bind_text(stmt2, 1, tblName8, -1, SQLITE_TRANSIENT);

								while (SQLITE_ROW == sqlite3_step(stmt2)) {
									TCHAR* name16 = utils::utf8to16((char *) sqlite3_column_text(stmt2, 0));
									isExact += addString(name16, isNoCheck);
									delete [] name16;
								}
							}
							sqlite3_finalize(stmt2);
						}

						delete [] tblName16;
					}
				}
				sqlite3_finalize(stmt);
			}
		}
	}

	if (start > 6 && utils::isStartBy(currLine, start - 7, TEXT("pragma"))) {
		for (int i = 0; PRAGMAS[i]; i++)
			isExact += addString(PRAGMAS[i]);
	} else if (
		((wLen == 0 && _tcschr(TEXT("\r\n\t ("), currLine[start]) != 0) || wLen > 0) &&
		((start > 2 && utils::isStartBy(currLine, start - 3, TEXT("on"))) ||
		(start > 4 && currLine[start] != TEXT('(') && (
			utils::isStartBy(currLine, start - 5, TEXT("from")) ||
			utils::isStartBy(currLine, start - 5, TEXT("join")) ||
			utils::isStartBy(currLine, start - 5, TEXT("into"))
		)) ||
		(start > 6 && utils::isStartBy(currLine, start - 7, TEXT("update"))))) {
		bool isNoCheck = wLen == 1 && word[0] == TEXT(' ');
		for (int i = 0; TABLES[i]; i++)
			isExact += addString(TABLES[i], isNoCheck);

		sqlite3_stmt *stmt;
		int rc = sqlite3_prepare_v2(db, "select name from pragma_database_list() union all select name from pragma_module_list where name not like 'pragma_%'", -1, &stmt, 0);
		while ((rc == SQLITE_OK) && (SQLITE_ROW == sqlite3_step(stmt))) {
			TCHAR* name16 = utils::utf8to16((char *) sqlite3_column_text(stmt, 0));
			isExact += addString(name16, isNoCheck);
			delete [] name16;
		}
		sqlite3_finalize(stmt);

		TCHAR buf[256];
		for (int i = 0; PRAGMAS[i]; i++) {
			_sntprintf(buf, 255, TEXT("pragma_%ls"), PRAGMAS[i]);
			isExact += addString(buf, isNoCheck);
		}
	} else if (wLen > 1) {
		for (int i = 0; FUNCTIONS[i]; i++)
			isExact += addString(FUNCTIONS[i]);

		for (int i = 0; SQL_KEYWORDS[i]; i++)
			isExact += addString(SQL_KEYWORDS[i]);

		for (int i = 0; TEMPLATES[i]; i++)
			isExact += addString(TEMPLATES[i]);

		for (int i = 0; TABLES[i]; i++)
			isExact += addString(TABLES[i]);
	}

	if (isExact && !isCtrlSpace)
		ListBox_ResetContent(hAutoComplete);

	int h = SendMessage(hAutoComplete, LB_GETITEMHEIGHT, 0, 0);
	int iCount = ListBox_GetCount(hAutoComplete);

	if (iCount) {
		POINT p = {0};
		GetCaretPos(&p);
		ClientToScreen(hEditorWnd, &p);

		RECT rc = {0};
		GetWindowRect(hAutoComplete, &rc);
		SetWindowPos(hAutoComplete, 0, p.x, p.y + utils::getTextSize(hFont, TEXT("Wg")).cy, rc.right - rc.left, MIN(h * iCount + 5, 150), SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowLongPtr(hAutoComplete, GWLP_USERDATA, (LONG_PTR)hEditorWnd);
		SetWindowLongPtr(hEditorWnd, GWLP_USERDATA, MAKELONG(currLineIdx + (dotPos && _tcslen(dotPos) != wLen ? end - _tcslen(dotPos) + 1 : start), currLineIdx + end));
	}

	ListBox_SetCurSel(hAutoComplete, 0);
	ShowWindow(hAutoComplete, iCount ? SW_SHOWNOACTIVATE : SW_HIDE);
	SetFocus(hEditorWnd);

	return false;
}

LRESULT CALLBACK cbNewAutoComplete(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_SHOWWINDOW: {
			if (!wParam)
				ShowWindow(hAutoCompleteHelp, SW_HIDE);
		}
		break;

		case LB_SETCURSEL:
		case LB_SETSEL: {
			LRESULT rc = CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);

			if (prefs::get("disable-autocomplete-help"))
				return rc;

			int itemNo = ListBox_GetCurSel(hWnd);

			TCHAR* help16 = 0;
			if (itemNo != -1) {
				TCHAR word16[1024];
				ListBox_GetText(hWnd, itemNo, word16);

				HWND hEditorWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
				TCHAR* prefix16 = getCurrentText(hEditorWnd);
				TCHAR altword16[1024] = {0};
				int len = _tcslen(prefix16);
				if (len > 0 && prefix16[len - 1] == TEXT('.'))
					_sntprintf(altword16, 1023, TEXT("%ls%ls"), prefix16, word16);
				delete [] prefix16;

				help16 = getHelp(word16, altword16);
			}
			SetWindowText(hAutoCompleteHelp, help16);

			if (help16) {
				RECT rc, rch, rcch;
				GetWindowRect(hWnd, &rc);
				GetWindowRect(hAutoCompleteHelp, &rch);
				GetClientRect(hAutoCompleteHelp, &rcch);

				int w = 2 * (rc.right - rc.left);
				int bw = (rch.right - rch.left) - rcch.right;
				int bh = (rch.bottom - rch.top) - rcch.bottom;

				rcch.right = w - bw - 20;
				rcch.bottom = 5;

				HDC hDC = GetDC(hAutoCompleteHelp);
				HFONT hOldFont = (HFONT) SelectObject(hDC, hFont);
				DrawText(hDC, help16, -1, &rcch, DT_LEFT | DT_MODIFYSTRING | DT_WORDBREAK | DT_CALCRECT);
				SelectObject(hDC, hOldFont);
				ReleaseDC(hAutoCompleteHelp, hDC);

				SetWindowPos(hAutoCompleteHelp, 0, rc.right + 10, rc.top, w, rcch.bottom + 10 + bh, SWP_NOZORDER | SWP_NOACTIVATE);
				ShowWindow(hAutoCompleteHelp, SW_SHOWNOACTIVATE);
				InvalidateRect(hAutoCompleteHelp, NULL, TRUE);

				delete [] help16;
			} else {
				ShowWindow(hAutoCompleteHelp, SW_HIDE);
			}
		}
		break;

		case WM_LBUTTONDOWN: {
			LRESULT res = CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
			SendMessage(hWnd, LB_SETCURSEL, ListBox_GetCurSel(hWnd), 0);
			HWND hEditorWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			SetFocus(hEditorWnd);
			return res;
		}
		break;
	}

	if (msg == WM_LBUTTONDBLCLK || (msg == WM_KEYUP && wParam == VK_RETURN) || (msg == WM_KEYUP && wParam == VK_ESCAPE)) {
		HWND hEditorWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		int rc = processAutoComplete(hEditorWnd, wParam == VK_ESCAPE ? VK_ESCAPE : VK_RETURN, true);
		SetFocus(hEditorWnd);
		return rc;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewAutoCompleteHelp(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_ERASEBKGND: {
			SetDCBrushColor((HDC)wParam, SendMessage(hTooltipWnd, TTM_GETTIPBKCOLOR, 0, 0));

			RECT rc{0};
			GetClientRect(hWnd, &rc);
			FillRect((HDC)wParam, &rc, (HBRUSH)GetStockObject(DC_BRUSH));

			return 1;
		}
		break;

		case WM_PAINT: {
			PAINTSTRUCT ps{0};
			ps.fErase = true;
			HDC hDC = BeginPaint(hWnd, &ps);

			TCHAR val16[2048];
			GetWindowText(hWnd, val16, 2047);

			RECT rc;
			GetClientRect(hWnd, &rc);
			rc.left = 10;
			rc.top = 5;
			rc.right -= 10;
			rc.bottom -= 5;

			HFONT hOldFont = (HFONT) SelectObject(hDC, hFont);
			SetBkColor(hDC, SendMessage(hTooltipWnd, TTM_GETTIPBKCOLOR, 0, 0));
			DrawText(hDC, val16, -1, &rc, DT_LEFT | DT_MODIFYSTRING | DT_WORDBREAK );
			SelectObject(hDC, hOldFont);
			EndPaint(hWnd, &ps);

			return 1;
		}
		break;

		case WM_NCHITTEST: {
			return HTCLIENT;
		}
		break;

		case WM_LBUTTONDOWN: {
			ShowWindow(hWnd, SW_HIDE);
			SetFocus(hEditorWnd);
			return TRUE;
		}
		break;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewAiChatMessages (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_PAINT: {
			if (GetWindowTextLength(hWnd))
				return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);

			PAINTSTRUCT ps {0};
			ps.fErase = FALSE;
			HDC hDC = BeginPaint(hWnd, &ps);

			RECT rc;
			GetClientRect(hWnd, &rc);

			FillRect(hDC, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

			const TCHAR hi16[] = TEXT("Hi, I'm AI SQL assistant.\nInput your request below.");
			HFONT hOldFont = (HFONT) SelectObject(hDC, hFont);
			RECT rc2 {0, 0, rc.right, 0};
			DrawText(hDC, hi16, -1, &rc2, DT_TOP | DT_WORDBREAK | DT_CENTER | DT_CALCRECT);

			rc.top = rc.bottom - rc2.bottom - 10;
			DrawText(hDC, hi16, -1, &rc, DT_CENTER | DT_MODIFYSTRING | DT_WORDBREAK);
			SelectObject(hDC, hOldFont);
			EndPaint(hWnd, &ps);

			return TRUE;
		}
		break;

		case WM_KEYDOWN: {
			bool isCtrl = HIWORD(GetKeyState(VK_CONTROL));
			if (LOWORD(wParam) == 0x57 && isCtrl)  // Ctrl + W
				SendMessage(GetParent(hWnd), WMU_TOGGLE_AI_WORD_WRAP, 0, 0);

			if (LOWORD(wParam) == 0x43 && isCtrl)  {// Ctrl + C
				int crStart, crEnd;
				SendMessage(hWnd, EM_GETSEL, (WPARAM)&crStart, (LPARAM)&crEnd);
				if (crStart == crEnd)
					return TRUE;

				int len = crEnd - crStart;
				TCHAR* text16 = new TCHAR[len + 1]{0};
				TEXTRANGE tr{{crStart, crEnd}, text16};
				SendMessage(hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
				utils::setClipboardText(text16);
				delete [] text16;

				return TRUE;
			}
		}
		break;

		case WM_MOUSEWHEEL: {
			if (LOWORD(wParam) == MK_CONTROL) {
				SendMessage(hMainWnd, WM_MOUSEWHEEL, wParam, lParam);
				return 1;
			}
		}
		break;

		case WM_RBUTTONDOWN: {
			POINT p;
			GetCursorPos(&p);
			TrackPopupMenu(hAiChatMenu, 0, p.x, p.y, 0, hMainWnd, NULL);
		}
		break;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewAiChatInput (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KEYDOWN: {
			bool isCtrl = HIWORD(GetKeyState(VK_CONTROL));
			if (wParam == VK_RETURN && !isCtrl) {
				SendMessage(GetParent(hWnd), WM_COMMAND, IDM_AI_SEND_MESSAGE, 0);
				return 1;
			}

			if (LOWORD(wParam) == 0x57 && isCtrl)  // Ctrl + W
				SendMessage(GetParent(hWnd), WMU_TOGGLE_AI_WORD_WRAP, 0, 0);
		}
		break;

		case WM_MOUSEWHEEL: {
			if (LOWORD(wParam) == MK_CONTROL) {
				SendMessage(hMainWnd, WM_MOUSEWHEEL, wParam, lParam);
				return 1;
			}
		}
		break;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewAiChat(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HWND hMessagesWnd = GetDlgItem(hWnd, IDC_AI_CHAT_MESSAGES);
	HWND hInputWnd = GetDlgItem(hWnd, IDC_AI_CHAT_INPUT);
	const int BTN_WIDTH = 30;

	switch (msg) {
		case WM_PAINT: {
			PAINTSTRUCT ps{0};
			HDC hDC = BeginPaint(hWnd, &ps);

			if (ps.rcPaint.right == 0) {
				EndPaint(hWnd, &ps);
				return true;
			}

			int tabH = PtrToInt(GetProp(hWnd, TEXT("TAB_HEIGHT")));
			bool isMaximized = prefs::get("ai-chat-maximized");

			RECT rc {0};
			GetClientRect(hWnd, &rc);
			int w = rc.right;

			HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			FillRect(hDC, &rc, hBrush);
			DeleteObject(hBrush);
			if (isMaximized) {
				RECT rc = {0, 0, BTN_WIDTH, tabH};
				hBrush = CreateSolidBrush(RGB(255, 255, 255));
				FillRect(hDC, &rc, hBrush);
				DeleteObject(hBrush);
			}

			HFONT hOldFont = (HFONT)SelectObject(hDC, hMenuFont);
			HPEN hBorderPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_GRAYTEXT));

			SetTextColor(hDC, RGB(0, 0, 0));
			const TCHAR* CAPTIONS16[4] = { isMaximized ? TEXT("-->") : TEXT("<--"), TEXT("<"), TEXT(">"), TEXT("+")};

			SelectObject(hDC, hBorderPen);
			MoveToEx(hDC, 0, tabH - 1, NULL);
			LineTo(hDC, w, tabH - 1);

			for (int btnNo = 0; btnNo < 4; btnNo++) {
				COLORREF bgColor = isMaximized && btnNo == 0 ? RGB(255, 255, 255) : GetSysColor(COLOR_BTNFACE);
				SetBkColor(hDC, bgColor);
				RECT rc = {btnNo * BTN_WIDTH, 3, (btnNo + 1) * BTN_WIDTH, tabH + 2};
				DrawText(hDC, CAPTIONS16[btnNo], -1, &rc, DT_CENTER | DT_BOTTOM);

				MoveToEx(hDC, btnNo * BTN_WIDTH, tabH - 1, NULL);
				LineTo(hDC, btnNo * BTN_WIDTH, 0);
				LineTo(hDC, (btnNo + 1) * BTN_WIDTH, 0);
				LineTo(hDC, (btnNo + 1) * BTN_WIDTH, tabH - 1);
			}

			DeleteObject(hBorderPen);
			SelectObject(hDC, hOldFont);
			EndPaint(hWnd, &ps);

			return true;
		}
		break;

		case WM_SIZE: {
			RECT rc;
			GetClientRect(hWnd, &rc);
			int ih = 100;
			int tabH = PtrToInt(GetProp(hWnd, TEXT("TAB_HEIGHT")));

			SetWindowPos(hMessagesWnd, 0, 0, tabH, rc.right, rc.bottom - ih - 5 - tabH, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(hInputWnd, 0, 0, rc.bottom - ih, rc.right, ih, SWP_NOACTIVATE | SWP_NOZORDER);

			rc = {0, 0, rc.right, tabH};
			InvalidateRect(hWnd, &rc, FALSE);

			return 1;
		}
		break;

		case WM_LBUTTONDOWN: {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			int tabH = PtrToInt(GetProp(hWnd, TEXT("TAB_HEIGHT")));

			if (x > 0 && x < 4 * BTN_WIDTH && y > 0 && y < tabH)
				SendMessage(hWnd, WM_COMMAND, IDM_AI_COMMAND + x / BTN_WIDTH, 0);
		}
		break;

		case WM_MOUSEMOVE: {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			int tabH = PtrToInt(GetProp(hWnd, TEXT("TAB_HEIGHT")));
			int prevTabNo = PtrToInt(GetProp(hWnd, TEXT("TABNO")));
			int tabNo = x > 0 && x < 4 * BTN_WIDTH && y > 0 && y < tabH ? x / BTN_WIDTH + 1 : 0;

			if ((prevTabNo == 0 && tabNo == 0) || prevTabNo == tabNo)
				return 0;

			if (tabNo == 0)
				return SendMessage(hWnd, WM_MOUSELEAVE, 0, 0);

			if (prevTabNo != tabNo) {
				HWND hTooltipWnd = (HWND)GetProp(hWnd, TEXT("TOOLTIP"));
				TOOLINFO ti{0};
				ti.cbSize = TTTOOLINFOW_V1_SIZE;
				SendMessage(hTooltipWnd, TTM_ENUMTOOLS, 0, (LPARAM)&ti);

				TCHAR title16[5][128] = {
					TEXT("ERROR"),
					TEXT("Increase/Reduce the AI Chat area"),
					TEXT("Load a previous chat"),
					TEXT("Load a next chat"),
					TEXT("Start a new chat")
				};

				POINT p {(int)((tabNo - 0.25) * BTN_WIDTH * 1.), tabH + 5};
				ClientToScreen(hWnd, &p);

				ti.lpszText = tabNo < 5 ? title16[tabNo] : title16[0];
				SendMessage(hTooltipWnd, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
				SendMessage(hTooltipWnd, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(p.x, p.y));
				SendMessage(hTooltipWnd, TTM_TRACKACTIVATE, TRUE, (LPARAM)(LPTOOLINFO) &ti);

				TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, hWnd, 0};
				TrackMouseEvent(&tme);
				SetProp(hWnd, TEXT("TABNO"), IntToPtr(tabNo));
			}
		}
		break;

		case WM_MOUSELEAVE: {
			HWND hTooltipWnd = (HWND)GetProp(hWnd, TEXT("TOOLTIP"));
			SendMessage(hTooltipWnd, TTM_TRACKACTIVATE, FALSE, 0);
			SetProp(hWnd, TEXT("TABNO"), 0);
		}
		break;

		case WM_COMMAND: {
			WORD cmd = LOWORD(wParam);

			if (cmd == IDM_AI_COMMAND) {
				prefs::set("ai-chat-maximized", !prefs::get("ai-chat-maximized"));
				SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);
			}

			if (cmd == IDM_AI_COMMAND + 1 || cmd == IDM_AI_COMMAND + 2) {
				int sid = prefs::get("ai-chat-sid");

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, cmd == IDM_AI_COMMAND + 1 ?
					"select coalesce(max(sid), ?1) from ai_messages where sid < iif(?1 > 0, ?1, sid + 1) and json_extract(details, '$.prompt-id') = 'chat-prompt' order by sid" : // prev
					"select coalesce(min(sid), ?1) from ai_messages where sid > iif(?1 > 0, ?1, sid) and json_extract(details, '$.prompt-id') = 'chat-prompt' order by sid",  // next
					-1, &stmt, 0)) {
					sqlite3_bind_int(stmt, 1, sid);

					if (SQLITE_ROW == sqlite3_step(stmt)) {
						int newSid = sqlite3_column_int(stmt, 0);
						if (newSid != sid) {
							prefs::set("ai-chat-sid", newSid);
							SendMessage(hWnd, WMU_LOAD_AI_CHAT, 0, 0);
						}
					}
				}
				sqlite3_finalize(stmt);
			}

			if (cmd == IDM_AI_COMMAND + 3) {
				prefs::set("ai-chat-sid", 0);
				SetWindowText(hMessagesWnd, 0);
				SetFocus(hInputWnd);
			}

			if (cmd == IDM_AI_SEND_MESSAGE) {
				GETTEXTLENGTHEX gtl = {GTL_NUMCHARS, 0};
				int len = SendMessage(hInputWnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 1200);
				TCHAR* query16 = new TCHAR[len + 1] {0};

				GETTEXTEX gt = {0};
				gt.flags = 0;
				gt.codepage = 1200;
				gt.cb = (len + 1) * sizeof(TCHAR);
				SendMessage(hInputWnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)query16);

				SendMessage(hWnd, WMU_ADD_AI_CHAT_MESSAGE, (WPARAM)query16, 1);

				LONG_PTR style = GetWindowLongPtr(hInputWnd, GWL_STYLE);
				SetWindowLongPtr(hInputWnd, GWL_STYLE, style | ES_READONLY | WS_DISABLED | WS_BORDER);
				SendMessage(hInputWnd, EM_SETBKGNDCOLOR, 0, RGB(220, 220, 220));
				UpdateWindow(hInputWnd);

				int sid = prefs::get("ai-chat-sid");
				if (sid == 0) {
					sid = aiGetSid();
					prefs::set("ai-chat-sid", sid);
				}

				TCHAR* res16 = aiRequest(TEXT("chat-prompt"), query16, 0, sid);
				if (res16) {
					SendMessage(hWnd, WMU_ADD_AI_CHAT_MESSAGE, (WPARAM)res16, 0);
					delete [] res16;
					SetWindowText(hInputWnd, 0);
				} else {
					SendMessage(hWnd, WMU_ADD_AI_CHAT_MESSAGE, (WPARAM)TEXT(AI_ERROR_MESSAGE), 0);
				}
				delete [] query16;

				SendMessage(hInputWnd, EM_SETBKGNDCOLOR, 0, RGB(255, 255, 255));
				SetWindowLongPtr(hInputWnd, GWL_STYLE, style);

				SendMessage(hMessagesWnd, EM_SCROLL, SB_BOTTOM, 0);
			}

			if (cmd == IDM_AI_CHAT_COPY || cmd == IDM_AI_CHAT_EXECUTE) {
				int crStart, crEnd;
				SendMessage(hMessagesWnd, EM_GETSEL, (WPARAM)&crStart, (LPARAM)&crEnd);

				TCHAR* text16 = 0;

				// Execute last QUERY:-message
				if (crStart == crEnd && cmd == IDM_AI_CHAT_EXECUTE) {
					int sid = prefs::get("ai-chat-sid");

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select substr(content, 7) from ai_messages where sid = ? and role = 'assistant' and is_aux = 0 and instr(content, 'QUERY:') = 1 order by id desc", -1, &stmt, 0)) {
						sqlite3_bind_int(stmt, 1, sid);
						if (SQLITE_ROW == sqlite3_step(stmt))
							text16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
					}
					sqlite3_finalize(stmt);

					if (text16 == 0)
						return 0;
				} else {
					int len = crStart == crEnd ? GetWindowTextLength(hMessagesWnd) : crEnd - crStart;
					text16 = new TCHAR[len + 1]{0};

					if (crStart == crEnd) {
						GetWindowText(hMessagesWnd, text16, len + 1);
					} else {
						TEXTRANGE tr{{crStart, crEnd}, text16};
						SendMessage(hMessagesWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
					}
				}

				if (cmd == IDM_AI_CHAT_COPY)
					utils::setClipboardText(text16);

				if (cmd == IDM_AI_CHAT_EXECUTE) {
					int viewId = GetTickCount();

					int qlen = _tcslen(text16);
					char* query8 = utils::utf16to8(text16);
					char* view8 = new char[qlen + 256];
					snprintf(view8, qlen + 256, "create view temp.ai_view%i as %s", viewId, query8);

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, view8, -1, &stmt, 0)) {
						sqlite3_step(stmt);

						TCHAR viewname16[128];
						_sntprintf(viewname16, 127, TEXT("temp.ai_view%i"), viewId);
						openDialog(IDD_EDITDATA, (DLGPROC)&dialogs::cbDlgEditData, (LPARAM)viewname16);
					} else {
						int rc = sqlite3_exec(db, query8, 0, 0, 0);
						if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW) {
							MessageBox(hWnd, TEXT("Done"), TEXT("Info"), MB_OK);
						} else {
							showDbError(hWnd);
						}
					}
					sqlite3_finalize(stmt);

					delete [] query8;
					delete [] view8;
				}

				delete [] text16;
			}
		}
		break;

		case WMU_SET_THEME: {
			setEditorFont(hMessagesWnd);
			setEditorFont(hInputWnd);
			PostMessage(hWnd, WMU_LOAD_AI_CHAT, 0, 0);
		}
		break;

		case WMU_INIT_AI_CHAT: {
			SetProp(hInputWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hInputWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewAiChatInput));
			SetProp(hMessagesWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hMessagesWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewAiChatMessages));
			SendMessage(hWnd, WMU_SET_THEME, 0, 0);
			SendMessage(hWnd, WMU_SET_THEME, 0, 0);

			float z = utils::getWndScale(hTabWnd).x;
			SetProp(hWnd, TEXT("TAB_HEIGHT"), IntToPtr(19 * z));

			SendMessage(hMessagesWnd, EM_SETTARGETDEVICE, 0, !prefs::get("ai-chat-word-wrap"));

			HWND hTooltipWnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, GetModuleHandle(0), NULL);
			TOOLINFO ti{0};
			ti.cbSize = TTTOOLINFOW_V1_SIZE;
			ti.hwnd = hWnd;
			ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS |  TTF_TRACK | TTF_ABSOLUTE;
			ti.uId = (UINT_PTR)hWnd;

			SendMessage(hTooltipWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
			SendMessage(hTooltipWnd, TTM_SETMAXTIPWIDTH, 0, 400);
			SendMessage(hTooltipWnd, WM_SETFONT, (LPARAM)hMenuFont, 0);
			SetProp(hWnd, TEXT("TOOLTIP"), hTooltipWnd);

			PostMessage(hWnd, WM_SIZE, 0, 0);
			PostMessage(hWnd, WMU_LOAD_AI_CHAT, 0, 0);
		}
		break;

		case WMU_LOAD_AI_CHAT: {
			int sid = prefs::get("ai-chat-sid");

			sqlite3_stmt* stmt;
			if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select role = 'user', iif(instr(content, 'QUERY:') = 1, substr(content, 7), content) from ai_messages where sid = ? and role in ('user', 'assistant') and is_aux = 0 order by id", -1, &stmt, 0)) {
				sqlite3_bind_int(stmt, 1, sid);

				SetWindowText(hMessagesWnd, 0);
				while (SQLITE_ROW == sqlite3_step(stmt)) {
					bool isUser = sqlite3_column_int(stmt, 0);
					TCHAR* msg16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
					SendMessage(hWnd, WMU_ADD_AI_CHAT_MESSAGE, (WPARAM)msg16, isUser);
					delete [] msg16;
				}
			}
			sqlite3_finalize(stmt);

			SendMessage(hMessagesWnd, EM_SCROLL, SB_BOTTOM, 0);
		}
		break;

		// wMaram = msg16, lParam = isUser = 0/1
		case WMU_ADD_AI_CHAT_MESSAGE: {
			TCHAR* msg16 = (TCHAR*)wParam;
			bool isUser = lParam;

			GETTEXTLENGTHEX gtl = {GTL_NUMCHARS, 0};
			int len = SendMessage(hMessagesWnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 1200);
			SendMessage(hMessagesWnd, EM_SETSEL, len, len);

			int mlen = _tcslen(msg16);
			TCHAR* buf16 = new TCHAR[mlen + 32]{0};
			_sntprintf(buf16, mlen + 32, TEXT("%ls%ls%ls"), isUser ? TEXT("Me: ") : TEXT("AI: "), msg16 + (_tcsncmp(msg16, TEXT("QUERY:"), 6) == 0 ? 6 : 0), isUser ? TEXT("\n") : TEXT("\n\n"));
			SendMessage(hMessagesWnd, EM_REPLACESEL, TRUE, (LPARAM)buf16);
			int blen = _tcslen(buf16);
			delete [] buf16;

			CHARFORMAT cf = {0};
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_COLOR;
			cf.crTextColor = isUser ? RGB(0, 0, 255) : RGB(0, 0, 0);
			SendMessage(hMessagesWnd, EM_SETSEL, len, len + blen);
			SendMessage(hMessagesWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			SendMessage(hMessagesWnd, EM_SETSEL, len + blen, len + blen);

			if (len == 0)
				RedrawWindow(hMessagesWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
		}
		break;

		case WMU_TOGGLE_AI_WORD_WRAP: {
			bool isWordWrap = !prefs::get("ai-chat-word-wrap");
			prefs::set("ai-chat-word-wrap", isWordWrap);
			SendMessage(hMessagesWnd, EM_SETTARGETDEVICE, 0, !isWordWrap);
		}
		break;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewResultHeader(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_COMMAND: {
			if (HIWORD(wParam) == EN_CHANGE &&
				LOWORD(wParam) >= IDC_HEADER_EDIT && (LOWORD(wParam) <= IDC_HEADER_EDIT + Header_GetItemCount(hWnd)) &&
				prefs::get("auto-filters")) {
				TCHAR buf[3] = {0};
				GetDlgItemText(hWnd, LOWORD(wParam), buf, 3);

				if (_tcslen(buf) != 1 || _tcschr(TEXT("<>=!"), buf[0]) == 0)
					SendMessage(GetParent(hWnd), WMU_UPDATE_RESULTSET, 0, 0);
			}
		}
		break;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewResultTab(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_LBUTTONDOWN: {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			int w = prefs::get("preview-width");
			RECT rc;
			GetClientRect(hWnd, &rc);

			if (rc.right - w - 10 <= x && x <= rc.right - w - 5 && y > 20) {
				SetProp(hWnd, TEXT("ISPSPLITTERMOVE"), (HANDLE)1);
				SetCapture(hWnd);
			}
		}
		break;

		case WM_LBUTTONUP: {
			if (GetProp(hWnd, TEXT("ISPSPLITTERMOVE"))) {
				ReleaseCapture();
				RemoveProp(hWnd, TEXT("ISPSPLITTERMOVE"));
			}
		}
		break;

		case WM_MOUSEMOVE: {
			if(wParam == MK_LBUTTON && GetProp(hWnd, TEXT("ISPSPLITTERMOVE"))) {
				LONG x = GET_X_LPARAM(lParam);
				RECT rc;
				GetClientRect(hWnd, &rc);
				prefs::set("preview-width", rc.right - x - 5);
				EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_RESIZETAB);
				InvalidateRect(hTabWnd, NULL, TRUE);
			}
		}
		break;

		case WM_NOTIFY: {
			NMHDR* pHdr = (LPNMHDR)lParam;
			int id = (int)(pHdr->idFrom / 100) * 100;
			if (id == IDC_TAB_ROWS) {
				if (pHdr->code == LVN_GETDISPINFO) {
					LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
					LV_ITEM* pItem = &(pDispInfo)->item;
					HWND hListWnd = pHdr->hwndFrom;

					if (pItem->mask & LVIF_TEXT)
						pItem->pszText = ListView_GetItemValueText(hListWnd, pItem->iItem, pItem->iSubItem);
				}

				if (pHdr->code == (UINT)NM_CUSTOMDRAW) {
					int result = CDRF_DODEFAULT;
					HWND hListWnd = pHdr->hwndFrom;
					int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
					byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
					int* modifiers = (int*)GetProp(hListWnd, TEXT("MODIFIERS"));

					if (!datatypes)
						return CDRF_DODEFAULT;

					NMLVCUSTOMDRAW* pCustomDraw = (LPNMLVCUSTOMDRAW)lParam;
					int rowNo = pCustomDraw->nmcd.dwItemSpec;
					int colNo = pCustomDraw->iSubItem;
					int colCount = ListView_GetColumnCount(hListWnd);

					int modifierNo = modifiers && colNo >= 0 && colNo < colCount ? modifiers[colNo] : 0; // ??? Some colNo less 0 or greater colCount
					modifierNo = modifierNo > 0 && modifierNo < MAX_PLUGIN_COUNT && plugins[modifierNo].hModule && plugins[modifierNo].type == ADDON_COLUMN_MODIFIER ? modifierNo : 0;

					if (pCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
						result = CDRF_NOTIFYITEMDRAW;

					if (pCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
						pCustomDraw->nmcd.lItemlParam = 0;
						if (ListView_GetItemState(hListWnd, rowNo, LVIS_SELECTED)) {
							pCustomDraw->nmcd.uItemState &= ~CDIS_SELECTED;
							pCustomDraw->nmcd.lItemlParam = 1;
						}
						result = CDRF_NOTIFYSUBITEMDRAW;
					}

					if (pCustomDraw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
						int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd)) - 1;
						rowNo = resultset[rowNo];

						bool isFocus = GetFocus() == hListWnd;
						bool isSelectedRow = pCustomDraw->nmcd.lItemlParam == 1;
						bool isCurrCell = rowNo == (int)(LONG_PTR)GetProp(hListWnd, TEXT("CURRENTROW")) &&
							colNo == (int)(LONG_PTR)GetProp(hListWnd, TEXT("CURRENTCOLUMN")) &&
							isFocus && prefs::get("show-preview");

						pCustomDraw->clrText = isSelectedRow && (!isCurrCell || (isCurrCell && utils::isColorDark(GRIDCOLORS[0]))) ? RGB(255, 255, 255) : RGB(0, 0, 0);
						pCustomDraw->clrTextBk = isCurrCell ? GRIDCOLORS[0] :
							isSelectedRow && isFocus ? RGB(10, 36, 106) :
							isSelectedRow && !isFocus ? RGB(212, 208, 200) :
							datatypes ? GRIDCOLORS[datatypes[colNo + colCount * rowNo]] :
							RGB(255, 255, 255);

						if (modifierNo && (plugins[modifierNo].setColor || plugins[modifierNo].render)) {
							TValue value = {0};
							if (ListView_GetItemValue(hListWnd, rowNo, colNo, &value)) {
								if (plugins[modifierNo].setColor)
									plugins[modifierNo].setColor(pCustomDraw, value.data, value.dataLen, value.dataType);

								if (plugins[modifierNo].render) {
									result = plugins[modifierNo].render(pCustomDraw, value.data, value.dataLen, value.dataType) ? CDRF_SKIPDEFAULT : result;
								}
							}
						}
					}

					return result;
				}

				if (pHdr->code == LVN_KEYDOWN) {
					HWND hListWnd = pHdr->hwndFrom;
					NMLVKEYDOWN* kd = (NMLVKEYDOWN*) lParam;
					if (kd->wVKey == VK_LEFT || kd->wVKey == VK_RIGHT) {
						int rowNo = (int)(LONG_PTR)GetProp(hListWnd, TEXT("CURRENTROW"));
						int colNo = (int)(LONG_PTR)GetProp(hListWnd, TEXT("CURRENTCOLUMN"));
						int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd));

						colNo = colNo + (kd->wVKey == VK_LEFT ? -1 : 1);
						colNo = colNo == 0 ? colCount - 1 : colNo == colCount ? 1 : colNo;
						SendMessage(hWnd, WMU_SET_CURRENT_CELL, rowNo, colNo);
					}
				}

				if (pHdr->code == (DWORD)LVN_ITEMCHANGED) {
					HWND hListWnd = pHdr->hwndFrom;
					NMLISTVIEW* lv = (NMLISTVIEW*)lParam;
					if (lv->uNewState & LVNI_SELECTED && !(lv->uOldState & LVNI_SELECTED)) {
						int colNo = (int)(LONG_PTR)GetProp(hListWnd, TEXT("CURRENTCOLUMN"));
						SendMessage(hWnd, WMU_SET_CURRENT_CELL, lv->iItem, colNo);
					}
				}
			}

			if (pHdr->code == HDN_ITEMCHANGED) {
				HWND hListWnd = GetParent(pHdr->hwndFrom);
				if ((pHdr->hwndFrom == ListView_GetHeader(hListWnd)) &&
					(hListWnd == GetDlgItem(hWnd, IDC_TAB_ROWS + (int)GetWindowLongPtr(hListWnd, GWLP_USERDATA))) &&
					IsWindowEnabled(pHdr->hwndFrom)) // Inside WMU_AUTO_COLUMN_SIZE the header is disabled
					SendMessage(hListWnd, WMU_UPDATE_FILTER_SIZE, 0, 0);
			}
		}
		break;

		// wParam = rowNo, lParam = colNo
		case WMU_SET_CURRENT_CELL: {
			HWND hListWnd = (HWND)SendMessage(hMainWnd, WMU_GET_CURRENT_RESULTSET, 0, 0);
			HWND hPreviewWnd = GetDlgItem(hTabWnd, IDC_TAB_PREVIEW + GetWindowLongPtr(hListWnd, GWLP_USERDATA));

			int prevRowNo = (int)(LONG_PTR)GetProp(hListWnd, TEXT("CURRENTROW"));
			int prevColNo = (int)(LONG_PTR)GetProp(hListWnd, TEXT("CURRENTCOLUMN"));

			int rowNo = wParam;
			int colNo = lParam;

			if (prevRowNo == rowNo && prevColNo == colNo)
				return false;

			if (rowNo < 0 || rowNo >= ListView_GetItemCount(hListWnd))
				rowNo = -1;

			if (colNo < 0 || colNo >= ListView_GetColumnCount(hListWnd))
				colNo = -1;

			RECT rc, rc2;
			ListView_GetSubItemRect(hListWnd, rowNo, colNo, LVIR_BOUNDS, &rc);

			// If the previous cell was partially visible then GetSubItemRect/InvalidateRect don't work
			// To solve it ListView_Scroll is called at first and only then cell rectangles are updated
			GetClientRect(hListWnd, &rc2);
			int w = rc.right - rc.left;
			int dx = rc2.right < rc.right ? rc.left - rc2.right + w : rc.left < 0 ? rc.left : 0;
			ListView_Scroll(hListWnd, dx, 0);

			ListView_GetSubItemRect(hListWnd, prevRowNo, prevColNo, LVIR_BOUNDS, &rc);
			InvalidateRect(hListWnd, &rc, TRUE);
			ListView_GetSubItemRect(hListWnd, rowNo, colNo, LVIR_BOUNDS, &rc);
			InvalidateRect(hListWnd, &rc, TRUE);

			SetProp(hListWnd, TEXT("CURRENTROW"), IntToPtr(rowNo));
			SetProp(hListWnd, TEXT("CURRENTCOLUMN"), IntToPtr(colNo));

			TValue value = {0};
			if (prefs::get("show-preview") && rowNo != -1 && colNo != -1 && ListView_GetItemValue(hListWnd, rowNo, colNo, &value)) {
				SetProp(hPreviewWnd, TEXT("DATA"), (HANDLE)value.data);
				SetProp(hPreviewWnd, TEXT("DATATYPE"), IntToPtr(value.dataType));
				SetProp(hPreviewWnd, TEXT("DATALEN"), IntToPtr(value.dataLen));

				SendMessage(hPreviewWnd, WMU_UPDATE_PREVIEW, 0, 0);
			}
		}
		break;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewMainTabRename(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_CHAR && wParam == VK_RETURN)
		return 0;

	if (msg == WM_KEYUP && wParam == VK_ESCAPE) {
		DestroyWindow(hWnd);
		return 0;
	}

	if (processEditKeys(hWnd, msg, wParam, lParam))
		return 0;

	if ((msg == WM_KEYDOWN && wParam == VK_RETURN) || (msg == WM_KILLFOCUS)) {
		HWND hTabWnd = GetParent(hWnd);
		int tabNo = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		int len = SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
		TCHAR tabName[len + 1] = {0};
		GetWindowText(hWnd, tabName, len + 1);
		SendMessage(hTabWnd, WMU_TAB_SET_TEXT, tabNo, (LPARAM)tabName);
		DestroyWindow(hWnd);
		return 0;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewStatusBar(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// Extend text to overflow it and therefore to enable tooltip
	if (msg == SB_SETTEXT) {
		TCHAR buf16[256] = {0};
		_sntprintf(buf16, 255, TEXT("%ls%*.*ls"), lParam ? (TCHAR*)lParam : TEXT(""), 255, 255, TEXT(""));

		CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, (LPARAM)buf16);
		return 1;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewMainTab(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int TAB_WIDTH = 0;
	int TAB_LENGTH = 17;
	int CLI_WIDTH = 0;

	if (msg == WM_PAINT || msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN  || msg == WM_MBUTTONDOWN || msg == WM_LBUTTONDBLCLK || msg == WM_MOUSEMOVE) {
		float z = utils::getWndScale(hWnd).x;
		TAB_WIDTH = 120 * z;
		CLI_WIDTH = 27 * z;
	}

	TCHAR* headers = (TCHAR*)GetProp(hWnd, TEXT("HEADERS"));
	LONG* styles = (LONG*)GetProp(hWnd, TEXT("STYLES"));
	int currTab = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENT"));
	int tabCount = (int)(LONG_PTR)GetProp(hWnd, TEXT("COUNT"));
	int clistyle = (int)(LONG_PTR)GetProp(hWnd, TEXT("CLISTYLE"));

	switch (msg) {
		case WM_NCDESTROY: {
			RemoveProp(hWnd, TEXT("CURRENT"));
			RemoveProp(hWnd, TEXT("NEXTTABNO"));
			RemoveProp(hWnd, TEXT("COUNT"));
			RemoveProp(hWnd, TEXT("CLISTYLE"));

			TCHAR* headers = (TCHAR*)GetProp(hWnd, TEXT("HEADERS"));
			if (headers)
				delete [] headers;
			RemoveProp(hWnd, TEXT("HEADERS"));

			int* styles = (int*)GetProp(hWnd, TEXT("STYLES"));
			if (styles)
				delete [] styles;
			RemoveProp(hWnd, TEXT("STYLES"));
		}
		break;

		// Used as WM_CREATE
		case WMU_TAB_ADD: {
			if (tabCount == MAX_TAB_COUNT)
				return -1;

			if (!headers) {
				headers = new TCHAR[MAX_TAB_COUNT * TAB_LENGTH]{0};
				SetProp(hWnd, TEXT("HEADERS"), (HANDLE)headers);
			}

			if (!styles) {
				styles = new LONG[MAX_TAB_COUNT]{0};
				SetProp(hWnd, TEXT("STYLES"), (HANDLE)styles);
			}

			tabCount++;
			int no = (int)(LONG_PTR)GetProp(hWnd, TEXT("NEXTTABNO")) + 1;
			_sntprintf(headers + (tabCount - 1) * TAB_LENGTH, TAB_LENGTH, TEXT("Editor #%i"), no);
			SetProp(hWnd, TEXT("NEXTTABNO"), IntToPtr(no));
			SetProp(hWnd, TEXT("COUNT"), IntToPtr(tabCount));

			NMHDR Hdr = {hWnd, (UINT)GetWindowLongPtr(hWnd, GWL_ID), NM_TAB_ADD};
			SendMessage(GetParent(hWnd), WM_NOTIFY, tabCount - 1, (LPARAM)&Hdr);

			return tabCount - 1;
		}
		break;

		case WMU_TAB_DELETE : {
			int tabNo = wParam;
			if (tabNo < 0 || tabNo > tabCount || tabCount == 1)
				return 0;

			NMHDR Hdr = {hWnd, (UINT)GetWindowLongPtr(hWnd, GWL_ID), NM_TAB_REQUEST_DELETE};
			if(SendMessage(GetParent(hWnd), WM_NOTIFY, tabNo, (LPARAM)&Hdr))
				return 0;

			for (int i = tabNo * TAB_LENGTH; i < (tabCount - 1) * TAB_LENGTH; i++)
				headers[i] = headers[i + TAB_LENGTH];
			for (int i = (tabCount - 1) * TAB_LENGTH; i < MAX_TAB_COUNT * TAB_LENGTH; i++)
				headers[i] = 0;

			for (int i = tabNo; i < tabCount; i++)
				styles[i] = styles[i + 1];

			if (tabNo < currTab)
				SetProp(hWnd, TEXT("CURRENT"), IntToPtr(currTab - 1));
			SetProp(hWnd, TEXT("COUNT"), IntToPtr(tabCount - 1));

			Hdr = {hWnd, (UINT)GetWindowLongPtr(hWnd, GWL_ID), NM_TAB_DELETE};
			SendMessage(GetParent(hWnd), WM_NOTIFY, tabNo, (LPARAM)&Hdr);

			return 1;
		}
		break;

		case WMU_TAB_SET_TEXT: {
			int tabNo = wParam;
			int tabCount = SendMessage(hWnd, WMU_TAB_GET_COUNT, 0, 0);
			TCHAR* text = (TCHAR*)lParam;

			if (tabNo < 0 || tabNo >= tabCount)
				return 1;

			int len = text ? _tcslen(text) : 0;
			for (int i = 0; i < TAB_LENGTH; i++)
				headers[tabNo * TAB_LENGTH + i] = i < len ? text[i] : 0;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

		case WMU_TAB_SET_STYLE: {
			int tabNo = wParam;

			if (tabNo < -2 || tabNo >= tabCount)
				return 1;

			if (tabNo == -1)
				SetProp(hWnd, TEXT("CLISTYLE"), (HANDLE)lParam);
			else
				styles[tabNo] = lParam;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

		case WMU_TAB_GET_STYLE: {
			int tabNo = wParam;
			return tabNo == -1 ? clistyle : tabNo < 0 || tabNo >= tabCount ? 0 : styles[tabNo];
		}
		break;

		case WMU_TAB_GET_TEXT: {
			int tabNo = wParam;
			TCHAR* buf = (TCHAR*)lParam;

			if (tabNo < 0 || tabNo >= tabCount || !buf)
				return 1;

			for (int i = 0; i < TAB_LENGTH; i++)
				buf[i] = headers[tabNo * TAB_LENGTH + i];
			buf[TAB_LENGTH - 1] = 0;
		}
		break;

		case WMU_TAB_SET_CURRENT: {
			if ((currTab == (int)wParam) && (lParam != 0))
				return 0;

			SetProp(hMainTabWnd, TEXT("CURRENT"), (HANDLE)wParam);
			InvalidateRect(hWnd, NULL, FALSE);

			NMHDR Hdr = {hWnd, (UINT)GetWindowLongPtr(hWnd, GWL_ID), NM_TAB_CHANGE};
			SendMessage(GetParent(hWnd), WM_NOTIFY, wParam, (LPARAM)&Hdr);

			return 1;
		}

		case WMU_TAB_GET_COUNT: {
			return tabCount;
		}

		case WMU_TAB_GET_CURRENT: {
			return currTab;
		}

		case WM_PAINT: {
			PAINTSTRUCT ps{0};
			HDC hDC = BeginPaint(hWnd, &ps);
			int w = ps.rcPaint.right;
			int h = ps.rcPaint.bottom;

			if (w == 0) {
				EndPaint(hWnd, &ps);
				return true;
			}

			RECT rc{0};
			GetClientRect(hWnd, &rc);
			HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			FillRect(hDC, &rc, hBrush);
			DeleteObject(hBrush);

			HFONT hOldFont = (HFONT)SelectObject(hDC, hMenuFont);
			HPEN hCrossPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
			HPEN hBorderPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_GRAYTEXT));
			HPEN hUnborderPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));

			// CLI
			if (currTab == -1) {
				RECT rc = {0, 0, CLI_WIDTH, h};
				HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
				FillRect(hDC, &rc, hBrush);
				DeleteObject(hBrush);
			}
			SetBkColor(hDC, currTab == -1 ? RGB(255, 255, 255) : GetSysColor(COLOR_BTNFACE));
			SetTextColor(hDC, clistyle ? RGB(255, 0, 0) : RGB(0, 0, 0));
			TextOut(hDC, 10, 3, TEXT(">_"), 2);

			// borders
			SelectObject(hDC, hBorderPen);
			MoveToEx(hDC, CLI_WIDTH, 0, NULL);
			LineTo(hDC, 0, 0);
			LineTo(hDC, 0, h - 1);
			if (currTab == -1)
				SelectObject(hDC, hUnborderPen);
			LineTo(hDC, CLI_WIDTH, h - 1);

			// Bottom line
			SelectObject(hDC, hBorderPen);
			MoveToEx(hDC, CLI_WIDTH, h - 1, NULL);
			LineTo(hDC, w, h - 1);

			for (int tabNo = 0; tabNo < tabCount; tabNo++) {
				int l = CLI_WIDTH + tabNo * TAB_WIDTH;
				int r = CLI_WIDTH + (tabNo + 1) * TAB_WIDTH;
				if (l > w)
					break;

				RECT rc = {l, 0, MIN(CLI_WIDTH + (tabNo + 1) * TAB_WIDTH, w), h};

				if (tabNo == currTab) {
					HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
					FillRect(hDC, &rc, hBrush);
					DeleteObject(hBrush);
				}

				SetTextColor(hDC, styles[tabNo] ? RGB(255, 0, 0) : GetSysColor(COLOR_BTNTEXT));
				SetBkColor(hDC, tabNo == currTab ? RGB(255, 255, 255) : GetSysColor(COLOR_BTNFACE));
				int pos = tabNo * TAB_LENGTH;
				int len = 0;
				for (len = 0; len < TAB_LENGTH && (headers[pos + len] != 0); len++);
				RECT rcText = {rc.left + 5, 0, rc.right, h};
				DrawText(hDC, (TCHAR*)headers + pos, len, &rcText, DT_VCENTER | DT_SINGLELINE);

				if (tabNo == 0) {
					SelectObject(hDC, hBorderPen);
					MoveToEx(hDC, CLI_WIDTH, 0, NULL);
					LineTo(hDC, CLI_WIDTH, h - 1);
				}

				// right border
				if (r < w) {
					SelectObject(hDC, hBorderPen);
					MoveToEx(hDC, r - 1, 0, NULL);
					LineTo(hDC, r - 1, h - 1);
				}

				// Top
				SelectObject(hDC, hBorderPen);
				MoveToEx(hDC, l, 0, NULL);
				LineTo(hDC, rc.right, 0);

				// Current bottom
				if (tabNo == currTab) {
					SelectObject(hDC, hUnborderPen);
					MoveToEx(hDC, l, h - 1, NULL);
					LineTo(hDC, rc.right - 1, h - 1);
				}

				// Cross
				if (tabCount > 1 && r < w) {
					SelectObject(hDC, hCrossPen);
					MoveToEx(hDC, r - h + h/3, h/3, NULL);
					LineTo(hDC, r - h/3, h - h/3);
					MoveToEx(hDC, r - h + h/3, h - h/3, NULL);
					LineTo(hDC, r - h/3, h/3);
				}
			}

			// Plus
			int c = CLI_WIDTH + tabCount * TAB_WIDTH;
			if (c + h < w) {
				SelectObject(hDC, hCrossPen);
				MoveToEx(hDC, c + h/4, h/2 + h % 2, NULL);
				LineTo(hDC, c + h - h/4, h/2 + h % 2);
				MoveToEx(hDC, c + h/2 + h % 2, h/4, NULL);
				LineTo(hDC, c + h/2 + h % 2, h - h/4);
			}

			DeleteObject(hCrossPen);
			DeleteObject(hBorderPen);
			DeleteObject(hUnborderPen);

			SelectObject(hDC, hOldFont);

			EndPaint(hWnd, &ps);
		}
		break;

		case WM_SIZE: {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

		case WM_LBUTTONDOWN: {
			POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			int tabNo = (p.x - CLI_WIDTH)/ TAB_WIDTH;
			int tabCount = SendMessage(hWnd, WMU_TAB_GET_COUNT, 0, 0);

			RECT rc;
			GetClientRect(hWnd, &rc);
			int w = rc.bottom;

			if (p.x < CLI_WIDTH) {
				SendMessage(hWnd, WMU_TAB_SET_CURRENT, -1, 1);
			} else if (tabNo < tabCount) {
				if (p.x > CLI_WIDTH + (tabNo + 1)* TAB_WIDTH - w) {
					if (SendMessage(hWnd, WMU_TAB_DELETE, tabNo, 0)) {
						if (tabNo == currTab)
							SendMessage(hWnd, WMU_TAB_SET_CURRENT, tabNo == tabCount - 1 ? tabNo - 1 : tabNo, 0);

						InvalidateRect(hWnd, NULL, FALSE);
					}
				} else {
					SendMessage(hWnd, WMU_TAB_SET_CURRENT, tabNo, 1);
				}

				SetProp(hWnd, TEXT("ISMOUSEDOWN"), (HWND)1);
				SetCapture(hWnd);
			} else if (p.x < CLI_WIDTH + tabNo * TAB_WIDTH + w) {
				SendMessage(hWnd, WMU_TAB_ADD, 0, 0);
			}

			return 0;
		}
		break;

		case WM_LBUTTONUP: {
			ReleaseCapture();
			RemoveProp(hWnd, TEXT("ISMOUSEDOWN"));
		}
		break;

		case WM_MOUSEMOVE: {
			if (wParam != MK_LBUTTON || !GetProp(hWnd, TEXT("ISMOUSEDOWN")))
				return 0;

			int x = GET_X_LPARAM(lParam);

			int tabNo = (x - CLI_WIDTH)/ TAB_WIDTH;
			if (x <= CLI_WIDTH || tabNo > tabCount - 1 || currTab == tabNo)
				return 0;

			for (int i = 0; i < TAB_LENGTH; i++) {
				TCHAR c = headers[tabNo * TAB_LENGTH + i];
				headers[tabNo * TAB_LENGTH + i] = headers[currTab * TAB_LENGTH + i];
				headers[currTab * TAB_LENGTH + i] = c;
			}

			TEditorTab* tabA = &tabs[tabNo];
			TEditorTab* tabB = &tabs[currTab];
			TEditorTab temp = *tabA;
			*tabA = *tabB;
			*tabB = temp;

			SendMessage(hMainTabWnd, WMU_TAB_SET_CURRENT, tabNo, 0);
		}
		break;

		case WM_MBUTTONDOWN: {
			POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			int tabNo = (p.x - CLI_WIDTH)/ TAB_WIDTH;
			int tabCount = SendMessage(hWnd, WMU_TAB_GET_COUNT, 0, 0);
			if (p.x > CLI_WIDTH && tabNo < tabCount && SendMessage(hWnd, WMU_TAB_DELETE, tabNo, 0)) {
				if (tabNo == currTab)
					SendMessage(hWnd, WMU_TAB_SET_CURRENT, tabNo == tabCount - 1 ? tabNo - 1 : tabNo, 0);

				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;
		}
		break;

		case WM_LBUTTONDBLCLK: {
			if (wParam == MK_LBUTTON) {
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				int tabNo = (p.x - CLI_WIDTH)/ TAB_WIDTH;
				int tabCount = SendMessage(hWnd, WMU_TAB_GET_COUNT, 0, 0);

				if (p.x > CLI_WIDTH && tabNo < tabCount) {
					TCHAR tabName[TAB_LENGTH] = {0};
					for (int i = 0; i < TAB_LENGTH; i++)
						tabName[i] = headers[tabNo * TAB_LENGTH + i];

					RECT rc;
					GetClientRect(hWnd, &rc);
					HWND hEdit = CreateWindow(WC_EDIT, tabName, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_CLIPSIBLINGS,
						CLI_WIDTH + tabNo * TAB_WIDTH, 0, TAB_WIDTH, rc.bottom, hWnd, (HMENU)IDC_TAB_EDIT, GetModuleHandle(0), 0);
					SendMessage(hEdit, WM_SETFONT, (LPARAM)hMenuFont, true);
					SetFocus(hEdit);
					int len = _tcslen(tabName);
					SendMessage(hEdit, EM_SETSEL, (WPARAM)len, (WPARAM)len);
					SetWindowLongPtr(hEdit, GWLP_USERDATA, tabNo);
					SetProp(hEdit, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)&cbNewMainTabRename));
				}
			}
		}
		break;

		default:
			return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK cbNewSchema(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WNDPROC cbDefault = (WNDPROC)GetProp(hWnd, TEXT("WNDPROC"));

	// overwrite black border
	if (msg == WM_PAINT) {
		cbDefault(hWnd, msg, wParam, lParam);

		RECT rc;
		GetClientRect(hWnd, &rc);

		HDC hDC = GetWindowDC(hWnd);
		HPEN hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_GRAYTEXT));
		HPEN oldPen = (HPEN)SelectObject(hDC, hPen);
		SelectObject(hDC, GetStockObject(NULL_BRUSH));
		Rectangle(hDC, 0, 0, rc.right + 2, rc.bottom + 2);

		SelectObject(hDC, oldPen);
		DeleteObject(hPen);
		ReleaseDC(hWnd, hDC);

		return 0;
	}

	return CallWindowProc(cbDefault, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewEditorSearch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WNDPROC cbDefault = (WNDPROC)GetProp(hWnd, TEXT("WNDPROC"));

	switch (msg) {
		case WM_PAINT: {
			cbDefault(hWnd, msg, wParam, lParam);

			RECT rc;
			GetClientRect(hWnd, &rc);

			HDC hDC = GetWindowDC(hWnd);
			HPEN hPen1 = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_GRAYTEXT));
			HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

			HPEN oldPen = (HPEN)SelectObject(hDC, hPen1);
			MoveToEx(hDC, 0, rc.bottom - 2, 0);
			LineTo(hDC, rc.right, rc.bottom - 2);
			SelectObject(hDC, hPen2);
			MoveToEx(hDC, 0, rc.bottom - 1, 0);
			LineTo(hDC, rc.right, rc.bottom - 1);

			SelectObject(hDC, oldPen);
			DeleteObject(hPen1);
			DeleteObject(hPen2);
			ReleaseDC(hWnd, hDC);

			return 0;
		}
		break;

		case WM_SIZE: {
			RECT rc;
			GetClientRect(GetDlgItem(hWnd, IDC_EDITOR_SEARCH_CLOSE), &rc);
			int s = rc.right - rc.left;
			GetWindowRect(GetDlgItem(hWnd, IDC_EDITOR_SEARCH_STRING), &rc);
			POINT p = {rc.left, rc.top};
			ScreenToClient(hWnd, &p);
			SetWindowPos(GetDlgItem(hWnd, IDC_EDITOR_SEARCH_CLOSE), 0, LOWORD(lParam) - s - p.y, p.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		}
		break;

		case WM_COMMAND: {
			WORD cmd = HIWORD(wParam);
			WORD idc = LOWORD(wParam);

			if (cmd == STN_CLICKED && (idc == IDC_EDITOR_SEARCH_CLOSE)) {
				ShowWindow(hWnd, SW_HIDE);
				SendMessage(hMainWnd, WMU_UPDATE_SIZES, 0, 0);
				SetFocus(hEditorWnd);
			}

			if (cmd == BN_CLICKED && (idc == IDC_EDITOR_SEARCH_PREV || idc == IDC_EDITOR_SEARCH_NEXT)) {
				prefs::set("case-sensitive", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_CASE_SENSITIVE)));
				GetDlgItemText(hWnd, IDC_EDITOR_SEARCH_STRING, searchString, 255);
				doEditorSearch(hEditorWnd, idc == IDC_EDITOR_SEARCH_PREV);
			}
		}
		break;
	}

	return CallWindowProc(cbDefault, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK cbNewEditorSearchString(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if ((msg == WM_CHAR || msg == WM_KEYDOWN || msg == WM_KEYUP) && wParam == VK_TAB) {
		if (msg == WM_KEYUP)
			SetFocus(hEditorWnd);

		return 0;
	}

	if ((msg == WM_CHAR || msg == WM_KEYDOWN) && (wParam == VK_RETURN || wParam == VK_ESCAPE)) {
		if (msg == WM_CHAR) // Prevent beep
			return 0;

		HWND hParentWnd = GetParent(hWnd);
		if (wParam == VK_RETURN)
			SendMessage(hParentWnd, WM_COMMAND,
				MAKEWPARAM(HIWORD(GetKeyState(VK_SHIFT)) ? IDC_EDITOR_SEARCH_PREV : IDC_EDITOR_SEARCH_NEXT, BN_CLICKED),
				(LPARAM)GetDlgItem(hParentWnd, IDC_EDITOR_SEARCH_NEXT));
		else
			SendMessage(hParentWnd, WM_COMMAND, MAKEWPARAM(IDC_EDITOR_SEARCH_CLOSE, STN_CLICKED), (LPARAM)GetDlgItem(hParentWnd, IDC_EDITOR_SEARCH_CLOSE));

		return true;
	}

	return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
}


// Returns
// * selected text if the editor has selection
// * word under cursor if text out of quotes and full quoted string otherwise
TCHAR* getCurrentText(HWND hWnd) {
	int crStart, crEnd;
	SendMessage(hWnd, EM_GETSEL, (WPARAM)&crStart, (LPARAM)&crEnd);

	TCHAR* text = 0;
	if (crStart == crEnd) {
		int currLineNo = SendMessage(hWnd, EM_LINEFROMCHAR, -1, 0);
		int currLineSize = SendMessage(hWnd, EM_LINELENGTH, crStart, 0);
		size_t currLineIdx = SendMessage(hWnd, EM_LINEINDEX, -1, 0);
		TCHAR currLine[currLineSize + 1]{0};
		currLine[0]  = currLineSize;

		SendMessage(hWnd, EM_GETLINE, currLineNo, (LPARAM)currLine);

		COLORREF qColor = (COLORREF)prefs::get("color-quoted");
		auto isQuoted = [hWnd, &currLine, qColor](int pos) {
			CHARFORMAT cf{0};
			cf.cbSize = sizeof(CHARFORMAT);
			cf.dwMask = CFM_COLOR;
			SendMessage(hWnd, EM_SETSEL, pos, pos);
			SendMessage(hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			return qColor == cf.crTextColor;
		};

		auto isContinue = [&currLine](int pos) {
			return _istalnum(currLine[pos]) || currLine[pos] == TEXT('_') || currLine[pos] == TEXT('.');
		};

		size_t start = crStart;
		while (start > currLineIdx && (isContinue(start - currLineIdx - 1) || isQuoted(start)))
			start--;

		size_t end = crStart;
		while ((end < currLineIdx + currLineSize) && (isContinue(end - currLineIdx) || isQuoted(end + 1)))
			end++;

		SendMessage(hWnd, EM_SETSEL, crStart, crEnd);

		text = new TCHAR[end - start + 1]{0};
		for (size_t i = 0; i < end - start; i++)
			text[i] = currLine[start - currLineIdx + i];
	} else {
		text = new TCHAR[crEnd - crStart + 1]{0};
		TEXTRANGE tr{{crStart, crEnd}, text};
		SendMessage(hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	}

	return text;
}

// Returns selected text if the editor has selection or entire text otherwise
TCHAR* getEditorText(HWND hWnd) {
	int crStart, crEnd;
	SendMessage(hWnd, EM_GETSEL, (WPARAM)&crStart, (LPARAM)&crEnd);

	int len = crStart == crEnd ? GetWindowTextLength(hWnd) : crEnd - crStart;
	TCHAR* text16 = new TCHAR[len + 1]{0};

	if (crStart == crEnd) {
		GetWindowText(hWnd, text16, len + 1);
	} else {
		TEXTRANGE tr{{crStart, crEnd}, text16};
		SendMessage(hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	}

	return text16;
}

void setEditorText(HWND hWnd, TCHAR* text16) {
	CHARRANGE range;
	SendMessage(hWnd, EM_EXGETSEL, 0, (LPARAM)&range);

	if (range.cpMin == range.cpMax)
		SendMessage(hWnd, EM_SETSEL, 0, -1);
	SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)text16);
	SendMessage(hWnd, EM_SETSEL, range.cpMin, range.cpMin);
}

HWND openDialog(int IDD, DLGPROC proc, LPARAM lParam) {
	if (IDD == IDD_TOOL_DATABASE_DIAGRAM || IDD == IDD_TOOL_COMPARE_DATABASE ||
		IDD == IDD_TOOL_DATABASE_SEARCH || IDD == IDD_TOOL_TEXT_COMPARISON ||
		IDD == IDD_TOOL_STATISTICS) {
		for (int dlgNo = 0; dlgNo < MAX_DIALOG_COUNT; dlgNo++) {
			HWND hDlg = hDialogs[dlgNo];
			if (PtrToInt(GetProp(hDlg, TEXT("IDD"))) == IDD) {
				SetForegroundWindow(hDlg);
				SetFocus(hDlg);
				return hDlg;
			}
		}
	}

	int no = 0;
	while (hDialogs[no])
		no++;

	if (no >= MAX_DIALOG_COUNT) {
		MessageBox(hMainWnd, TEXT("Max dialog count (32) is exceeded"), 0, 0);
		return 0;
	}

	HWND hDlg = CreateDialogParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD), hMainWnd, proc, lParam);
	hDialogs[no] = hDlg;
	ShowWindow(hDlg, SW_SHOW);
	if (GetAncestor(GetFocus(), GA_ROOT) != hDlg)
		SetFocus(hDlg);

	RECT rc;
	GetWindowRect(hDlg, &rc);
	SetWindowPos(hDlg, 0, rc.left + no * 10, rc.top + no * 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	return hDlg;
}

bool wrapSelectedText(HWND hEditorWnd, int key) {
	CHARRANGE range;
	SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&range);
	if (range.cpMin == range.cpMax)
		return false;

	SendMessage(hEditorWnd, WM_SETREDRAW, FALSE, 0);

	TCHAR l[2] = {0};
	TCHAR r[2] = {0};
	_sntprintf(l, 2, TEXT("%c"), key == TEXT('"') || key == TEXT('\'') ? key : TEXT('('));
	_sntprintf(r, 2, TEXT("%c"), key == TEXT('"') || key == TEXT('\'') ? key : TEXT(')'));

	SendMessage(hEditorWnd, EM_SETSEL, range.cpMin, range.cpMin);
	SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (WPARAM)l);
	SendMessage(hEditorWnd, EM_SETSEL, range.cpMax + 1, range.cpMax + 1);
	SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (WPARAM)r);
	SendMessage(hEditorWnd, EM_SETSEL, range.cpMax + 2, range.cpMax + 2);

	SendMessage(hEditorWnd, WM_SETREDRAW, TRUE, 0);
	processHighlight(hEditorWnd, true, true, true);

	return true;
}

bool toggleWordWrap(HWND hEditorWnd) {
	bool isWordWrap = true;
	for (int i = 0; i < MAX_TAB_COUNT + MAX_DIALOG_COUNT; i++) {
		hEditors[i] = IsWindow(hEditors[i]) ? hEditors[i] : 0;
		if (hEditors[i] == hEditorWnd) {
			isWordWrap = false;
			hEditors[i] = 0;
		}
	}

	for (int i = 0; isWordWrap && (i < MAX_TAB_COUNT + MAX_DIALOG_COUNT); i++) {
		if (hEditors[i] == 0) {
			hEditors[i] = hEditorWnd;
			break;
		}
	}

	SendMessage(hEditorWnd, EM_SETTARGETDEVICE,(WPARAM)NULL, !isWordWrap);
	return isWordWrap;
}

bool toggleTextCase (HWND hEditorWnd) {
	SendMessage(hEditorWnd, WM_SETREDRAW, false, 0);
	CHARRANGE range;
	SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&range);

	int caretPos = -1;
	if (range.cpMin == range.cpMax) {
		caretPos = range.cpMin;
		int lineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, range.cpMin, 0);
		int start = SendMessage(hEditorWnd, EM_LINEINDEX, lineNo, 0);
		int len = SendMessage(hEditorWnd, EM_LINELENGTH, start, 0);
		TCHAR line[len + 3]{0};
		line[0] = len + 1;
		SendMessage(hEditorWnd, EM_GETLINE, lineNo, (WPARAM)line);

		TCHAR breakers[] = TEXT(" \"'`\n\t-;:()[],=<>/");

		for (int pos = range.cpMin - start; (pos > 0) && (_tcschr(breakers, line[pos - 1]) == 0); pos--)
			range.cpMin--;

		for (int pos = range.cpMax - start; (pos < len) && (_tcschr(breakers, line[pos]) == 0); pos++)
			range.cpMax++;
		//
	}
	SendMessage(hEditorWnd, EM_SETSEL, range.cpMin, range.cpMax);

	int size = range.cpMax - range.cpMin;
	TCHAR text[size + 1]{0};
	SendMessage(hEditorWnd, EM_GETSELTEXT, size + 1, (LPARAM)text);
	if (text[0] == _totupper(text[0]))
		_tcslwr(text);
	else
		_tcsupr(text);

	SendMessage(hEditorWnd, EM_REPLACESEL, true, (WPARAM)text);
	SendMessage(hEditorWnd, WM_SETREDRAW, true, 0);

	if (caretPos != -1)
		range = {caretPos, caretPos};
	PostMessage(hEditorWnd, EM_SETSEL, range.cpMin, range.cpMax);
	return true;
}

bool toggleComment (HWND hEditorWnd) {
	SendMessage(hEditorWnd, WM_SETREDRAW, false, 0);

	CHARRANGE range;
	SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&range);

	if (range.cpMin == range.cpMax) {
		int lineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, range.cpMin, 0);
		int charNo = SendMessage(hEditorWnd, EM_LINEINDEX, lineNo, 0);
		int len = SendMessage(hEditorWnd, EM_LINELENGTH, charNo, 0);

		TCHAR line[len + 3]{0};
		line[0] = len + 1;
		SendMessage(hEditorWnd, EM_GETLINE, lineNo, (WPARAM)line);

		int pos = 0;
		while (pos < len && _tcschr(TEXT(" \t"), line[pos]))
			pos++;

		if (len - pos > 2) {
			int start = charNo + pos;
			if (_tcsncmp(line + pos, TEXT("--"), 2) == 0) {
				SendMessage(hEditorWnd, EM_SETSEL, start, start + 2);
				SendMessage(hEditorWnd, EM_REPLACESEL, true, (WPARAM)TEXT(""));
				SendMessage(hEditorWnd, EM_SETSEL, range.cpMin - 2, range.cpMin - 2);
			} else {
				COLORREF cColor = prefs::get("color-comment");
				if (cColor == RichEdit_GetTextColor(hEditorWnd, range.cpMin)) {
					int from = range.cpMin;
					while (from > 0 && cColor == RichEdit_GetTextColor(hEditorWnd, from - 1))
						from--;

					int to = range.cpMin;
					while (cColor == RichEdit_GetTextColor(hEditorWnd, to))
						to++;

					SendMessage(hEditorWnd, EM_SETSEL, from, from + 2);
					SendMessage(hEditorWnd, EM_REPLACESEL, true, (WPARAM)TEXT(""));

					SendMessage(hEditorWnd, EM_SETSEL, to - 4, to - 2);
					TCHAR buf[3];
					SendMessage(hEditorWnd, EM_GETSELTEXT, 0, (WPARAM)buf);
					if (_tcsncmp(buf, TEXT("*/"), 2) == 0) {
						SendMessage(hEditorWnd, EM_REPLACESEL, true, (WPARAM)TEXT(""));
					}
					SendMessage(hEditorWnd, EM_SETSEL, range.cpMin - 2, range.cpMin - 2);
				} else {
					SendMessage(hEditorWnd, EM_SETSEL, start, start);
					SendMessage(hEditorWnd, EM_REPLACESEL, true, (WPARAM)TEXT("--"));
					SendMessage(hEditorWnd, EM_SETSEL, range.cpMin + 2, range.cpMin + 2);
				}
			}
		}
	} else {
		TCHAR* selection = new TCHAR[range.cpMax - range.cpMin + 1];
		TEXTRANGE rg = {range, selection};
		SendMessage(hEditorWnd, EM_GETTEXTRANGE, 0, (LPARAM)&rg);
		TCHAR* trimmed = utils::trim(selection);
		delete [] selection;

		int tLen = _tcslen(trimmed);
		bool hasComment = tLen > 3 && _tcsncmp(trimmed, TEXT("/*"), 2) == 0 && _tcsncmp(trimmed + tLen - 2, TEXT("*/"), 2) == 0;
		if (hasComment) {
			SendMessage(hEditorWnd, EM_SETSEL, range.cpMin, range.cpMax);
			trimmed[tLen - 2] = 0;
			SendMessage(hEditorWnd, EM_REPLACESEL, true, (WPARAM)(trimmed + 2));
		} else {
			SendMessage(hEditorWnd, EM_SETSEL, range.cpMin, range.cpMin);
			SendMessage(hEditorWnd, EM_REPLACESEL, true, (WPARAM)TEXT("/*"));
			SendMessage(hEditorWnd, EM_SETSEL, range.cpMax + 2, range.cpMax + 2);
			SendMessage(hEditorWnd, EM_REPLACESEL, true, (WPARAM)TEXT("*/"));
		}
		delete [] trimmed;
	}

	SendMessage(hEditorWnd, WM_SETREDRAW, true, 0);
	processHighlight(hEditorWnd, true, false, false);

	return true;
}

bool pasteText (HWND hEditorWnd, bool detectCSV) {
	HWND hParentWnd = GetAncestor(hEditorWnd, GA_ROOT);
	TCHAR* clipboard = utils::getClipboardText();
	int len = _tcslen(clipboard);

	// Is clipboard contain tab-table?
	bool isTableData = false;
	if (hParentWnd == hMainWnd) {
		int rowCount = 2;
		int delimCount = 4;
		int colCount[rowCount][delimCount];
		memset(colCount, 0, sizeof(int) * (size_t)(rowCount * delimCount));
		int pos = 0;
		bool inQuote = false;

		for (int rowNo = 0; rowNo < rowCount && pos < len; rowNo++) {
			for (; pos < len; pos++) {
				TCHAR c = clipboard[pos];
				if (!inQuote && c == TEXT('\n'))
					break;

				if (c == TEXT('"'))
					inQuote = !inQuote;

				for (int delimNo = 0; delimNo < delimCount && !inQuote; delimNo++)
					colCount[rowNo][delimNo] += clipboard[pos] == tools::DELIMITERS[delimNo][0];
			}
			pos++;
		}

		for (int delimNo = 0; delimNo < delimCount && !isTableData; delimNo++) {
			isTableData = colCount[0][delimNo] > 0;
			for (int rowNo = 1; rowNo < rowCount; rowNo++)
				isTableData = isTableData && colCount[0][delimNo] == colCount[rowNo][delimNo];
		}
	}

	if (isTableData) {
		TCHAR* lwr = _tcsdup(clipboard);
		_tcslwr(lwr);

		auto hasString = [](const TCHAR* text, const TCHAR* word) {
			bool res = false;
			int tlen = _tcslen(text);
			int wlen = _tcslen(word);
			for (int pos = 0; pos < tlen - wlen && !res; pos++)
				res = _tcsstr(text + pos, word) != NULL && (pos == tlen - wlen - 1 || !_istalnum(text[pos + wlen]));

			return res;
		};

		if ((hasString(lwr, TEXT("select")) && hasString(lwr, TEXT("from"))) ||
			(hasString(lwr, TEXT("delete")) && hasString(lwr, TEXT("from"))) ||
			(hasString(lwr, TEXT("update")) && hasString(lwr, TEXT("set"))) ||
			hasString(lwr, TEXT("insert into")) ||
			hasString(lwr, TEXT("alter table")) ||
			(
				(hasString(lwr, TEXT("create")) || hasString(lwr, TEXT("drop"))) &&
				(hasString(lwr, TEXT("table")) || hasString(lwr, TEXT("view")) || hasString(lwr, TEXT("trigger")) || hasString(lwr, TEXT("index")))
			))
			isTableData = false;

		delete [] lwr;
	}

	if (isTableData) {
		int rc = MessageBox(hMainWnd, TEXT("The clipboard contains a table data.\nWould you like to import it as CSV data?"), TEXT("Info"), MB_YESNOCANCEL | MB_ICONQUESTION);
		isTableData = rc != IDNO;
		if (rc == IDYES) {
			TCHAR path16[MAX_PATH];
			_sntprintf(path16, MAX_PATH, TEXT("%ls\\clipboard.csv"), TMP_PATH);

			FILE* f = _tfopen(path16, TEXT("wb, ccs=UTF-8"));
			if (f == NULL) {
				MessageBox(hParentWnd, TEXT("Can't store the clipboard to a temporary file"), NULL, MB_OK);
				return false;
			}
			char* clipboard8 = utils::utf16to8(clipboard);
			fwrite(clipboard8, len, 1, f);
			delete [] clipboard8;
			fclose(f);

			int rc = DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_IMPORT_CSV), hMainWnd, (DLGPROC)tools::cbDlgImportCSV, (LPARAM)path16);
			if (rc != -1) {
				TCHAR msg16[256];
				_sntprintf(msg16, 255, TEXT("Done.\nImported %i rows."), rc);
				MessageBox(hMainWnd, msg16, TEXT("Info"), MB_OK);
				updateTree(TABLE);
			}

			DeleteFile(path16);
		}
	}

	if (!isTableData && len > 0)
		SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)clipboard);

	delete [] clipboard;
	return true;
}

void switchDialog(HWND hDlg, bool isNext) {
	int i, j;
	for (i = 0; i < MAX_DIALOG_COUNT && (hDialogs[i] != hDlg); i++);
	if (isNext) {
		for (j = i + 1; j <= MAX_DIALOG_COUNT && (hDialogs[j] == 0); j++);
		if (j == MAX_DIALOG_COUNT)
			for (j = 0; j <= i && (hDialogs[j] == 0); j++);
	} else {
		for (j = i - 1; j != -1 && (hDialogs[j] == 0); j--);
		if (j == -1)
			for (j = MAX_DIALOG_COUNT - 1; j >= i && (hDialogs[j] == 0); j--);
	}

	if (i != j)
		SetForegroundWindow(hDialogs[j]);
}

void saveQuery(const char* storage, const char* query) {
	char sql[256];
	sprintf(sql, "replace into %s (query, time) values (?1, ?2)", storage);

	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, sql, -1, &stmt, 0)) {
		sqlite3_bind_text(stmt, 1, query, strlen(query), SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 2, std::time(0));
		sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
}

// Used for Edit controls
// Impelements Ctr + A/Backspace/Del shortcuts
bool processEditKeys(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	bool isControl = HIWORD(GetKeyState(VK_CONTROL));

	// Prevent Beep
	if (msg == WM_CHAR && isControl) {
		int vkKey = MapVirtualKey(LOBYTE(HIWORD(lParam)), MAPVK_VSC_TO_VK);
		return vkKey == 0x41 || vkKey == VK_BACK || vkKey == VK_DELETE;
	}

	if (msg == WM_KEYDOWN && wParam == 0x41 && isControl) { // Ctrl + A
		SendMessage(hWnd, EM_SETSEL, 0, -1);
		return true;
	}

	if (msg == WM_KEYDOWN && (wParam == VK_BACK || wParam == VK_DELETE) && isControl) {
		int size = GetWindowTextLength(hWnd);
		TCHAR text[size + 1]{0};
		GetWindowText(hWnd, text, size + 1);

		int start = 0, end = 0;
		SendMessage(hWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

		if (start != end) {
			SendMessage(hWnd, EM_REPLACESEL, true, (LPARAM)TEXT(""));
		} else {
			int pos = start;

			if (wParam == VK_BACK) {
				bool isSpace = pos > 0 && text[pos - 1] == TEXT(' ');
				while ((pos > 0 && text[pos - 1] != TEXT(' ')) || isSpace) {
					isSpace = isSpace && text[pos - 1] == TEXT(' ');
					pos--;
				}

				for (int i = pos; i < size - start + pos; i++)
					text[i] = text[start - pos + i];
				text[size - start + pos] = 0;
			}

			if (wParam == VK_DELETE) {
				for (; pos < size && text[pos] != TEXT(' '); pos++);
				if (pos == start && start < size)
					pos = start + 1;

				for (int i = start; i < size + start - pos; i++)
					text[i] = text[pos - start + i];
				text[size + start - pos] = 0;
				pos = start;
			}

			SetWindowText(hWnd, text);
			SendMessage(hWnd, EM_SETSEL, pos, pos);
		}
		return true;
	}

	return false;
}

unsigned int __stdcall checkUpdate (void* data) {
	time_t t = time(0);
	tm* now = localtime(&t);
	int date = (now->tm_year + 1900) * 10000 + (now->tm_mon + 1) * 100 + now->tm_mday;

	if (date != prefs::get("last-update-check")) {
		char* buf8 = utils::httpRequest("GET", "api.github.com/repos/little-brother/sqlite-gui/commits/master");
		if (buf8 == 0)
			return 0;

		TCHAR* buf16 = utils::utf8to16(buf8);
		TCHAR* message = _tcsstr(buf16, TEXT("\"message\""));
		TCHAR* tree = _tcsstr(buf16, TEXT("\"tree\""));

		if (message && _tcsstr(message, TEXT(GUI_VERSION)) == 0) {
			TCHAR msg[strlen(buf8)]{0};
			_tcsncpy(msg, message + 11, _tcslen(message) - _tcslen(tree) - 13);
			TCHAR* msg2 = utils::replaceAll(msg, TEXT("\\n"), TEXT("\n"));
			_sntprintf(msg, 1023, TEXT("%ls\n\nWould you like to download it?"), msg2);
			if (IDYES == MessageBox(hMainWnd, msg, TEXT("New version was released"), MB_YESNO))
				ShellExecute(0, 0, TEXT("https://github.com/little-brother/sqlite-gui/releases/latest"), 0, 0 , SW_SHOW);
			delete [] msg2;
		}
		delete [] buf16;
		delete [] buf8;

		prefs::set("last-update-check", date);
	}

	return 1;
}

void createResultControls(HWND hTabWnd, int resultNo) {
	HWND hRowsWnd = createResultList(hTabWnd, resultNo);
	ShowWindow(hRowsWnd, SW_HIDE); // hEdit = 0 if hRowsWnd is not visible

	HWND hMessageWnd = CreateWindow(TEXT("RICHEDIT50W"), NULL, WS_CHILD | WS_CLIPSIBLINGS | ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL, 20, 20, 100, 100, hTabWnd, (HMENU)IntToPtr(IDC_TAB_MESSAGE + resultNo), GetModuleHandle(0), NULL);
	SendMessage(hMessageWnd, EM_SETTARGETDEVICE, 0, 0);
	SendMessage(hMessageWnd, EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_BTNFACE));
	CreateWindow(WC_LISTBOX, NULL, WS_CHILD, 300, 0, 400, 100, hRowsWnd, (HMENU)IDC_REFLIST, GetModuleHandle(0), 0);

	HWND hPreviewWnd = CreateWindow(WC_STATIC, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_CENTER | WS_BORDER | SS_CENTERIMAGE, 20, 20, 100, 100, hTabWnd, (HMENU)IntToPtr(IDC_TAB_PREVIEW + resultNo), GetModuleHandle(0), NULL);
	SetProp(hPreviewWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hPreviewWnd, GWLP_WNDPROC, (LONG_PTR)dialogs::cbNewValueViewer));
	SetProp(hPreviewWnd, TEXT("INFO"), (HANDLE)(new TCHAR[255]));
	SetProp(hPreviewWnd, TEXT("EXT"), (HANDLE)(new TCHAR[32]));
	SendMessage(hPreviewWnd, WM_SETFONT, (LPARAM)hFont, 0);
	CreateWindowEx(WS_EX_TOPMOST, WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | SS_NOTIFY, 20, 20, 100, 100, hPreviewWnd, (HMENU)IDC_PREVIEW_INFO, GetModuleHandle(0), NULL);

	SetWindowLongPtr(hMessageWnd, GWLP_USERDATA, resultNo);
	SetWindowLongPtr(hPreviewWnd, GWLP_USERDATA, resultNo);
}

HWND createResultList(HWND hParentWnd, int resultNo) {
	HWND hWnd = CreateWindow(WC_LISTVIEW, NULL, WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP | WS_CHILD | LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA, 0, 0, 0, 0, hParentWnd, (HMENU)IntToPtr(IDC_TAB_ROWS + resultNo), GetModuleHandle(0), NULL);
	HWND hHeader = ListView_GetHeader(hWnd);

	for (int colNo = 1; colNo < MAX_RESULT_COLUMN_COUNT; colNo++) {
		// Use WS_BORDER to vertical text aligment
		HWND hEdit = CreateWindowEx(WS_EX_TOPMOST, WC_EDIT, NULL, WS_VISIBLE | ES_CENTER | ES_AUTOHSCROLL | WS_CHILD | WS_TABSTOP | WS_BORDER,
			0, 0, 0, 0, hHeader, (HMENU)(INT_PTR)(IDC_HEADER_EDIT + colNo), GetModuleHandle(0), NULL);
		SendMessage(hEdit, WM_SETFONT, (LPARAM)hFont, TRUE);
		cbOldResultTabFilterEdit = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)cbNewResultTabFilterEdit);
	}

	SetProp(hHeader, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hHeader, GWLP_WNDPROC, (LONG_PTR)cbNewResultHeader));
	EnumChildWindows(hHeader, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETPARENTFONT);

	SetWindowLongPtr(hWnd, GWLP_USERDATA, resultNo);
	SendMessage(hWnd, WM_SETFONT, (LPARAM)hFont, false);

	return hWnd;
}

bool saveResultToTable(sqlite3* db, const TCHAR* table16, int tabNo, int resultNo, int searchNext) {
	HWND hTabWnd = tabs[tabNo].hTabWnd;
	HWND hListWnd = GetDlgItem(hTabWnd, IDC_TAB_ROWS + resultNo);
	HWND hHeader = ListView_GetHeader(hListWnd);
	int colCount = ListView_GetColumnCount(hListWnd);
	int rowCount = searchNext == LVNI_ALL ? ListView_GetItemCount(hListWnd) : ListView_GetSelectedCount(hListWnd);

	bool rc = true;

	TCHAR* schema16 = utils::getTableName(table16, true);
	TCHAR* tablename16 = utils::getTableName(table16, false);
	TCHAR* qschema16 = utils::double_quote(schema16);
	TCHAR* qtablename16 = utils::double_quote(tablename16);

	if (searchNext == LVNI_ALL && rowCount == prefs::get("row-limit")) {
		int len = _tcslen(tabs[tabNo].tabTooltips[resultNo]) + 512;
		TCHAR query16[len + 1];
		_sntprintf(query16, len, TEXT("create table %ls.%ls as %ls"), qschema16, qtablename16, tabs[tabNo].tabTooltips[resultNo]);
		char* query8 = utils::utf16to8(query16);
		rc = sqlite3_exec(db, query8, 0, 0, 0) == SQLITE_OK;
		delete [] query8;
	} else {
		int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
		byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
		unsigned char** blobs = (unsigned char**)GetProp(hListWnd, TEXT("BLOBS"));

		char create8[colCount * 255 + 1024]{0};
		char insert8[colCount * 255 + 1024]{0};

		char* qschema8 = utils::utf16to8(qschema16);
		char* qtablename8 = utils::utf16to8(qtablename16);
		sprintf(create8, "create table %s.%s (", qschema8, qtablename8);
		sprintf(insert8, "insert into %s.%s (", qschema8, qtablename8);
		delete [] qschema8;
		delete [] qtablename8;

		for (int colNo = 1; colNo < colCount; colNo++) {
			TCHAR colname16[256];
			Header_GetItemText(hHeader, colNo, colname16, 255);

			char* colname8 = utils::utf16to8(colname16);
			strcat(create8, "\"");
			strcat(create8, colname8);
			strcat(create8, "\"");

			strcat(insert8, "\"");
			strcat(insert8, colname8);
			strcat(insert8, "\"");
			delete [] colname8;

			if (datatypes) {
				strcat(create8,
					datatypes[colNo] == SQLITE_FLOAT ? " real" :
					datatypes[colNo] == SQLITE_INTEGER ? " integer" :
					datatypes[colNo] == SQLITE_TEXT ? " text" :
					""
				);
			}

			if (colNo != colCount - 1) {
				strcat(create8, ", ");
				strcat(insert8, ", ");
			}
		}
		strcat(create8, ")");

		char placeholders8[(colCount  - 1) * 2]{0}; // count = 3 => ?, ?, ?
		for (int i = 0; i < (colCount - 1) * 2 - 1; i++)
			placeholders8[i] = i % 2 ? ',' : '?';

		strcat(insert8, ") values (");
		strcat(insert8, placeholders8);
		strcat(insert8, ")");

		rc = sqlite3_exec(db, "begin", 0, 0, 0) == SQLITE_OK;
		rc = rc && (sqlite3_exec(db, create8, 0, 0, 0) == SQLITE_OK);
		if (rc) {
			sqlite3_stmt *stmt;
			if (SQLITE_OK == sqlite3_prepare_v2(db, insert8, -1, &stmt, 0)) {
				int rowNo = -1;

				while(rc && (rowNo = ListView_GetNextItem(hListWnd, rowNo, searchNext)) != -1) {
					for(int colNo = 1; colNo < colCount; colNo++) {
						TCHAR* val16 = ListView_GetItemValueText(hListWnd, rowNo, colNo);
						int no = colNo + resultset[rowNo] * (colCount - 1);
						const unsigned char* blob = datatypes && blobs && datatypes[no] == SQLITE_BLOB ? blobs[no] : 0;
						if (!blob) {
							char* val8 =  utils::utf16to8(val16);
							dbutils::bind_variant(stmt, colNo, val8, datatypes && datatypes[no] == SQLITE_TEXT);
							delete [] val8;
						} else {
							sqlite3_bind_blob(stmt, colNo, blob + 4, utils::getBlobSize(blob) - 4, SQLITE_TRANSIENT);
						}
					}

					rc = sqlite3_step(stmt) == SQLITE_DONE;
					sqlite3_reset(stmt);
				}
			}
			sqlite3_finalize(stmt);
		}
	}

	if (!rc)
		showDbError(hMainWnd);

	delete [] schema16;
	delete [] tablename16;
	delete [] qschema16;
	delete [] qtablename16;

	sqlite3_exec(db, rc ? "commit" : "rollback", 0, 0, 0);

	return rc;
}

bool startHttpServer() {
	if (!prefs::get("http-server"))
		return false;

	sqlite3* _db;
	if (openConnection(&_db, sqlite3_db_filename(db, 0)))
		http::start(prefs::get("http-server-port"), _db, prefs::get("http-server-debug"));

	return true;
}

bool stopHttpServer() {
	return http::stop();
}

HFONT loadFont() {
	HDC hDC = GetDC(hMainWnd);
	char* fontFamily8 = prefs::get("font-family", "Courier New"); // Only TrueType
	TCHAR* fontFamily16 = utils::utf8to16(fontFamily8);
	HFONT hFont = CreateFont (-MulDiv(prefs::get("font-size"), GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0,
		FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		fontFamily16);

	ReleaseDC(hMainWnd, hDC);
	delete [] fontFamily16;
	delete [] fontFamily8;

	return hFont;
}

TCHAR* getHelp(TCHAR* word, TCHAR* altword) {
	TCHAR* res16 = 0;

	// Is function or pragma?
	sqlite3_stmt *stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
		"select signature || char(10) || char(10) || description || char(10) || char(10) || example || iif(source is not null, char(10) || char(10) || 'Source: ' || source, '')" \
		"from help where (keyword = lower(?1) or alias = lower(?1)) and not exists(select 1 from main.addons a where a.name = source and type = 0 and enable = 0) " \
		"union all "\
		"select name || iif(help is not null, char(10) || char(10) || help, '') || char(10) || char(10) || 'Source: custom function' from functions where lower(name) = lower(?1)", -1, &stmt, 0)) {
		char* word8 = utils::utf16to8(word);
		sqlite3_bind_text(stmt, 1, word8, strlen(word8), SQLITE_TRANSIENT);
		delete [] word8;

		if (SQLITE_ROW == sqlite3_step(stmt))
			res16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
	}
	sqlite3_finalize(stmt);

	// Is table
	if (!res16) {
		for (int i = 0; (res16 == 0) && (i < (altword != 0 ? 2 : 1)); i++) {
			TCHAR* txt = i == 0 ? word: altword;

			TCHAR* tablename16 = utils::getTableName(txt, false);
			TCHAR* schema16 = utils::getTableName(txt, true);
			char* tablename8 = utils::utf16to8(tablename16);
			char* schema8 = utils::utf16to8(schema16);

			char type8[10] = {0};
			if (strcmp(tablename8, "sqlite_master") == 0 ||
				strcmp(tablename8, "sqlite_sequence") == 0 ||
				strcmp(tablename8, "dbstat") == 0 ||
				strcmp(tablename8, "sqlite_stat4") == 0) {
				snprintf(type8, 9, "table");
			} else {
				int len = strlen(tablename8) + strlen(schema8) + 512;
				char query8[len + 1];
				snprintf(query8, len, "select type from \"%s\".sqlite_master where type in ('table', 'view') and tbl_name = ?1", schema8);

				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
					sqlite3_bind_text(stmt, 1, tablename8, strlen(tablename8), SQLITE_TRANSIENT);

					if (SQLITE_ROW == sqlite3_step(stmt) && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
						snprintf(type8, 9, (char*)sqlite3_column_text(stmt, 0));
				}
				sqlite3_finalize(stmt);
			}

			if (strlen(type8)) {
				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db,
					"select upper(?1) || ' (' || ?3 || ')' || char(10) || char(10) || group_concat(name || ': ' || type || iif(pk,' [pk]',''), char(10)) from pragma_table_xinfo where schema = ?2 and arg = ?1", -1, &stmt, 0)) {
					sqlite3_bind_text(stmt, 1, tablename8, strlen(tablename8), SQLITE_TRANSIENT);
					sqlite3_bind_text(stmt, 2, schema8, strlen(schema8), SQLITE_TRANSIENT);
					sqlite3_bind_text(stmt, 3, type8, strlen(type8), SQLITE_TRANSIENT);

					if (SQLITE_ROW == sqlite3_step(stmt) && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
						res16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
				}
				sqlite3_finalize(stmt);
			}

			delete [] tablename8;
			delete [] schema8;
			delete [] tablename16;
			delete [] schema16;
		}
	}

	if (res16) {
		TCHAR *tres16 = utils::trim(res16);
		delete [] res16;
		res16 = tres16;
	}

	if (res16 && _tcslen(res16) == 0) {
		delete [] res16;
		res16 = 0;
	}

	return res16;
}

void createTooltip(HWND hWnd) {
	if (hTooltipWnd)
		return;

	hTooltipWnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, GetModuleHandle(0), NULL);
	TOOLINFO ti{0};
	ti.cbSize = TTTOOLINFOW_V1_SIZE;
	ti.hwnd = hWnd;
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS |  TTF_TRACK;
	ti.uId = (UINT_PTR)hWnd;

	SendMessage(hTooltipWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
	SendMessage(hTooltipWnd, TTM_SETMAXTIPWIDTH, 0, 400);
}

void showTooltip(int x, int y, TCHAR* text16) {
	TOOLINFO ti{0};
	ti.cbSize = TTTOOLINFOW_V1_SIZE;
	SendMessage(hTooltipWnd, TTM_ENUMTOOLS, 0, (LPARAM)&ti);

	ti.lpszText = text16;
	SendMessage(hTooltipWnd, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	SendMessage(hTooltipWnd, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(x, y));
	SendMessage(hTooltipWnd, TTM_TRACKACTIVATE, TRUE, (LPARAM)(LPTOOLINFO) &ti);
}

void hideTooltip() {
	SendMessage(hTooltipWnd, TTM_TRACKACTIVATE, FALSE, 0);
}

void showPopup(const TCHAR* text16, HWND hParentWnd) {
	SetWindowText(hAutoCompleteHelp, text16);

	if (text16 == 0) {
		ShowWindow(hAutoCompleteHelp, SW_HIDE);
		return;
	}

	DRAWTEXTPARAMS p {0};
	p.cbSize = sizeof(DRAWTEXTPARAMS);
	p.iLeftMargin = 10;
	p.iRightMargin = 10;

	RECT rc {0};
	HDC hDC = GetDC(hAutoCompleteHelp);
	HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

	DrawTextEx(hDC, (TCHAR*)text16, -1, &rc, DT_LEFT | DT_NOCLIP | DT_CALCRECT, &p);
	SelectObject(hDC, hOldFont);
	ReleaseDC(hAutoCompleteHelp, hDC);

	SetWindowPos(hAutoCompleteHelp, 0, 0, 0, rc.right + 20, rc.bottom + 10, SWP_NOZORDER);
	utils::alignDialog(hAutoCompleteHelp, hParentWnd);
	ShowWindow(hAutoCompleteHelp, SW_SHOW);
	RedrawWindow(hAutoCompleteHelp, 0, 0, RDW_ERASE);
}

void logger(ULONG_PTR type, TCHAR* msg16) {
	if (prefs::get("use-logger") == 0)
		return;

	if (hLoggerWnd == 0 || !IsWindow(hLoggerWnd))
		hLoggerWnd = FindWindow(TEXT("logger-logger-class"), NULL);

	if (hLoggerWnd == 0) {
		TCHAR loggerPath16[MAX_PATH + 1];
		_sntprintf(loggerPath16, MAX_PATH, TEXT("%ls\\logger.exe"), APP_PATH);
		if (utils::isFileExists(loggerPath16)) {
			ShellExecute(0, 0, loggerPath16, 0, 0, SW_SHOW);
			Sleep(100);
			hLoggerWnd = FindWindow(TEXT("logger-logger-class"), NULL);
		}
	}

	if (hLoggerWnd == 0)
		return;

	COPYDATASTRUCT data = {0};
	data.dwData = type;
	data.cbData = msg16 ? 2 * _tcslen(msg16) + 2 : 0;
	data.lpData = msg16;
	SendMessage(hLoggerWnd, WM_COPYDATA, (WPARAM)hMainWnd, (LPARAM)&data);
}

/* Simple query formatter */
//  0 - casual, 1 - -- comment, 2 - / * comment * /, 3 - 'quoted', 4 - "quoted", 5 - `quoted`
int getTextMode(TCHAR c, TCHAR cn, TCHAR cp, int mode, int len) {
	return
		c == TEXT('\'') && mode == 0 ? 3 :
		c == TEXT('"') && mode == 0 ? 4 :
		c == TEXT('`') && mode == 0 ? 5 :

		c == TEXT('\'') && mode == 3 && cp != c && len > 0 ? 0 :
		c == TEXT('"') && mode == 4 && cp != c && len > 0 ? 0 :
		c == TEXT('`') && mode == 5 && cp != c && len > 0 ? 0 :

		c == TEXT('-') && cn == TEXT('-') && mode == 0 ? 1 :
		c == TEXT('/') && cn == TEXT('*') && mode == 0 ? 2 :
		cp == TEXT('*') && c == TEXT('/') && mode == 2 ? 0 :
		cp == TEXT('\n') && mode == 1 && !(c == TEXT('-') && cn == TEXT('-')) ? 0 :
		mode;
}

int processBlock (int startPos, int startIndent, const TCHAR* input, int* indents, bool* newlines) {
	int pos = startPos;
	int currIndent = startIndent;
	int mode = 0;
	int len = 0;

	bool isStatement =
		utils::isStartBy(input, pos, TEXT("select")) ||
		utils::isStartBy(input, pos, TEXT("insert into")) ||
		utils::isStartBy(input, pos, TEXT("delete from"));

	bool isWith = utils::isStartBy(input, pos, TEXT("with"));

	while (pos < (int)_tcslen(input)) {
		TCHAR c = input[pos];
		TCHAR cn = input[pos + 1];
		TCHAR cp = pos > 0 ? input[pos - 1] : 0;

		int pmode = mode;
		mode = getTextMode(c, cn, cp, mode, len);
		len = mode != pmode || (mode == pmode && mode >= 3 && cp == c && len > 1) ? 1 : len + 1;

		if (mode == 0) {
			if (input[pos] == TEXT('(')) {
				if (isWith) {
					newlines[pos] = true;
					indents[pos] = startIndent - 1;
					currIndent = startIndent - 1;
				}
				pos += processBlock(pos + 1, currIndent + 2, input, indents, newlines) + 1;
				currIndent = startIndent;
				continue;
			}

			if (input[pos] == TEXT(')')) {
				break;
			}

			if (isWith && (input[pos - 2] == ',' || utils::isStartBy(input, pos, TEXT("select")))) {
				newlines[pos] = true;
				indents[pos] = startIndent;
				currIndent = startIndent;
			}

			if (isWith && utils::isStartBy(input, pos, TEXT("select"))) {
				isWith = false;
				isStatement = true;
			}

			// 6 - strlen + 1
			bool isValues = pos > 6 && utils::isStartBy(input, pos - 6, TEXT("values"));
			int indent =
				utils::isStartBy(input, pos, TEXT("delete from")) ? 1 :
				utils::isStartBy(input, pos, TEXT("values")) ? 1 :
				isValues ? 7 :
				!utils::isPrecedeBy(input, pos, TEXT("delete")) && utils::isStartBy(input, pos, TEXT("from")) ? 3 :
				utils::isStartBy(input, pos, TEXT("inner join")) ? 3 :
				utils::isStartBy(input, pos, TEXT("left join")) ? 3 :
				utils::isStartBy(input, pos, TEXT("left outer join")) ? 3 :
				!utils::isPrecedeBy(input, pos, TEXT("left")) && !utils::isPrecedeBy(input, pos, TEXT("inner")) && !utils::isPrecedeBy(input, pos, TEXT("outer")) && utils::isStartBy(input, pos, TEXT("join")) ? 3 :
				utils::isStartBy(input, pos, TEXT("where")) ? 2 :
				utils::isStartBy(input, pos, TEXT("order by")) ? 2 :
				utils::isStartBy(input, pos, TEXT("group by")) ? 2 :
				utils::isStartBy(input, pos, TEXT("having")) ? 1 :
				utils::isStartBy(input, pos, TEXT("limit")) ? 2 :
				utils::isStartBy(input, pos, TEXT("set")) ? 4 :
				pos > 0 && input[pos - 1] == TEXT(',') && isStatement ? (utils::isStartBy(input, pos + 1, TEXT("--")) || utils::isStartBy(input, pos + 1, TEXT("/*")) ? 1 : 7) :
				pos > 0 && input[pos - 1] == TEXT(';') ? 1 :
				0;

			if (indent > 0) {
				indent--;
				isStatement = (pos > 0 && input[pos - 1] == TEXT(',')) || isValues;
				indents[pos] = pos > 0 && input[pos - 1] == TEXT(';') ? 0 : indent + startIndent;
				newlines[pos] = pos > 0 && input[pos - 1] != TEXT('\n') && !utils::isStartBy(input, pos + 1, TEXT("--")) && !utils::isStartBy(input, pos + 1, TEXT("/*"));
			}
			currIndent = indent > 0 ? startIndent + indent : currIndent + 1;
		} else {
			currIndent++;
		}

		pos++;
	}

	return pos - startPos + 1;
}

TCHAR* formatQuery (TCHAR* query16) {
	int qLen = _tcslen(query16);

	logger(LOGGER_FORMAT, query16);

	TCHAR* buf = new TCHAR[qLen + 1]{0};
	int mode = 0;
	int pos = 0;
	int bPos = 0;
	int len = 0;

	// Minify
	while (pos < (int)_tcslen(query16)) {
		TCHAR c = query16[pos];
		TCHAR cn = query16[pos + 1];
		TCHAR cp = bPos > 0 ? buf[bPos - 1] : 0;

		int pmode = mode;
		mode = getTextMode(c, cn, cp, mode, len);
		len = mode != pmode || (mode == pmode && mode >= 3 && cp == c && len > 1) ? 1 : len + 1;

		if (mode == 0) {
			if (_tcschr(TEXT(" \t\r\n"), c) == 0) {
				buf[bPos] = c;
				bPos++;
			}

			if (_tcschr(TEXT("\t\r\n"), c) != 0 && (_istalpha(cn) || cn == TEXT('_')) && (_istalnum(cp) || cp == TEXT('_'))) {
				buf[bPos] = ' ';
				bPos++;
			}

			// Keep a line starting comment --
			if (bPos > 0 && buf[bPos - 1] != TEXT('\n') && c == TEXT('\n') && cn == TEXT('-') && query16[pos + 2] == TEXT('-')) {
				buf[bPos] = c;
				bPos++;
			}

			if (c == TEXT(' ') && (_istalnum(cn) || cn == TEXT('_')) && (_istalnum(cp) || cp == TEXT('_'))) {
				buf[bPos] = TEXT(' ');
				bPos++;
			}
		} else {
			buf[bPos] = c;
			bPos++;
		}

		pos++;
	}

	// Add spaces
	pos = 0;
	bPos = 0;
	mode = 0;
	len = 0;
	TCHAR* buf2 = new TCHAR[MAX_TEXT_LENGTH]{0};
	while (pos < (int)_tcslen(buf)) {
		TCHAR c = buf[pos];
		TCHAR cn = buf[pos + 1];
		TCHAR cp = pos > 0 ? buf[pos - 1] : 0;

		int pmode = mode;
		mode = getTextMode(c, cn, cp, mode, len);
		len = mode != pmode || (mode == pmode && mode >= 3 && cp == c && len > 1) ? 1 : len + 1;

		if (mode == 0 && pos > 0 && cp != TEXT(' ') && c != cp && c != TEXT(';') && (
			_tcschr(TEXT("(<!*+"), c) != 0 ||
			(_tcschr(TEXT("'\"`"), cp) != 0 && _tcschr(TEXT(".,;)"), c) == 0 && pmode == 0) ||
			(c == TEXT('>') && cp != TEXT('<')) ||
			(c == TEXT('=') && cp != TEXT('!')) ||
			(c == TEXT('/') && cp != TEXT('*')) ||
			(c == TEXT('-') && cp != TEXT('-') && cn != TEXT('-')) ||
			(c == TEXT('|') && cn == TEXT('|'))
			)) {

			bool isFunc = false;
			if (c == TEXT('(')) {
				int spos = pos - 1;
				while (spos > 0 && (_istalnum(buf[spos]) || buf[spos] == TEXT('_')))
					spos--;
				TCHAR fname[pos - 1 - spos + 1]{0};
				_tcsncpy(fname, buf + spos + 1, pos - spos - 1);
				_tcslwr(fname);
				for (int i = 0; !isFunc && FUNCTIONS[i]; i++)
					isFunc = _tcscmp(fname, FUNCTIONS[i]) == 0;
				isFunc = isFunc || (_tcsncmp(fname, TEXT("pragma_"), 7) == 0);
			}

			if (!isFunc && cp != TEXT(')')) {
				buf2[bPos] = ' ';
				bPos++;
			}
		}

		buf2[bPos] = c;
		bPos++;

		if (mode == 0 && (
			(_tcschr(TEXT("=>,*+"), c) != 0) ||
			(c == TEXT(')') && cn != TEXT(')') && cn != TEXT(',') && cn != TEXT(';')) ||
			(c == TEXT('<') && cn != TEXT('>')) ||
			(c == TEXT('!') && cn != TEXT('=')) ||
			(c == TEXT('/') && cn != TEXT('*')) ||
			(c == TEXT('-') && cn != TEXT('-')) ||
			(c == TEXT('|') && cp == TEXT('|')) ||
			(_tcschr(TEXT("(."), c) == 0 && _tcschr(TEXT("\"'`"), cn) && pmode == 0)
			)) {
			buf2[bPos] = TEXT(' ');
			bPos++;
		}

		pos++;
	}

	// Change keyword/function case
	int keywordCase = prefs::get("format-keyword-case");
	int functionCase = prefs::get("format-function-case");
	if (keywordCase || functionCase) {
		pos = 0;
		mode = 0;
		len = 0;
		while (pos < (int)_tcslen(buf2)) {
			TCHAR c = buf2[pos];
			TCHAR cn = buf2[pos + 1];
			TCHAR cp = pos > 0 ? buf2[pos - 1] : 0;

			int pmode = mode;
			mode = getTextMode(c, cn, cp, mode, len);
			len = mode != pmode || (mode == pmode && mode >= 3 && cp == c && len > 1) ? 1 : len + 1;

			if (mode == 0 && (pos == 0 || (pos > 0 && !_istalpha(cp)))) {
				int epos = pos;
				while (epos < (int)_tcslen(buf2) && (_istalnum(buf2[epos]) || buf2[epos] == TEXT('_')))
					epos++;

				if (epos != pos) {
					TCHAR word[epos - pos + 1]{0};
					_tcsncpy(word, buf2 + pos, epos - pos);
					_tcslwr(word);

					for (int i = 0; keywordCase && SQL_KEYWORDS[i]; i++) {
						if ((int)_tcscmp(word, SQL_KEYWORDS[i]) == 0) {
							for (int j = 0; j < (int)_tcslen(word); j++)
								buf2[pos + j] = keywordCase == 1 ? _totlower(buf2[pos + j]) : _totupper(buf2[pos + j]);
							break;
						}
					}

					for (int i = 0; functionCase && FUNCTIONS[i]; i++) {
						if (_tcscmp(word, FUNCTIONS[i]) == 0) {
							for (int j = 0; j < (int)_tcslen(word); j++)
								buf2[pos + j] = keywordCase == 1 ? _totlower(buf2[pos + j]) : _totupper(buf2[pos + j]);
							break;
						}
					}
				}
			}

			pos++;
		}
	}

	// Format
	pos = 0;
	bPos = 0;

	// calculate indents
	int indents[_tcslen(buf2)]{0};
	bool newlines[_tcslen(buf2)]{0};
	processBlock(0, 0, buf2, indents, newlines);

	TCHAR* buf3 = new TCHAR[_tcslen(buf2) * 10 + 1]{0};
	for (int pos = 0; pos < (int)_tcslen(buf2); pos++) {
		if (newlines[pos]) {
			buf3[bPos] = TEXT('\n');
			bPos++;
		}

		for (int i = 0; i < indents[pos]; i++) {
			buf3[bPos] = TEXT(' ');
			bPos++;
		}

		buf3[bPos] = buf2[pos];
		bPos++;
	}

	delete [] buf;
	delete [] buf2;

	logger(LOGGER_FORMAT, 0);

	return buf3;
}

int aiGetSid() {
	int sid = 1;
	sqlite3_stmt* stmt;
	if ((SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select coalesce(max(sid), 0) + 1 from ai_messages", -1, &stmt, 0)) && (SQLITE_ROW == sqlite3_step(stmt)))
		sid = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	return sid;
}

bool aiNeedStop(int sid) {
	int tabCount = SendMessage(hMainTabWnd, WMU_TAB_GET_COUNT, 0, 0);
	for (int tabNo = 0; tabNo < tabCount; tabNo++) {
		if (tabs[tabNo].aiSid == sid)
			return tabs[tabNo].aiNeedStop;
	}

	return false;
}

TCHAR* aiRequest(const TCHAR* promptId, const TCHAR* arg1, const TCHAR* arg2, int sid) {
	TCHAR* res16 = 0;

	char* prompt8 = 0;
	char* headers8 = 0;
	char* options8 = 0;
	char* data8 = 0;
	char* tables8 = 0;
	char* functions8 = 0;
	char* init8 = 0;
	char* details8 = 0;
	char* messages8 = 0;

	if (sid == 0)
		sid = aiGetSid();

	if (aiNeedStop(sid))
		return 0;

	char* url8 = prefs::get("ai-api-url", DEFAULT_AI_API_URL);
	char* model8 = prefs::get("ai-model", "openai-large");
	char* promptId8 = utils::utf16to8(promptId);
	bool hasDbAccess = prefs::get("ai-has-db-access");

	auto getValue = [](sqlite3_stmt* stmt, int colNo) {
		char* param8 = 0;
		if (sqlite3_column_type(stmt, colNo) != SQLITE_NULL) {
			param8 = new char[sqlite3_column_bytes(stmt, colNo) + 1] {0};
			strcpy(param8, (const char*)sqlite3_column_text(stmt, colNo));
		}

		return param8;
	};

	auto registerMessage = [] (int sid, const char* role8, const char* content8, const char* details8 = 0) {
		bool isAux = false;

		if (strcmp(role8, "user") == 0) {
			sqlite3_stmt* stmt;
			if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select is_aux and instr(content, 'CONFIRMED') = 0 from ai_messages where id = (select max(id) from ai_messages where sid = ?1 and role = 'assistant')", -1, &stmt, 0)) {
				sqlite3_bind_int(stmt, 1, sid);
				sqlite3_step(stmt);
				isAux = sqlite3_column_int(stmt, 0) == 1;
			}
			sqlite3_finalize(stmt);
		}

		if (strcmp(role8, "assistant") == 0 && (strstr(content8, "GET:") == content8 || strstr(content8, "QUERY:") == content8 || strstr(content8, "CONFIRMED") == content8))
			isAux = true;

		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "insert into ai_messages (sid, role, content, details, is_aux) values (?1, ?2, ?3, ?4, ?5)", -1, &stmt, 0)) {
			sqlite3_bind_int(stmt, 1, sid);
			sqlite3_bind_text(stmt, 2, role8, -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, content8, -1, SQLITE_TRANSIENT);

			if (details8) {
				sqlite3_bind_text(stmt, 4, details8, -1, SQLITE_TRANSIENT);
			} else {
				sqlite3_bind_null(stmt, 4);
			}

			sqlite3_bind_int(stmt, 5, isAux);

			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	};

	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(db,
			"with t (name, type, columns) as (select t.name, t.type, " \
			"iif(t.type in ('table', 'view'), group_concat(c.name || iif(c.pk, ' [pk]', '') , ', '), null) columns " \
			"from sqlite_master t left join pragma_table_xinfo c on t.tbl_name = c.arg " \
			"where t.sql is not null and t.type in ('table', 'view') and t.name <> 'sqlite_sequence' " \
			"group by t.type, t.name order by t.type, t.name) " \
			"select group_concat(iif(type = 'view', '(view)', '') || name || '(' || columns || ')', ', ') from t",
			-1, &stmt, 0)) {

		if (SQLITE_ROW == sqlite3_step(stmt))
			tables8 = getValue(stmt, 0);
	} else {
		if (SQLITE_OK == sqlite3_prepare_v2(db,
			"select group_concat(iif(type = 'view', '(view)', '') || name, ', ') from sqlite_master where type in ('table', 'view') and name <> 'sqlite_sequence'",
			-1, &stmt, 0)) {
				if (SQLITE_ROW == sqlite3_step(stmt))
					tables8 = getValue(stmt, 0);
			}
	}
	sqlite3_finalize(stmt);

	if (SQLITE_OK == sqlite3_prepare_v2(db, "select group_concat(name, ', ') from pragma_function_list where builtin = 0", -1, &stmt, 0)) {
		if (SQLITE_ROW == sqlite3_step(stmt))
			functions8 = getValue(stmt, 0);
	}
	sqlite3_finalize(stmt);

	if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
			"with " \
			"p(value) as (select value from ai_config where model in (?1, 'default') and param = ?2 order by iif(model <> 'default', 0, 1))," \
			"ps(value) as (select json_extract(details, '$.query') from ai_messages where sid = ?7 and role = 'user' order by id asc limit 1)," \
			"h(value) as (select value from ai_config where model in (?1, 'default') and param = 'request-headers' order by iif(model <> 'default', 0, 1))," \
			"o(value) as (select value from ai_config where model in (?1, 'default') and param = 'request-options' order by iif(model <> 'default', 0, 1))," \
			"i(value) as (select value from ai_config where model in (?1, 'default') and param = ?8 order by iif(model <> 'default', 0, 1))," \
			"idef(value) as (select value from ai_config where model in (?1, 'default') and param = ?9 order by iif(model <> 'default', 0, 1))" \
			"select " \
			"(select replace(replace(replace(replace(replace( "\
				"iif (?2 <> '', (select value from p limit 1), '{{QUERY}}'), "\
				"'{{TABLES}}', ?3), " \
				"'{{FUNCTIONS}}', ?4), " \
				"'{{QUERY}}', ?5), " \
				"'{{ERROR}}', ?6), " \
				"'{{PROMPT}}', coalesce((select value from ps), ''))), " \
			"(select value from h), " \
			"(select value from o), " \
			"(select replace(replace(value, '{{TABLES}}', ?3), '{{FUNCTIONS}}', ?4) from i), " \
			"(select replace(replace(value, '{{TABLES}}', ?3), '{{FUNCTIONS}}', ?4) from idef), " \
			"(select json_object('query', ?5, 'prompt-id', ?2))",
			-1, &stmt, 0)) {

		sqlite3_bind_text(stmt, 1, model8, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, promptId8, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 3, tables8 ? tables8 : "", -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 4, functions8 ? functions8 : "", -1, SQLITE_TRANSIENT);

		char* arg1_8 = utils::utf16to8(arg1);
		sqlite3_bind_text(stmt, 5, arg1_8, -1, SQLITE_TRANSIENT);
		delete [] arg1_8;

		char* arg2_8 = utils::utf16to8(arg2);
		sqlite3_bind_text(stmt, 6, arg2_8, -1, SQLITE_TRANSIENT);
		delete [] arg2_8;

		sqlite3_bind_int(stmt, 7, sid);

		char action8[32]{0}; // chat, explain, execute, fix-error
		for(int i = 0; i < 31 && (i == 3 || promptId8[i] != '-'); i++)
			action8[i] = promptId8[i];

		char initPrompt8[128] {0};
		snprintf(initPrompt8, 127, "%s%s-init-prompt", action8, hasDbAccess ? "" : "-no-access");
		sqlite3_bind_text(stmt, 8, initPrompt8, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 9, hasDbAccess ? "default-init-prompt" : "default-no-access-init-prompt", -1, SQLITE_TRANSIENT);

		if (SQLITE_ROW == sqlite3_step(stmt)) {
			prompt8 = getValue(stmt, 0);
			headers8 = getValue(stmt, 1);
			options8 = getValue(stmt, 2);
			init8 = sqlite3_column_type(stmt, 3) == SQLITE_NULL ? getValue(stmt, 4) : getValue(stmt, 3);
			details8 = getValue(stmt, 5);
		}
	}
	sqlite3_finalize(stmt);

	if (prompt8) {
		if (init8) {
			sqlite3_stmt* stmt;
			if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
				"select " \
				"(select json_extract(details, '$.database') from ai_messages where sid = ?1 and role = 'system' order by id desc), " \
				"json_object('prompt-id', ?2, 'database', ?3), " \
				"(select replace(value, '{{TABLES}}', ?5) from ai_config where model in (?4, 'default') and param = 'on-change-db-prompt' order by iif(model <> 'default', 0, 1))",
				-1, &stmt, 0)) {
				sqlite3_bind_int(stmt, 1, sid);
				sqlite3_bind_text(stmt, 2, promptId8, -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(stmt, 3, sqlite3_db_filename(db, 0), -1, SQLITE_TRANSIENT);

				sqlite3_bind_text(stmt, 4, model8, -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(stmt, 5, tables8, -1, SQLITE_TRANSIENT);

				sqlite3_step(stmt);

				const char* database8 = sqlite3_column_type(stmt, 0) != SQLITE_NULL ? (const char*)sqlite3_column_text(stmt, 0) : 0;
				const char* details8 = (const char*)sqlite3_column_text(stmt, 1);
				const char* prompt8 = (const char*)sqlite3_column_text(stmt, 2);
				bool isDbChanged = database8 && strcmp(database8, sqlite3_db_filename(db, 0));
				if (database8 == 0 || isDbChanged)
					registerMessage(sid, "system", isDbChanged ? prompt8 : init8, details8);
			}

			sqlite3_finalize(stmt);
		}

		registerMessage(sid, "user", prompt8, details8);

		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "with t (role, content) as (select role, content from ai_messages where sid = ?1 order by id) select json_group_array(json_object('role', role, 'content', content)) from t", -1, &stmt, 0)) {
			sqlite3_bind_int(stmt, 1, sid);
			if (SQLITE_ROW == sqlite3_step(stmt))
				messages8 = getValue(stmt, 0);
		}
		sqlite3_finalize(stmt);

		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
				"select json_patch(json_patch(json_object('messages', json_extract(?2, '$'), " \
				"'private', json('true')), iif(?1 <> 'default', json_object('model', ?1), '{}')), ?3)",
				-1, &stmt, 0)) {
			sqlite3_bind_text(stmt, 1, model8, -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, messages8, -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, options8 ? options8 : "{}", -1, SQLITE_TRANSIENT);

			if (SQLITE_ROW == sqlite3_step(stmt) && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
				data8 = getValue(stmt, 0);
		}
		sqlite3_finalize(stmt);

		delete [] prompt8;
	}

	if (data8) {
		DWORD statusCode = 0;

		int timeout = prefs::get("ai-request-timeout");
		int delay = prefs::get( "ai-delay-between-requests");
		for (int attemptNo = 0; attemptNo < prefs::get("ai-request-attempt-count") && res16 == 0 && aiNeedStop(sid) == 0; attemptNo++) {
			char* res8 = utils::httpRequest("POST", url8, headers8, data8, 0, &statusCode, timeout);
			if (res8 && statusCode == 200) {
				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select json_extract(?1, '$.choices[0].message.content')", -1, &stmt, 0)) {
					sqlite3_bind_text(stmt, 1, res8, -1, SQLITE_TRANSIENT);
					if (SQLITE_ROW == sqlite3_step(stmt) && sqlite3_column_type(stmt, 0) == SQLITE_TEXT) {
						char* content8 = strdup((const char*)sqlite3_column_text(stmt, 0));
						char* advert8 = strstr(content8, "QUERY:") == content8 || strstr(content8, "GET:") == content8 ? strstr(content8, "---") : 0;
						if (advert8)
							advert8[0] = 0;

						res16 = utils::utf8to16(content8);

						registerMessage(sid, "assistant", content8, res8);
						free(content8);
					}
				}
				sqlite3_finalize(stmt);
			} else {
				if (delay > 0 && aiNeedStop(sid) == 0)
					Sleep(delay);
			}

			if (res8)
				delete [] res8;
		}

		delete [] data8;
	}

	if (tables8)
		delete [] tables8;

	if (functions8)
		delete [] functions8;

	if (init8)
		delete [] init8;

	if (details8)
		delete [] details8;

	if (messages8)
		delete [] messages8;

	if (headers8)
		delete [] headers8;

	delete [] model8;
	delete [] url8;
	delete [] promptId8;

	// Stop by timeout/abort/chain requests limit
	if (res16 == 0)
		return 0;

	if (_tcsstr(res16, TEXT("GET:")) == res16 || _tcsstr(res16, TEXT("QUERY:")) == res16) {
		bool isQuery = res16[0] == TEXT('Q');
		TCHAR* query16 = _tcsdup(res16 + (isQuery ? 6 : 4));
		TCHAR* err16 = dbutils::getQueryError(db, query16);
		if (err16) {
			delete [] res16;
			res16 = aiRequest(TEXT("on-error-prompt"), query16, err16, sid);
			delete [] err16;
		} else {
			char* query8 = utils::utf16to8(query16);
			int rowCount = 0;
			char* csv8 = dbutils::queryCSV(db, query8, true, ",", true, &rowCount, "(null)");
			char* msg8 = new char[strlen(csv8) + 64];
			sprintf(msg8, "%s%s", csv8, rowCount ? "" : "\nNo rows\n");
			TCHAR* msg16 = utils::utf8to16(msg8);

			delete [] res16;
			res16 = isQuery ? aiRequest(TEXT("result-confirm-prompt"), query16, msg16, sid) : aiRequest(0, msg16, 0, sid);

			delete [] query8;
			delete [] csv8;
			delete [] msg8;
			delete [] msg16;
		}

		free(query16);
	} else if (_tcsstr(res16, TEXT("CONFIRMED")) == res16) {
		delete [] res16;
		res16 = 0;

		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
				"select id, content from ai_messages where sid = ?1 and role = 'assistant' and instr(content, 'QUERY:') = 1 order by id desc limit 1",
				-1, &stmt, 0)) {
			sqlite3_bind_int(stmt, 1, sid);

			if (SQLITE_ROW == sqlite3_step(stmt)) {
				int id = sqlite3_column_int(stmt, 0);
				char query8[256] {0};
				snprintf(query8, 255, "update ai_messages set is_aux = 0 where id = %i", id);
				sqlite3_exec(prefs::db, query8, 0, 0, 0);

				res16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
			}
		}
		sqlite3_finalize(stmt);
	}  else {
		// Do nothing
	}

	return res16;
}
