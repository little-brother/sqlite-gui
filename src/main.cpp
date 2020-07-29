#include "global.h"
#include "resource.h"
#include "prefs.h"
#include "utils.h"
#include "tools.h"
#include "dialogs.h"

const char *TYPES8[5] = {"current", "table", "view", "index", "trigger"};
const TCHAR *TYPES16[5] = {TEXT("current"), TEXT("table"), TEXT("view"), TEXT("index"), TEXT("trigger")};
const TCHAR *TYPES16u[5] = {TEXT("CURRENT"), TEXT("TABLE"), TEXT("VIEW"), TEXT("INDEX"), TEXT("TRIGGER")};
const TCHAR *TYPES16p[5] = {TEXT(""), TEXT("Tables"), TEXT("Views"), TEXT("Indexes"), TEXT("Triggers")};
const TCHAR *transactionStates[] = {TEXT(" IN TRANSACTION "),TEXT("")};

// AutoComplete
const TCHAR *SQL_KEYWORDS[] = {TEXT("abort"), TEXT("action"), TEXT("add"), TEXT("after"), TEXT("all"), TEXT("alter"), TEXT("always"), TEXT("analyze"), TEXT("and"), TEXT("as"), TEXT("asc"), TEXT("attach"), TEXT("autoincrement"), TEXT("before"), TEXT("begin"), TEXT("between"), TEXT("by"), TEXT("cascade"), TEXT("case"), TEXT("cast"), TEXT("check"), TEXT("collate"), TEXT("column"), TEXT("commit"), TEXT("conflict"), TEXT("constraint"), TEXT("create"), TEXT("cross"), TEXT("current"), TEXT("current_date"), TEXT("current_time"), TEXT("current_timestamp"), TEXT("database"), TEXT("default"), TEXT("deferrable"), TEXT("deferred"), TEXT("delete"), TEXT("desc"), TEXT("detach"), TEXT("distinct"), TEXT("do"), TEXT("drop"), TEXT("each"), TEXT("else"), TEXT("end"), TEXT("escape"), TEXT("except"), TEXT("exclude"), TEXT("exclusive"), TEXT("exists"), TEXT("explain"), TEXT("fail"), TEXT("filter"), TEXT("first"), TEXT("following"), TEXT("for"), TEXT("foreign"), TEXT("from"), TEXT("full"), TEXT("generated"), TEXT("glob"), TEXT("group"), TEXT("groups"), TEXT("having"), TEXT("if"), TEXT("ignore"), TEXT("immediate"), TEXT("in"), TEXT("index"), TEXT("indexed"), TEXT("initially"), TEXT("inner"), TEXT("insert"), TEXT("instead"), TEXT("intersect"), TEXT("into"), TEXT("is"), TEXT("isnull"), TEXT("join"), TEXT("key"), TEXT("last"), TEXT("left"), TEXT("like"), TEXT("limit"), TEXT("match"), TEXT("natural"), TEXT("no"), TEXT("not"), TEXT("nothing"), TEXT("notnull"), TEXT("null"), TEXT("nulls"), TEXT("of"), TEXT("offset"), TEXT("on"), TEXT("or"), TEXT("order"), TEXT("others"), TEXT("outer"), TEXT("over"), TEXT("partition"), TEXT("plan"), TEXT("pragma"), TEXT("preceding"), TEXT("primary"), TEXT("query"), TEXT("raise"), TEXT("range"), TEXT("recursive"), TEXT("references"), TEXT("regexp"), TEXT("reindex"), TEXT("release"), TEXT("rename"), TEXT("replace"), TEXT("restrict"), TEXT("right"), TEXT("rollback"), TEXT("row"), TEXT("rows"), TEXT("savepoint"), TEXT("select"), TEXT("set"), TEXT("table"), TEXT("temp"), TEXT("temporary"), TEXT("then"), TEXT("ties"), TEXT("to"), TEXT("transaction"), TEXT("trigger"), TEXT("unbounded"), TEXT("union"), TEXT("unique"), TEXT("update"), TEXT("using"), TEXT("vacuum"), TEXT("values"), TEXT("view"), TEXT("virtual"), TEXT("when"), TEXT("where"), TEXT("window"), TEXT("with"), TEXT("without"), TEXT('\0')};
const TCHAR *PRAGMAS[1024] = {0};
const TCHAR *FUNCTIONS[1024] = {0};
const TCHAR *TABLES[1024] = {0};

sqlite3 *db;
HWND hMainWnd, hToolbarWnd, hStatusWnd, hTreeWnd, hEditorWnd, hTabWnd, hDialog, hSortingResultWnd, hAutoComplete; // hTab.lParam is current ListView HWND
HMENU hMainMenu, hDbMenu, hEditorMenu, hResultMenu, hEditDataMenu;
TCHAR tabTooltips[MAX_RESULT_COUNT][MAX_TEXT_LENGTH];
char *recents[100] = {0};
bool isEditorChange = false;

HTREEITEM treeItems[5]; // 0 - current
HMENU treeMenus[6]; // 0 - add/refresh menu, 5 - column
TCHAR treeEditName[255];
TCHAR editTableData16[255]; // filled on DataEdit Dialog

HFONT hDefFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

// storage for cell who triggered context menu; IDM_RESULT_COPY_CELL, IDM_RESULT_COPY_ROW and view/edit row-dialog
ListViewCell currCell;

bool isMoveX = false;
bool isMoveY = false;
int top = 0;


