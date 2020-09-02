#include "global.h"
#include "missing.h"
#include "resource.h"
#include "prefs.h"
#include "utils.h"
#include "tools.h"
#include "dialogs.h"

#include "tom.h"
#include <richole.h>
#include <unknwn.h>

#define DEFINE_GUIDXXX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) EXTERN_C const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
DEFINE_GUIDXXX(IID_ITextDocument,0x8CC497C0,0xA1DF,0x11CE,0x80,0x98, 0x00,0xAA,0x00,0x47,0xBE,0x5D);

const char *TYPES8[5] = {"current", "table", "view", "index", "trigger"};
const TCHAR *TYPES16[5] = {TEXT("current"), TEXT("table"), TEXT("view"), TEXT("index"), TEXT("trigger")};
const TCHAR *TYPES16u[5] = {TEXT("CURRENT"), TEXT("TABLE"), TEXT("VIEW"), TEXT("INDEX"), TEXT("TRIGGER")};
const TCHAR *TYPES16p[5] = {TEXT(""), TEXT("Tables"), TEXT("Views"), TEXT("Indexes"), TEXT("Triggers")};
const TCHAR *transactionStates[] = {TEXT(" TRN"),TEXT("")};

// AutoComplete
const TCHAR *SQL_KEYWORDS[] = {TEXT("abort"), TEXT("action"), TEXT("add"), TEXT("after"), TEXT("all"), TEXT("alter"), TEXT("always"), TEXT("analyze"), TEXT("and"), TEXT("as"), TEXT("asc"), TEXT("attach"), TEXT("autoincrement"), TEXT("before"), TEXT("begin"), TEXT("between"), TEXT("by"), TEXT("cascade"), TEXT("case"), TEXT("cast"), TEXT("check"), TEXT("collate"), TEXT("column"), TEXT("commit"), TEXT("conflict"), TEXT("constraint"), TEXT("create"), TEXT("cross"), TEXT("current"), TEXT("current_date"), TEXT("current_time"), TEXT("current_timestamp"), TEXT("database"), TEXT("default"), TEXT("deferrable"), TEXT("deferred"), TEXT("delete"), TEXT("desc"), TEXT("detach"), TEXT("distinct"), TEXT("do"), TEXT("drop"), TEXT("each"), TEXT("else"), TEXT("end"), TEXT("escape"), TEXT("except"), TEXT("exclude"), TEXT("exclusive"), TEXT("exists"), TEXT("explain"), TEXT("fail"), TEXT("filter"), TEXT("first"), TEXT("following"), TEXT("for"), TEXT("foreign"), TEXT("from"), TEXT("full"), TEXT("generated"), TEXT("glob"), TEXT("group"), TEXT("groups"), TEXT("having"), TEXT("if"), TEXT("ignore"), TEXT("immediate"), TEXT("in"), TEXT("index"), TEXT("indexed"), TEXT("initially"), TEXT("inner"), TEXT("insert"), TEXT("instead"), TEXT("intersect"), TEXT("into"), TEXT("is"), TEXT("isnull"), TEXT("join"), TEXT("key"), TEXT("last"), TEXT("left"), TEXT("like"), TEXT("limit"), TEXT("match"), TEXT("natural"), TEXT("no"), TEXT("not"), TEXT("nothing"), TEXT("notnull"), TEXT("null"), TEXT("nulls"), TEXT("of"), TEXT("offset"), TEXT("on"), TEXT("or"), TEXT("order"), TEXT("others"), TEXT("outer"), TEXT("over"), TEXT("partition"), TEXT("plan"), TEXT("pragma"), TEXT("preceding"), TEXT("primary"), TEXT("query"), TEXT("raise"), TEXT("range"), TEXT("recursive"), TEXT("references"), TEXT("regexp"), TEXT("reindex"), TEXT("release"), TEXT("rename"), TEXT("replace"), TEXT("restrict"), TEXT("right"), TEXT("rollback"), TEXT("row"), TEXT("rows"), TEXT("savepoint"), TEXT("select"), TEXT("set"), TEXT("table"), TEXT("temp"), TEXT("temporary"), TEXT("then"), TEXT("ties"), TEXT("to"), TEXT("transaction"), TEXT("trigger"), TEXT("unbounded"), TEXT("union"), TEXT("unique"), TEXT("update"), TEXT("using"), TEXT("vacuum"), TEXT("values"), TEXT("view"), TEXT("virtual"), TEXT("when"), TEXT("where"), TEXT("window"), TEXT("with"), TEXT("without"), TEXT('\0')};
const TCHAR *PRAGMAS[1024] = {0};
const TCHAR *FUNCTIONS[1024] = {0};
const TCHAR *TABLES[1024] = {0};

sqlite3 *db;
HWND hMainWnd, hToolbarWnd, hStatusWnd, hTreeWnd, hEditorWnd, hTabWnd, hEditorTipWnd, hDialog, hSortingResultWnd, hAutoComplete; // hTab.lParam is current ListView HWND
HMENU hMainMenu, hDbMenu, hEditorMenu, hResultMenu, hEditDataMenu;

TCHAR tabTooltips[MAX_RESULT_COUNT][MAX_TEXT_LENGTH]{0};
TCHAR queryElapsedTimes[MAX_RESULT_COUNT][64]{0};

char *recents[100] = {0};

HTREEITEM treeItems[5]; // 0 - current
HMENU treeMenus[6]; // 0 - add/refresh menu, 5 - column
TCHAR treeEditName[255];
TCHAR editTableData16[255]; // filled on DataEdit Dialog
TCHAR searchString[255]{0};

HFONT hDefFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
HACCEL hAccel = LoadAccelerators(0, MAKEINTRESOURCE(IDA_ACCEL));

// storage for cell who triggered context menu; IDM_RESULT_COPY_CELL, IDM_RESULT_COPY_ROW and view/edit row-dialog
ListViewCell currCell;

bool isMoveX = false;
bool isMoveY = false;
int top = 0;

int currParenthesisPos[] = {-1, -1};
bool isRequireHighligth = false, isRequireParenthesisHighligth = false;

TCHAR appPath[MAX_PATH]{0};