WNDPROC cbOldTreeItemEdit;
LRESULT CALLBACK cbNewTreeItemEdit(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void executeMultiQuery(bool isPlan = false);
void executeQuery(TCHAR* query);

void openDb(const TCHAR* path);
void closeDb();

void updateRecentList();
void updateTree(int type = 0);
void updateSizes(bool isPadding = false);
void updateTransactionState();

LRESULT CALLBACK cbMainWindow (HWND, UINT, WPARAM, LPARAM);
int CALLBACK cbListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

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

	InitCommonControls();
	LoadLibrary(TEXT("msftedit.dll"));

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
		{4, IDM_EXECUTE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0L, (INT_PTR)TEXT("Execute")}
	};

	hToolbarWnd = CreateToolbarEx (hMainWnd, WS_CHILD |  WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS |TBSTYLE_FLAT | TBSTYLE_LIST, IDC_TOOLBAR, 0, NULL, 0,
		tbButtons, sizeof(tbButtons)/sizeof(tbButtons[0]), 0, 0, 0, 0, sizeof (TBBUTTON));
	SendMessage(hToolbarWnd, TB_SETIMAGELIST,0, (LPARAM)ImageList_LoadBitmap(GetModuleHandle (0), MAKEINTRESOURCE(IDB_TOOLBAR), 0, 0, RGB(255,255,255)));
	hStatusWnd = CreateStatusWindow(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, NULL, hMainWnd, IDC_STATUSBAR);
	int sizes[4] = {80, 150, 255, -1};
	SendMessage(hStatusWnd, SB_SETPARTS, 4, (LPARAM)&sizes);

	char version8[32];
	sprintf(version8, " SQLite: %s", SQLITE_VERSION);
	TCHAR* version16 = utils::utf8to16(version8);
	SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)version16);
	delete [] version16;
	SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)TEXT(" GUI: 0.9.5"));

    hTreeWnd = CreateWindowEx(0, WC_TREEVIEW, NULL, WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT  | WS_DISABLED | TVS_EDITLABELS /*| TVS_SHOWSELALWAYS*/, 0, 0, 100, 100, hMainWnd, (HMENU)IDC_TREE, hInstance,  NULL);
	hEditorWnd = CreateWindowEx(0, TEXT("RICHEDIT50W"), NULL, WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP | ES_NOHIDESEL, 100, 0, 100, 100, hMainWnd, (HMENU)IDC_EDITOR, hInstance,  NULL);
	hTabWnd = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_TOOLTIPS, 100, 100, 100, 100, hMainWnd, (HMENU)IDC_TAB, hInstance, NULL);

	SendMessage(hEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_UPDATE | ENM_KEYEVENTS);

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

	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDA_ACCEL));

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

		if (prefs::get("restore-db"))
			PostMessage(hMainWnd, WM_COMMAND, GetMenuItemID(hDbMenu, 3), 0);
	}

    ShowWindow (hMainWnd, prefs::get("maximized") == 1 ? SW_MAXIMIZE : SW_SHOW);
    SetFocus(hEditorWnd);
	hAutoComplete = CreateWindowEx(WS_EX_TOPMOST, WC_LISTBOX, NULL, WS_CHILD | WS_BORDER, 0, 0, 150, 200, hEditorWnd, (HMENU)IDC_AUTOCOMPLETE, GetModuleHandle(0), NULL);
	SendMessage(hAutoComplete, WM_SETFONT, (LPARAM)hDefFont, true);

	while (GetMessage(&msg, NULL, 0, 0)) {
		if (TranslateAccelerator(hMainWnd, hAccel, &msg))
			continue;

		if (!IsDialogMessage(hMainWnd, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

    return msg.wParam;
}

LRESULT CALLBACK cbMainWindow (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
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
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			isMoveX = (abs(x - prefs::get("splitter-width")) < 10);
			isMoveY = (x > prefs::get("splitter-width") + 10) && (abs(y - top - prefs::get("splitter-height")) < 10);
        }
		break;

        case WM_LBUTTONUP: {
        	if (isMoveX || isMoveY)
				updateSizes(true);
			isMoveX = FALSE;
			isMoveY = FALSE;
        }
		break;

        case WM_MOUSEMOVE: {
			DWORD x = LOWORD(lParam);
			DWORD y = HIWORD(lParam);

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
			return DefWindowProc (hwnd, message, wParam, lParam);
		}
		break;

		case WM_SYSCOMMAND: {
			if (wParam == SC_MAXIMIZE || wParam == SC_RESTORE)
				prefs::set("maximized", wParam == SC_MAXIMIZE);
			return DefWindowProc (hwnd, message, wParam, lParam);
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
			int x = LOWORD(lParam), y = HIWORD(lParam);
			if ((HWND)wParam == hEditorWnd)
				TrackPopupMenu(hEditorMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, hMainWnd, NULL);

			if ((HWND)wParam == hTreeWnd) {
				TVITEM ti;
				ti.hItem = treeItems[0];
				ti.mask = TVIF_PARAM;
				TreeView_GetItem(hTreeWnd, &ti);
				if (ti.lParam) // Root elements have negative values
					TrackPopupMenu(treeMenus[ti.lParam > 0 ? ti.lParam : 0], TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, hMainWnd, NULL);
			}
		}
		break;

		case WM_COMMAND: {
			WORD cmd = LOWORD(wParam);

			if (cmd == IDM_EXIT)
				SendMessage(hMainWnd, WM_DESTROY, 0, 0);

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

			if (cmd == IDM_NEXTRESULT) {
				int pos = TabCtrl_GetCurSel(hTabWnd);
				if (pos != -1) {
					int count = TabCtrl_GetItemCount(hTabWnd);
					TabCtrl_SetCurFocus(hTabWnd, (pos + 1) % count);
					SetFocus(hEditorWnd);
				}
			}

			if (cmd == IDM_QUERY_DATA || cmd == IDM_EDIT_DATA || cmd == IDM_DROP || cmd == IDM_ADD_COLUMN) {
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
					TCHAR query[256];
					_stprintf(query, TEXT("select * from \"%s\";\n"), name16);
					SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (LPARAM)query);
					SetFocus(hEditorWnd);

					executeQuery(query);
					SendMessage(hMainWnd, WM_SIZE, 0, 0);
					SendMessage(hTabWnd, TCM_SETCURFOCUS, TabCtrl_GetItemCount(hTabWnd) - 1, 0);
				}

				if (cmd == IDM_EDIT_DATA) {
					_tcscpy(editTableData16, name16);
					DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_QUERYLIST), hMainWnd, (DLGPROC)&dialogs::cbDlgEditData);
				}

				if (cmd == IDM_DROP) {
					TCHAR query[256];
					_stprintf(query, TEXT("Are you sure you want to delete the \"%s\" %s?"), name16, TYPES16[type]);
					if (MessageBox(hMainWnd, query, TEXT("Delete confirmation"), MB_OKCANCEL) == IDOK) {
						_stprintf(query, TEXT("drop %s \"%s\";"), TYPES16[type], name16);
						if (executeCommandQuery(query))
							updateTree(type);
					}
				}

				if (cmd == IDM_ADD_COLUMN) {
					_tcscpy(editTableData16, name16);
					int rc = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADD_COLUMN), hMainWnd, (DLGPROC)&dialogs::cbDlgAddColumn);
					if (rc != DLG_CANCEL)
						updateTree(TABLE);
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
					DialogBox (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADDEDIT), hMainWnd, (DLGPROC)&dialogs::cbDlgAdd);
				if (rc != DLG_CANCEL)
					updateTree(rc);
			}

			if (cmd == IDM_EDIT) {
				int rc = DialogBox (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADDEDIT), hMainWnd, (DLGPROC)&dialogs::cbDlgEdit);
				if (rc != DLG_CANCEL)
					updateTree(rc);
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
				TCHAR buff[MAX_PATH];
				GetMenuString(hDbMenu, cmd, buff, MAX_PATH, MF_BYCOMMAND);
				openDb(buff);
			}

			if (cmd == IDM_SAVE && IDYES == MessageBox(0, TEXT("Save query?"), TEXT("Confirmation"), MB_YESNO)) {
				CHARRANGE range;
				SendMessage(hEditorWnd, EM_EXGETSEL, 0, (LPARAM)&range);

				bool isSelection = range.cpMin != range.cpMax;
				int size =  isSelection ? range.cpMax - range.cpMin + 1 : GetWindowTextLength(hEditorWnd);
				if (size > 0) {
					TCHAR* text16 = new TCHAR[size + 1]{0};
					if (SendMessage(hEditorWnd, isSelection ? EM_GETSELTEXT : WM_GETTEXT, size + 1, (LPARAM)text16)) {
						TCHAR* ttext16 = utils::trim(text16, true);
						char* text8 = utils::utf16to8(ttext16);
						prefs::setQuery("gists", text8);
						delete [] ttext16;
						delete [] text8;
					}

					delete [] text16;
				}
				SetFocus(hEditorWnd);
			}

			// ToDo: Send explicitly LVM_GETITEMTEXT to get text length
			if (cmd == IDM_RESULT_COPY_CELL) {
				TCHAR buf[MAX_TEXT_LENGTH];
				ListView_GetItemText(currCell.hListWnd, currCell.iItem, currCell.iSubItem, buf, MAX_TEXT_LENGTH);
				utils::setClipboardText(buf);
			}

			if (cmd == IDM_RESULT_COPY_ROW || cmd == IDM_RESULT_EXPORT) {
				HWND hListWnd = currCell.hListWnd;
				HWND hHeader =  ListView_GetHeader(hListWnd);

				int colCount = (int)SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0L);
				int rowCount = cmd == IDM_RESULT_COPY_ROW ? ListView_GetSelectedCount(hListWnd) : ListView_GetItemCount(hListWnd);
				int searchNext = cmd == IDM_RESULT_COPY_ROW ? LVNI_SELECTED : LVNI_ALL;
				const TCHAR* delimiter16 = cmd == IDM_RESULT_COPY_ROW ? tools::DELIMITERS[0] : tools::DELIMITERS[prefs::get("csv-export-delimiter")];
				const TCHAR* newLine16 = cmd == IDM_RESULT_COPY_ROW || !prefs::get("csv-export-is-unix-line") ? TEXT("\r\n") : TEXT("\n");

				if (colCount && rowCount) {
					TCHAR *res = new TCHAR[MAX_TEXT_LENGTH * colCount * rowCount]{0};
					TCHAR buf[MAX_TEXT_LENGTH] = {0};
					int rowNo = -1;
					while((rowNo = ListView_GetNextItem(hListWnd, rowNo, searchNext)) != -1) {
						for(int colNo = 0; colNo < colCount; colNo++) {
							ListView_GetItemText(hListWnd, rowNo, colNo, buf, MAX_TEXT_LENGTH);
							_tcscat(res, buf);
							_tcscat(res, colNo != colCount - 1 ? delimiter16 : newLine16);
						}
					}

					if (cmd == IDM_RESULT_COPY_ROW)
						utils::setClipboardText(res);

					TCHAR path16[MAX_PATH] = {0};
					if (cmd == IDM_RESULT_EXPORT && utils::saveFile(path16, TEXT("CSV files\0*.csv\0All\0*.*\0"))) {
						FILE* f = _tfopen(path16, TEXT("w, ccs=UTF-8"));
						if (f) {
							_ftprintf(f, res);
							fclose(f);
						} else {
							MessageBox(hwnd, TEXT("Error to open file"), NULL, MB_OK);
						}
					}

					delete [] res;
				}
			}

			if (cmd == IDM_EDITOR_CUT)
				SendMessage(hEditorWnd, WM_CUT, 0, 0);

			if (cmd == IDM_EDITOR_COPY)
				SendMessage(hEditorWnd, WM_COPY, 0, 0);

			if (cmd == IDM_EDITOR_PASTE)
				SendMessage(hEditorWnd, WM_PASTE, 0, 0);

			if (cmd == IDM_EDITOR_DELETE)
				SendMessage (hEditorWnd, EM_REPLACESEL, TRUE, 0);

			if (cmd == IDM_ABOUT) {
				TCHAR buf[MAX_TEXT_LENGTH];
				LoadString(GetModuleHandle(NULL), IDS_ABOUT, buf, MAX_TEXT_LENGTH);
				MessageBox(hMainWnd, buf, TEXT("About"), MB_OK);
			}

			if (cmd == IDM_TIPS) {
				TCHAR buf[MAX_TEXT_LENGTH];
				LoadString(GetModuleHandle(NULL), IDS_TIPS, buf, MAX_TEXT_LENGTH);
				MessageBox(hMainWnd, buf, TEXT("Tips"), MB_OK);
			}

			if (cmd == IDM_HOMEPAGE)
				ShellExecute(0, 0, TEXT("https://github.com/little-brother/sqlite-gui"), 0, 0 , SW_SHOW);

			if (cmd == IDM_HISTORY || cmd == IDM_GISTS) {
				DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_QUERYLIST), hMainWnd, (DLGPROC)&dialogs::cbDlgQueryList, (LPARAM)cmd);
				SetFocus(hEditorWnd);
			}

			if (cmd == IDM_IMPORT_SQL) {
				TCHAR path16[MAX_PATH];
				if(utils::openFile(path16, TEXT("*.sql\0*.sql\0All\0*.*\0")))
					tools::importSqlFile(path16);
			}

			if (cmd == IDM_IMPORT_CSV) {
				TCHAR path16[MAX_PATH];
				if(utils::openFile(path16, TEXT("CSV files\0*.csv\0All\0*.*\0"))) {
					int rc = DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_IMPORT_CSV), hMainWnd, (DLGPROC)&tools::cbDlgImportCSV, (LPARAM)path16);
					if (rc != DLG_CANCEL)
						updateTree(TABLE);
				}
			}

			if (cmd == IDM_EXPORT_CSV)
				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_EXPORT_CSV), hMainWnd, (DLGPROC)&tools::cbDlgExportCSV);

			if (cmd == IDM_EXPORT_SQL)
				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_EXPORT_SQL), hMainWnd, (DLGPROC)&tools::cbDlgExportSQL);

			if (cmd == IDM_CHECK_INTEGRITY) {
				TCHAR* res = getDbValue(TEXT("pragma integrity_check"));
				MessageBox(hMainWnd, res, TEXT("Check result"), MB_OK);
				delete [] res;
			}

			if (cmd == IDM_VACUUM && IDYES == MessageBox(0, TEXT("The operation may take some time. Continue?"), TEXT("Confirmation"), MB_YESNO))
				sqlite3_exec(db, "vacuum;", 0, 0, 0);

			if (LOWORD(wParam) == IDC_EDITOR && HIWORD(wParam) == EN_CHANGE && prefs::get("use-highlight"))
				updateHighlighting(hEditorWnd);

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
				DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hMainWnd, (DLGPROC)&dialogs::cbDlgRow, (LPARAM)ROW_VIEW);
			}

			if ((HWND)GetParent(pHdr->hwndFrom) == hTabWnd && pHdr->code == (DWORD)LVN_KEYDOWN) {
				NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;
				if (kd->wVKey == 0x43 && GetKeyState(VK_CONTROL)) {// Ctrl + C
					currCell = {kd->hdr.hwndFrom, 0, 0};
					PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_RESULT_COPY_ROW, 0), 0);
				}
			}

			if (pF->nmhdr.hwndFrom == hEditorWnd && pF->msg == WM_RBUTTONDOWN) {
				POINT Pos;
				GetCursorPos(&Pos);
				TrackPopupMenu(hEditorMenu, 0,Pos.x, Pos.y, 0, hMainWnd, NULL);
			}

			if (pHdr->hwndFrom == hTreeWnd && pHdr->code == (DWORD)NM_DBLCLK) {
				HTREEITEM hParent = TreeView_GetParent(hTreeWnd, treeItems[0]);
				if (hParent != NULL)
					PostMessage(hMainWnd, WM_COMMAND, hParent == treeItems[TABLE] || hParent == treeItems[VIEW] ? IDM_QUERY_DATA : IDM_EDIT, 0);
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

				if (pMi->item.pszText != NULL && _tcslen(pMi->item.pszText) > 0) {
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

					if (type == 1) {
						_stprintf(query, TEXT("alter table \"%s\" rename to \"%s\""), treeEditName, pMi->item.pszText);
					} else if (type > 1) {
						const TCHAR* ddl = getDDL(treeEditName, pMi->item.lParam);
						TCHAR* newDdl = utils::replace(ddl, treeEditName, pMi->item.pszText, _tcslen(TEXT("create ")) + _tcslen(TYPES16u[type]));
						_stprintf(query, TEXT("drop %s \"%s\"; %s;"), TYPES16[type], treeEditName, newDdl);
						delete [] ddl;
						delete [] newDdl;
					}
					return executeCommandQuery(query);
				} else {
					return false;
				}
			}

			if (pHdr->code == TTN_GETDISPINFO) {
				LPTOOLTIPTEXT pTtt = (LPTOOLTIPTEXT) lParam;
				if (pTtt->hdr.idFrom < MAX_RESULT_COUNT) {
					SendMessage(pTtt->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, MAX_TEXT_LENGTH);
					pTtt->lpszText = tabTooltips[pTtt->hdr.idFrom];
				}
			}

			if (pHdr->code == (DWORD)NM_RCLICK && !_tcscmp(wndClass, WC_LISTVIEW)) {
				NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
				currCell = {ia->hdr.hwndFrom, ia->iItem, ia->iSubItem};

				POINT p;
				GetCursorPos(&p);
				TrackPopupMenu(hResultMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hMainWnd, NULL);
			}

			if (pHdr->code == EN_MSGFILTER && wParam == IDC_EDITOR)
				return processAutoComplete(pF);

			if (pHdr->code == LVN_COLUMNCLICK) {
				NMLISTVIEW* pLV   = (NMLISTVIEW*)lParam;
				hSortingResultWnd = pHdr->hwndFrom;

				// lParam is used to store tab no, so use window text to store sorting params
				TCHAR buf[32];
				GetWindowText(hSortingResultWnd, buf, 32);
				DWORD param = _tcstol(buf, NULL, 10);
				param = MAKELPARAM(pLV->iSubItem, LOWORD(param) == pLV->iSubItem ? !HIWORD(param) : 0);
				ListView_SortItems (pHdr->hwndFrom, cbListComparator, param);
				_ultot(param, buf, 10);
				SetWindowText(hSortingResultWnd, buf);

				for (int i = 0; i < ListView_GetItemCount(hSortingResultWnd); i++) {
					LVITEM lvi = {0};
					lvi.iItem = i;
					lvi.mask = LVIF_PARAM;
					lvi.iSubItem = 0;
					lvi.lParam = i;
					ListView_SetItem(hSortingResultWnd, &lvi);
				}
			}
		}
		break;

        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