WNDPROC cbOldTreeItemEdit;
LRESULT CALLBACK cbNewTreeItemEdit(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void executeMultiQuery(bool isPlan = false);
void executeQuery(TCHAR* query, bool isPlan = false);

void openDb(const TCHAR* path);
void closeDb();

void enableMenu();
void disableMenu();
void setToolbarButtonState(int id, byte state);
void updateRecentList();
void updateTree(int type = 0);
void updateSizes(bool isPadding = false);
void updateTransactionState();
bool isQueryValid(const char* query);

WNDPROC cbOldAutoComplete;
LRESULT CALLBACK cbNewAutoComplete(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK cbMainWindow (HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	setlocale(LC_ALL, "");

	MSG msg;
	WNDCLASSEX wc;

	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("sqlite-gui-class");
	wc.lpfnWndProc = cbMainWindow;
	wc.style = CS_DBLCLKS;
	wc.cbSize = sizeof (WNDCLASSEX);
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOGO));
	wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOGO));
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.lpszMenuName = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

	if (!RegisterClassEx (&wc))
		return EXIT_FAILURE;

	LoadLibrary(TEXT("msftedit.dll"));

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&icex);

	if (!prefs::load()) {
		MessageBox(0, TEXT("Settings loading failed"), TEXT("Error"), MB_OK);
		return EXIT_FAILURE;
	}

	hMainWnd = CreateWindowEx (0, TEXT("sqlite-gui-class"), TEXT("sqlite-gui"), WS_OVERLAPPEDWINDOW,
		prefs::get("x"), prefs::get("y"), prefs::get("width"), prefs::get("height"),
		HWND_DESKTOP, 0, hInstance, NULL);

	hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_MAIN));
	ModifyMenu(hMainMenu, 3, MF_BYPOSITION | MFT_RIGHTJUSTIFY, 0, TEXT("?"));
	SetMenu(hMainWnd, hMainMenu);

	TBBUTTON tbButtons [ ] = {
		{0, IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0L, (INT_PTR)TEXT("Open")},
		{1, IDM_CLOSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0L, (INT_PTR)TEXT("Close")},
		{-1, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0L, 0},
		{2, IDM_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0L, (INT_PTR)TEXT("Save")},
		{3, IDM_PLAN, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0L, (INT_PTR)TEXT("Plan")},
		{4, IDM_EXECUTE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0L, (INT_PTR)TEXT("Execute")},
		{5, IDM_INTERRUPT, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0L, (INT_PTR)TEXT("Interrupt")}
	};

	hToolbarWnd = CreateToolbarEx (hMainWnd, WS_CHILD |  WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS |TBSTYLE_FLAT | TBSTYLE_LIST, IDC_TOOLBAR, 0, NULL, 0,
		tbButtons, sizeof(tbButtons)/sizeof(tbButtons[0]), 0, 0, 0, 0, sizeof (TBBUTTON));
	SendMessage(hToolbarWnd, TB_SETIMAGELIST,0, (LPARAM)ImageList_LoadBitmap(GetModuleHandle (0), MAKEINTRESOURCE(IDB_TOOLBAR), 0, 0, RGB(255,255,255)));
	hStatusWnd = CreateStatusWindow(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, NULL, hMainWnd, IDC_STATUSBAR);
	int sizes[5] = {80, 150, 186, 300, -1};
	SendMessage(hStatusWnd, SB_SETPARTS, 5, (LPARAM)&sizes);

	char version8[32];
	sprintf(version8, " SQLite: %s", SQLITE_VERSION);
	TCHAR* version16 = utils::utf8to16(version8);
	SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)version16);
	delete [] version16;
	SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)TEXT(" GUI: 1.1.1"));

	hTreeWnd = CreateWindowEx(0, WC_TREEVIEW, NULL, WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT  | WS_DISABLED | TVS_EDITLABELS /*| TVS_SHOWSELALWAYS*/, 0, 0, 100, 100, hMainWnd, (HMENU)IDC_TREE, hInstance,  NULL);
	hEditorWnd = CreateWindowEx(0, TEXT("RICHEDIT50W"), NULL, WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP | ES_NOHIDESEL, 100, 0, 100, 100, hMainWnd, (HMENU)IDC_EDITOR, hInstance,  NULL);
	hEditorTipWnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hMainWnd, NULL, hInstance, NULL);
	hTabWnd = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_TOOLTIPS, 100, 100, 100, 100, hMainWnd, (HMENU)IDC_TAB, hInstance, NULL);

	SendMessage(hEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_KEYEVENTS);

	hDbMenu = GetSubMenu(hMainMenu, 0);
	updateRecentList();

	hEditorMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_EDITOR));
	hEditorMenu = GetSubMenu(hEditorMenu, 0);

	hResultMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_RESULT));
	hResultMenu = GetSubMenu(hResultMenu, 0);

	hEditDataMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_EDIT_DATA));
	hEditDataMenu = GetSubMenu(hEditDataMenu, 0);

	HMENU hMenu;
	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_TREE));
	treeMenus[0] = GetSubMenu(hMenu, 0);

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_TABLE));
	treeMenus[TABLE] = GetSubMenu(hMenu, 0);

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_VIEW));
	treeMenus[VIEW] = GetSubMenu(hMenu, 0);

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_INDEX));
	treeMenus[INDEX] = GetSubMenu(hMenu, 0);

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_TRIGGER));
	treeMenus[TRIGGER] = GetSubMenu(hMenu, 0);

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MENU_COLUMN));
	treeMenus[COLUMN] = GetSubMenu(hMenu, 0);

	EnumChildWindows(hMainWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETDEFFONT);
	setEditorFont(hEditorWnd);
	setTreeFont(hTreeWnd);

	GetCurrentDirectory(MAX_PATH, appPath);

	if (strlen(lpCmdLine)) {
		int nArgs = 0;
		TCHAR** args = CommandLineToArgvW(GetCommandLine(), &nArgs);
		openDb(args[1]);
	} else {
		if (prefs::get("restore-editor")) {
			char* text8 = prefs::get("editor-text", "");
			TCHAR* text16 = utils::utf8to16(text8);
			SetWindowText(hEditorWnd, text16);
			delete [] text8;
			delete [] text16;
		}

		if (prefs::get("restore-db")) {
			int recent = GetMenuItemID(hDbMenu, 3);
			TCHAR demo[] = TEXT("bookstore.sqlite");
			if (!recent && utils::isFileExists(demo))
				openDb(demo);
			else
				PostMessage(hMainWnd, WM_COMMAND, recent, 0);
		}
	}

	ShowWindow (hMainWnd, prefs::get("maximized") == 1 ? SW_MAXIMIZE : SW_SHOW);
	SetFocus(hEditorWnd);

	hAutoComplete = CreateWindowEx(0, WC_LISTBOX, NULL, WS_CHILD | WS_BORDER, 0, 0, 150, 200, hMainWnd, (HMENU)IDC_AUTOCOMPLETE, GetModuleHandle(0), NULL);
	SendMessage(hAutoComplete, WM_SETFONT, (LPARAM)hDefFont, true);
	cbOldAutoComplete = (WNDPROC)SetWindowLong(hAutoComplete, GWL_WNDPROC, (LONG)cbNewAutoComplete);
	SetWindowLong(hEditorWnd, GWL_USERDATA, (LONG)hAutoComplete);
	SetWindowLong(hAutoComplete, GWL_USERDATA, (LONG)hEditorWnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		if (TranslateAccelerator(hMainWnd, hAccel, &msg))
			continue;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK cbMainWindow (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_DESTROY: {
			sqlite3_close(db);

			int size = GetWindowTextLength(hEditorWnd);
			TCHAR* text16 = new TCHAR[size + 1]{0};
			GetWindowText(hEditorWnd, text16, size + 1);
			char *text8 = utils::utf16to8(text16);
			prefs::set("editor-text", text8);

			delete [] text8;
			delete [] text16;

			if (!prefs::save())
				MessageBox(0, TEXT("Settings saving failed"), TEXT("Error"), MB_OK);

			PostQuitMessage (0);
		}
		break;

		case WM_SIZE: {
			SendMessage(hToolbarWnd, WM_SIZE, 0, 0);
			SendMessage(hStatusWnd, WM_SIZE, 0, 0);

			updateSizes(true);
		}
		break;

		case WM_LBUTTONDOWN: {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			isMoveX = (abs(x - prefs::get("splitter-width")) < 10);
			isMoveY = (x > prefs::get("splitter-width") + 10) && (abs(y - top - prefs::get("splitter-height")) < 10);

			if (isMoveX || isMoveY)
				SetCapture(hMainWnd);
		}
		break;

		case WM_LBUTTONUP: {
			if (isMoveX || isMoveY) {
				updateSizes(true);
				ReleaseCapture();
			}
			isMoveX = FALSE;
			isMoveY = FALSE;
		}
		break;

		case WM_MOUSEMOVE: {
			DWORD x = GET_X_LPARAM(lParam);
			DWORD y = GET_Y_LPARAM(lParam);

			isMoveX = isMoveX && (wParam == MK_LBUTTON);
			isMoveY = isMoveY && (wParam == MK_LBUTTON);

			if (isMoveX) {
				prefs::set("splitter-width", x - 3);
				updateSizes();
			}

			if (isMoveY) {
				prefs::set("splitter-height", y - 3 - top);
				updateSizes();
			}
		}
		break;

		case WM_NCLBUTTONDBLCLK: {
			WINDOWPLACEMENT wp = {};
			wp.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hMainWnd, &wp);
			prefs::set("maximized", (int)(wp.showCmd != SW_SHOWMAXIMIZED));
			return DefWindowProc (hWnd, message, wParam, lParam);
		}
		break;

		case WM_SYSCOMMAND: {
			if (wParam == SC_MAXIMIZE || wParam == SC_RESTORE)
				prefs::set("maximized", wParam == SC_MAXIMIZE);
			return DefWindowProc (hWnd, message, wParam, lParam);
		}
		break;

		case WM_EXITSIZEMOVE: {
			RECT rc;
			GetWindowRect(hMainWnd, &rc);
			prefs::set("x", rc.left);
			prefs::set("y", rc.top);
			prefs::set("width", rc.right - rc.left);
			prefs::set("height", rc.bottom - rc.top);
			updateSizes(true);
		}
		break;

		case WM_CONTEXTMENU: {
			POINT p = {LOWORD(lParam), HIWORD(lParam)};
			bool isContextKey = p.x == 65535 && p.y == 65535;
			if ((HWND)wParam == hEditorWnd && !isContextKey)
				TrackPopupMenu(hEditorMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hMainWnd, NULL);


			if ((HWND)wParam == hTreeWnd) {
				TVITEM ti;
				ti.hItem = treeItems[0];
				ti.mask = TVIF_PARAM;
				TreeView_GetItem(hTreeWnd, &ti);

				if (isContextKey) {
					RECT rc = {0};
					TreeView_GetItemRect(hTreeWnd, ti.hItem, &rc, TRUE); // is the macros problem?
					p.x = rc.left + 10;
					p.y = rc.top + 10;
					ClientToScreen(hTreeWnd, &p);
				}
				if (ti.lParam) // Root elements have negative values
					TrackPopupMenu(treeMenus[ti.lParam > 0 ? ti.lParam : 0], TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hMainWnd, NULL);
			}
		}
		break;

		case WM_COMMAND: {
			WORD cmd = LOWORD(wParam);

			if (cmd == IDM_EXIT)
				SendMessage(hMainWnd, WM_CLOSE, 0, 0);

			if (cmd == IDM_OPEN) {
				TCHAR path[MAX_PATH]{0};
				if (utils::openFile(path, TEXT("*.sqlite\0*.sqlite\0*.db\0*.db\0All\0*.*\0")))
					openDb(path);
			}

			if (cmd == IDM_CLOSE)
				closeDb();

			if (cmd == IDM_ATTACH) {
				TCHAR path16[MAX_PATH]{0};
				if (utils::openFile(path16, TEXT("*.sqlite\0*.sqlite\0*.db\0*.db\0All\0*.*\0"))) {
					TCHAR name16[256]{0};
					_tsplitpath(path16, NULL, NULL, name16, NULL);
					for(int i = 0; name16[i]; i++)
						name16[i] = _totlower(name16[i]);
					TCHAR query16[256]{0};
					_stprintf(query16, TEXT("attach database \"%s\" as \"%s\""), path16, name16);
					executeCommandQuery(query16);
				}
			}

			if (cmd == IDM_SETTINGS)
				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_SETTINGS), hMainWnd, (DLGPROC)&dialogs::cbDlgSettings);

			if (cmd == IDM_EXECUTE)
				executeMultiQuery();

			if (cmd == IDM_PLAN)
				executeMultiQuery(true);

			if (cmd == IDM_INTERRUPT)
				sqlite3_interrupt(db);

			// By accelerator
			if (cmd == IDM_NEXT_RESULT) {
				int pos = TabCtrl_GetCurSel(hTabWnd);
				if (pos != -1) {
					int count = TabCtrl_GetItemCount(hTabWnd);
					TabCtrl_SetCurFocus(hTabWnd, (pos + 1) % count);
				}
			}

			if (cmd == IDM_PROCESS_TAB) {
				if (GetFocus() != hEditorWnd)
					return SendMessage(hMainWnd, WMU_CHANGE_FOCUS, 0, 0);

				int start, end;
				SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

				int dir = GetAsyncKeyState(VK_SHIFT) ? -1 : +1;
				int startLineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, start, 0);
				int endLineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, end, 0);

				TCHAR indent[5]{0};
				_tcscpy(indent, dir == -1 ? TEXT("") : dialogs::INDENTS[prefs::get("editor-indent")]);
				if(dir > 0)
					dir = _tcslen(indent);

				for (int currLineNo = startLineNo; currLineNo < endLineNo + 1; currLineNo++) {
					int currLineIdx = SendMessage(hEditorWnd, EM_LINEINDEX, currLineNo, 0);
					int currLineSize = SendMessage(hEditorWnd, EM_LINELENGTH, currLineIdx, 0);
					TCHAR currLine[currLineSize + 1]{0};
					currLine[0] = currLineSize;
					SendMessage(hEditorWnd, EM_GETLINE, currLineNo, (LPARAM)currLine);

					// Choose to move focus or change indents
					if (currLineNo == startLineNo && start == end && !(
						(start == currLineIdx && currLineSize > 0) ||
						(start - currLineIdx > 0 && currLine[start - currLineIdx - 1] == '\t') ||
						(start - currLineIdx > 0 && currLine[start - currLineIdx - 1] == ' ')))
						return SendMessage(hMainWnd, WMU_CHANGE_FOCUS, 0, 0);

					if ((dir == -1 && currLine[0] != '\t'  && currLine[0] != ' ') || !currLineSize)
						continue;

					SendMessage(hEditorWnd, EM_SETSEL, currLineIdx, currLineIdx + (dir == -1));
					SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)indent);
				}

				if (start == end)
					SendMessage(hEditorWnd, EM_SETSEL, start + dir , start + dir);
				else {
					start = SendMessage(hEditorWnd, EM_LINEINDEX, startLineNo, 0);
					end = SendMessage(hEditorWnd, EM_LINEINDEX, endLineNo, 0);
					end += SendMessage(hEditorWnd, EM_LINELENGTH, end, 0);

					SendMessage(hEditorWnd, EM_SETSEL, start, end);
				}

				isRequireHighligth = false;
				isRequireParenthesisHighligth = false;
			}

			if (cmd == IDM_ESCAPE) {
				if (!SendMessage(hToolbarWnd, TB_ISBUTTONHIDDEN, IDM_INTERRUPT, 0))
					return SendMessage(hWnd, WM_COMMAND, IDM_INTERRUPT, 0);


				if(prefs::get("exit-by-escape") && !IsWindowVisible(hAutoComplete))
					SendMessage(hMainWnd, WM_CLOSE, 0, 0);
			}

			if (cmd == IDM_QUERY_DATA || cmd == IDM_EDIT_DATA || cmd == IDM_DROP || cmd == IDM_ERASE_DATA || cmd == IDM_ADD_COLUMN) {
				TCHAR name16[256] = {0};
				TV_ITEM tv;
				tv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
				tv.hItem = treeItems[0];
				tv.pszText = name16;
				tv.cchTextMax = 256;

				if(!TreeView_GetItem(hTreeWnd, &tv))
					break;

				int type = abs(tv.lParam);
				if (cmd == IDM_QUERY_DATA) {
					int len = GetWindowTextLength(hEditorWnd);
					SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);

					TCHAR query[256];
					_stprintf(query, TEXT("select * from \"%s\";\n"), name16);
					len += _tcslen(query);
					SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (LPARAM)query);
					SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
					SetFocus(hEditorWnd);

					executeQuery(query);
					SendMessage(hMainWnd, WM_SIZE, 0, 0);
					SendMessage(hTabWnd, TCM_SETCURFOCUS, TabCtrl_GetItemCount(hTabWnd) - 1, 0);

					SetFocus(hEditorWnd);
					return 1;
				}

				if (cmd == IDM_EDIT_DATA) {
					_tcscpy(editTableData16, name16);
					DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_EDITDATA), hMainWnd, (DLGPROC)&dialogs::cbDlgEditData);
					SetFocus(hTreeWnd);
				}

				if (cmd == IDM_DROP) {
					TCHAR query[256];
					_stprintf(query, TEXT("Are you sure you want to delete the %s \"%s\"?"), TYPES16[type], name16);
					if (MessageBox(hMainWnd, query, TEXT("Delete confirmation"), MB_OKCANCEL) == IDOK) {
						_stprintf(query, TEXT("drop %s \"%s\";"), TYPES16[type], name16);
						if (executeCommandQuery(query))
							updateTree(type);
						else
							showDbError(hWnd);
					}
					SetFocus(hTreeWnd);
				}

				if (cmd == IDM_ERASE_DATA) {
					TCHAR query[256];
					_stprintf(query, TEXT("Are you sure you want to delete all data from \"%s\"?"), name16);
					if (MessageBox(hMainWnd, query, TEXT("Delete confirmation"), MB_OKCANCEL) == IDOK) {
						_stprintf(query, TEXT("delete from \"%s\";"), name16);
						if (!executeCommandQuery(query))
							showDbError(hWnd);
					}
					SetFocus(hTreeWnd);
				}

				if (cmd == IDM_ADD_COLUMN) {
					_tcscpy(editTableData16, name16);
					int rc = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADD_COLUMN), hMainWnd, (DLGPROC)&dialogs::cbDlgAddColumn);
					if (rc != DLG_CANCEL)
						updateTree(TABLE);
					SetFocus(hTreeWnd);
				}
			}

			if (cmd == IDM_RENAME)
				(void)TreeView_EditLabel(hTreeWnd, treeItems[0]);

			if (cmd == IDM_ADD) {
				TV_ITEM tv;
				tv.mask = TVIF_HANDLE | TVIF_PARAM;
				tv.hItem = treeItems[0];

				if(!TreeView_GetItem(hTreeWnd, &tv) || !tv.lParam)
					return 0;

				int type = abs(tv.lParam);

				int rc = type == TABLE  ?
					DialogBox (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADD_TABLE), hMainWnd, (DLGPROC)&dialogs::cbDlgAddTable) :
					DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADDEDIT), hMainWnd, (DLGPROC)&dialogs::cbDlgAddEdit, IDM_ADD);
				if (rc != DLG_CANCEL)
					updateTree(rc);
				SetFocus(hTreeWnd);
			}

			if (cmd == IDM_EDIT) {
				int rc = DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADDEDIT), hMainWnd, (DLGPROC)&dialogs::cbDlgAddEdit, IDM_EDIT);
				if (rc != DLG_CANCEL)
					updateTree(rc);
				SetFocus(hTreeWnd);
			}

			if (cmd == IDM_REFRESH) {
				TV_ITEM tv;
				tv.mask = TVIF_PARAM | TVIF_STATE;
				tv.hItem = treeItems[0];

				int type = !TreeView_GetItem(hTreeWnd, &tv) || !tv.lParam ? 0 : abs(tv.lParam);
				updateTree(type);
				type && TreeView_SelectItem(hTreeWnd, treeItems[type]);
				return 0;
			}

			if (cmd >= IDM_RECENT && cmd <= IDM_RECENT + 5) {
				TCHAR buf[MAX_PATH];
				GetMenuString(hDbMenu, cmd, buf, MAX_PATH, MF_BYCOMMAND);
				openDb(buf);
			}

			if (cmd == IDM_SAVE && IDYES == MessageBox(0, TEXT("Save query?"), TEXT("Confirmation"), MB_YESNO)) {
				CHARRANGE range;
				SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&range);

				bool isSelection = range.cpMin != range.cpMax;
				int size =  isSelection ? range.cpMax - range.cpMin + 1 : GetWindowTextLength(hEditorWnd);
				if (size > 0) {
					TCHAR* text16 = new TCHAR[size + 1]{0};
					if (SendMessage(hEditorWnd, isSelection ? EM_GETSELTEXT : WM_GETTEXT, size + 1, (LPARAM)text16)) {
						TCHAR* ttext16 = utils::trim(text16);
						char* text8 = utils::utf16to8(ttext16);
						prefs::setQuery("gists", text8);
						delete [] ttext16;
						delete [] text8;
					}

					delete [] text16;
				}
				SetFocus(hEditorWnd);
			}

			if (cmd == IDM_RESULT_COPY_CELL || cmd == IDM_RESULT_COPY_ROW || cmd == IDM_RESULT_EXPORT)
				onListViewMenu(cmd);

			if (cmd == IDM_EDITOR_CUT)
				SendMessage(hEditorWnd, WM_CUT, 0, 0);

			if (cmd == IDM_EDITOR_COPY)
				SendMessage(hEditorWnd, WM_COPY, 0, 0);

			if (cmd == IDM_EDITOR_PASTE)
				SendMessage(hEditorWnd, WM_PASTE, 0, 0);

			if (cmd == IDM_EDITOR_DELETE)
				SendMessage (hEditorWnd, EM_REPLACESEL, TRUE, 0);


			if (cmd == IDM_ABOUT || cmd == IDM_TIPS || cmd == IDM_EXTENSIONS) {
				HMENU hMenu = GetSubMenu(hMainMenu, 3);
				TCHAR title[255];
				GetMenuString(hMenu, cmd, title, 255, MF_BYCOMMAND);
				TCHAR buf[MAX_TEXT_LENGTH];
				LoadString(GetModuleHandle(NULL), cmd == IDM_ABOUT ? IDS_ABOUT : cmd == IDM_TIPS ? IDS_TIPS : IDS_EXTENSIONS, buf, MAX_TEXT_LENGTH);
				MessageBox(hMainWnd, buf, title, MB_OK);
			}


			if (cmd == IDM_HOMEPAGE)
				ShellExecute(0, 0, TEXT("https://github.com/little-brother/sqlite-gui"), 0, 0 , SW_SHOW);

			if (cmd == IDM_HISTORY || cmd == IDM_GISTS) {
				DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_QUERYLIST), hMainWnd, (DLGPROC)&dialogs::cbDlgQueryList, (LPARAM)cmd);
				SetFocus(hEditorWnd);
			}

			if (cmd == IDM_IMPORT_SQL) {
				TCHAR path16[MAX_PATH];
				if(utils::openFile(path16, TEXT("*.sql\0*.sql\0All\0*.*\0")) && tools::importSqlFile(path16)) {
					updateTree();
					MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);
				}
			}

			if (cmd == IDM_IMPORT_CSV) {
				TCHAR path16[MAX_PATH];
				if(utils::openFile(path16, TEXT("CSV files\0*.csv\0All\0*.*\0"))) {
					int rc = DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_IMPORT_CSV), hMainWnd, (DLGPROC)&tools::cbDlgImportCSV, (LPARAM)path16);
					if (rc != DLG_CANCEL)
						updateTree(TABLE);
				}
			}

			if ((cmd == IDM_EXPORT_CSV) && (DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_EXPORT_CSV), hMainWnd, (DLGPROC)&tools::cbDlgExportCSV) == DLG_OK))
				MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);

			if ((cmd == IDM_EXPORT_SQL) && (DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_EXPORT_SQL), hMainWnd, (DLGPROC)&tools::cbDlgExportSQL) == DLG_OK))
				MessageBox(hMainWnd, TEXT("Done"), TEXT("Info"), MB_OK);

			if (cmd == IDM_GENERATE_DATA)
				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_GENERATE_DATA), hMainWnd, (DLGPROC)&tools::cbDlgDataGenerator);

			if (cmd == IDM_DATABASE_DIAGRAM)
				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_DATABASE_DIAGRAM), hMainWnd, (DLGPROC)&tools::cbDlgDatabaseDiagram);

			if (cmd == IDM_CHECK_INTEGRITY) {
				TCHAR* res = getDbValue(TEXT("pragma integrity_check"));
				MessageBox(hMainWnd, res, TEXT("Check result"), MB_OK);
				delete [] res;
			}

			if (cmd == IDM_VACUUM && IDYES == MessageBox(0, TEXT("The operation may take some time. Continue?"), TEXT("Confirmation"), MB_YESNO)) {
				if(SQLITE_OK == sqlite3_exec(db, "vacuum;", 0, 0, 0))
					MessageBox(hMainWnd, TEXT("Done"), TEXT("Vacuum"), MB_OK);
				else
					showDbError(hMainWnd);
			}

			if (LOWORD(wParam) == IDC_EDITOR && HIWORD(wParam) == EN_CHANGE && prefs::get("use-highlight") && !isRequireHighligth) {
				PostMessage(hMainWnd, WMU_HIGHLIGHT, 0, 0);
				isRequireHighligth = true;
			}

			if (wParam == IDCANCEL)
				SendMessage(hMainWnd, WM_CLOSE, 0, 0);
		}
		break;

		case WM_NOTIFY: {
			NMHDR* pHdr = (LPNMHDR)lParam;
			MSGFILTER * pF = (MSGFILTER *)lParam;
			TCHAR wndClass[256];
			GetClassName(pHdr->hwndFrom, wndClass, 256);

			if (pHdr->hwndFrom == hTabWnd && pHdr->code == TCN_SELCHANGE)
				EnumChildWindows(hMainWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_UPDATETAB);

			if ((HWND)GetParent(pHdr->hwndFrom) == hTabWnd && pHdr->code == (DWORD)NM_DBLCLK) {
				NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
				currCell = {ia->hdr.hwndFrom, ia->iItem, ia->iSubItem};
				DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hMainWnd, (DLGPROC)&dialogs::cbDlgRow, MAKELPARAM(ROW_VIEW, 1));
			}

			if ((HWND)GetParent(pHdr->hwndFrom) == hTabWnd && pHdr->code == (DWORD)LVN_KEYDOWN) {
				NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;
				if (kd->wVKey == 0x43 && GetKeyState(VK_CONTROL)) {// Ctrl + C
					currCell = {kd->hdr.hwndFrom, 0, 0};
					PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_RESULT_COPY_ROW, 0), 0);
				}

				if (kd->wVKey == VK_RETURN) {
					currCell = {pHdr->hwndFrom, ListView_GetNextItem(pHdr->hwndFrom, -1, LVNI_SELECTED), 0};
					DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hMainWnd, (DLGPROC)&dialogs::cbDlgRow, MAKELPARAM(ROW_VIEW, 1));
					SetFocus(pHdr->hwndFrom);
				}

				bool isNum = kd->wVKey >= 0x31 && kd->wVKey <= 0x39;
				bool isNumPad = kd->wVKey >= 0x61 && kd->wVKey <= 0x69;
				if ((isNum || isNumPad) && GetKeyState(VK_CONTROL)) // Ctrl + 1-9
					return sortListView(pHdr->hwndFrom, kd->wVKey - (isNum ? 0x31 : 0x61) + 1 );
			}

			if (pHdr->hwndFrom == hEditorWnd && pHdr->code == WM_RBUTTONDOWN) {
				POINT Pos;
				GetCursorPos(&Pos);
				TrackPopupMenu(hEditorMenu, 0,Pos.x, Pos.y, 0, hMainWnd, NULL);
			}

			if (pF->nmhdr.hwndFrom == hEditorWnd && pF->msg == WM_KEYDOWN && pF->wParam == VK_OEM_2 && GetAsyncKeyState(VK_CONTROL)) { // Ctrl + ?
				int crPos;
				SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&crPos, (LPARAM)&crPos);
				SendMessage(hMainWnd, WMU_SHOW_TABLE_INFO, crPos, 0);
				return 1;
			}

			if (pF->nmhdr.hwndFrom == hEditorWnd && pF->msg == WM_KEYUP && pF->wParam == VK_CONTROL && IsWindowVisible(hEditorTipWnd))
				SendMessage(hEditorTipWnd, TTM_TRACKACTIVATE, false, 0);

			if (pHdr->hwndFrom == hEditorWnd && pHdr->code == EN_SELCHANGE && !isRequireParenthesisHighligth) {
				SELCHANGE *pSc = (SELCHANGE *)lParam;
				if (pSc->seltyp > 0)
					return 1;

				PostMessage(hMainWnd, WMU_HIGHLIGHT, 0, 0);
				isRequireParenthesisHighligth = true;
			}

			if (pHdr->hwndFrom == hTreeWnd && (pHdr->code == (DWORD)NM_DBLCLK || pHdr->code == (DWORD)NM_RETURN)) {
				HTREEITEM hParent = TreeView_GetParent(hTreeWnd, treeItems[0]);
				if (hParent != NULL)
					PostMessage(hMainWnd, WM_COMMAND, hParent == treeItems[TABLE] || hParent == treeItems[VIEW] ? IDM_EDIT_DATA : IDM_EDIT, 0);
				return 1;
			}

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == (DWORD)TVN_KEYDOWN) {
				NMTVKEYDOWN* pKd = (LPNMTVKEYDOWN) lParam;
				if ((pKd->wVKey == VK_DELETE)) {
					TVITEM ti{0};
					ti.hItem = treeItems[0];
					ti.mask = TVIF_PARAM;
					TreeView_GetItem(hTreeWnd, &ti);
					if (ti.lParam == TABLE || ti.lParam == VIEW || ti.lParam == TRIGGER || ti.lParam == INDEX)
						PostMessage(hMainWnd, WM_COMMAND, IDM_DROP, 0);
				}
				return 1;
			}

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == TVN_SELCHANGED)
				treeItems[0] = TreeView_GetSelection(hTreeWnd);

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == TVN_BEGINLABELEDIT) {
				const NMTVDISPINFO * pMi = (LPNMTVDISPINFO)lParam;
				HWND hEdit = TreeView_GetEditControl(hTreeWnd);
				cbOldTreeItemEdit = (WNDPROC) SetWindowLongPtr(hEdit, GWL_WNDPROC, (LONG_PTR)&cbNewTreeItemEdit);

				_tcscpy(treeEditName, pMi->item.pszText);
				SetWindowText(hEdit, _tcstok(pMi->item.pszText, TEXT(":")));
			}

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == TVN_ENDLABELEDIT) {
				NMTVDISPINFO * pMi = (LPNMTVDISPINFO)lParam;
				int type = pMi->item.lParam;

				if (pMi->item.pszText == NULL || _tcslen(pMi->item.pszText) == 0)
					return false;

				TCHAR query[32000];
				if (type == COLUMN) {
					TCHAR tblname16[255];
					HTREEITEM hParent = TreeView_GetParent(hTreeWnd, treeItems[0]);
					TVITEM ti;
					ti.hItem = hParent;
					ti.mask = TVIF_PARAM | TVIF_TEXT;
					ti.pszText = tblname16;
					ti.cchTextMax = 255;
					TreeView_GetItem(hTreeWnd, &ti);
					if (ti.lParam != 1)
						break;

					TCHAR* colDesc16 = _tcstok(treeEditName, TEXT(":"));
					colDesc16 = colDesc16 ? _tcstok (NULL, TEXT(":")) : 0;

					_stprintf(query, TEXT("alter table \"%s\" rename column \"%s\" to \"%s\""), tblname16, treeEditName, pMi->item.pszText);
					if (_tcscmp(treeEditName, pMi->item.pszText) && !executeCommandQuery(query))
						return false;

					TCHAR item16[300] = {0};
					_stprintf(item16, TEXT("%s:%s"), pMi->item.pszText, colDesc16);
					pMi->item.pszText = item16;
					pMi->item.cchTextMax = _tcslen(item16) + 1;
					return true;
				}

				if (type == TABLE) {
					_stprintf(query, TEXT("alter table \"%s\" rename to \"%s\""), treeEditName, pMi->item.pszText);
				} else if (type > 1) {
					const TCHAR* ddl = getDDL(treeEditName, pMi->item.lParam);
					TCHAR* newDdl = utils::replace(ddl, treeEditName, pMi->item.pszText, _tcslen(TEXT("create ")) + _tcslen(TYPES16u[type]));
					_stprintf(query, TEXT("drop %s \"%s\"; %s;"), TYPES16[type], treeEditName, newDdl);
					delete [] ddl;
					delete [] newDdl;
				}

				return _tcscmp(treeEditName, pMi->item.pszText) && executeCommandQuery(query);
			}

			if (pHdr->code == TTN_GETDISPINFO) {
				LPTOOLTIPTEXT pTtt = (LPTOOLTIPTEXT) lParam;
				bool isToolTip = pTtt->hdr.hwndFrom == (HWND)SendMessage(hToolbarWnd, TB_GETTOOLTIPS, 0, 0);
				bool isTabTip = pTtt->hdr.hwndFrom == (HWND)SendMessage(hTabWnd, TCM_GETTOOLTIPS, 0, 0) && (pTtt->hdr.idFrom < MAX_RESULT_COUNT);
				if (isToolTip || isTabTip) {
					SendMessage(pTtt->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, MAX_TEXT_LENGTH);
					if (isTabTip) {
						pTtt->lpszText = tabTooltips[pTtt->hdr.idFrom];
					}
					else {
						pTtt->hinst = GetModuleHandle(0);
						pTtt->lpszText = (LPWSTR)pTtt->hdr.idFrom;
					}
				}
			}

			if (pHdr->code == (DWORD)NM_RCLICK && !_tcscmp(wndClass, WC_LISTVIEW)) {
				NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
				currCell = {ia->hdr.hwndFrom, ia->iItem, ia->iSubItem};

				POINT p;
				GetCursorPos(&p);
				TrackPopupMenu(hResultMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hMainWnd, NULL);
			}

			if (pHdr->code == EN_MSGFILTER && wParam == IDC_EDITOR) {
				return processEditorKey(pF);
			}

			if (pHdr->code == LVN_COLUMNCLICK) {
				NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
				return sortListView(pHdr->hwndFrom, pLV->iSubItem);
			}
		}
		break;

		case WM_PARENTNOTIFY: {
			if (LOWORD(wParam) == WM_LBUTTONDOWN && IsWindowVisible(hAutoComplete) && (GetFocus() != hAutoComplete))
				ShowWindow(hAutoComplete, SW_HIDE);

			// Open Table view
			POINT p {LOWORD(lParam), HIWORD(lParam)};
			if(ChildWindowFromPoint(hMainWnd, p) == hEditorWnd && GetAsyncKeyState(VK_CONTROL)) {
				RECT rect = {0};
				GetWindowRect(hEditorWnd, &rect);
				POINT p2 = {rect.left, rect.top};
				ScreenToClient(hMainWnd, &p2);

				p.x = p.x - p2.x;
				p.y = p.y - p2.y;
				int crPos = SendMessage(hEditorWnd, EM_CHARFROMPOS, 0, (LPARAM)&p);
				SendMessage(hMainWnd, WMU_SHOW_TABLE_INFO, crPos, 0);
			}
		}
		break;

		case WMU_HIGHLIGHT: {
			processHightlight(hEditorWnd, isRequireHighligth, isRequireParenthesisHighligth);
			isRequireHighligth = false;
			isRequireParenthesisHighligth = false;
		}
		break;

		case WMU_CHANGE_FOCUS: {
			int dir = GetAsyncKeyState(VK_SHIFT) ? -1 : +1;
			HWND hWndList[] = {hTreeWnd, hEditorWnd, (HWND)GetWindowLong(hTabWnd, GWL_USERDATA)};
			HWND hFocus = GetFocus();
			int idx = hFocus == hTreeWnd ? 0 : hFocus == hEditorWnd ? 1 : 2;
			HWND hNextWnd = hWndList[(idx + (dir == -1 ? -1 : 1) + 3) % 3];

			if (hNextWnd == hWndList[2] && !TabCtrl_GetItemCount(hTabWnd))
				hNextWnd = hFocus == hTreeWnd ? hEditorWnd : hTreeWnd;

			SetFocus(hNextWnd);
			return 1;
		}
		break;

		case WMU_SHOW_TABLE_INFO: {
			TCHAR* word = getWordFromCursor(hEditorWnd, wParam);
			for(int i = 0; TABLES[i] && i < 1024; i++) {
				if (_tcsicmp(word, TABLES[i]) == 0) {
					POINT p{0};
					GetCaretPos(&p);
					ClientToScreen(hEditorWnd, &p);

					TCHAR query[MAX_TEXT_LENGTH] {0};
					_stprintf(query, TEXT("select group_concat(name || ': ' || type || iif(pk,' [pk]',''),'\n') from pragma_table_info(\"%s\")"), word);
					TCHAR* desc = getDbValue(query);

					TOOLINFO ti = { 0 };
					ti.cbSize = sizeof(ti);
					ti.hwnd = hMainWnd;
					ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS |  TTF_TRACK;
					ti.uId = (UINT_PTR)hMainWnd;
					ti.lpszText = desc;
					SendMessage(hEditorTipWnd, TTM_SETMAXTIPWIDTH, 0, MAX_TEXT_LENGTH);
					SendMessage(hEditorTipWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
					SendMessage(hEditorTipWnd, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD) MAKELONG(p.x, p.y + 20));
					SendMessage(hEditorTipWnd, TTM_TRACKACTIVATE, true, (LPARAM)(LPTOOLINFO) &ti);
					delete [] desc;
					break;
				}
			}
			delete [] word;
		}
		break;

		default:
			return DefWindowProc (hWnd, message, wParam, lParam);
	}

	return 0;
}