LRESULT CALLBACK cbNewTreeItemEdit(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_GETDLGCODE)
        return (DLGC_WANTALLKEYS | CallWindowProc(cbOldTreeItemEdit, hwnd, uMsg, wParam, lParam));

    return CallWindowProc(cbOldTreeItemEdit, hwnd, uMsg, wParam, lParam);
}

int CALLBACK cbListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	int colNo = LOWORD(lParamSort);
	int order = HIWORD(lParamSort);

    TCHAR buf[256];
    TCHAR buf2[256];

	ListView_GetItemText(hSortingResultWnd, lParam1, colNo, buf, 256);
    ListView_GetItemText(hSortingResultWnd, lParam2, colNo, buf2, 256);

	double num = _tcstod(buf, NULL);
    double num2 = _tcstod(buf2, NULL);

    return (order ? 1 : -1) * (num && num2 ? (num2 > num ? 1 : num2 < num ? -1: 0) : _tcscoll(buf2, buf));
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

	TCHAR* buf = new TCHAR[size + 1];
	if (!SendMessage(hEditorWnd, isSelection ? EM_GETSELTEXT : WM_GETTEXT, size + 1, (LPARAM)buf)) {
		delete [] buf;
		return;
	}

	// Remove comments
	for (int i = 0; i < size; i++) {
		if (buf[i] == '-' && buf[i+1] == '-') {
			while (i < size && buf[i] != '\n' && buf[i] != '\r') {
				buf[i] = ' ';
				i++;
			}
		}
	}

	auto needPlan = [](TCHAR* query) {
		bool res = false;
		int i = 0;
		while (query[i] == ' ' || query[i] == '\n' || query[i] == '\r')
			i++;

		TCHAR explain[] = TEXT("explain");
		for (int j = 0; (j < (int)_tcslen(explain)) && (i + j < (int)_tcslen(query)); j++)
			res = res || _totlower(query[i + j]) != explain[j];

		return res;
	};

	TCHAR* query = _tcstok (buf, TEXT(";"));
	while (query != NULL) {
		bool isEmpty = true;
		for(int i = 0; i < (int)_tcslen(query); i++)
			isEmpty = isEmpty && (query[i] == ' ' || query[i] == '\n' || query[i] == '\r');

		if (!isEmpty) {
			TCHAR* q = new TCHAR[_tcslen(query)  + 64];
			_stprintf(q, isPlan && needPlan(query) ? TEXT("explain query plan %s") : TEXT("%s"), query);
			TCHAR* tq = utils::trim(q, true);
			executeQuery(tq);
			delete [] tq;
			delete [] q;
		}

		query = _tcstok (NULL, TEXT(";"));
	}

	EnumChildWindows(hTabWnd, (WNDENUMPROC)cbEnumChildren, ACTION_RESIZETAB);

	delete [] buf;
	updateTransactionState();
}

void executeQuery(TCHAR* query) {
	TCHAR* tquery = utils::trim(query);
	char* sql8 = utils::utf16to8(tquery);

	int resultNo = TabCtrl_GetItemCount(hTabWnd);

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, sql8, -1, &stmt, 0);
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
			sqlite3_free(err8);
			delete [] err16;
		}
	}

	if (rc == SQLITE_OK)
		prefs::setQuery("history", sql8);
	delete [] sql8;

	TCHAR caption[64];
	_stprintf(caption, rowCount > 0 ? TEXT("Result #%i (%i rows)") : TEXT("Result #%i"), resultNo + 1, rowCount);
	_stprintf(tabTooltips[resultNo], TEXT("%s"), tquery);

	TCITEM tci;
	tci.mask = TCIF_TEXT | TCIF_IMAGE;
	tci.iImage = -1;
	tci.pszText = caption;
	tci.cchTextMax = _tcslen(caption);
	TabCtrl_InsertItem(hTabWnd, resultNo, &tci);

	updateTransactionState();
	ShowWindow(hResultWnd, resultNo == 0 ? SW_SHOW : SW_HIDE);
	SetWindowLong(hResultWnd, GWL_USERDATA, resultNo);
	if (resultNo == 0)
		SetWindowLong(hTabWnd, GWL_USERDATA, (LONG)hResultWnd); // Also see ACTION_UPDATETAB
}