LRESULT CALLBACK cbNewTreeItemEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_GETDLGCODE)
		return (DLGC_WANTALLKEYS | CallWindowProc(cbOldTreeItemEdit, hWnd, msg, wParam, lParam));

	return CallWindowProc(cbOldTreeItemEdit, hWnd, msg, wParam, lParam);
}


int CALLBACK cbListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	int colNo = LOWORD(lParamSort);
	int order = HIWORD(lParamSort);

	TCHAR buf[256];
	TCHAR buf2[256];

	ListView_GetItemText(hSortingResultWnd, lParam1, colNo, buf, 256);
	ListView_GetItemText(hSortingResultWnd, lParam2, colNo, buf2, 256);

	double num, num2;
	bool isNum = utils::isNumber(buf, &num) && utils::isNumber(buf2, &num2);
	return (order ? 1 : -1) * (isNum ? (num > num2 ? 1 : -1) : _tcscoll(buf, buf2));
}

void executeMultiQuery(bool isPlan) {
	if (!db)
		return;

	TabCtrl_DeleteAllItems(hTabWnd);
	EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_DESTROY);

	CHARRANGE range;
	SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&range);

	bool isSelection = range.cpMin != range.cpMax;
	int size =  isSelection ? range.cpMax - range.cpMin + 1 : GetWindowTextLength(hEditorWnd);
	if (size <= 0)
		return;

	TCHAR buf[size + 1];
	if (!SendMessage(hEditorWnd, isSelection ? EM_GETSELTEXT : WM_GETTEXT, size + 1, (LPARAM)buf))
		return;

	disableMenu();
	setToolbarButtonState(IDM_INTERRUPT, TBSTATE_ENABLED);

	// Remove comments
	for (int i = 0; i < size; i++) {
		if (buf[i] == '-' && buf[i+1] == '-') {
			while (i < size && buf[i] != '\n' && buf[i] != '\r') {
				buf[i] = ' ';
				i++;
			}
		}
	}

	TCHAR query16[size + 1]{0};
	TCHAR* chunk16 = _tcstok (buf, TEXT(";"));
	int resultNo = 0;
	while (chunk16 != NULL) {
		_tcscat(query16, chunk16);
		_tcscat(query16, TEXT(";"));

		char* query8 = utils::utf16to8(query16);
		if (sqlite3_complete(query8)) {
			if (resultNo == MAX_RESULT_COUNT) {
				MessageBox(hMainWnd, TEXT("Max number of queries (32) exceeded. Use \"Tools\" > \"Execute SQL file\""), NULL, MB_OK);
				break;
			}

			query16[_tcslen(query16) - 1] = 0; // remove last ;
			TCHAR* trimmed16 = utils::trim(query16);
			if(_tcslen(trimmed16) > 1)
				executeQuery(trimmed16, isPlan);

			resultNo++;
			query16[0] = 0;
			delete [] trimmed16;
		}
		delete [] query8;

		chunk16 = _tcstok (NULL, TEXT(";"));
	}

	EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_RESIZETAB);

	updateTransactionState();
	enableMenu();
}