bool executeCommandQuery(const TCHAR* query) {
	bool isAutoTransaction = sqlite3_get_autocommit(db) > 0;
	if (isAutoTransaction)
		sqlite3_exec(db, "begin", NULL, 0, NULL);

	char* sql8 = utils::utf16to8(query);
	char* err8 = 0;

	int rc = sqlite3_exec(db, sql8, NULL, 0 , &err8);
	if (err8) {
		TCHAR* err16 = utils::utf8to16(err8);
		MessageBox(hMainWnd, err16, TEXT("Error"), MB_OK);
		delete [] err16;
	}
	sqlite3_free(err8);
	delete [] sql8;

	if (isAutoTransaction)
		sqlite3_exec(db, SQLITE_OK == rc ? "commit" : "rollback", NULL, 0, NULL);

	updateTransactionState();
	return rc == SQLITE_OK;
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
	updateTree();
	TreeView_Expand(hTreeWnd, treeItems[TABLE], TVE_EXPAND);
	TreeView_Expand(hTreeWnd, treeItems[VIEW], TVE_EXPAND);
    EnableWindow(hTreeWnd, true);
	updateTransactionState();

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

	// load extensions
	if (prefs::get("autoload-extensions")) {
		TCHAR extensions[2048] = TEXT("Loaded extensions: ");
		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(TEXT("./extensions/*.dll"), &ffd);
		bool isLoad = false;

		if (hFind != INVALID_HANDLE_VALUE) {
			sqlite3_enable_load_extension(db, true);
			do {
				char path8[255]{0};
				char* file8 = utils::utf16to8(ffd.cFileName);
				sprintf(path8, "./extensions/%s", file8);

				if (SQLITE_OK == sqlite3_load_extension(db, path8, NULL, NULL)) {
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
		SendMessage(hStatusWnd, SB_SETTEXT, 3, (LPARAM)(isLoad ? extensions : TEXT("")));
	}

	if (prefs::get("use-legacy-rename"))
		sqlite3_exec(db, "pragma legacy_alter_table = 1", 0, 0, 0);

	if (!PRAGMAS[0]) {
		sqlite3_stmt *stmt;
		int rc = sqlite3_prepare_v2(db, "select 'pragma_' || name || '()' from pragma_pragma_list()", -1, &stmt, 0);
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
}

void closeDb() {
	SetWindowText(hMainWnd, TEXT("No database selected"));
	EnableWindow(hTreeWnd, false);
	TreeView_DeleteAllItems(hTreeWnd);
	sqlite3_close(db);
	db = NULL;

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
}

void setEditorFont(HWND hWnd) {
	char* family8 = prefs::get("font-family", "Arial"); // Only TrueType
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
	char* fontFamily8 = prefs::get("font-family", "Arial"); // Only TrueType
	TCHAR* fontFamily16 = utils::utf8to16(fontFamily8);
    HFONT hFont = CreateFont (-MulDiv(prefs::get("font-size"), GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0,
		FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS,
		fontFamily16);

	SendMessage (hWnd, WM_SETFONT, WPARAM (hFont), TRUE);
	ReleaseDC(hWnd, hDC);

	delete [] fontFamily8;
	delete [] fontFamily16;
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
		if (GetWindowLong(hWnd, GWL_USERDATA) == TabCtrl_GetCurSel(hTabWnd)) {
			SetWindowLong(hTabWnd, GWL_USERDATA, (LONG)hWnd);
			ShowWindow(hWnd, SW_SHOW);
			SetFocus(hWnd);
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
		"select t.name, t.type, case when t.type in ('table', 'view') then group_concat(c.name || ': ' || c.type || case when c.pk then ' [pk]' else '' end,',') else null end columns " \
		"from sqlite_master t, pragma_table_info(t.tbl_name) c " \
		"where t.sql is not null and t.type = coalesce(?1, t.type) " \
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
}

void updateTransactionState() {
	SendMessage(hStatusWnd, SB_SETTEXT, 2, (LPARAM)(transactionStates[sqlite3_get_autocommit(db) > 0]));
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

int setListViewData(HWND hListWnd, sqlite3_stmt *stmt) {
	int colCount = sqlite3_column_count(stmt);
	HWND hHeader = ListView_GetHeader(hListWnd);

	if (hHeader == NULL || Header_GetItemCount(hHeader) == 0) {
		for (int i = 0; i < colCount; i++) {
			TCHAR* name16 = utils::utf8to16(sqlite3_column_name(stmt, i));
			LVCOLUMN lvc;
			lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.iSubItem = i;
			lvc.pszText = name16;
			lvc.cchTextMax = _tcslen(name16) + 1;
			lvc.cx = 100;
			ListView_InsertColumn(hListWnd, i, &lvc);
			delete [] name16;
		}
	}

	ListView_DeleteAllItems(hListWnd);

	int rowNo = 0;
	while(sqlite3_step(stmt) == SQLITE_ROW) {
		char* name8 =  (char *) sqlite3_column_text(stmt, 0);
		TCHAR* name16 = utils::utf8to16(name8);

		if (rowNo == 0) {
			for (int i = 0; i < colCount; i++) {
				int type = sqlite3_column_type(stmt, i);
				LVCOLUMN lvc = {mask: LVCF_FMT};
				lvc.fmt = type == SQLITE_INTEGER || type == SQLITE_FLOAT ? LVCFMT_RIGHT : LVCFMT_LEFT;
				ListView_SetColumn(hListWnd, i, &lvc);
			}
		}

		LVITEM lvi = {0};
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.iItem = rowNo;
		lvi.pszText = name16;
		lvi.cchTextMax = _tcslen(name16) + 1;
		lvi.lParam = rowNo;
		ListView_InsertItem(hListWnd, &lvi);
		delete [] name16;

		for (int i = 1; i < colCount; i++) {
			bool isNull = sqlite3_column_type(stmt, i) == SQLITE_NULL;
			char* name8 = (char *) sqlite3_column_text(stmt, i);
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

	if (SQLITE_DONE != sqlite3_errcode(db)) {
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

		sqlite3_free(err8);
		delete [] err16;
		delete [] msg16;
	}

	sqlite3_finalize(stmt);

	ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | 0x10000000 /*LVS_EX_AUTOSIZECOLUMNS*/);
	for (int i = 0; i < colCount; i++) {
		ListView_SetColumnWidth(hListWnd, i, LVSCW_AUTOSIZE_USEHEADER);
		if (ListView_GetColumnWidth(hListWnd, i) > 200)
			ListView_SetColumnWidth(hListWnd, i, 200);
	}

	return rowNo;
}

void updateHighlighting(HWND hWnd) {
	SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);
	int size = GetWindowTextLength(hWnd);
	TCHAR* text = new TCHAR[size + 1]{0};
	GetWindowText(hWnd, text, size + 1);

	CHARFORMAT cf = {0};
	cf.cbSize = sizeof(CHARFORMAT) ;
	SendMessage(hWnd, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM) &cf);
	cf.dwMask = CFM_COLOR | CFM_BOLD;
	cf.dwEffects = 0;
	cf.crTextColor = RGB(255, 0, 0);

	int carriagePos;
	SendMessage(hWnd, EM_GETSEL, (WPARAM)&carriagePos, (WPARAM)&carriagePos);

	int pos = 0;
	int rCount = 0;
	int mode = 0; // 0 - casual, 1 - keyword, 2 - quotted string, 3 - comment
	COLORREF colors[4] = {RGB(0, 0, 0), RGB(0, 0, 200), RGB(0, 200, 0), RGB(255,0,0)};

	while (pos < size) {
		mode = text[pos] == TEXT('-') && (pos < size - 1) && text[pos + 1] == TEXT('-') ? 3 :
			text[pos] == TEXT('"') || text[pos] == TEXT('\'') ? 2 :
			text[pos] == TEXT(';') || text[pos] == TEXT('\n') ? 0 :
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
				} while (
					text[pos] != TEXT(' ') &&
					text[pos] != TEXT('"') &&
					text[pos] != TEXT('\'') &&
					text[pos] != TEXT('\n') &&
					text[pos] != TEXT('-') &&
					text[pos] != TEXT(';') &&
					text[pos] != TEXT('(') &&
					text[pos] != TEXT(')') &&
					text[pos] != TEXT(',') &&
					pos < size);

				TCHAR* buf = new TCHAR[pos - start + 1]{0};
				for(int i = 0; i < pos - start; i++)
					buf[i] = _totlower(text[start + i]);

				TCHAR* tbuf = utils::trim(buf);
				bool isKeyWord = false;
				int i = 0;
				while (SQL_KEYWORDS[i] && !isKeyWord && tbuf) {
					isKeyWord = !_tcscmp(SQL_KEYWORDS[i], tbuf);
					i++;
				}

				i = 0;
				while (FUNCTIONS[i] && !isKeyWord && tbuf) {
					isKeyWord = !_tcscmp(FUNCTIONS[i], tbuf);
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

		cf.crTextColor = colors[mode];
		cf.dwEffects = mode == 1 ? CFM_BOLD : 0;
		SendMessage(hWnd, EM_SETSEL, from, to);
		SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	}

	SendMessage(hWnd, EM_SETSEL, carriagePos, carriagePos);
	delete [] text;
	SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(hWnd, 0, TRUE);
}

bool processAutoComplete(MSGFILTER* pF) {
	int key = pF->wParam;
	bool isKeyDown = pF->lParam & (1U << 31);
	HWND hParent = pF->nmhdr.hwndFrom;

	if (IsWindowVisible(hAutoComplete)) {
		if (isKeyDown) {
			if (key == VK_ESCAPE)
				ShowWindow(hAutoComplete, SW_HIDE);

			if (key == VK_UP || key == VK_DOWN) {
				int iCount = ListBox_GetCount(hAutoComplete);
				int pos = ListBox_GetCurSel(hAutoComplete) + (key == VK_UP ? -1 : 1);
				ListBox_SetCurSel(hAutoComplete, pos % iCount);
			}

			if (key == VK_RETURN) {
				TCHAR buf[256] = {0};
				int pos = ListBox_GetCurSel(hAutoComplete);
				ListBox_GetText(hAutoComplete, pos, buf);
				int start = ListBox_GetItemData(hAutoComplete, pos);
				TCHAR word[256] = {0};

				_tcsncpy(word, buf + start, _tcslen(buf) - start);
				SendMessage(hParent, EM_REPLACESEL, TRUE, (LPARAM)word);
				ShowWindow(hAutoComplete, SW_HIDE);
			}
		}

		if (key == VK_ESCAPE || key == VK_UP || key == VK_DOWN || key == VK_RETURN) {
			pF->wParam = 0; // Stop propagation
			return true;
		}
	}

	if (!isKeyDown)
		return false;

	POINT p = {0};
	GetCaretPos(&p);
	int crPos;
	SendMessage(hParent, EM_GETSEL, (WPARAM)&crPos, (WPARAM)&crPos);

	int size = crPos - SendMessage(hParent, EM_LINEINDEX, -1, 0); // Get line backward
	TCHAR buf[size + 1] = {0};
	/* !!! Doesn't work for RICHEDIT_CLASS if a richedit lays inside dialog */
	TEXTRANGE tr = {chrg: {cpMin: crPos - size, cpMax: crPos}, lpstrText: buf};
	SendMessage(hParent, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

	TCHAR cbuf[2] = {0}; // next one char
	TEXTRANGE ctr = {chrg: {cpMin: crPos, cpMax: crPos + 1}, lpstrText: cbuf};
	SendMessage(hParent, EM_GETTEXTRANGE, 0, (LPARAM)&ctr);
	if (cbuf[0] != 0 && cbuf[0] != TEXT(' ') && cbuf[0] != TEXT('(') && cbuf[0] != TEXT(')'))
		return false;


	int start = size;
	while (start > 0 && buf[start - 1] != TEXT(' ') && buf[start - 1] != TEXT('\n') && buf[start - 1] != TEXT(';') && buf[start - 1] != TEXT('"') && buf[start - 1] != TEXT('\''))
		start--;

	TCHAR word[size - start + 1] = {0};
	for (int i = 0; i < size - start; i++)
		word[i] = _totlower(buf[start + i]); // lower case

	ListBox_ResetContent(hAutoComplete);
	size_t wLen = _tcslen(word);

	auto addString = [&word, wLen](HWND hWnd, const TCHAR* str) {
		if(!_tcsncmp(str, word, wLen) && _tcslen(str) != wLen) {
			int pos = ListBox_AddString(hWnd, str);
			ListBox_SetItemData(hAutoComplete, pos, wLen);
		}
	};

	if (wLen > 1) {
		if (word[wLen - 1] == TEXT('.')) { // tablename. or aliasname.
			word[wLen - 1] = 0;
			wLen--;

			size_t tLen = GetWindowTextLength(hParent);
			TCHAR* text = new TCHAR[tLen + 1]{0};
			GetWindowText(hParent, text, tLen + 1);

			for (int i = 0; TABLES[i]; i++)	{
				bool isSuitable = false;
				isSuitable = _tcslen(TABLES[i]) == wLen && !_tcscmp(TABLES[i], word);

				size_t tbl_aLen = _tcslen(TABLES[i]) + wLen;
				TCHAR* tbl_a = new TCHAR[tbl_aLen + 1 + 1 + 2] {0};

				if (!isSuitable) {
					_stprintf(tbl_a, TEXT("%s %s"), TABLES[i], word); // tablename alias
					TCHAR* p = _tcsstr(text, tbl_a);
					isSuitable = p && // found
						(_tcslen(p) == tLen || (!_istalpha((p - tbl_aLen)[0]) || !_istdigit((p - tbl_aLen)[0]))) &&// not xxxtablename alias
						(p + tbl_aLen == 0 || (!_istalpha((p + tbl_aLen)[0]) || !_istdigit((p + tbl_aLen)[0]))); // not tablename aliasxxx
				}

				if(!isSuitable) {
					_stprintf(tbl_a, TEXT("\"%s\" %s"), TABLES[i], word); // "tablename" alias
					TCHAR* p = _tcsstr(text, tbl_a);
					isSuitable = p && // found
						(_tcslen(p) == tLen || (!_istalpha((p - tbl_aLen)[0]) || !_istdigit((p - tbl_aLen)[0]))) &&// not "xxxtablename" alias
						(p + tbl_aLen == 0 || (!_istalpha((p + tbl_aLen)[0]) || !_istdigit((p + tbl_aLen)[0]))); // not "tablename" aliasxxx
				}
				delete [] tbl_a;


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
		} else {
			for (int i = 0; TABLES[i]; i++)
				addString(hAutoComplete, TABLES[i]);

			for (int i = 0; FUNCTIONS[i]; i++)
				addString(hAutoComplete, FUNCTIONS[i]);

			for (int i = 0; PRAGMAS[i]; i++)
				addString(hAutoComplete, PRAGMAS[i]);

			for (int i = 0; SQL_KEYWORDS[i]; i++)
				addString(hAutoComplete, SQL_KEYWORDS[i]);
		}
	}
	int h = SendMessage(hAutoComplete, LB_GETITEMHEIGHT, 0, 0);
	int iCount = ListBox_GetCount(hAutoComplete);
	RECT rc = {0};
	GetClientRect(hAutoComplete, &rc);
	rc.bottom = rc.top + h * iCount;

	rc.right += GetSystemMetrics(SM_CXEDGE) * 2 - 2;
	rc.bottom += GetSystemMetrics(SM_CXEDGE) * 2;
	SetWindowPos(hAutoComplete, 0, 0, 0, rc.right, rc.bottom > 150 ? 150 : rc.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	if (!ShowWindow(hAutoComplete, iCount > 0 ? SW_SHOW : SW_HIDE))
		SetWindowPos(hAutoComplete, 0, p.x - 10, p.y + 20, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	ListBox_SetCurSel(hAutoComplete, 0);

	return false;
}