void executeQuery(TCHAR* query, bool isPlan) {
	TCHAR* tquery = utils::trim(query);
	char* sql8 = utils::utf16to8(tquery);

	int resultNo = TabCtrl_GetItemCount(hTabWnd);
	DWORD tStart, tEnd;
	tStart = GetTickCount();

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, sql8, -1, &stmt, 0);
	if (rc == SQLITE_OK && isPlan && !sqlite3_stmt_isexplain(stmt)) {
		sqlite3_finalize(stmt);
		char* new8 = new char[strlen(sql8) + strlen("explain query plan ") + 1]{0};
		sprintf(new8, "explain query plan %s", sql8);
		delete [] sql8;
		sql8 = new8;
		rc = sqlite3_prepare_v2(db, sql8, -1, &stmt, 0);
	}
	int colCount = rc == SQLITE_OK ? sqlite3_column_count(stmt) : 0;

	HWND hResultWnd = 0;
	int rowCount = 0;
	if (rc == SQLITE_OK && colCount > 0) {
		hResultWnd = CreateWindow(WC_LISTVIEW, NULL, WS_TABSTOP | WS_CHILD | LVS_AUTOARRANGE | LVS_REPORT | LVS_SHOWSELALWAYS, 20, 20, 100, 100, hTabWnd, (HMENU)0, GetModuleHandle(0), NULL);
		rowCount = setListViewData(hResultWnd, stmt);
		rc = sqlite3_errcode(db);
		if (rc != SQLITE_OK && rowCount == 0) {
			DestroyWindow(hResultWnd);
			hResultWnd = 0;
		}
		ListView_SetItemState (hResultWnd, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
	}

	if (hResultWnd == 0) {
		hResultWnd = CreateWindow(WC_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 20, 100, 100, hTabWnd, (HMENU)0, GetModuleHandle(0), NULL);
		if (rc == SQLITE_OK && SQLITE_DONE == sqlite3_step(stmt)) {
			TCHAR text[64];
			_stprintf(text, TEXT("%i rows have been changed"), sqlite3_changes(db));
			SetWindowText(hResultWnd, text);
			sqlite3_finalize(stmt);
		} else {
			char *err8 = (char*)sqlite3_errmsg(db);
			TCHAR* err16 = utils::utf8to16(err8);
			SetWindowText(hResultWnd, err16);
			delete [] err16;
		}
	}

	if (rc == SQLITE_OK)
		prefs::setQuery("history", sql8);
	delete [] sql8;

	tEnd = GetTickCount();
	_stprintf(queryElapsedTimes[resultNo], TEXT("Elapsed: %.2fs"), (tEnd - tStart) / 1000.0);

	TCHAR caption[128];
	if (rowCount < 0)
		_stprintf(caption, TEXT("Result #%i (Cut to %i rows)"), resultNo + 1, -rowCount);
	if (rowCount == 0)
		_stprintf(caption, TEXT("Result #%i"), resultNo + 1);
	if (rowCount > 0)
		_stprintf(caption, rowCount > 0 ? TEXT("Result #%i (%i rows)") : TEXT("Result #%i"), resultNo + 1, rowCount);
	_stprintf(tabTooltips[resultNo], TEXT("%s"), utils::replaceAll(tquery, TEXT("\t"), TEXT("    ")));

	TCITEM tci;
	tci.mask = TCIF_TEXT | TCIF_IMAGE;
	tci.iImage = -1;
	tci.pszText = caption;
	tci.cchTextMax = _tcslen(caption);
	TabCtrl_InsertItem(hTabWnd, resultNo, &tci);

	updateTransactionState();
	ShowWindow(hResultWnd, resultNo == 0 ? SW_SHOW : SW_HIDE);
	SetWindowLong(hResultWnd, GWL_USERDATA, resultNo);
	if (resultNo == 0) {
		SetWindowLong(hTabWnd, GWL_USERDATA, (LONG)hResultWnd); // Also see ACTION_UPDATETAB
		SendMessage(hStatusWnd, SB_SETTEXT, 3, (LPARAM)queryElapsedTimes[resultNo]);
	}
	cbEnumChildren(hResultWnd, ACTION_RESIZETAB);
}

bool executeCommandQuery(const TCHAR* query) {
	bool isAutoTransaction = sqlite3_get_autocommit(db) > 0;
	if (isAutoTransaction)
		sqlite3_exec(db, "begin", NULL, 0, NULL);

	char* sql8 = utils::utf16to8(query);
	int rc = sqlite3_exec(db, sql8, NULL, 0 , 0);
	if (SQLITE_OK != rc)
		showDbError(hMainWnd);
	delete [] sql8;

	if (isAutoTransaction)
		sqlite3_exec(db, SQLITE_OK == rc ? "commit" : "rollback", NULL, 0, NULL);

	updateTransactionState();
	return SQLITE_OK == rc;
}

void updateRecentList() {
	HMENU hMenu = GetSubMenu(hMainMenu, 0);
	int size = GetMenuItemCount(hMenu);
	int recentCount = prefs::getRecents(recents);
	int afterRecentCount = 5;

	if (!recentCount)
		return;

	for (int i = 3; i < size - afterRecentCount; i++)
		RemoveMenu(hMenu, 3, MF_BYPOSITION);

	int count = 0;
	for (int i = 0; i < recentCount && count < 5; i++) {
		TCHAR* path16 = utils::utf8to16(recents[i]);
		struct _stat stats;
		if (_tstat(path16, &stats) == 0) {
			MENUITEMINFO mi = {0};
			mi.cbSize = sizeof(MENUITEMINFO);
			mi.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
			mi.wID = IDM_RECENT + count;
			mi.dwTypeData = path16;
			InsertMenuItem(hDbMenu, 3 + count, true, &mi);
			count++;
		}
		delete [] recents[i];
		delete [] path16;
	}
};

void openDb(const TCHAR* path) {
	closeDb();

	char* path8 = utils::utf16to8(path);
	if (SQLITE_OK != sqlite3_open(path8, &db)) {
		MessageBox(hMainWnd, TEXT("Error to open database"), TEXT("Error"), MB_OK);
		return;
	}

	TCHAR prev[MAX_PATH];
	GetMenuString(hDbMenu, 3, prev, MAX_PATH, MF_BYPOSITION);
	if (_tcscmp(prev, path) != 0) {
		prefs::setRecent(path8);
		updateRecentList();
	}

	delete [] path8;
	SetWindowText(hMainWnd, path);

	TCHAR searchPath[MAX_PATH]{0};
	_stprintf(searchPath, TEXT("%s\\extensions\\*.dll"), appPath);

	// load extensions
	if (prefs::get("autoload-extensions")) {
		TCHAR extensions[2048] = TEXT("Loaded extensions: ");
		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(searchPath, &ffd);
		bool isLoad = false;

		if (hFind != INVALID_HANDLE_VALUE) {
			sqlite3_enable_load_extension(db, true);
			do {
				TCHAR file16[MAX_PATH]{0};
				_stprintf(file16, TEXT("%s/extensions/%s"), appPath, ffd.cFileName);
				char* file8 = utils::utf16to8(file16);

				if (SQLITE_OK == sqlite3_load_extension(db, file8, NULL, NULL)) {
					if (isLoad)
						_tcscat(extensions, TEXT(", "));

					TCHAR filename16[256]{0};
					_tsplitpath(ffd.cFileName, NULL, NULL, filename16, NULL);
					_tcscat(extensions, filename16);
					isLoad = true;
				}
				delete [] file8;
			} while ((FindNextFile(hFind, &ffd)));
			FindClose(hFind);
		}
		SendMessage(hStatusWnd, SB_SETTEXT, 4, (LPARAM)(isLoad ? extensions : TEXT("")));
	}

	updateTree();
	TreeView_Expand(hTreeWnd, treeItems[TABLE], TVE_EXPAND);
	TreeView_Expand(hTreeWnd, treeItems[VIEW], TVE_EXPAND);
	EnableWindow(hTreeWnd, true);
	updateTransactionState();

	if (prefs::get("use-legacy-rename"))
		sqlite3_exec(db, "pragma legacy_alter_table = 1", 0, 0, 0);

	if (!PRAGMAS[0]) {
		sqlite3_stmt *stmt;
		int rc = sqlite3_prepare_v2(db, "select name from pragma_pragma_list()", -1, &stmt, 0);
		int rowNo = 0;
		while (rc == SQLITE_OK && SQLITE_ROW == sqlite3_step(stmt)) {
			PRAGMAS[rowNo] = utils::utf8to16((char *) sqlite3_column_text(stmt, 0));
			rowNo++;
		}
		PRAGMAS[rowNo] = 0;
		sqlite3_finalize(stmt);
	}

	if (!FUNCTIONS[0]) {
		sqlite3_stmt *stmt;
		int rc = sqlite3_prepare_v2(db, "select distinct name from pragma_function_list()", -1, &stmt, 0);
		int rowNo = 0;
		while (rc == SQLITE_OK && SQLITE_ROW == sqlite3_step(stmt)) {
			FUNCTIONS[rowNo] = utils::utf8to16((char *) sqlite3_column_text(stmt, 0));
			rowNo++;
		}
		FUNCTIONS[rowNo] = 0;
		sqlite3_finalize(stmt);
	}

	enableMenu();

	auto progressHandler = [](void* args) {
		MSG msg;
		while (PeekMessage(&msg, hMainWnd, 0, 0, 0)) {
			GetMessage(&msg, NULL, 0, 0);
			if (TranslateAccelerator(hMainWnd, hAccel, &msg))
				continue;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return 0;
	};
	sqlite3_progress_handler(db, 10, progressHandler, NULL);
}

void closeDb() {
	SetWindowText(hMainWnd, TEXT("No database selected"));
	EnableWindow(hTreeWnd, false);
	TreeView_DeleteAllItems(hTreeWnd);
	sqlite3_close(db);
	db = NULL;

	disableMenu();
}

void setEditorFont(HWND hWnd) {
	char* family8 = prefs::get("font-family", "Courier New"); // Only TrueType
	TCHAR* family16 = utils::utf8to16(family8);

	CHARFORMAT cf = {0};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE | CFM_SIZE;
	cf.bCharSet = DEFAULT_CHARSET;
	cf.bPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	cf.yHeight = prefs::get("font-size") * 20;
	_tcscpy(cf.szFaceName, family16);
	SendMessage(hWnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

	delete [] family8;
	delete [] family16;
}

void setTreeFont(HWND hWnd) {
	HDC hDC = GetDC(hTreeWnd);
	char* fontFamily8 = prefs::get("font-family", "Courier New"); // Only TrueType
	TCHAR* fontFamily16 = utils::utf8to16(fontFamily8);
	HFONT hFont = CreateFont (-MulDiv(prefs::get("font-size"), GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0,
		FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS,
		fontFamily16);

	SendMessage (hWnd, WM_SETFONT, WPARAM (hFont), TRUE);
	ReleaseDC(hWnd, hDC);

	delete [] fontFamily8;
	delete [] fontFamily16;
}

void disableMenu() {
	HMENU hMenu;
	hMenu = GetSubMenu(hMainMenu, 0);
	EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_GRAYED);
	EnableMenuItem(hMenu, GetMenuItemCount(hMenu) - 4, MF_BYPOSITION | MF_GRAYED);

	hMenu = GetSubMenu(hMainMenu, 1);
	EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_GRAYED);
	EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_GRAYED);

	hMenu = GetSubMenu(hMainMenu, 2);
	for (int i = 0; i < GetMenuItemCount(hMenu); i++)
		EnableMenuItem(hMenu, i, MF_BYPOSITION | MF_GRAYED);

	setToolbarButtonState(IDM_OPEN, TBSTATE_INDETERMINATE);
	setToolbarButtonState(IDM_CLOSE, TBSTATE_INDETERMINATE);
	setToolbarButtonState(IDM_PLAN, TBSTATE_INDETERMINATE);
	setToolbarButtonState(IDM_EXECUTE, TBSTATE_INDETERMINATE);
}

void enableMenu() {
	HMENU hMenu;
	hMenu = GetSubMenu(hMainMenu, 0);
	EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_ENABLED);
	EnableMenuItem(hMenu, GetMenuItemCount(hMenu) - 4, MF_BYPOSITION | MF_ENABLED);

	hMenu = GetSubMenu(hMainMenu, 1);
	EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_ENABLED);
	EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_ENABLED);

	hMenu = GetSubMenu(hMainMenu, 2);
	for (int i = 0; i < GetMenuItemCount(hMenu); i++)
		EnableMenuItem(hMenu, i, MF_BYPOSITION | MF_ENABLED);

	hMenu = GetSubMenu(hMainMenu, 2);
	EnableMenuItem(hMenu, 4, isQueryValid("select rownum(1) from generate_series(1,1,1)") ? MF_BYPOSITION | MF_ENABLED : MF_BYPOSITION | MF_DISABLED | MF_GRAYED);

	setToolbarButtonState(IDM_OPEN, TBSTATE_ENABLED);
	setToolbarButtonState(IDM_CLOSE, TBSTATE_ENABLED);
	setToolbarButtonState(IDM_PLAN, TBSTATE_ENABLED);
	setToolbarButtonState(IDM_EXECUTE, TBSTATE_ENABLED);
	setToolbarButtonState(IDM_INTERRUPT, TBSTATE_HIDDEN);
}

void setToolbarButtonState(int id, byte state) {
	TBBUTTONINFO tbi{0};
	tbi.cbSize = sizeof(TBBUTTONINFO);
	tbi.dwMask = TBIF_STATE;
	tbi.fsState = state;

	SendMessage(hToolbarWnd, TB_SETBUTTONINFO, id, (LPARAM)&tbi);
}

bool CALLBACK cbEnumChildren (HWND hWnd, LPARAM action) {
	if (hWnd == NULL)
		return true;

	if (action == ACTION_DESTROY) {
		DestroyWindow(hWnd);
	}

	if (action == ACTION_SETDEFFONT) {
		SendMessage(hWnd, WM_SETFONT, (LPARAM)hDefFont, true);
	}

	if (action == ACTION_RESIZETAB && GetParent(hWnd) == hTabWnd) {
		RECT rect, rect2;
		GetClientRect(hTabWnd, &rect);
		TabCtrl_GetItemRect(hTabWnd, 0, &rect2);

		SetWindowPos(hWnd, 0, rect.left + 5, rect.top + rect2.bottom + 5, rect.right - rect.left - 10, rect.bottom - rect.top - rect2.bottom - 10, 0);
	}

	if (action == ACTION_UPDATETAB && GetParent(hWnd) == hTabWnd) {
		int resultNo = TabCtrl_GetCurSel(hTabWnd);
		if (GetWindowLong(hWnd, GWL_USERDATA) == resultNo) {
			SetWindowLong(hTabWnd, GWL_USERDATA, (LONG)hWnd);
			ShowWindow(hWnd, SW_SHOW);
			SetFocus(hWnd);
			int pos = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			ListView_SetItemState (hWnd, pos == -1 ? 0 : pos, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
			SendMessage(hStatusWnd, SB_SETTEXT, 3, (LPARAM)queryElapsedTimes[resultNo]);
		} else {
			ShowWindow(hWnd, SW_HIDE);
		}
	}

	return true;
}


void updateSizes(bool isPadding) {
	RECT rc, rcStatus, rcToolbar;
	GetClientRect(hMainWnd, &rc);
	GetClientRect(hStatusWnd, &rcStatus);
	GetClientRect(hToolbarWnd, &rcToolbar);

	top = rcToolbar.bottom + 3;
	int h = rc.bottom - rcStatus.bottom - rcToolbar.bottom - 3;

	int splitterW = prefs::get("splitter-width");
	int splitterH = prefs::get("splitter-height");

	SetWindowPos(hTreeWnd, 0, 0, top + 2, splitterW, h - 2, SWP_NOZORDER );
	SetWindowPos(hEditorWnd, 0, splitterW + 5, top + 2, rc.right - splitterW - 5, splitterH, SWP_NOZORDER );
	SetWindowPos(hTabWnd, 0, splitterW + 5, top + splitterH + 5, rc.right - splitterW - 5, h - splitterH - 4, SWP_NOZORDER );

	if (isPadding) {
		RECT rc;
		GetClientRect(hEditorWnd, &rc);
		rc.left += 5;
		rc.top += 5;
		rc.right -= 5;
		rc.bottom -= 5;
		SendMessage(hEditorWnd, EM_SETRECT, 0, (LPARAM)&rc);

		hTabWnd != 0 && EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_RESIZETAB);
	}
}

void updateTree(int type) {
	if (type < 0) {
		printf("(updateTree) Error: illegal value of type: %i\n", type);
		return;
	}

	auto TreeView_AddItem = [](const TCHAR* caption, HTREEITEM parent = TVI_ROOT, int type = 0) {
		TVITEM tvi{0};
		TVINSERTSTRUCT tvins{0};
		tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
		tvi.pszText = (TCHAR*)caption;
		tvi.cchTextMax = _tcslen(caption) + 1;
		tvi.lParam = type;

		tvins.item = tvi;
		tvins.hInsertAfter = parent != TVI_ROOT || type == 0 ? TVI_LAST : abs(type) == 1 ? TVI_FIRST : treeItems[abs(type) - 1];
		tvins.hParent = parent;
		return (HTREEITEM)SendMessage(hTreeWnd, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
	};

	bool isExpanded = false;
	if (type) {
		TV_ITEM tv{0};
		tv.mask = TVIF_STATE;
		tv.hItem = treeItems[type];
		isExpanded = TreeView_GetItem(hTreeWnd, &tv) && (tv.state & TVIS_EXPANDED);
	}

	TreeView_DeleteItem(hTreeWnd, type == 0 ? TVI_ROOT : treeItems[type]);
	if (type == 0) {
		for (int i = 1; i < 5; i++)
			treeItems[i] = TreeView_AddItem(TYPES16p[i], TVI_ROOT, -i);
	} else {
		treeItems[type] = TreeView_AddItem(TYPES16p[type], TVI_ROOT, -type);
	}

	sqlite3_stmt *stmt;
	char sql[] =
		"select t.name, t.type, " \
		"iif(t.type in ('table', 'view'), group_concat(c.name || ': ' || c.type || iif(c.pk,' [pk]',''),','), null) columns " \
		"from sqlite_master t, pragma_table_info(t.tbl_name) c " \
		"where t.sql is not null and t.type = coalesce(?1, t.type) and t.name <> 'sqlite_sequence'" \
		"group by t.type, t.name";
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (type)
			sqlite3_bind_text(stmt, 1, TYPES8[type], strlen(TYPES8[type]),  SQLITE_TRANSIENT);
	else
			sqlite3_bind_null(stmt, 1);

	while (rc == SQLITE_OK && SQLITE_ROW == sqlite3_step(stmt)) {
		TCHAR* name16 = utils::utf8to16((char *) sqlite3_column_text(stmt, 0));
		TCHAR* type16 = utils::utf8to16((char *) sqlite3_column_text(stmt, 1));
		TCHAR* columns16 = utils::utf8to16((char *) sqlite3_column_text(stmt, 2));

		HTREEITEM hItem = 0;
		if (!_tcscmp(type16, TEXT("table")))
			hItem = TreeView_AddItem(name16, treeItems[TABLE], TABLE);

		if (!_tcscmp(type16, TEXT("view")))
			hItem = TreeView_AddItem(name16, treeItems[VIEW], VIEW);

		if (!_tcscmp(type16, TEXT("trigger")))
			hItem = TreeView_AddItem(name16, treeItems[TRIGGER], TRIGGER);

		if (!_tcscmp(type16, TEXT("index")))
			hItem = TreeView_AddItem(name16, treeItems[INDEX], INDEX);

		if (!_tcscmp(type16, TEXT("table")) || !_tcscmp(type16, TEXT("view"))) {
			TCHAR* column16 = _tcstok (columns16, TEXT(","));
			while (column16 != NULL) {
				TreeView_AddItem(column16, hItem, !_tcscmp(type16, TEXT("table")) ? COLUMN : 0);
				column16 = _tcstok (NULL, TEXT(","));
			}
		}

		delete [] type16;
		delete [] name16;
		delete [] columns16;
	}
	sqlite3_finalize(stmt);

	if (type != VIEW && SQLITE_OK != sqlite3_errcode(db)) {
		showDbError(hMainWnd);
		return;
	}

	if (SQLITE_OK != sqlite3_errcode(db)) {
		updateTree(TABLE);
		updateTree(INDEX);
		updateTree(TRIGGER);
		sqlite3_prepare_v2(db, "select name from sqlite_master where type = 'view'", -1, &stmt, 0);

		while (SQLITE_ROW == sqlite3_step(stmt)) {
			char* name8 = (char *) sqlite3_column_text(stmt, 0);
			TCHAR* name16 = utils::utf8to16(name8);
			HTREEITEM hItem = TreeView_AddItem(name16, treeItems[VIEW], VIEW);
			delete [] name16;

			sqlite3_stmt *substmt;
			char sql8[1024];
			sprintf(sql8, "select name || ': ' || type || case when pk then ' [pk]' else '' end from pragma_table_info('%s') order by cid", name8);
			sqlite3_prepare_v2(db, sql8, -1, &substmt, 0);
			while(SQLITE_ROW == sqlite3_step(substmt)) {
				TCHAR* column16 = utils::utf8to16((char *) sqlite3_column_text(substmt, 0));
				TreeView_AddItem(column16, hItem, 0);
				delete [] column16;
			}

			if (SQLITE_DONE != sqlite3_errcode(db)) {
				TreeView_AddItem(TEXT("Error..."), hItem, 0);
				TreeView_Expand(hTreeWnd, hItem, TVE_EXPAND);
			}
			sqlite3_finalize(substmt);
		}
		sqlite3_finalize(stmt);
	}

	if (isExpanded)
		TreeView_Expand(hTreeWnd, treeItems[type], TVE_EXPAND);

	for (int i = 0; i < 1024; i++)
		if (TABLES[i]) {
			delete [] TABLES[i];
			TABLES[i] = 0;
		}

	int tblNo = 0;
	for (int type = 1; type < 3; type++) { // TABLE + VIEW
		HTREEITEM hItem = TreeView_GetChild(hTreeWnd, treeItems[type]);
		while (hItem) {
			TCHAR* name16 = new TCHAR[256]{0};
			TV_ITEM tv = {0};
			tv.mask = TVIF_TEXT;
			tv.hItem = hItem;
			tv.cchTextMax = 256;
			tv.pszText = name16;
			TreeView_GetItem(hTreeWnd, &tv);
			TABLES[tblNo] = name16;
			tblNo++;

			hItem = TreeView_GetNextSibling(hTreeWnd, hItem);
		}
	}
	InvalidateRect(hTreeWnd, 0, TRUE);
}

void updateTransactionState() {
	SendMessage(hStatusWnd, SB_SETTEXT, 2, (LPARAM)(transactionStates[sqlite3_get_autocommit(db) > 0]));
}

bool isQueryValid(const char* query) {
	bool res = false;

	sqlite3_stmt *stmt;
	res = SQLITE_OK == sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_finalize(stmt);

	return res;
}

bool showDbError(HWND hWnd, char* err8) {
	TCHAR* err16 = err8 ? utils::utf8to16(err8) : utils::utf8to16(sqlite3_errmsg(db));
	MessageBox(hWnd, err16, NULL, 0);
	delete [] err16;
	return true;
}

TCHAR* getDbValue(const TCHAR* query16) {
	sqlite3_stmt *stmt;
	char* query8 = utils::utf16to8(query16);
	int rc = sqlite3_prepare_v2(db, query8, -1, &stmt, 0);
	rc = rc == SQLITE_OK ? sqlite3_step(stmt) : rc;

	TCHAR* res =  utils::utf8to16(rc == SQLITE_ROW ? (char*)sqlite3_column_text(stmt, 0) : "");
	sqlite3_finalize(stmt);
	delete [] query8;
	return res;
}

TCHAR* getDDL(TCHAR* name16, int type) {
	sqlite3_stmt *stmt;
	if (SQLITE_OK != sqlite3_prepare_v2(db, "select sql from sqlite_master where type = ?1 and name = ?2", -1, &stmt, 0)) {
		sqlite3_finalize(stmt);
		return NULL;
	}

	char* name8 = utils::utf16to8(name16);
	sqlite3_bind_text(stmt, 1, TYPES8[type], strlen(TYPES8[type]),  SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, name8, strlen(name8),  SQLITE_TRANSIENT);
	delete [] name8;

	if (SQLITE_ROW != sqlite3_step(stmt)) {
		sqlite3_finalize(stmt);
		return NULL;
	}

	TCHAR* res = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);
	return res;
}

// A ListView with one column has broken sort. So, the first column is a row number
int setListViewData(HWND hListWnd, sqlite3_stmt *stmt) {
	int colCount = sqlite3_column_count(stmt);
	HWND hHeader = ListView_GetHeader(hListWnd);

	if (hHeader == NULL || Header_GetItemCount(hHeader) == 0) {
		for (int i = 0; i <= colCount; i++) {
			TCHAR* name16 = utils::utf8to16(i > 0 ? sqlite3_column_name(stmt, i - 1) : "rowno");
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
	while(sqlite3_step(stmt) == SQLITE_ROW) {
		if (rowLimit > 0 && rowNo == rowLimit) {
			isStopByLimit = true;
			break;
		}

		if (rowNo == 0) {
			for (int i = 1; i <= colCount; i++) {
				int type = sqlite3_column_type(stmt, i - 1);
				LVCOLUMN lvc = {mask: LVCF_FMT};
				lvc.fmt = type == SQLITE_INTEGER || type == SQLITE_FLOAT ? LVCFMT_RIGHT : LVCFMT_LEFT;
				ListView_SetColumn(hListWnd, i, &lvc);
			}
		}

		TCHAR name16[64];
		_stprintf(name16, TEXT("%i"), rowNo + 1);

		LVITEM lvi = {0};
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.iItem = rowNo;
		lvi.pszText = name16;
		lvi.cchTextMax = _tcslen(name16) + 1;
		lvi.lParam = rowNo;
		ListView_InsertItem(hListWnd, &lvi);


		for (int i = 1; i <= colCount; i++) {
			bool isNull = sqlite3_column_type(stmt, i - 1) == SQLITE_NULL;
			char* name8 = (char *) sqlite3_column_text(stmt, i - 1);
			TCHAR* name16 = utils::utf8to16(isNull ? "" : name8);

			lvi.iSubItem = i;
			lvi.mask = LVIF_TEXT;
			lvi.pszText = name16;
			lvi.cchTextMax = _tcslen(name16);

			ListView_SetItem(hListWnd, &lvi);
			delete [] name16;
		}

		rowNo++;
	}

	if (SQLITE_DONE != sqlite3_errcode(db) && !isStopByLimit) {
		char *err8 = (char*)sqlite3_errmsg(db);
		TCHAR* err16 = utils::utf8to16(err8);
		TCHAR* msg16 = new TCHAR[_tcslen(err16) + 256]{0};
		_stprintf(msg16, TEXT("*** Terminate with error: %s"), err16);
		LVITEM lvi = {0};
		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 0;
		lvi.iItem = 0;
		lvi.pszText = msg16;
		lvi.cchTextMax = _tcslen(msg16) + 1;
		ListView_InsertItem(hListWnd, &lvi);

		delete [] err16;
		delete [] msg16;
	}

	sqlite3_finalize(stmt);

	ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_AUTOSIZECOLUMNS);
	for (int i = 1; i <= colCount; i++) {
		ListView_SetColumnWidth(hListWnd, i, LVSCW_AUTOSIZE_USEHEADER);
		if (ListView_GetColumnWidth(hListWnd, i) > 200)
			ListView_SetColumnWidth(hListWnd, i, 200);
	}

	return isStopByLimit ? -rowNo : rowNo;
}

LRESULT onListViewMenu(int cmd) {
	if (cmd == IDM_RESULT_COPY_CELL) {
		TCHAR buf[MAX_TEXT_LENGTH];
		ListView_GetItemText(currCell.hListWnd, currCell.iItem, currCell.iSubItem, buf, MAX_TEXT_LENGTH);
		utils::setClipboardText(buf);
	}

	if (cmd == IDM_RESULT_COPY_ROW || cmd == IDM_RESULT_EXPORT) {
		HWND hListWnd = currCell.hListWnd;
		HWND hHeader =  ListView_GetHeader(hListWnd);
		int colCount = (int)SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0L);
		int rowCount = ListView_GetSelectedCount(hListWnd);
		int searchNext = LVNI_SELECTED;
		if (cmd == IDM_RESULT_EXPORT && rowCount < 2) {
			rowCount = ListView_GetItemCount(hListWnd);
			searchNext = LVNI_ALL;
		}

		const TCHAR* delimiter16 = cmd == IDM_RESULT_COPY_ROW ? tools::DELIMITERS[0] : tools::DELIMITERS[prefs::get("csv-export-delimiter")];
		const TCHAR* newLine16 = cmd == IDM_RESULT_COPY_ROW || !prefs::get("csv-export-is-unix-line") ? TEXT("\r\n") : TEXT("\n");

		if (colCount && rowCount) {
			TCHAR* res = new TCHAR[MAX_TEXT_LENGTH * colCount * rowCount]{0};
			TCHAR buf[MAX_TEXT_LENGTH]{0};
			int rowNo = -1;
			while((rowNo = ListView_GetNextItem(hListWnd, rowNo, searchNext)) != -1) {
				for(int colNo = 1; colNo < colCount; colNo++) {
					ListView_GetItemText(hListWnd, rowNo, colNo, buf, MAX_TEXT_LENGTH);
					_tcscat(res, buf);
					_tcscat(res, colNo != colCount - 1 ? delimiter16 : newLine16);
				}
			}

			if (cmd == IDM_RESULT_COPY_ROW)
				utils::setClipboardText(res);

			TCHAR path16[MAX_PATH] {0};
			if (cmd == IDM_RESULT_EXPORT && utils::saveFile(path16, TEXT("CSV files\0*.csv\0All\0*.*\0"))) {

				FILE* f = _tfopen(path16, TEXT("w, ccs=UTF-8"));
				if (f) {
					_ftprintf(f, res);
					fclose(f);
				} else {
					MessageBox(hMainWnd, TEXT("Error to open file"), NULL, MB_OK);
				}
			}

			delete [] res;
		}
	}

	return true;
}

void updateHighlighting(HWND hWnd) {
	int size = GetWindowTextLength(hWnd);
	TCHAR* text = new TCHAR[size + 1]{0};
	GetWindowText(hWnd, text, size + 1);

	POINT scrollPos{0};
	SendMessage(hWnd, EM_GETSCROLLPOS, 0, (LPARAM)&scrollPos);

	CHARFORMAT2 cf2 = {0};
	cf2.cbSize = sizeof(CHARFORMAT2) ;
	SendMessage(hWnd, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM) &cf2);
	cf2.dwMask = CFM_BACKCOLOR | CFM_COLOR | CFM_BOLD;
	cf2.crBackColor = RGB(255, 255, 255);
	cf2.dwEffects = 0;
	cf2.crTextColor = RGB(255, 0, 0);

	int crPos;
	SendMessage(hWnd, EM_GETSEL, (WPARAM)&crPos, (WPARAM)&crPos);

	int pos = 0;
	int rCount = 0;
	int mode = 0; // 0 - casual, 1 - keyword, 2 - quotted string, 3 - comment
	COLORREF colors[4] = {RGB(0, 0, 0), RGB(0, 0, 200), RGB(0, 200, 0), RGB(255,0,0)};
	TCHAR breakers[] = TEXT(" \"'\n\t-;:(),=<>");

	while (pos < size) {
		mode = text[pos] == TEXT('-') && (pos < size - 1) && text[pos + 1] == TEXT('-') ? 3 :
			text[pos] == TEXT('"') || text[pos] == TEXT('\'') ? 2 :
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
			case 1: {
				do {
					rCount += text[pos] == TEXT('\r');
					pos++;
				} while (!_tcschr(breakers, text[pos]) && pos < size);

				TCHAR* buf = new TCHAR[pos - start + 1]{0};
				for(int i = 0; i < pos - start; i++)
					buf[i] = _totlower(text[start + i]);

				TCHAR* tbuf = utils::trim(buf);
				bool isKeyWord = false;
				int i = 0;
				while (SQL_KEYWORDS[i] && !isKeyWord && tbuf) {
					isKeyWord = !_tcsicmp(SQL_KEYWORDS[i], tbuf);
					i++;
				}

				i = 0;
				while (FUNCTIONS[i] && !isKeyWord && tbuf) {
					isKeyWord = !_tcsicmp(FUNCTIONS[i], tbuf);
					i++;
				}
				mode = isKeyWord;
				delete [] buf;
				delete [] tbuf;
			}
			break;

			case 2: {
				TCHAR q = text[start];
				do {
					rCount += text[pos] == TEXT('\r');
					pos++;
				} while (!((text[pos] == q) && (text[pos - 1] != TEXT('\\'))) && pos < size);

				if (text[pos] == q)
					pos++;
			}
			break;

			case 3: {
				while (text[pos] != TEXT('\n') && pos < size) {
					rCount += text[pos] == TEXT('\r');
					pos++;
				}
			}
			break;
		}

		int from = start - prCount;
		int to = pos - rCount;
		from = from < 0 ? 0 : from;
		to = from > to ? from : to;

		cf2.crTextColor = colors[mode];
		cf2.dwEffects = mode == 1 ? CFM_BOLD : 0;

		SendMessage(hWnd, EM_SETSEL, from, to);
		SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf2);
	}

	delete [] text;

	SendMessage(hWnd, EM_SETSEL, crPos, crPos);
	SendMessage(hWnd, EM_SETSCROLLPOS, 0, (LPARAM)&scrollPos);

	currParenthesisPos[0] = -1;
	currParenthesisPos[1] = -1;
}

void updateParenthesisHighlighting(HWND hWnd) {
	// crPos, crLine, etc -  where the cursor is standing
	int crPos;
	SendMessage(hWnd, EM_GETSEL, (WPARAM)&crPos, (WPARAM)&crPos);

	auto setChar = [hWnd](int pos, bool isSelected) {
		CHARFORMAT2 cf = {0};
		cf.cbSize = sizeof(CHARFORMAT2);
		cf.dwMask = CFM_BACKCOLOR | CFM_EFFECTS | CFM_BOLD;
		cf.dwEffects = isSelected ? CFM_BOLD : 0;
		cf.crBackColor = isSelected ? RGB(128, 255, 255) : RGB(255, 255, 255);
		SendMessage(hWnd, EM_SETSEL, pos, pos + 1);
		SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	};

	auto isText = [hWnd](int pos) {
		CHARFORMAT cf = {0};
		cf.cbSize = sizeof(CHARFORMAT);
		cf.dwMask = CFM_COLOR;

		SendMessage(hWnd, EM_SETSEL, pos, pos + 1);
		SendMessage(hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
		return cf.crTextColor == 0;
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

	TCHAR parentheses[] = TEXT("[]{}()");
	TCHAR* chr = _tcschr(parentheses, crLine[pos]);

	if (chr && chr[0] && isText(crLineIdx + pos)) {
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
				if (chr2 && chr2[0] && isText(currLineIdx + pos)) {
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
	}

	SendMessage(hWnd, EM_SETSEL, crPos, crPos);
}

void processHightlight(HWND hEditorWnd, bool isRequireHighligth, bool isRequireParenthesisHighligth) {
	if (!isRequireHighligth && !isRequireParenthesisHighligth)
		return;

	IUnknown *tr_code = NULL;
	ITextDocument *td_code;

	SendMessage(hEditorWnd, EM_GETOLEINTERFACE, 0, (LPARAM)&tr_code);
	if(tr_code == (IRichEditOle*)NULL) {
		MessageBox(0, TEXT("Error when trying to get RichEdit OLE Object"), NULL, 0);
		return;
	}
	tr_code->QueryInterface(IID_ITextDocument,(void**)&td_code);

	SendMessage(hEditorWnd, WM_SETREDRAW, FALSE, 0);
	SendMessage(hEditorWnd, EM_SETEVENTMASK, 0, ENM_NONE);
	td_code->Undo(tomSuspend, NULL);

	if (isRequireHighligth)
		updateHighlighting(hEditorWnd);

	if (isRequireParenthesisHighligth)
		updateParenthesisHighlighting(hEditorWnd);

	td_code->Undo(tomResume, NULL);
	SendMessage(hEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE |  ENM_KEYEVENTS);
	SendMessage(hEditorWnd, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(hEditorWnd, 0, TRUE);

	td_code->Release();
	tr_code->Release();
}

bool sortListView(HWND hListWnd, int colNo) {
	hSortingResultWnd = hListWnd;
	// lParam is used to store tab no, so use window text to store sorting params
	TCHAR buf[32];
	GetWindowText(hListWnd, buf, 32);
	DWORD param = _tcstol(buf, NULL, 10);
	WORD order = LOWORD(param) == colNo ? !HIWORD(param) : 1;
	param = MAKELPARAM(colNo, order);
	ListView_SortItems (hListWnd, cbListComparator, param);
	_ultot(param, buf, 10);
	SetWindowText(hListWnd, buf);

	for (int i = 0; i < ListView_GetItemCount(hListWnd); i++) {
		LVITEM lvi = {0};
		lvi.iItem = i;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.lParam = i;
		ListView_SetItem(hListWnd, &lvi);
	}

	return true;
}

void search(HWND hWnd) {
	int len = _tcslen(searchString);
	if (!len)
		return;

	int crPos;
	SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&crPos, (LPARAM)&crPos);
	FINDTEXT ft = {{crPos, GetWindowTextLength(hWnd)}, searchString};
	int pos = SendMessage(hWnd, EM_FINDTEXT, FR_DOWN, (LPARAM)&ft);
	if (pos == -1 && crPos != 0) {
		ft.chrg.cpMin = 0;
		pos = SendMessage(hWnd, EM_FINDTEXT, FR_DOWN, (LPARAM)&ft);
	}

	if (pos != -1) {
		SendMessage(hWnd, EM_SETSEL, pos, pos + len);
	} else {
		MessageBeep(0);
	}
}

bool processEditorKey(MSGFILTER* pF) {
	HWND hWnd = pF->nmhdr.hwndFrom;

	int key = pF->wParam;
	int vkKey = MapVirtualKey(LOBYTE(HIWORD(pF->lParam)), MAPVK_VSC_TO_VK);
	bool isKeyDown = pF->lParam & (1U << 31);


	if (hWnd != hEditorWnd && key == VK_RETURN && GetAsyncKeyState(VK_CONTROL)) {
		PostMessage(GetParent(hWnd), WM_COMMAND, IDC_DLG_OK, 0);
		pF->wParam = 0;
		return true;
	}

	if (key == 0x46 && GetAsyncKeyState(VK_CONTROL) && isKeyDown) { // Ctrl + F
		if (DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_FIND), hMainWnd, (DLGPROC)&dialogs::cbDlgFind, (LPARAM)hWnd) == DLG_OK)
			search(hWnd);
		SetFocus(hWnd);
		pF->wParam = 0;
		return true;
	}

	if (vkKey == VK_F3 && !isKeyDown) {
		search(hWnd);
		pF->wParam = 0;
		return true;
	}

	bool res = processAutoComplete(hWnd, key, isKeyDown);
	if (res)
		pF->wParam = 0; // Stop propagation

	return res;
}

bool processAutoComplete(HWND hEditorWnd, int key, bool isKeyDown) {
	HWND hAutoComplete = (HWND)GetWindowLong(hEditorWnd, GWL_USERDATA);

	// https://stackoverflow.com/questions/8161741/handling-keyboard-input-in-win32-wm-char-or-wm-keydown-wm-keyup
	// Shift + 7 and Shift + 9 equals to up and down
	bool isNavKey = !GetAsyncKeyState(VK_SHIFT) && (key == VK_ESCAPE || key == VK_UP || key == VK_DOWN || key == VK_RETURN);
	bool isCtrlSpace = GetAsyncKeyState(VK_CONTROL) && (key == VK_SPACE);

	if (IsWindowVisible(hAutoComplete) && isNavKey) {
		if (isKeyDown) {
			if (key == VK_ESCAPE)
				ShowWindow(hAutoComplete, SW_HIDE);

			if (key == VK_UP || key == VK_DOWN) {
				int iCount = ListBox_GetCount(hAutoComplete);
				int pos = ListBox_GetCurSel(hAutoComplete);
				ListBox_SetCurSel(hAutoComplete, (pos + (key == VK_UP ? -1 : 1) + iCount) % iCount);
			}

			if (key == VK_RETURN) {
				TCHAR buf[256] = {0};
				int pos = ListBox_GetCurSel(hAutoComplete);
				ListBox_GetText(hAutoComplete, pos, buf);
				long data = GetWindowLong(hAutoComplete, GWL_ID);
				SendMessage(hEditorWnd, WM_SETREDRAW, FALSE, 0);
				SendMessage(hEditorWnd, EM_SETSEL, LOWORD(data), HIWORD(data)); //-1
				SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (LPARAM)buf);
				SendMessage(hEditorWnd, WM_SETREDRAW, TRUE, 0);
				InvalidateRect(hEditorWnd, 0, TRUE);
				ShowWindow(hAutoComplete, SW_HIDE);
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

	int crPos;
	SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&crPos, 0);

	int currLineNo = SendMessage(hEditorWnd, EM_LINEFROMCHAR, -1, 0);
	int currLineSize = SendMessage(hEditorWnd, EM_LINELENGTH, crPos, 0);
	int currLineIdx = SendMessage(hEditorWnd, EM_LINEINDEX, -1, 0);
	TCHAR currLine[currLineSize + 1]{0};
	currLine[0] = currLineSize;
	SendMessage(hEditorWnd, EM_GETLINE, currLineNo, (LPARAM)currLine);
	size_t start = crPos - currLineIdx;
	size_t end = start;
	TCHAR breakers[] = TEXT(" \"'\n\r\+-;:(),=<>!");

	while(start > 0 && !_tcschr(breakers, currLine[start - 1]))
		start--;

	while(end < _tcslen(currLine) && !_tcschr(breakers, currLine[end]))
		end++;

	TCHAR word[end - start + 1] = {0};
	for (size_t i = 0; i < end - start; i++)
		word[i] = currLine[start + i];

	size_t wPos = crPos - currLineIdx - start; // Cursor position in the word

	ListBox_ResetContent(hAutoComplete);
	size_t wLen = _tcslen(word);

	const HWND hWnd = hAutoComplete;
	auto addString = [&word, wLen, hWnd, wPos, isCtrlSpace](const TCHAR* str, bool noEntryCheck = false) {
		if (!_tcsnicmp(str, word, wLen) && _tcslen(str) == wLen)
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
		ListBox_AddString(hWnd, str2);
		return 0;
	};

	int isExact = 0;
	bool isDotEnd = false;
	// tablename. or aliasname.
	if (wLen > 1 && word[wLen - 1] == TEXT('.') && wPos == wLen) {
		isDotEnd = true;

		word[wLen - 1] = 0;
		wLen--;

		size_t tLen = GetWindowTextLength(hEditorWnd);
		TCHAR text[tLen + 1]{0};
		GetWindowText(hEditorWnd, text, tLen + 1);
		_tcslwr(text);

		for (int i = 0; TABLES[i]; i++)	{
			bool isSuitable = false;
			isSuitable = _tcslen(TABLES[i]) == wLen && !_tcsicmp(TABLES[i], word);

			size_t tbl_aLen = _tcslen(TABLES[i]) + wLen;
			TCHAR tbl_a[tbl_aLen + 1 + 1 + 2] {0}; // +1 - space, +1 - \0, +2 for quotes
			TCHAR tbl_t[][8] = {TEXT("%s %s"), TEXT("\"%s\" %s"), TEXT("'%s' %s"), TEXT("[%s] %s")};
			for (int j = 0; j < 4 && !isSuitable; j++) {
				_stprintf(tbl_a, tbl_t[j], TABLES[i], word); // tablename alias
				_tcslwr(tbl_a);
				TCHAR* p = _tcsstr(text, tbl_a);

				isSuitable = p && // found
					(_tcslen(p) == tLen || _tcschr(breakers, (p - 1)[0])) && // not xxxtablename alias
					(p + tbl_aLen == '\0' || _tcschr(breakers, (p + tbl_aLen + (j == 0 ? 1 : 3))[0])); // not tablename aliasxxx
			}

			if (isSuitable) {
				char* table8 = utils::utf16to8(TABLES[i]);
				sqlite3_stmt *stmt;
				sqlite3_prepare_v2(db, "select name from pragma_table_info(?1) order by cid", -1, &stmt, 0);
				sqlite3_bind_text(stmt, 1, table8, strlen(table8), SQLITE_TRANSIENT);
				while (SQLITE_ROW == sqlite3_step(stmt)) {
					TCHAR* column16 =  utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
					int pos = ListBox_AddString(hAutoComplete, column16);
					ListBox_SetItemData(hAutoComplete, pos, 0);
					delete [] column16;
				}
				sqlite3_finalize(stmt);

				delete [] table8;
				break;
			}
		}

		if (_tcsicmp(word, TEXT("temp")) == 0) {
			sqlite3_stmt *stmt;
			int rc = sqlite3_prepare_v2(db, "select name from temp.sqlite_master where type in ('table', 'view')", -1, &stmt, 0);
			while (rc == SQLITE_OK && SQLITE_ROW == sqlite3_step(stmt)) {
				TCHAR* name16 = utils::utf8to16((char *) sqlite3_column_text(stmt, 0));
				isExact += addString(name16, true);
				delete [] name16;
			}
			sqlite3_finalize(stmt);
			addString(TEXT("sqlite_master"), true);
			addString(TEXT("sqlite_sequence"), true);
		}
	}

	if (start > 6 && !_tcsnicmp(currLine + start - 7, TEXT("pragma "), 7)) {
		for (int i = 0; PRAGMAS[i]; i++)
			isExact += addString(PRAGMAS[i]);
	} else if (start > 4 && (!_tcsnicmp(currLine + start - 5, TEXT("from "), 5) || !_tcsnicmp(currLine + start - 5, TEXT("join "), 5))) {
		bool isNoCheck = wLen == 1 && word[0] == TEXT(' ');
		for (int i = 0; TABLES[i]; i++)
			isExact += addString(TABLES[i], isNoCheck);

		addString(TEXT("dbstat"));
		addString(TEXT("sqlite_master"));
		addString(TEXT("sqlite_sequence"));
		if (isQueryValid("select value from generate_series(1, 1, 1)"))
			isExact += addString(TEXT("generate_series"));

		TCHAR buf[255];
		for (int i = 0; PRAGMAS[i]; i++) {
			_stprintf(buf, TEXT("pragma_%s()"), PRAGMAS[i]);
			isExact += addString(buf, isNoCheck);
		}
	} else if (wLen > 1) {
		for (int i = 0; FUNCTIONS[i]; i++)
			isExact += addString(FUNCTIONS[i]);

		for (int i = 0; SQL_KEYWORDS[i]; i++)
			isExact += addString(SQL_KEYWORDS[i]);

		for (int i = 0; TABLES[i]; i++)
			isExact += addString(TABLES[i]);
	}

	if (isExact && !isCtrlSpace)
		ListBox_ResetContent(hAutoComplete);

	int h = SendMessage(hAutoComplete, LB_GETITEMHEIGHT, 0, 0);
	int iCount = ListBox_GetCount(hAutoComplete);
	RECT rc = {0};
	GetClientRect(hAutoComplete, &rc);
	rc.bottom = rc.top + h * iCount;

	rc.right += GetSystemMetrics(SM_CXEDGE) * 2 - 2;
	rc.bottom += GetSystemMetrics(SM_CXEDGE) * 2;
	SetWindowPos(hAutoComplete, 0, 0, 0, rc.right, rc.bottom > 150 ? 150 : rc.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	if (iCount) {
		POINT p = {0};
		GetCaretPos(&p);
		ClientToScreen(hEditorWnd, &p);
		ScreenToClient(GetParent(hAutoComplete), &p);
		SetWindowPos(hAutoComplete, 0, p.x, p.y + 20, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

		SetCapture(hAutoComplete);

		if (isDotEnd)
			SetWindowLong(hAutoComplete, GWL_ID, MAKELONG(currLineIdx + end, currLineIdx + end));
		else
			SetWindowLong(hAutoComplete, GWL_ID, MAKELONG(currLineIdx + start, currLineIdx + end));

		ShowWindow(hAutoComplete, SW_SHOW);

		// Up AutoComplete to top. WS_EX_MOSTTOP doesn't work as expected :(
		SetWindowPos(hAutoComplete, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		InvalidateRect(hEditorWnd, 0, 0);
		ValidateRect(hEditorWnd, NULL);
		ValidateRect(hTabWnd, NULL);
	} else {
		ShowWindow(hAutoComplete, SW_HIDE);
	}


	ListBox_SetCurSel(hAutoComplete, 0);

	return false;
}

LRESULT CALLBACK cbNewAutoComplete(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_LBUTTONDBLCLK || (msg == WM_KEYUP && wParam == VK_RETURN) || (msg == WM_KEYUP && wParam == VK_ESCAPE)) {
		HWND hEditorWnd = (HWND)GetWindowLong(hWnd, GWL_USERDATA);
		int rc = processAutoComplete(hEditorWnd, wParam == VK_ESCAPE ? VK_ESCAPE : VK_RETURN, true);
		SetFocus(hEditorWnd);
		return rc;
	}

	if (msg == WM_SHOWWINDOW && !wParam) // hide
		ReleaseCapture();

	// Outside click
	if (msg == WM_LBUTTONDOWN && (LOWORD(lParam) > 32768 || HIWORD(lParam) > 32678)) {
		ShowWindow(hWnd, SW_HIDE);
		SetFocus(hEditorWnd);
	}

	return CallWindowProc(cbOldAutoComplete, hWnd, msg, wParam, lParam);
}

TCHAR* getWordFromCursor(HWND hWnd, int pos) {
	int crPos = pos;
	if (pos == -1)
		SendMessage(hWnd, EM_GETSEL, (WPARAM)&crPos, (LPARAM)&crPos);

	SendMessage(hWnd, EM_SETSEL, crPos, crPos);
	int currLineNo = SendMessage(hWnd, EM_LINEFROMCHAR, -1, 0);
	int currLineSize = SendMessage(hWnd, EM_LINELENGTH, crPos, 0);
	int currLineIdx = SendMessage(hWnd, EM_LINEINDEX, -1, 0);
	TCHAR currLine[currLineSize + 1]{0};
	currLine[0] = currLineSize;

	SendMessage(hWnd, EM_GETLINE, currLineNo, (LPARAM)currLine);
	size_t start = crPos - currLineIdx;
	size_t end = start;
	TCHAR breakers[] = TEXT(" \"'\n\r\+-;:(),=<>!");

	while(start > 0 && !_tcschr(breakers, currLine[start - 1]))
		start--;

	if(end > 0 && _tcschr(breakers, currLine[end]))
		end--;
	while(end < _tcslen(currLine) - 1 && !_tcschr(breakers, currLine[end + 1]))
		end++;

	if (start > end)
		end = start;

	TCHAR* word = new TCHAR[end - start + 1];
	for (size_t i = 0; i <= end - start; i++)
		word[i] = currLine[start + i];
	word[end - start + 1] = '\0';

	return word;
}


