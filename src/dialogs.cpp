#include <windows.h>
#include <shlobj.h>
#include <winreg.h>
#include "tinf.h"
#include "resource.h"
#include "global.h"
#include "prefs.h"
#include "utils.h"
#include "dbutils.h"
#include "dialogs.h"
#include "tools.h"

namespace dialogs {
	WNDPROC cbOldEditDataEdit, cbOldAddTableCell, cbOldAddTableComboboxEdit, cbOldAddTableHeader, cbOldHeaderEdit, cbOldGridColorEdit, cbOldChartOptions;
	LRESULT CALLBACK cbNewEditDataEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewEditDataCombobox(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewEditDataComboboxEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewAddTableCell(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewAddTableComboboxEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewAddTableHeader(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewFilterEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewRowEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewChart(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewChartOptions(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgFKSelector (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	COLORREF GetBrushColor(HBRUSH brush);

	const TCHAR* DATATYPES16[] = {TEXT("integer"), TEXT("real"), TEXT("text"), TEXT("blob"), TEXT("any"), TEXT("json"), TEXT("jsonb"), 0};
	const TCHAR* INDENT_LABELS[] = {TEXT("Tab"), TEXT("2 spaces"), TEXT("4 spaces"), 0};
	const TCHAR* INDENTS[] = {TEXT("\t"), TEXT("  "), TEXT("    ")};
	const TCHAR* tooltips[] = {
		TEXT("Column number\nNo have matter."),
		TEXT("Column name\nThere are no restrictions, but recommended to avoid\n* Any non-alpha or non-digit symbols e.g. space or percentile\n* The length greater than 255 characters"),
		TEXT("Column type\nSQLite supports 5 types: NULL, INTEGER, REAL, TEXT and BLOB. Only these types and ANY are allowed in the strict mode. Any other type will be convert to one of them. Can be empty."),
		TEXT("PRIMARY KEY\nIn SQL standard, the primary key column must not contain NULL values. However, to make the current version of SQLite compatible with the earlier version, SQLite allows the primary key column to contain NULL values.\n\nIf only one column has the primary key flag then this column will be AUTOINCREMENT."),
		TEXT("NOT NULL constraint\nCheck to prevent NULL values in column."),
		TEXT("UNIQUE constraint\nEnsures all values in a column or a group of columns are distinct from one another or unique. SQLite treats all NULL values are different, therefore, a column with a UNIQUE constraint and without NOT NULL can have multiple NULL values."),
		TEXT("Default value\nConstraint will insert this value in a column in case if column value null or empty."),
		TEXT("CHECK constraints\nAllow you to define expressions to test values whenever they are inserted into\nor updated within a column e.g length(phone) >= 10"),
		0,
		0
	};

	HMENU hEditDataMenu = GetSubMenu(LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDC_MENU_EDIT_DATA)), 0);
	HMENU hViewDataMenu = GetSubMenu(LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDC_MENU_VIEW_DATA)), 0);

	// lParam = (TEXT)[action, type][table16:etc]
	// action: 0 - add, 1 - view, 2 - edit, 3 - view+
	BOOL CALLBACK cbDlgAddViewEdit (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TCHAR* params = (TCHAR*)lParam;
				int action = LOBYTE(params[0]);
				int type = HIBYTE(params[0]);
				TCHAR* fullname16 = params + 1;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, type);

				int len = 64 + _tcslen(fullname16);
				TCHAR buf[len + 1];
				TCHAR* ufullname16 = _tcsdup(fullname16);
				_tcsupr(ufullname16);
				_sntprintf(buf, len,
					action == 0 ? TEXT("Add %ls") :
					action == 1 || action == 3 ? TEXT("View %ls %ls") :
					TEXT("Edit %ls %ls"), TYPES16[type], ufullname16);
				free(ufullname16);
				SetWindowText(hWnd, buf);

				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				SetProp(hEditorWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hEditorWnd, GWLP_WNDPROC, (LONG_PTR)cbNewEditor));
				SendMessage(hEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_KEYEVENTS);
				setEditorFont(hEditorWnd);

				TCHAR* schema16 = utils::getTableName(fullname16, true);
				TCHAR* tablename16 = utils::getTableName(fullname16);

				if (action != 0) {
					TCHAR* ddl = getDDL(schema16, tablename16, type, action == 2 ? 1 : action == 3 ? 2 : 0);
					if (ddl) {
						SetWindowText(hEditorWnd, ddl);
						delete [] ddl;
					} else {
						SetWindowText(hEditorWnd, TEXT("Error to get DDL"));
					}
				}

				if (action == 1 || action == 3)
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_OK), SW_HIDE);

				if (action)
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_EXAMPLE), SW_HIDE);

				delete [] schema16;
				delete [] tablename16;

				if (prefs::get("word-wrap"))
					toggleWordWrap(hEditorWnd);

				utils::alignDialog(hWnd, GetAncestor(hWnd, GA_ROOT));
				SetFocus(hEditorWnd);
			}
			break;

			case WM_SIZE: {
				POINTFLOAT s = utils::getDlgScale(hWnd);

				RECT rc;
				GetClientRect(hWnd, &rc);
				int H = rc.bottom - (IsWindowVisible(GetDlgItem(hWnd, IDC_DLG_OK)) ? (14 + 5 - 2) * s.y : 0) - 2 * 5 * s.y;
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_EDITOR), 0, 0, 0, rc.right - rc.left - 2 * 5 * s.x, H, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_EXAMPLE), 0, 5 * s.x , H + 2 * 5 * s.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_OK), 0, rc.right - rc.left - (54 + 5) * s.x, H + 2 * 5 * s.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			break;

			case WM_CONTEXTMENU: {
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				bool isContextKey = p.x == -1 && p.y == -1;
				if ((HWND)wParam == GetDlgItem(hWnd, IDC_DLG_EDITOR) && !isContextKey)
					TrackPopupMenu(hEditorMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
			}
			break;

			case WM_COMMAND: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);

				if (LOWORD(wParam) == IDC_DLG_EDITOR && HIWORD(wParam) == EN_CHANGE)
					SendMessage((HWND)lParam, WMU_TEXT_CHANGED, 0, 0);

				if (wParam == IDC_DLG_EXAMPLE) {
					TCHAR buf[1024];
					int type = GetWindowLongPtr(hWnd, GWLP_USERDATA);
					LoadString(GetModuleHandle(NULL), IDS_CREATE_DDL + type, buf, 1024);
					SetWindowText(GetDlgItem(hWnd, IDC_DLG_EDITOR), buf);
				}

				if (wParam == IDM_EDITOR_COMMENT)
					toggleComment(hEditorWnd);

				if (wParam == IDM_EDITOR_CUT)
					SendMessage(hEditorWnd, WM_CUT, 0, 0);

				if (wParam == IDM_EDITOR_COPY)
					SendMessage(hEditorWnd, WM_COPY, 0, 0);

				if (wParam == IDM_EDITOR_PASTE)
					pasteText(hEditorWnd);

				if (wParam == IDM_EDITOR_DELETE)
					SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, 0);

				if (wParam == IDM_EDITOR_FIND) {
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_FIND), hWnd, (DLGPROC)cbDlgFind, (LPARAM)hEditorWnd);
					SetForegroundWindow(hWnd);
					SetFocus(hEditorWnd);
				}

				if (wParam == IDM_EDITOR_FORMAT)
					formatQuery(hEditorWnd);

				if (wParam == IDC_DLG_OK) {
					int size = GetWindowTextLength(hEditorWnd) + 1;
					TCHAR query16[size]{0};
					GetWindowText(hEditorWnd, query16, size);
					char* query8 = utils::utf16to8(query16);
					int rc = sqlite3_exec(db, query8, NULL, 0 , 0);
					delete [] query8;

					SetFocus(hEditorWnd);
					if (SQLITE_OK == rc) {
						SendMessage(hMainWnd, WMU_OBJECT_CREATED, (WPARAM)GetWindowLongPtr(hWnd, GWLP_USERDATA), 0);
						SendMessage(hWnd, WM_CLOSE, 0, 0);
					} else {
						showDbError(hMainWnd);
					}
				}
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (wParam == IDC_DLG_EDITOR && pHdr->code == EN_SELCHANGE)
					return SendMessage(pHdr->hwndFrom, WMU_SELECTION_CHANGED, wParam, lParam);

				if (wParam == IDC_DLG_EDITOR && pHdr->code == EN_MSGFILTER)
					return processEditorEvents((MSGFILTER*)lParam);

			}
			break;

			case WM_CLOSE: {
				// If modal
				EndDialog(hWnd, DLG_CANCEL);

				// If non-modal
				SendMessage(hMainWnd, WMU_UNREGISTER_DIALOG, (WPARAM)hWnd, 0);
				DestroyWindow(hWnd);
			}
			break;
		}

		return false;
	}


	// lParam, USERDATA = in-out buffer: in - schema, out - new table name
	BOOL CALLBACK cbDlgAddTable (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
				TCHAR* schema16 = (TCHAR*)lParam;
				if (_tcscmp(schema16, TEXT("main"))) {
					TCHAR title16[512];
					_sntprintf(title16, 511, TEXT("Add table to %ls"), schema16);
					SetWindowText(hWnd, title16);
				}

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);

				float z = utils::getWndScale(hWnd).x;
				const TCHAR* colNames[] = {TEXT("#"), TEXT("Name"), TEXT("Type"), TEXT("PK"), TEXT("NN"), TEXT("UQ"), TEXT("Default"), TEXT("Check"), 0};
				int colWidths[] = {(int)(30 * z), (int)(145 * z), (int)(58 * z), (int)(30 * z), (int)(30 * z), (int)(30 * z), 0, 0, 0};

				for (int i = 0; colNames[i]; i++) {
					LVCOLUMN lvc = {0};
					lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
					lvc.iSubItem = i;
					lvc.pszText = (TCHAR*)colNames[i];
					lvc.cchTextMax = _tcslen(colNames[i]) + 1;
					lvc.cx = colWidths[i];
					lvc.fmt = colWidths[i] < 80 * z ? LVCFMT_CENTER : LVCFMT_LEFT;
					ListView_InsertColumn(hListWnd, i, &lvc);
				}

				LVCOLUMN lvc = {mask: LVCF_FMT, fmt: LVCFMT_RIGHT};
				ListView_SetColumn(hListWnd, 0, &lvc);

				SendMessage(hWnd, WMU_ADD_ROW, 0, 0);
				ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

				HWND hHeader = ListView_GetHeader(hListWnd);
				cbOldAddTableHeader = (WNDPROC)SetWindowLongPtr(hHeader, GWLP_WNDPROC, (LONG_PTR)cbNewAddTableHeader);
				SetWindowLongPtr(hHeader, GWLP_USERDATA, 1000); // ~ -1
				SetWindowTheme(hHeader, TEXT(" "), TEXT(" "));

				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumFixEditHeights, (LPARAM)utils::getEditHeight(hWnd));
			}
			break;

			case WM_COMMAND: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);
				if (wParam == IDC_DLG_OK) {
					int rowCount = ListView_GetItemCount(hListWnd);
					bool isWithoutRowid = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISWITHOUT_ROWID));
					bool isStrict = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_STRICT));
					int pkCount = 0;
					TCHAR pk16[1024] = {0};
					for (int i = 0; i < rowCount; i++) {
						TCHAR colName16[255];
						TCHAR isPK[2];
						ListView_GetItemText(hListWnd, i, 1, colName16, 255);
						ListView_GetItemText(hListWnd, i, 3, isPK, 2);
						if (_tcslen(colName16) > 0 && _tcslen(isPK)) {
							if (_tcslen(pk16))
								_tcscat(pk16, TEXT("\",\""));
							_tcscat(pk16, colName16);
							pkCount++;
						}
					}

					int colCount = 0;
					bool isStrictValid = true;
					TCHAR columns16[MAX_TEXT_LENGTH] = {0};
					for (int rowNo = 0; rowNo < rowCount; rowNo++) {
						TCHAR* row[8] = {0};
						for (int colNo = 0; colNo < 8; colNo++) {
							row[colNo] = new TCHAR[512]{0};
							ListView_GetItemText(hListWnd, rowNo, colNo, row[colNo], 512);
						}

						if (!_tcslen(row[1]))
							continue;

						isStrictValid = isStrict && isStrictValid && (
							_tcsicmp(row[2], TEXT("INT")) == 0 ||
							_tcsicmp(row[2], TEXT("INTEGER")) == 0 ||
							_tcsicmp(row[2], TEXT("TEXT")) == 0 ||
							_tcsicmp(row[2], TEXT("REAL")) == 0 ||
							_tcsicmp(row[2], TEXT("BLOB")) == 0 ||
							_tcsicmp(row[2], TEXT("ANY")) == 0
						);

						TCHAR colDefinition16[2048];
						// name type [NOT NULL] [DEFAULT ...] [CHECK(...)] [PRIMARY KEY] [AUTOINCREMENT] [UNIQUE]
						_sntprintf(colDefinition16, 2048, TEXT("\"%ls\" %ls%ls%ls%ls%ls%ls%ls%ls%ls%ls"),
							row[1], // name
							row[2], // type
							_tcslen(row[4]) ? TEXT(" not null") : TEXT(""),
							_tcslen(row[6]) ? TEXT(" default \"") : TEXT(""),
							_tcslen(row[6]) ? row[6] : TEXT(""),
							_tcslen(row[6]) ? TEXT("\"") : TEXT(""),
							_tcslen(row[7]) ? TEXT(" check(") : TEXT(""),
							_tcslen(row[7]) ? row[7] : TEXT(""),
							_tcslen(row[7]) ? TEXT(")") : TEXT(""),
							!_tcslen(row[3]) || pkCount > 1 ? TEXT("") : _tcsstr(TEXT("int"), row[2]) != NULL && !isWithoutRowid ? TEXT(" primary key autoincrement")	: TEXT(" primary key"),
							_tcslen(row[5]) ? TEXT(" unique") : TEXT("")
							);

						if (_tcslen(columns16))
							_tcscat(columns16, TEXT(",\n"));
						_tcscat(columns16, colDefinition16);

						for (int i = 0; i < 8; i++)
							delete [] row[i];

						colCount++;
					}

					TCHAR tblName16[255] = {0};
					GetDlgItemText(hWnd, IDC_DLG_TABLENAME, tblName16, 255);

					if (!colCount || !_tcslen(tblName16)) {
						MessageBox(hWnd, TEXT("The table should have a name and at least one column"), NULL, 0);
						return 0;
					}

					if (isStrict && !isStrictValid) {
						MessageBox(hWnd, TEXT("Only INT, INTEGER, TEXT, REAL, BLOB or ANY\ntypes are allowed in the strict mode."), NULL, 0);
						return 0;
					}

					TCHAR* fullname16 = utils::getFullTableName((TCHAR*)GetWindowLongPtr(hWnd, GWLP_USERDATA), tblName16, false);
					TCHAR query16[MAX_TEXT_LENGTH] = {0};
					_sntprintf(query16, MAX_TEXT_LENGTH, TEXT("create table %ls (\n%ls%ls%ls%ls\n)%ls%ls"),
						fullname16,
						columns16,
						pkCount > 1 ? TEXT(", primary key(\"") : TEXT(""),
						pkCount > 1 ? pk16 : TEXT(""),
						pkCount > 1 ? TEXT("\")") : TEXT(""),
						isWithoutRowid ? TEXT(" without rowid") : TEXT(""),
						isStrict ? TEXT(" strict") : TEXT("")
					);
					delete [] fullname16;

					char* query8 = utils::utf16to8(query16);
					int rc = sqlite3_exec(db, query8, NULL, 0 , 0);
					delete [] query8;

					if (SQLITE_OK == rc) {
						TCHAR* out16 = (TCHAR*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
						_tcscpy(out16, tblName16);
						EndDialog(hWnd, DLG_OK);
					} else {
						showDbError(hMainWnd);
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);

				if (wParam == IDC_DLG_MORE) {
					HWND hBtn = GetDlgItem(hWnd, IDC_DLG_MORE);
					bool isOpen = GetWindowLongPtr(hBtn, GWLP_USERDATA);
					SetWindowLongPtr(hBtn, GWLP_USERDATA, !isOpen);
					SetWindowText(hBtn, isOpen ? TEXT(">>") : TEXT("<<"));
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_ISWITHOUT_ROWID), isOpen ? SW_HIDE : SW_SHOW);

					float z = utils::getWndScale(hWnd).x;
					RECT rc;
					GetWindowRect(hWnd, &rc);
					SetWindowPos(hWnd, 0, 0, 0, rc.right - rc.left + (isOpen ? -250 : 250) * z, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

					GetWindowRect(hListWnd, &rc);
					SetWindowPos(hListWnd, 0, 0, 0, rc.right - rc.left + (isOpen ? -250 : 250) * z, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

					LVCOLUMN lvc = {mask: LVCF_WIDTH, fmt: 0, cx: + (isOpen ? 0 : 125) * (int)z};
					ListView_SetColumn(hListWnd, 6, &lvc);
					ListView_SetColumn(hListWnd, 7, &lvc);
				}

				if (wParam == IDC_DLG_ROW_ADD || wParam == IDC_DLG_ROW_DEL || wParam == IDC_DLG_ROW_UP || wParam == IDC_DLG_ROW_DOWN) {
					DestroyWindow(FindWindowEx(hListWnd, 0, WC_COMBOBOX, 0));
					DestroyWindow(FindWindowEx(hListWnd, 0, WC_EDIT, 0));
				}

				if (wParam == IDC_DLG_ROW_ADD)
					SendMessage(hWnd, WMU_ADD_ROW, 0, 0);

				if (wParam == IDC_DLG_ROW_DEL) {
					int pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					if (pos != -1) {
						ListView_DeleteItem(hListWnd, pos);
						SendMessage(hWnd, WMU_UPDATE_ROWNO, 0, 0);
						ListView_SetItemState (hListWnd, pos, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
					}
				}

				if (wParam == IDC_DLG_ROW_UP || wParam == IDC_DLG_ROW_DOWN) {
					int pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					if (pos == -1 || (pos == 0 && wParam == IDC_DLG_ROW_UP) || (pos == ListView_GetItemCount(hListWnd) - 1 && wParam == IDC_DLG_ROW_DOWN))
						return true;

					pos = wParam == IDC_DLG_ROW_UP ? pos - 1 : pos;

					HWND hHeader = ListView_GetHeader(hListWnd);
					for (int i = 0; i < Header_GetItemCount(hHeader); i++) {
						TCHAR buf[255]{0};
						ListView_GetItemText(hListWnd, pos, i, buf, 255);
						LVITEM lvi = {0};
						lvi.mask = LVIF_TEXT;
						lvi.iItem = pos + 2;
						lvi.iSubItem = i;
						lvi.pszText = buf;
						lvi.cchTextMax = 255;
						if (i == 0)
							ListView_InsertItem(hListWnd, &lvi);
						else
							ListView_SetItem(hListWnd, &lvi);
					}
					ListView_DeleteItem(hListWnd, pos);
					if (wParam == IDC_DLG_ROW_DOWN)
						ListView_SetItemState (hListWnd, pos + 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);

					SendMessage(hWnd, WMU_UPDATE_ROWNO, 0, 0);
				}
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;

				if (pHdr->code == (DWORD)NM_CLICK && pHdr->idFrom == IDC_DLG_COLUMNS) {
					HWND hListWnd = pHdr->hwndFrom;
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;

					DestroyWindow(FindWindowEx(hListWnd, 0, WC_COMBOBOX, 0));
					DestroyWindow(FindWindowEx(hListWnd, 0, WC_EDIT, 0));

					if (ia->iItem == -1) {
						int rowNo = ListView_GetItemCount(hListWnd) - 1;
						TCHAR colName16[3]{0};
						ListView_GetItemText(hListWnd, rowNo, 1, colName16, 2);
						if (_tcslen(colName16) > 0) {
							SendMessage(hWnd, WM_COMMAND, IDC_DLG_ROW_ADD, 0);
							rowNo++;
						}
						ListView_SetItemState(hListWnd, rowNo + 1, LVIS_SELECTED, LVIS_SELECTED);
						return true;
					}

					RECT rect;
					ListView_GetSubItemRect(hListWnd, ia->iItem, ia->iSubItem, LVIR_BOUNDS, &rect);
					int h = rect.bottom - rect.top;
					int w = ListView_GetColumnWidth(hListWnd, ia->iSubItem);

					TCHAR buf[1024];
					ListView_GetItemText(hListWnd, ia->iItem, ia->iSubItem, buf, MAX_TEXT_LENGTH);

					if (ia->iSubItem == 3 || ia->iSubItem == 4 || ia->iSubItem == 5) {
						ListView_SetItemText(hListWnd, ia->iItem, ia->iSubItem, (TCHAR*)(_tcslen(buf) ? TEXT("") : TEXT("v")));
						return true;
					}

					HWND hCell = 0;
					if (ia->iSubItem == 2) {
						hCell = CreateWindow(WC_COMBOBOX, buf, CBS_DROPDOWN | CBS_HASSTRINGS | WS_VISIBLE | WS_CHILD | WS_TABSTOP, rect.left, rect.top - 4, w + 18, 200, hListWnd, NULL, GetModuleHandle(0), NULL);
						ComboBox_AddString(hCell, TEXT(""));
						for (int i = 0; DATATYPES16[i]; i++)
							ComboBox_AddString(hCell, DATATYPES16[i]);
						ComboBox_SetText(hCell, buf);

						COMBOBOXINFO ci{0};
						ci.cbSize = sizeof(COMBOBOXINFO);
						GetComboBoxInfo(hCell, &ci);
						cbOldAddTableComboboxEdit = (WNDPROC)SetWindowLongPtr(ci.hwndItem, GWLP_WNDPROC, (LONG_PTR)cbNewAddTableComboboxEdit);
					}

					if (ia->iSubItem == 1 || ia->iSubItem == 6 || ia->iSubItem == 7) {
						hCell = CreateWindowEx(0, WC_EDIT, buf, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, rect.left, rect.top, w, h - 1, hListWnd, 0, GetModuleHandle(NULL), NULL);
						int end = GetWindowTextLength(hCell);
						SendMessage(hCell, EM_SETSEL, end, end);
					}

					if (hCell) {
						SetWindowLongPtr(hCell, GWLP_USERDATA, MAKELPARAM(ia->iItem, ia->iSubItem));
						cbOldAddTableCell = (WNDPROC)SetWindowLongPtr(hCell, GWLP_WNDPROC, (LONG_PTR)cbNewAddTableCell);
						SendMessage(hCell, WM_SETFONT, (LPARAM)SendMessage(hWnd, WM_GETFONT, 0, 0), true);
						SetFocus(hCell);
					}
				}

				if (pHdr->code == LVN_KEYDOWN && pHdr->idFrom == IDC_DLG_COLUMNS) {
					NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;
					if (kd->wVKey == VK_DELETE)
						SendMessage(hWnd, WM_COMMAND, IDC_DLG_ROW_DEL, 0);
				}

				if (wParam == IDC_DLG_EDITOR) {
					NMHDR* pHdr = (LPNMHDR)lParam;
					if (pHdr->code == EN_MSGFILTER)
						return processEditorEvents((MSGFILTER*)lParam);
				}
			}
			break;

			case WMU_ADD_ROW: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);

				LVITEM lvi = {0};
				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(hListWnd);
				lvi.pszText = (TCHAR*)TEXT("");
				lvi.cchTextMax = 1;
				ListView_InsertItem(hListWnd, &lvi);
				SendMessage(hWnd, WMU_UPDATE_ROWNO, 0, 0);
			}
			break;

			case WMU_UPDATE_ROWNO: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);
				for (int i = 0; i < ListView_GetItemCount(hListWnd); i++) {
					TCHAR buf[32];
					_itot(i + 1, buf, 10);
					LVITEM lvi = {0};
					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 0;
					lvi.iItem = i;
					lvi.pszText = buf;
					lvi.cchTextMax = 32;
					ListView_SetItem(hListWnd, &lvi);
				}
			}
			break;

			case WM_DESTROY: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);
				DestroyWindow(FindWindowEx(hListWnd, 0, WC_COMBOBOX, 0));
				DestroyWindow(FindWindowEx(hListWnd, 0, WC_EDIT, 0));
			}
			break;

			case WM_CLOSE: {
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgQueryList (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
				SetWindowPos(hWnd, 0, prefs::get("x") + 40, prefs::get("y") + 80, prefs::get("width") - 80, prefs::get("height") - 120,  SWP_NOZORDER);
				ShowWindow (hWnd, prefs::get("maximized") == 1 ? SW_MAXIMIZE : SW_SHOW);
				SetWindowText(hWnd, lParam == IDM_HISTORY ? TEXT("Query history") : TEXT("Saved queries"));


				SendMessage(GetDlgItem(hWnd, IDC_DLG_QUERYFILTER), WM_SETFONT, (LPARAM)hFont, true);
				SendMessage(GetDlgItem(hWnd, IDC_DLG_QUERYLIST), WM_SETFONT, (LPARAM)hFont, true);
				SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
				SendMessage(hWnd, WM_SIZE, 0, 0);

				SetProp(hWnd, TEXT("MENU"), (HANDLE)LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDC_MENU_QUERYLIST)));
				SetProp(hWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewEdit));
			}
			break;

			case WM_TIMER: {
				if (wParam == IDT_EDIT_DATA) {
					KillTimer(hWnd, IDT_EDIT_DATA);
					SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
				}
			}
			break;

			case WM_SIZE: {
				HWND hFilterWnd = GetDlgItem(hWnd, IDC_DLG_QUERYFILTER);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);

				RECT rc = {0, 14, 0, 0};
				MapDialogRect(hWnd, &rc);
				int h = rc.top;

				GetClientRect(hWnd, &rc);
				SetWindowPos(hFilterWnd, 0, 0, 0, rc.right - rc.left, h, SWP_NOZORDER | SWP_NOMOVE);
				SetWindowPos(hListWnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top - h - 1, SWP_NOZORDER | SWP_NOMOVE);

				LVCOLUMN lvc;
				lvc.mask = LVCF_WIDTH;
				lvc.cx = 0;
				ListView_SetColumn(hListWnd, 2, &lvc);

				lvc.mask = LVCF_WIDTH;
				lvc.cx = rc.right - rc.left - 130;
				ListView_SetColumn(hListWnd, 3, &lvc);
			}
			break;

			case WM_CONTEXTMENU: {
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				HMENU hMenu = GetSubMenu((HMENU)GetProp(hWnd, TEXT("MENU")), 0);
				int isOne = ListView_GetSelectedCount(hListWnd) == 1;
				EnableMenuItem(hMenu, IDM_QUERY_ADD_OLD, MF_BYCOMMAND | (isOne ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, IDM_QUERY_ADD_NEW, MF_BYCOMMAND | (isOne ? MF_ENABLED : MF_GRAYED));
				TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (pHdr->idFrom != IDC_DLG_QUERYLIST)
					return 0;

				if (pHdr->code == (DWORD)NM_DBLCLK)
					SendMessage(hWnd, WM_COMMAND, IDM_QUERY_ADD_OLD, 0);

				if (pHdr->code == LVN_KEYDOWN) {
					NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;
					int isOne = ListView_GetSelectedCount(pHdr->hwndFrom) == 1;

					if (kd->wVKey == VK_DELETE)
						SendMessage(hWnd, WM_COMMAND, IDM_QUERY_DELETE, 0);

					if (isOne && kd->wVKey == VK_RETURN)
						SendMessage(hWnd, WM_COMMAND, IDM_QUERY_ADD_OLD, 0);

					if (isOne && kd->wVKey == VK_SPACE)
						SendMessage(hWnd, WM_COMMAND, IDM_QUERY_ADD_NEW, 0);

					if (isOne && kd->wVKey == 0x43) // Ctrl + C
						SendMessage(hWnd, WM_COMMAND, IDM_QUERY_COPY, 0);

					if (kd->wVKey == 0x41 && HIWORD(GetKeyState(VK_CONTROL))) // Ctrl + A
						ListView_SetItemState(pHdr->hwndFrom, -1, LVIS_SELECTED, LVIS_SELECTED);
				}
			}
			break;

			case WM_COMMAND: {
				if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetDlgItem(hWnd, IDC_DLG_QUERYFILTER) && (HWND)lParam == GetFocus()) {
					KillTimer(hWnd, IDT_EDIT_DATA);
					SetTimer(hWnd, IDT_EDIT_DATA, 300, NULL);
					return true;
				}

				if (wParam == IDM_QUERY_DELETE) {
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);

					char sql8[256];
					sprintf(sql8, "delete from %s where query = ?1", GetWindowLongPtr(hWnd, GWLP_USERDATA) == IDM_HISTORY ? "history" : "gists");

					TCHAR query16[MAX_TEXT_LENGTH];
					int pos = -1;
					sqlite3_exec(prefs::db, "begin;", 0, 0, 0);
					while((pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED)) != -1) {
						ListView_GetItemText(hListWnd, pos, 2, query16, MAX_TEXT_LENGTH);

						sqlite3_stmt* stmt;
						if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, sql8, -1, &stmt, 0)) {
							char* query8 = utils::utf16to8(query16);
							sqlite3_bind_text(stmt, 1, query8, strlen(query8), SQLITE_TRANSIENT);
							delete [] query8;
							sqlite3_step(stmt);
						}
						sqlite3_finalize(stmt);

						ListView_DeleteItem(hListWnd, pos);
					}
					sqlite3_exec(prefs::db, "commit;", 0, 0, 0);
				}

				if (wParam == IDM_QUERY_ADD_NEW || wParam == IDM_QUERY_ADD_OLD || wParam == IDM_QUERY_COPY) {
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
					int pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					if (pos == -1)
						return false;

					TCHAR* query16 = new TCHAR[MAX_TEXT_LENGTH] {0};
					ListView_GetItemText(hListWnd, pos, 2, query16, MAX_TEXT_LENGTH);

					if (wParam == IDM_QUERY_ADD_OLD || wParam == IDM_QUERY_ADD_NEW) {
						if (wParam == IDM_QUERY_ADD_NEW)
							PostMessage(hMainWnd, WMU_OPEN_NEW_TAB, 0, 0);

						PostMessage(hMainWnd, WMU_APPEND_TEXT, (WPARAM)query16, 0); // query16 will be free there
						EndDialog(hWnd, DLG_CANCEL);
					} else {
						utils::setClipboardText(query16);
						delete [] query16;
					}

					if (wParam == IDM_QUERY_ADD_OLD || wParam == IDM_QUERY_ADD_NEW)
						EndDialog(hWnd, DLG_OK);
				}
			}
			break;

			case WMU_UPDATE_DATA: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				HWND hFilterWnd = GetDlgItem(hWnd, IDC_DLG_QUERYFILTER);

				int size = GetWindowTextLength(hFilterWnd);
				TCHAR filter16[size + 1]{0};
				GetWindowText(hFilterWnd, filter16, size + 1);

				int idx = GetWindowLongPtr(hWnd, GWLP_USERDATA);
				char* filter8 = utils::utf16to8(filter16);

				char sql8[1024];
				// the third column with replacing to fix issue #160
				sprintf(sql8, "select strftime('%%Y-%%m-%%d %%H:%%M', time, 'unixepoch') Date, query, replace(query, char(10), ' ') Query from %s %s order by time desc limit %i",
					idx == IDM_HISTORY ? "history" : "gists", strlen(filter8) ? "where query like '%' || ?1 || '%'" : "", prefs::get("max-query-count"));

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, sql8, -1, &stmt, 0)) {
					if (strlen(filter8))
						sqlite3_bind_text(stmt, 1, filter8, strlen(filter8), SQLITE_TRANSIENT);
					ListView_SetData(hListWnd, stmt);

					ListView_SetColumnWidth(hListWnd, 2, 0);
				}
				sqlite3_finalize(stmt);
				delete [] filter8;
				SetFocus(GetDlgItem(hWnd, IDC_DLG_QUERYFILTER));
				return true;
			}
			break;

			case WM_CLOSE: {
				DestroyMenu((HMENU)GetProp(hWnd, TEXT("MENU")));
				RemoveProp(hWnd, TEXT("MENU"));
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	bool createValueSelector(HWND hParentWnd, RECT* rc, const TCHAR* value16, const char* schema8, const char* tablename8, const char* column8) {
		HWND hValuesWnd = CreateWindowEx(WS_EX_TOPMOST, WC_COMBOBOX, NULL,
			CBS_DROPDOWN | CBS_HASSTRINGS | CBS_AUTOHSCROLL | WS_VISIBLE | WS_CHILD,
			rc->left, rc->top, rc->right - rc->left, 200,
			hParentWnd, 0, GetModuleHandle(0), NULL);

		SetProp(hValuesWnd, TEXT("ISSAVE"), IntToPtr(1));
		SetProp(hValuesWnd, TEXT("PARENT"), hParentWnd);

		char* query8 = new char[strlen(column8) + strlen(tablename8) + strlen(schema8) + 512];
		sprintf(query8, "select distinct trim(\"%s\") from \"%s\".\"%s\" where typeof(\"%s\") not in ('null', 'blob') and trim(coalesce(\"%s\", '')) <> '' and (?1 is null or \"%s\" like '%%' || ?1 || '%%') order by 1 limit 100",
					column8, schema8, tablename8, column8, column8, column8);
		SetProp(hValuesWnd, TEXT("QUERY8"), query8);

		SetProp(hValuesWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hValuesWnd, GWLP_WNDPROC, (LONG_PTR)cbNewEditDataCombobox));
		SendMessage(hValuesWnd, WM_SETFONT, (LPARAM)hFont, TRUE);

		COMBOBOXINFO ci{0};
		ci.cbSize = sizeof(COMBOBOXINFO);
		GetComboBoxInfo(hValuesWnd, &ci);
		SetProp(ci.hwndItem, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(ci.hwndItem, GWLP_WNDPROC, (LONG_PTR)cbNewEditDataComboboxEdit));

		if (value16)
			ComboBox_SetText(hValuesWnd, value16);

		SendMessage(hValuesWnd, WMU_UPDATE_DATA, 0, 0);

		if (ComboBox_GetCount(hValuesWnd) == 1) {
			DestroyWindow(hValuesWnd);
			return false;
		}

		SetFocus(hValuesWnd);

		return true;
	}

	LRESULT CALLBACK cbNewEditDataCombobox(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_GETDLGCODE: {
				return (DLGC_WANTALLKEYS | CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam));
			}
			break;

			case WM_DESTROY: {
				SendMessage(hWnd, CB_SHOWDROPDOWN, 0, 0);
				if (GetProp(hWnd, TEXT("ISSAVE"))) {
					int len = GetWindowTextLength(hWnd);
					TCHAR value16[len + 1]{0};
					GetWindowText(hWnd, value16, len + 1);
					SendMessage((HWND)GetProp(hWnd, TEXT("PARENT")), WMU_SET_VALUE, (LPARAM)value16, 0);
				}

				char* query8 = (char*)GetProp(hWnd, TEXT("QUERY8"));
				delete [] query8;

				RemoveProp(hWnd, TEXT("ISSAVE"));
				RemoveProp(hWnd, TEXT("PARENT"));
				RemoveProp(hWnd, TEXT("QUERY8"));
				RemoveProp(hWnd, TEXT("WNDPROC"));
			}
			break;

			case WM_KILLFOCUS: {
				if (GetParent((HWND)wParam) != hWnd) {
					DestroyWindow(hWnd);
					return true;
				}
			}
			break;

			case WM_KEYDOWN: {
				if (wParam == VK_RETURN) {
					DestroyWindow(hWnd);
					return true;
				}

				if (wParam == VK_ESCAPE) {
					RemoveProp(hWnd, TEXT("ISSAVE"));
					DestroyWindow(hWnd);
					return true;
				}

				if (wParam == 0x41 && HIWORD(GetKeyState(VK_CONTROL))) { // Ctrl + A
					SendMessage(hWnd, CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
					return true;
				}
			}
			break;

			case WM_COMMAND: {
				if (HIWORD(wParam) == EN_CHANGE) {
					SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
					return 0;
				}
			}
			break;

			case WMU_UPDATE_DATA: {
				char* query8 = (char*)GetProp(hWnd, TEXT("QUERY8"));

				TCHAR value16[256]{0};
				ComboBox_GetText(hWnd, value16, 255);

				SetWindowRedraw(hWnd, FALSE);
				ComboBox_ShowDropdown(hWnd, FALSE);

				int pos = 0;
				SendMessage(hWnd, CB_GETEDITSEL, (WPARAM)&pos, 0);
				if (wParam == 0)
					pos = _tcslen(value16);
				ComboBox_ResetContent(hWnd);

				ComboBox_AddString(hWnd, TEXT(""));
				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
					if (_tcslen(value16) > 0) {
						char* value8 = utils::utf16to8(value16);
						sqlite3_bind_text(stmt, 1, value8, strlen(value8), SQLITE_TRANSIENT);
						delete [] value8;
					}

					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* value16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
						ComboBox_AddString(hWnd, value16);
						delete [] value16;
					}
				}
				sqlite3_finalize(stmt);

				ComboBox_ShowDropdown(hWnd, TRUE);
				SetWindowText(hWnd, value16);
				SendMessage(hWnd, CB_SETEDITSEL, 0, MAKELPARAM(pos, pos));

				SetWindowRedraw(hWnd, TRUE);
				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;
		}

		return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK cbNewEditDataComboboxEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_GETDLGCODE)
				return (DLGC_WANTALLKEYS | CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam));

		if (msg == WM_KEYDOWN && (wParam == VK_RETURN || wParam == VK_ESCAPE || (wParam == 0x41 && HIWORD(GetKeyState(VK_CONTROL))))) {
			SendMessage(GetParent(hWnd), msg, wParam, lParam);
			return 0;
		}

		if (msg == WM_KILLFOCUS) {
			SendMessage(GetParent(hWnd), msg, wParam, lParam);
			return 0;
		}

		if (msg == WM_KEYUP) {
			if (wParam == VK_DOWN || wParam == VK_UP) {
				COMBOBOXINFO ci{0};
				ci.cbSize = sizeof(COMBOBOXINFO);
				GetComboBoxInfo(GetParent(hWnd), &ci);
				SetFocus(ci.hwndList);
				return 0;
			}
		}

		return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
	}

	#define SB_EDITADATA_ROW_COUNT  0
	#define SB_EDITADATA_TRIGGERS   1
	#define SB_EDITADATA_INDEXES    2
	#define SB_EDITADATA_SIZE       3

	// lParam - table16
	BOOL CALLBACK cbDlgEditData (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, DWLP_USER, IDD_EDITDATA);
				TCHAR* schema16 = utils::getTableName((TCHAR*)lParam, true);
				TCHAR* tablename16 = utils::getTableName((TCHAR*)lParam);

				char* tablename8 = utils::utf16to8(tablename16);
				SetProp(hWnd, TEXT("TABLENAME8"), (HANDLE)tablename8);

				char* schema8 = utils::utf16to8(schema16);
				SetProp(hWnd, TEXT("SCHEMA8"), (HANDLE)schema8);

				delete [] tablename16;
				delete [] schema16;

				char query8[MAX_TEXT_LENGTH]{0};

				sqlite3_stmt *stmt;
				bool isTable = false;
				sprintf(query8, "select lower(type) = 'table' from \"%s\".sqlite_master where tbl_name = ?1 and type in ('view', 'table')", schema8);
				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
					sqlite3_bind_text(stmt, 1, tablename8, strlen(tablename8), SQLITE_TRANSIENT);
					if (SQLITE_ROW == sqlite3_step(stmt))
						isTable = sqlite3_column_int(stmt, 0);
				}
				sqlite3_finalize(stmt);
				SetProp(hWnd, TEXT("ISTABLE"), IntToPtr(+isTable));

				bool hasRowid = false;
				if (isTable) {
					char* qschema8 = utils::double_quote(schema8);
					char* qtablename8 = utils::double_quote(tablename8);
					sprintf(query8, "select rowid from %s.%s limit 1", qschema8, qtablename8);
					delete [] qschema8;
					delete [] qtablename8;
					hasRowid = SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0);
					sqlite3_finalize(stmt);
				}
				SetProp(hWnd, TEXT("HASROWID"), IntToPtr(hasRowid));

				if (!hasRowid) {
					sprintf(query8,
						"select group_concat(dq(name), ','), " \
						"'md5(coalesce(' || group_concat(dq(name), ', \"~~~\") || ''***'' || coalesce(') || ', \"~~~\"))', " \
						"count(1) "
						"from pragma_table_info(?2) where %s and schema = ?1 order by pk ", isTable ? "pk > 0" : "1=1");
					if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
						sqlite3_bind_text(stmt, 1, schema8, strlen(schema8), SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt, 2, tablename8, strlen(tablename8), SQLITE_TRANSIENT);
						if (SQLITE_ROW == sqlite3_step(stmt)) {
							SetProp(hWnd, TEXT("KEYS8"), strdup((char*)sqlite3_column_text(stmt, 0)));
							SetProp(hWnd, TEXT("MD5KEYS8"), strdup((char*)sqlite3_column_text(stmt, 1)));
							SetProp(hWnd, TEXT("KEYCOUNT"), IntToPtr(sqlite3_column_int(stmt, 2)));
						}
					} else {
						sqlite3_finalize(stmt);
						showDbError(hWnd);
						SendMessage(hWnd, WM_CLOSE, 0, 0);
						return 0;
					}
					sqlite3_finalize(stmt);
				}

				bool isReadOnly = sqlite3_db_readonly(db, 0);
				bool canInsert = !isReadOnly;
				bool canUpdate = !isReadOnly;
				bool canDelete = !isReadOnly;

				// View with "instead of"-triggers
				if (!isTable && !isReadOnly) {
					sprintf(query8, "select sum(instr(lower(sql), 'instead of insert')), sum(instr(lower(sql), 'instead of update')), " \
						"sum(instr(lower(sql), 'instead of delete')) from \"%s\".sqlite_master where tbl_name = '%s' and type = 'trigger'",
						schema8, tablename8);

					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
						sqlite3_step(stmt);
						canInsert = sqlite3_column_int(stmt, 0) > 0;
						canUpdate = sqlite3_column_int(stmt, 1) > 0;
						canDelete = sqlite3_column_int(stmt, 2) > 0;
					}
					sqlite3_finalize(stmt);
				}
				SetProp(hWnd, TEXT("CANINSERT"), (HANDLE)canInsert);
				SetProp(hWnd, TEXT("CANUPDATE"), (HANDLE)canUpdate);
				SetProp(hWnd, TEXT("CANDELETE"), (HANDLE)canDelete);

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				HWND hHeader = ListView_GetHeader(hListWnd);
				LONG_PTR styles = GetWindowLongPtr(hHeader, GWL_STYLE);
				SetWindowLongPtr(hHeader, GWL_STYLE, styles | HDS_FILTERBAR);
				CreateWindow(WC_LISTBOX, NULL, WS_CHILD, 300, 0, 400, 100, hListWnd, (HMENU)IDC_REFLIST, GetModuleHandle(0), 0);
				SendMessage(hListWnd, WM_SETFONT, (WPARAM)hFont, false);

				SetProp(hWnd, TEXT("CURRENTROW"), (HANDLE)0);
				SetProp(hWnd, TEXT("CURRENTCOLUMN"), (HANDLE)0);

				DWORD tStart = GetTickCount();
				SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
				SetProp(hWnd, TEXT("FORCEUPDATE"), (HANDLE)(!isTable && (GetTickCount() - tStart < 300)));

				int btnCount = 0;
				TBBUTTON tbButtons [10]{0};

				if (TRUE) {
					tbButtons[btnCount] = {2, IDM_ROW_REFRESH, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)TEXT("Refresh")};
					btnCount++;
				}
				if (canInsert) {
					tbButtons[btnCount] = {0, IDM_ROW_ADD, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)TEXT("Add")};
					btnCount++;
				}
				if (canDelete) {
					tbButtons[btnCount] = {1, IDM_ROW_DELETE, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0, (INT_PTR)TEXT("Delete")};
					btnCount++;
				}

				if (TRUE) {
					tbButtons[btnCount] = {-1, IDM_LAST_SEPARATOR, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, 0};
					btnCount++;
				}
				if (TRUE) {
					tbButtons[btnCount] = {-1, IDM_FILTER_TYPE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, !prefs::get("edit-data-filter-mode"), 0};
					btnCount++;
				}

				HWND hToolbarWnd = CreateToolbarEx (hWnd, WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_LIST, IDC_DLG_TOOLBAR, 0, NULL, 0,
					tbButtons, btnCount, 0, 0, 0, 0, sizeof (TBBUTTON));

				int idc = GetSystemMetrics(SM_CXSMICON) <= 16 ? IDB_TOOLBAR_DATA16 : IDB_TOOLBAR_DATA24;
				SendMessage(hToolbarWnd, TB_SETIMAGELIST, 0, (LPARAM)ImageList_LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(idc), 0, 0, RGB(255, 255, 255)));

				HFONT hToolbarFont = (HFONT)SendMessage(hToolbarWnd, WM_GETFONT, 0, 0);

				TBBUTTONINFO tbi{0};
				tbi.cbSize = sizeof(TBBUTTONINFO);
				tbi.dwMask = TBIF_SIZE;
				tbi.cx = utils::getTextSize(hToolbarFont, TEXT("Where")).cx + (idc == IDB_TOOLBAR_DATA16 ? 16 : 24) + 2 * LOWORD(SendMessage(hToolbarWnd, TB_GETPADDING, 0, 0));
				SendMessage(hToolbarWnd, TB_SETBUTTONINFO, IDM_FILTER_TYPE, (LPARAM)&tbi);
				PostMessage(hWnd, WM_COMMAND, IDM_FILTER_TYPE, 0);

				HWND hFilterWnd = CreateWindowEx(0L, WC_EDIT, NULL, WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 0, 0, 0, 0, hToolbarWnd, (HMENU) IDC_DLG_FILTER, GetModuleHandle(0), 0);
				SendMessage(hFilterWnd, WM_SETFONT, (LPARAM)hToolbarFont, true);
				cbOldHeaderEdit = (WNDPROC)SetWindowLongPtr(hFilterWnd, GWLP_WNDPROC, (LONG_PTR)cbNewFilterEdit);

				int colCount = Header_GetItemCount(hHeader);
				for (int i = 0; i < colCount; i++) {
					RECT rc;
					Header_GetItemRect(hHeader, i, &rc);
					HWND hEdit = CreateWindowEx(WS_EX_TOPMOST, WC_EDIT, NULL, ES_CENTER | ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD | WS_BORDER, 0, 0, 0, 0, hHeader, (HMENU)IntToPtr(IDC_HEADER_EDIT + i), GetModuleHandle(0), NULL);
					SendMessage(hEdit, WM_SETFONT, (LPARAM)hFont, true);
					cbOldHeaderEdit = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)cbNewFilterEdit);
				}

				// If view is invalid then colCount = 0
				if (colCount > 0) {
					int* widths = new int[colCount]{0};
					for (int i = 0; i < colCount; i++)
						widths[i] = ListView_GetColumnWidth(hListWnd, i);
					SetProp(hWnd, TEXT("WIDTHS"), (HANDLE)widths);

					bool* generated = new bool[colCount + 1]{0};
					if (SQLITE_OK == sqlite3_prepare_v2(db, "select instr(lower(type), 'generated always') > 0 from pragma_table_xinfo where schema = ?1 and arg = ?2 order by cid", -1, &stmt, 0)) {
						sqlite3_bind_text(stmt, 1, schema8, -1, SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt, 2, tablename8, -1, SQLITE_TRANSIENT);
						for (int i = 0; i < colCount; i++) {
							sqlite3_step(stmt);
							generated[i + 1] = sqlite3_column_int(stmt, 0); // first column is the row length, the last is a rowid
						}
					}
					sqlite3_finalize(stmt);
					SetProp(hWnd, TEXT("GENERATED"), (HANDLE)generated);

					BYTE* decltypes = new BYTE[colCount + 1]{0};
					char query8[255 + strlen(schema8) + strlen(tablename8)];
					sprintf(query8, "select * from \"%s\".\"%s\" where 1 = 2", schema8, tablename8);
					if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
						sqlite3_step(stmt);
						for (int colNo = 0; colNo < sqlite3_column_count(stmt); colNo++)
							decltypes[colNo + 1] = dbutils::sqlite3_type(sqlite3_column_decltype(stmt, colNo));
					}
					sqlite3_finalize(stmt);
					SetProp(hWnd, TEXT("DECLTYPES"), (HANDLE)decltypes);

					char** fkSelects = new char*[colCount + 1]{0};
					int* fkColumns = new int[colCount + 1]{0};
					if (SQLITE_OK == sqlite3_prepare_v2(db,
						"with i as (select * from pragma_table_xinfo where schema = ?1 and arg = ?2), " \
						"fk as (select * from pragma_foreign_key_list where schema = ?1 and arg = ?2) " \
						"select fk.\"table\" is not null, 'select * from ' || ?1 || '.' || quote(fk.\"table\"), " \
						"(select cid from pragma_table_xinfo where schema = ?1 and arg = fk.\"table\" and name = fk.\"to\") cid " \
						"from i left join fk on i.name = fk.\"from\" order by i.cid", -1, &stmt, 0)) {
						sqlite3_bind_text(stmt, 1, schema8, -1, SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt, 2, tablename8, -1, SQLITE_TRANSIENT);
						int colNo = 0;
						while (SQLITE_ROW == sqlite3_step(stmt) && (colNo < colCount)) {
							if (sqlite3_column_int(stmt, 0) == 1) {
								const char* select8 = (const char*)sqlite3_column_text(stmt, 1);
								fkSelects[colNo + 1] = new char[strlen(select8) + 1] {0};
								strcpy(fkSelects[colNo + 1], select8);

								fkColumns[colNo + 1] = sqlite3_column_int(stmt, 2);
							}

							colNo++;
						}
					}
					sqlite3_finalize(stmt);
					SetProp(hWnd, TEXT("FKSELECTS"), (HANDLE)fkSelects);
					SetProp(hWnd, TEXT("FKCOLUMNS"), (HANDLE)fkColumns);
				}

				HWND hStatusWnd = CreateStatusWindow(WS_CHILD | WS_VISIBLE, NULL, hWnd, IDC_DLG_STATUSBAR);
				float z = utils::getWndScale(hMainWnd).x;
				int sizes[5] = {(int)(z * 100), (int)(z * 200), (int)(z * 300), (int)(z * 400), -1};
				SendMessage(hStatusWnd, SB_SETPARTS, 5, (LPARAM)&sizes);

				SetProp(hWnd, TEXT("TRIGGERMENU"), (HANDLE)CreatePopupMenu());
				SetProp(hWnd, TEXT("INDEXMENU"), (HANDLE)CreatePopupMenu());
				SendMessage(hWnd, WMU_UPDATE_META, 0, 0);

				RECT rc;
				Header_GetItemRect(hHeader, colCount - 1, &rc);
				int w = MAX(rc.right + GetSystemMetrics(SM_CXVSCROLL) + 2 * GetSystemMetrics(SM_CXSIZEFRAME) + 2 * GetSystemMetrics(SM_CXEDGE), 300);
				int h = 0;

				int rowCount = ListView_GetItemCount(hListWnd);
				if (rowCount > 0) {
					GetWindowRect(hToolbarWnd, &rc);
					h += rc.bottom - rc.top;

					ListView_GetItemRect(hListWnd, 0, &rc, LVIR_BOUNDS);
					h += rc.top + (rc.bottom - rc.top + 1) * (rowCount + 3);
				}

				SetWindowPos(hWnd, 0, 0, 0, w, MAX(h, 300), SWP_NOZORDER | SWP_NOMOVE);
				utils::alignDialog(hWnd, hMainWnd, true);

				ListView_SetItemState(hListWnd, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
				if (!hMainWnd)
					createTooltip(hWnd);
			}
			break;

			case WM_SIZE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
				HWND hStatusWnd = GetDlgItem(hWnd, IDC_DLG_STATUSBAR);

				SendMessage(hToolbarWnd, WM_SIZE, 0, 0);
				SendMessage(hStatusWnd, WM_SIZE, 0, 0);

				RECT wrc, trc, src, brc;
				GetClientRect(hWnd, &wrc);
				GetClientRect(hToolbarWnd, &trc);
				GetClientRect(hStatusWnd, &src);
				SetWindowPos(hListWnd, 0, 0, trc.bottom + 2, wrc.right - wrc.left, wrc.bottom - wrc.top - trc.bottom - src.bottom - 2, SWP_NOZORDER);

				HWND hFilterWnd = GetDlgItem(hToolbarWnd, IDC_DLG_FILTER);
				SendMessage(hToolbarWnd, TB_GETRECT, IDM_FILTER_TYPE, (LPARAM)&brc);
				SetWindowPos(hFilterWnd, 0, brc.right + 2, 2, trc.right - brc.right - 4, brc.bottom - 2, SWP_NOZORDER);
			}
			break;

			case WM_SYSCOMMAND: {
				return wParam != VK_MENU && (lParam >> 16) <= 0; // https://stackoverflow.com/a/9627980/6121703
			}
			break;

			case WMU_UPDATE_DATA: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				HWND hHeader = ListView_GetHeader(hListWnd);
				bool isTable = GetProp(hWnd, TEXT("ISTABLE"));
				bool hasRowid = GetProp(hWnd, TEXT("HASROWID"));
				HWND hFilterWnd = GetDlgItem(GetDlgItem(hWnd, IDC_DLG_TOOLBAR),IDC_DLG_FILTER);

				int size = GetWindowTextLength(hFilterWnd);
				TCHAR filter16[size + 1]{0};
				GetWindowText(hFilterWnd, filter16, size + 1);

				SetWindowText(hWnd, TEXT("Fetching data..."));

				char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
				char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
				char* md5keys = (char*)GetProp(hWnd, TEXT("MD5KEYS8"));

				bool isFilter = Toolbar_GetButtonData(GetDlgItem(hWnd, IDC_DLG_TOOLBAR), IDM_FILTER_TYPE) && _tcslen(filter16) > 0;

				TCHAR where16[MAX_TEXT_LENGTH]{0};
				_tcscat(where16, TEXT("where ("));
				if (isFilter) {
					_tcscat(where16, TEXT("0")); // where (0 or col1 = ... or ...);
					for (int colNo = 1; colNo < Header_GetItemCount(hHeader); colNo++) {
						TCHAR colname16[256]{0};
						Header_GetItemText(hHeader, colNo, colname16, 255);
						_tcscat(where16, TEXT(" or \""));
						_tcscat(where16, colname16);

						_tcscat(where16,
							filter16[0] == TEXT('=') ? TEXT("\" = :filter ") :
							filter16[0] == TEXT('/') ? TEXT("\" regexp :filter ") :
							filter16[0] == TEXT('!') ? TEXT("\" not like '%' || :filter || '%' ") :
							TEXT("\" like '%' || :filter || '%' ")
						);
					}
				} else {
					_tcscat(where16, _tcslen(filter16) ? filter16 : TEXT("1 = 1"));
				}
				_tcscat(where16, TEXT(") "));

				for (int colNo = 1; colNo < Header_GetItemCount(hHeader); colNo++) {
					HWND hEdit = GetDlgItem(hHeader, IDC_HEADER_EDIT + colNo);
					if (GetWindowTextLength(hEdit) > 0) {
						TCHAR colname16[256]{0};
						Header_GetItemText(hHeader, colNo, colname16, 255);
						_tcscat(where16, TEXT(" and "));
						TCHAR* qcolname16 = utils::double_quote(colname16);
						_tcscat(where16, qcolname16);
						delete [] qcolname16;

						TCHAR buf16[2]{0};
						GetWindowText(hEdit, buf16, 2);
						_tcscat(where16,
							buf16[0] == TEXT('=') ? TEXT(" = ? ") :
							buf16[0] == TEXT('/') ? TEXT(" regexp ? ") :
							buf16[0] == TEXT('!') ? TEXT(" not like '%' || ? || '%' ") :
							buf16[0] == TEXT('>') ? TEXT(" > ? ") :
							buf16[0] == TEXT('<') ? TEXT(" < ? ") :
							TEXT(" like '%' || ? || '%' ")
						);
					}
				}
				char* where8 = utils::utf16to8(where16);

				char query8[MAX_TEXT_LENGTH]{0};
				char* qschema8 = utils::double_quote(schema8);
				char* qtablename8 = utils::double_quote(tablename8);
				sprintf(query8, "select *, %s rowid from %s.%s t %s", hasRowid ? "rowid" : md5keys, qschema8, qtablename8, where8 && strlen(where8) ? where8 : "");
				delete [] qschema8;
				delete [] qtablename8;

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {

					int bindNo = 0;
					if (isFilter) {
						char* value8 = utils::utf16to8(filter16[0] == TEXT('=') || filter16[0] == TEXT('/') || filter16[0] == TEXT('!') ? filter16 + 1 : filter16);
						dbutils::bind_variant(stmt, bindNo + 1, value8);
						delete [] value8;
						bindNo++;
					}

					int colCount = sqlite3_column_count(stmt);
					for (int colNo = 1; (colNo < colCount) && strlen(where8); colNo++) {
						HWND hEdit = GetDlgItem(hHeader, IDC_HEADER_EDIT + colNo);
						int size = GetWindowTextLength(hEdit);
						if (size > 0) {
							TCHAR value16[size + 1]{0};
							GetWindowText(hEdit, value16, size + 1);
							char* value8 = utils::utf16to8(value16[0] == TEXT('=') || value16[0] == TEXT('/') || value16[0] == TEXT('!') || value16[0] == TEXT('<') || value16[0] == TEXT('>') ? value16 + 1 : value16);
							dbutils::bind_variant(stmt, bindNo + 1, value8);
							delete [] value8;
							bindNo++;
						}
					}

					ShowWindow(hListWnd, SW_HIDE);
					int rowCount = ListView_SetData(hListWnd, stmt, true);
					ListView_SetColumnWidth(hListWnd, colCount, 0); // last column is rowid

					int* widths = (int*)GetProp(hWnd, TEXT("WIDTHS"));
					for (int i = 0; i < colCount && widths; i++)
						ListView_SetColumnWidth(hListWnd, i, widths[i]);
					ShowWindow(hListWnd, SW_SHOW);

					int len = strlen(tablename8) + strlen(schema8) + 255;
					TCHAR buf[len + 1];
					TCHAR* schema16 = utils::utf8to16(schema8);
					TCHAR* tablename16 = utils::utf8to16(tablename8);
					TCHAR* name16 = utils::getFullTableName(schema16, tablename16);
					_tcsupr(name16);
					_sntprintf(buf, len, TEXT("%ls %ls [%ls%i rows]"), isTable ? TEXT("Table") : TEXT("View"), _tcscmp(schema16, TEXT("main")) == 0 ? tablename16 : name16, rowCount < 0 ? TEXT("Show only first ") : TEXT(""), abs(rowCount));
					delete [] schema16;
					delete [] tablename16;
					delete [] name16;
					SetWindowText(hWnd, buf);
				} else {
					showDbError(hWnd);
				}
				sqlite3_finalize(stmt);

				int currRow = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
				int currCol = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN"));

				ListView_SetItemState(hListWnd, currRow, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				PostMessage(hWnd, WMU_SET_CURRENT_CELL, currRow, currCol);

				delete [] where8;
				PostMessage(hWnd, WMU_UPDATE_COLSIZE, 0, 0);
				InvalidateRect(hHeader, NULL, true);
				return true;
			}
			break;

			case WMU_UPDATE_META: {
				HWND hStatusWnd = GetDlgItem(hWnd, IDC_DLG_STATUSBAR);
				bool isTable = GetProp(hWnd, TEXT("ISTABLE"));
				char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
				char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
				char* qschema8 = utils::double_quote(schema8);
				char* qtablename8 = utils::double_quote(tablename8);

				for (int menuNo = 0; menuNo < 2; menuNo++) {
					HMENU hMenu = (HMENU)GetProp(hWnd, menuNo == 0 ? TEXT("TRIGGERMENU") : TEXT("INDEXMENU"));
					for (int i = 0; i < GetMenuItemCount(hMenu); i++)
						RemoveMenu(hMenu, 0, MF_BYPOSITION);

					sqlite3_stmt *stmt;
					char query8[2048];
					snprintf(query8, 2047, "select name from %s.sqlite_master where type = '%s' and tbl_name = ?1 and sql is not null order by 1", qschema8, menuNo == 0 ? "trigger" : "index");
					if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
						int itemNo = 0;
						sqlite3_bind_text(stmt, 1, tablename8, -1, SQLITE_TRANSIENT);
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							TCHAR* name16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
							AppendMenu(hMenu, MF_STRING, (menuNo == 0 ? IDM_MENU_TRIGGER : IDM_MENU_INDEX) + itemNo, name16);
							delete [] name16;
							itemNo++;
						}

						TCHAR count16[32];
						_sntprintf(count16, 31, menuNo == 0 ? TEXT(" TRIGGERS: %i") : TEXT(" INDEXES: %i"), itemNo);
						SendMessage(hStatusWnd, SB_SETTEXT, menuNo == 0 ? SB_EDITADATA_TRIGGERS : SB_EDITADATA_INDEXES, (LPARAM)count16);
					}
					sqlite3_finalize(stmt);
				}

				sqlite3_stmt *stmt;
				char query8[2048];
				snprintf(query8, 2047, "select count(1) from %s.%s", qschema8, qtablename8);

				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
					sqlite3_step(stmt);
					TCHAR count16[32];
					_sntprintf(count16, 31, TEXT(" ROWS: %i"), sqlite3_column_int(stmt, 0));
					SendMessage(hStatusWnd, SB_SETTEXT, SB_EDITADATA_ROW_COUNT, (LPARAM)count16);
				}
				sqlite3_finalize(stmt);

				if (isTable) {
					sqlite3_stmt *stmt;
					char query8[2048];
					snprintf(query8, 2047, "select ' SIZE: ' || tosize(SUM(pgsize)) total from dbstat where schema = ?1 and name = ?2");
					if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
						sqlite3_bind_text(stmt, 1, schema8, -1, SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt, 2, tablename8, -1, SQLITE_TRANSIENT);

						sqlite3_step(stmt);

						TCHAR* size16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
						SendMessage(hStatusWnd, SB_SETTEXT, SB_EDITADATA_SIZE, (LPARAM)size16);
						delete [] size16;
					}
					sqlite3_finalize(stmt);
				} else {
					SendMessage(hStatusWnd, SB_SETTEXT, SB_EDITADATA_SIZE, 0);
				}

				delete [] qschema8;
				delete [] qtablename8;
			}
			break;

			case WMU_UPDATE_COLSIZE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				HWND hHeader = ListView_GetHeader(hListWnd);
				int colCount = Header_GetItemCount(hHeader);
				SendMessage(hHeader, WM_SIZE, 0, 0);
				for (int colNo = 0; colNo < colCount; colNo++) {
					RECT rc;
					Header_GetItemRect(hHeader, colNo, &rc);
					int h2 = round((rc.bottom - rc.top) / 2);
					SetWindowPos(GetDlgItem(hHeader, IDC_HEADER_EDIT + colNo), 0, rc.left, h2, rc.right - rc.left, h2 + 1, SWP_NOZORDER);
				}
			}
			break;

			case WMU_SET_CURRENT_CELL: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				RECT rc, rc2;

				// Reset previous position
				int currRow = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
				int currCol = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN"));
				ListView_GetSubItemRect(hListWnd, currRow, currCol, LVIR_BOUNDS, &rc);
				InvalidateRect(hListWnd, &rc, false);

				currRow = wParam;
				currCol = lParam;

				SetProp(hWnd, TEXT("CURRENTROW"), IntToPtr(currRow));
				SetProp(hWnd, TEXT("CURRENTCOLUMN"), IntToPtr(currCol));

				ListView_GetSubItemRect(hListWnd, currRow, currCol, LVIR_BOUNDS, &rc);
				GetClientRect(hListWnd, &rc2);
				int w = rc.right - rc.left;
				int dx = rc2.right < rc.right ? rc.left - rc2.right + w : rc.left < 0 ? rc.left : 0;
				ListView_Scroll(hListWnd, currCol > 0 ? dx : 0, 0);

				ListView_EnsureVisible(hListWnd, currRow, FALSE);

				InvalidateRect(hListWnd, &rc, false);
			}
			break;

			case WMU_SYNC_CURRENT_CELL: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				int currCol = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN"));
				int rowNo = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
				SendMessage(hWnd, WMU_SET_CURRENT_CELL, rowNo, currCol);
			}
			break;

			// wParam = data8, lParam = dataSize
			case WMU_SET_CURRENT_CELL_BLOB:
			// wParam = text8, lParam = SQLITE_TYPE or 0 (auto for text, integer, float and null)
			case WMU_SET_CURRENT_CELL_VALUE: {
				int rowNo = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
				int colNo = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN"));
				char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
				char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
				char* md5keys8 = (char*)GetProp(hWnd, TEXT("MD5KEYS8"));
				bool hasRowid = GetProp(hWnd, TEXT("HASROWID"));
				byte* decltypes = (byte*)GetProp(hWnd, TEXT("DECLTYPES"));

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				HWND hHeader = (HWND)ListView_GetHeader(hListWnd);
				int colCount = Header_GetItemCount(hHeader);
				TCHAR column16[256]{0};
				Header_GetItemText(hHeader, colNo, column16, 255);

				TCHAR*** cache = (TCHAR***)GetProp(hListWnd, TEXT("CACHE"));
				int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
				byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
				unsigned char** blobs = (unsigned char**)GetProp(hListWnd, TEXT("BLOBS"));

				int _rowNo = resultset[rowNo];
				TCHAR* rowid16 = cache[_rowNo][colCount - 1];
				int idx = colNo + (colCount - 1) * _rowNo;

				char* column8 = utils::utf16to8(column16);
				const char* value8 = (const char*)wParam;
				const unsigned char* data8 = (const unsigned char*)wParam;
				char* rowid8 = utils::utf16to8(rowid16);
				byte isBlob = msg == WMU_SET_CURRENT_CELL_BLOB;
				int dataSize = isBlob ? lParam : 0;

				char query8[256 + 2 * strlen(schema8) + strlen(tablename8) + strlen(column8) + (hasRowid ? 0 : strlen(md5keys8))];
				sprintf(query8, "update \"%s\".\"%s\" set \"%s\" = ?1 where %s = ?2", schema8, tablename8, column8, hasRowid ? "rowid" : md5keys8);

				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
					sqlite3_bind_text(stmt, 2, rowid8, -1, SQLITE_TRANSIENT);

					BYTE type = 0;
					if (isBlob) {
						type = SQLITE_BLOB;
						sqlite3_bind_blob(stmt, 1, data8, dataSize, SQLITE_TRANSIENT);
					} else {
						type = dbutils::detectSqliteType(value8, decltypes && decltypes[colNo - 1] == SQLITE_TEXT);
						dbutils::bind_variant(stmt, 1, value8, decltypes && decltypes[colNo - 1] == SQLITE_TEXT);
					}

					if (SQLITE_DONE == sqlite3_step(stmt)) {
						datatypes[idx] = type;

						if (cache[_rowNo][colNo])
							free(cache[_rowNo][colNo]);

						if (blobs[idx])
							delete [] blobs[idx];

						if (isBlob) {
							cache[_rowNo][colNo] = utils::toBlobSize(dataSize);
							blobs[idx] = utils::toBlob(dataSize, data8);
						} else {
							cache[_rowNo][colNo] = utils::utf8to16(value8);
							blobs[idx] = 0;
						}
					}

					ListView_RedrawItems(hListWnd, rowNo, rowNo);
				} else {
					showDbError(hWnd);
				}
				sqlite3_finalize(stmt);

				delete [] column8;
				delete [] rowid8;

				if (GetProp(hWnd, TEXT("FORCEUPDATE")))
					SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
			}
			break;

			// wParam = 0/1 - store a previous text or not
			case WMU_EDIT_VALUE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				bool withText = wParam != 0;
				int currRow = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
				int currCol = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN"));

				bool* generated = (bool*)GetProp(GetParent(hListWnd), TEXT("GENERATED"));
				bool isGenerated = (generated != NULL) && (generated[currCol] != 0);
				if (isGenerated)
					return MessageBeep(0);

				RECT rect;
				ListView_GetSubItemRect(hListWnd, currRow, currCol, LVIR_BOUNDS, &rect);
				int h = rect.bottom - rect.top;
				int w = ListView_GetColumnWidth(hListWnd, currCol);

				TCHAR* value16 = new TCHAR[MAX_TEXT_LENGTH + 1];
				ListView_GetItemText(hListWnd, currRow, currCol, value16, MAX_TEXT_LENGTH);

				if (_tcsstr(value16, TEXT("(BLOB:")) == value16 || currCol < 1) {
					delete [] value16;
					return true;
				}

				HWND hEdit = CreateWindowEx(0, WC_EDIT, withText ? value16 : NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, rect.left, rect.top, w, h, hListWnd, (HMENU)IDC_DLG_EDIT_VALUE, GetModuleHandle(NULL), NULL);
				int end = GetWindowTextLength(hEdit);
				SendMessage(hEdit, EM_SETSEL, end, end);
				SendMessage(hEdit, WM_SETFONT, (LPARAM)hFont, true);
				cbOldEditDataEdit = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)cbNewEditDataEdit);
				SetFocus(hEdit);
				delete [] value16;

				if (lParam && (lParam != VK_SPACE))
					keybd_event(lParam, 0, 0, 0);
			}
			break;

			// wParam - init text or NULL
			case WMU_CREATE_VALUE_SELECTOR: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				int currRow = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
				int currCol = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN"));

				RECT rc;
				ListView_GetSubItemRect(hListWnd, currRow, currCol, LVIR_BOUNDS, &rc);
				POINT p = {rc.left, rc.top};
				ClientToScreen(hListWnd, &p);
				ScreenToClient(hWnd, &p);
				rc = {p.x, p.y, p.x + ListView_GetColumnWidth(hListWnd, currCol), 200};

				char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
				char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));

				TCHAR column16[256]{0};
				Header_GetItemText(ListView_GetHeader(hListWnd), currCol, column16, 255);
				char* column8 = utils::utf16to8(column16);

				TCHAR value16[1024]{0};
				if (wParam) {
					_sntprintf(value16, 1023, TEXT("%ls"), (TCHAR*)wParam);
				} else {
					ListView_GetItemText(hListWnd, currRow, currCol, value16, 1023);
				}

				SetProp(hWnd, TEXT("VALUEROW"), IntToPtr(currRow));
				SetProp(hWnd, TEXT("VALUECOLUMN"), IntToPtr(currCol));

				createValueSelector(hWnd, &rc, value16, schema8, tablename8, column8);
				delete [] column8;
			}
			break;

			case WMU_OPEN_FK_VALUE_SELECTOR: {
				DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_FK_SELECTOR), hWnd, (DLGPROC)cbDlgFKSelector, (LPARAM)hWnd);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				bool hasRowid = GetProp(hWnd, TEXT("HASROWID"));
				bool canInsert = GetProp(hWnd, TEXT("CANINSERT"));
				bool canUpdate = GetProp(hWnd, TEXT("CANUPDATE"));
				bool canDelete = GetProp(hWnd, TEXT("CANDELETE"));
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);

				if (pHdr->code == LVN_GETDISPINFO && pHdr->hwndFrom == hListWnd) {
					LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
					LV_ITEM* pItem = &(pDispInfo)->item;
					HWND hListWnd = pHdr->hwndFrom;

					TCHAR*** cache = (TCHAR***)GetProp(hListWnd, TEXT("CACHE"));
					int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
					if(cache && resultset && pItem->mask & LVIF_TEXT) {
						int rowNo = resultset[pItem->iItem];
						pItem->pszText = cache[rowNo][pItem->iSubItem];
					}
				}

				if (pHdr->code == LVN_COLUMNCLICK && pHdr->hwndFrom == hListWnd) {
					NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
					return ListView_Sort(pHdr->hwndFrom, pLV->iSubItem);
				}

				if (pHdr->code == (DWORD)NM_RCLICK && pHdr->hwndFrom == hListWnd) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					SendMessage(hWnd, WMU_SET_CURRENT_CELL, ia->iItem, ia->iSubItem);

					if (ListView_GetSelectedCount(hListWnd) == 0)
						return 0;

					POINT p;
					GetCursorPos(&p);

					HMENU hMenu = !canUpdate ? hViewDataMenu : hEditDataMenu;
					UINT state[2] = {
						MF_BYCOMMAND | MF_DISABLED | MF_GRAYED,
						MF_BYCOMMAND | MF_ENABLED
					};

					HWND hHeader = ListView_GetHeader(hListWnd);
					int colCount = Header_GetItemCount(hHeader);
					BYTE* datatypes = (BYTE*)GetProp(hListWnd, TEXT("DATATYPES"));
					int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
					int _rowNo = resultset[ia->iItem];
					BYTE type = datatypes[ia->iSubItem + (colCount - 1) * _rowNo];

					EnableMenuItem(hMenu, IDM_RESULT_VALUE_FILTER, state[type == SQLITE_INTEGER || type == SQLITE_FLOAT || type == SQLITE_TEXT]);

					if (hMenu == hEditDataMenu) {
						bool* generated = (bool*)GetProp(GetParent(hListWnd), TEXT("GENERATED"));
						bool isGenerated = (generated != NULL) && (generated[ia->iSubItem] != 0);

						EnableMenuItem(hMenu, IDM_VALUE_EDIT, state[!isGenerated && type != SQLITE_BLOB]);
						EnableMenuItem(hMenu, IDM_VALUE_NULL, state[type != SQLITE_NULL]);

						EnableMenuItem(hMenu, IDM_ROW_EDIT, state[canUpdate]);
						EnableMenuItem(hMenu, IDM_ROW_DELETE, state[canDelete]);
						EnableMenuItem(hMenu, IDM_ROW_DUPLICATE, state[hasRowid && canUpdate]);
					}

					TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
				}

				if (pHdr->code == (DWORD)NM_CLICK && HIWORD(GetKeyState(VK_MENU))) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					SendMessage(hWnd, WMU_SET_CURRENT_CELL, ia->iItem, ia->iSubItem);
					return ListView_ShowRef(hListWnd, ia->iItem, ia->iSubItem);
				}

				if (!canUpdate && pHdr->code == (DWORD)NM_DBLCLK && pHdr->hwndFrom == hListWnd) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					if (ia->iItem != -1)
						SendMessage(hWnd, WM_COMMAND, IDM_ROW_EDIT, 0);
				}

				if (pHdr->code == (DWORD)NM_CLICK && pHdr->hwndFrom == hListWnd) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					SendMessage(hWnd, WMU_SET_CURRENT_CELL, ia->iItem, ia->iSubItem);

					if (ia->iItem != -1 && ListView_GetSelectedCount(pHdr->hwndFrom) == 1 && HIWORD(GetKeyState(VK_CONTROL)))
						PostMessage(hWnd, WM_COMMAND, IDM_VALUE_EDIT, 0);

					return true;
				}

				if ((pHdr->hwndFrom == hListWnd) && (pHdr->code == (UINT)NM_CUSTOMDRAW)) {
					NMLVCUSTOMDRAW* pCustomDraw = (LPNMLVCUSTOMDRAW)lParam;

					int result = CDRF_DODEFAULT;
					if (pCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
						result = CDRF_NOTIFYITEMDRAW;

					if (pCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
						pCustomDraw->nmcd.lItemlParam = 0;
						if (ListView_GetItemState(pHdr->hwndFrom, pCustomDraw->nmcd.dwItemSpec, LVIS_SELECTED)) {
							pCustomDraw->nmcd.uItemState &= ~CDIS_SELECTED;
							pCustomDraw->nmcd.lItemlParam = 1;
						}

						result = CDRF_NOTIFYSUBITEMDRAW;
					}

					if (pCustomDraw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
						int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
						byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
						int rowNo = pCustomDraw->nmcd.dwItemSpec;
						int colNo = pCustomDraw->iSubItem;
						int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd)) - 1;

						bool isSelectedRow = pCustomDraw->nmcd.lItemlParam == 1;
						bool isCurrCell = rowNo == (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW")) && colNo == (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN")) && (GetFocus() == hListWnd);

						pCustomDraw->clrText = isSelectedRow && (!isCurrCell || (isCurrCell && utils::isColorDark(GRIDCOLORS[0]))) ? RGB(255, 255, 255) : RGB(0, 0, 0);
						pCustomDraw->clrTextBk = isCurrCell ? GRIDCOLORS[0] :
							isSelectedRow ? RGB(10, 36, 106) :
							datatypes ? GRIDCOLORS[datatypes[colNo + colCount * resultset[rowNo]]] :
							RGB(255, 255, 255);
					}

					SetWindowLongPtr(hWnd, DWLP_MSGRESULT, result);
					return true;
				}

				if (pHdr->code == LVN_KEYDOWN && pHdr->hwndFrom == hListWnd) {
					NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;

					bool isControl = HIWORD(GetKeyState(VK_CONTROL));
					bool isAlt = HIWORD(GetKeyState(VK_MENU));
					if (isAlt)
						ListView_SetItemState (hListWnd, -1, 0, LVIS_FOCUSED);

					if (kd->wVKey == VK_RETURN) {
						int pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
						if (hListWnd == GetFocus() && pos != -1) {
							SetProp(hWnd, TEXT("CURRENTROW"), IntToPtr(pos));
							PostMessage(hWnd, WM_COMMAND, isAlt ? IDM_VALUE_EDIT : IDM_ROW_EDIT, 0);
						}
						return true;
					}

					if ((kd->wVKey == VK_LEFT || kd->wVKey == VK_RIGHT) && isControl && isAlt) { // Toggle dialogs
						return true;
					}

					if (kd->wVKey == VK_F4) {
						PostMessage(hWnd, WM_COMMAND, IDM_VALUE_VIEW, 0);
						return true;
					}

					if (kd->wVKey == VK_F5) {
						PostMessage(hWnd, WM_COMMAND, IDM_ROW_REFRESH, 0);
						return true;
					}

					if (kd->wVKey == 0x41 && isControl) { // Ctrl + A - Select All
						ListView_SetItemState(hListWnd, -1, LVIS_SELECTED, LVIS_SELECTED);
						return true;
					}

					int currRow = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
					int currCol = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN"));
					bool isValid = currRow >= 0 && currCol > 0;

					if (kd->wVKey == 0x43 && isControl && isValid) { // Ctrl + C
						PostMessage(hWnd, WM_COMMAND, IDM_RESULT_COPY_ROW, 0);
						return true;
					}

					if (kd->wVKey == 0x56 && isControl && isValid) { // Ctrl + V
						TCHAR* clipboard16 = utils::getClipboardText();
						char* clipboard8 = utils::utf16to8(clipboard16);
						SendMessage(hWnd, WMU_SET_CURRENT_CELL_VALUE, (WPARAM)clipboard8, (LPARAM)SQLITE_TEXT);
						delete [] clipboard16;
						delete [] clipboard8;
						return true;
					}

					if (canDelete && !isAlt && kd->wVKey == VK_DELETE) {
						PostMessage(hWnd, WM_COMMAND, IDM_ROW_DELETE, 0);
						return true;
					}

					if (canUpdate && isAlt && kd->wVKey == VK_DELETE) {
						PostMessage(hWnd, WM_COMMAND, IDM_VALUE_NULL, 0);
						return true;
					}

					if ((canUpdate || canDelete) && (kd->wVKey == VK_NEXT || kd->wVKey == VK_PRIOR || kd->wVKey == VK_HOME || kd->wVKey == VK_END)) {
						PostMessage(hWnd, WMU_SYNC_CURRENT_CELL, 0, 0);
						return true;
					}

					if ((canUpdate || canDelete) && (kd->wVKey == VK_UP || kd->wVKey == VK_DOWN)) {
						int rowCount = ListView_GetItemCount(hListWnd);
						if (rowCount == 0)
							return false;

						int rowNo = (currRow + rowCount + (kd->wVKey == VK_UP ? -1 : 1)) % rowCount;
						SendMessage(hWnd, WMU_SET_CURRENT_CELL, rowNo, currCol);
						ListView_SetItemState(hListWnd, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
						ListView_SetItemState(hListWnd, rowNo, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, true);
						return true;
					}

					if (isControl && kd->wVKey == 0x49) { // Ctrl + I
						for (int rowNo = 0; rowNo < ListView_GetItemCount(hListWnd); rowNo++) {
							BOOL isSelected = ListView_GetItemState(hListWnd, rowNo, LVIS_SELECTED) & LVIS_SELECTED;
							ListView_SetItemState(hListWnd, rowNo, isSelected ? 0 : LVIS_SELECTED, LVIS_SELECTED);
						}
					}

					if (canUpdate && (kd->wVKey == VK_LEFT || kd->wVKey == VK_RIGHT)) {
						int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd));
						int colNo = (currCol + colCount + (kd->wVKey == VK_LEFT ? -1 : 1)) % colCount;
						colNo = colNo == 0 && kd->wVKey == VK_LEFT ? colCount - 2 : colNo == colCount - 1 && kd->wVKey == VK_RIGHT ? 1 : colNo;
						SendMessage(hWnd, WMU_SET_CURRENT_CELL, currRow, colNo);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, true);
						return true;
					}

					bool isNum = kd->wVKey >= 0x31 && kd->wVKey <= 0x39;
					bool isNumPad = kd->wVKey >= 0x61 && kd->wVKey <= 0x69;
					if ((isNum || isNumPad) && isControl) {// Ctrl + 1-9
						ListView_Sort(pHdr->hwndFrom, kd->wVKey - (isNum ? 0x31 : 0x61) + 1);
						if (canUpdate)
							PostMessage(hWnd, WMU_SYNC_CURRENT_CELL, 0, 0);
						return true;
					}

					if (canUpdate && !isControl && (
						(kd->wVKey == VK_SPACE && !isAlt) ||
						(kd->wVKey == VK_OEM_PLUS) || (kd->wVKey == VK_OEM_MINUS) || (kd->wVKey == VK_OEM_COMMA) || (kd->wVKey == VK_OEM_PERIOD) || // +-,.
						(kd->wVKey >= 0x30 && kd->wVKey <= 0x5A) || // 0, 1, ..., y, z
						(kd->wVKey >= 0x60 && kd->wVKey <= 0x6F) // Numpad
						) && isValid) {
						SendMessage(hWnd, WMU_EDIT_VALUE, 0, kd->wVKey);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, true);
						return true;
					}

					if (canUpdate && kd->wVKey == VK_F2 && isValid) {
						SendMessage(hWnd, WMU_EDIT_VALUE, 1, 0);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, true);
						return true;
					}

					if (canInsert && kd->wVKey == VK_INSERT)
						SendMessage(hWnd, WM_COMMAND, IDM_ROW_ADD, 0);

					if (canUpdate && kd->wVKey == VK_SPACE && isControl) { // Ctrl + Space
						int colNo = PtrToInt(GetProp(hWnd, TEXT("CURRENTCOLUMN")));
						char** fkSelects = (char**)GetProp(hWnd, TEXT("FKSELECTS"));
						bool hasFK = fkSelects && colNo != -1 && fkSelects[colNo];
						SendMessage(hWnd, hasFK ? WMU_OPEN_FK_VALUE_SELECTOR : WMU_CREATE_VALUE_SELECTOR, 0, 0);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, TRUE);
						return true;
					}
				}

				if (pHdr->code == (DWORD)NM_DBLCLK && pHdr->hwndFrom == hListWnd) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;

					if (ia->iItem == -1)
						return canInsert ? SendMessage(hWnd, WM_COMMAND, IDC_DLG_ROW_ADD, 0) : 0;

					if (ia->iSubItem == 0 || ia->iSubItem == Header_GetItemCount(ListView_GetHeader(hListWnd)) - 1)
						return true;

					SetProp(hWnd, TEXT("CURRENTROW"), IntToPtr(ia->iItem));
					SetProp(hWnd, TEXT("CURRENTCOLUMN"), IntToPtr(ia->iSubItem));
					if (canUpdate)
						return SendMessage(hWnd, WMU_EDIT_VALUE, 1, 0);
				}

				// This event is triggered on ListView_SetData too. So ListView is hiding to prevent processing of the notification.
				if (pHdr->code == HDN_ITEMCHANGED && pHdr->hwndFrom == ListView_GetHeader(hListWnd) && IsWindowVisible(hListWnd)) {
					int* widths = (int*)GetProp(hWnd, TEXT("WIDTHS"));
					int colNo = ((LPNMHEADER)lParam)->iItem;
					widths[colNo] = ListView_GetColumnWidth(hListWnd, colNo);
					SendMessage(hWnd, WMU_UPDATE_COLSIZE, 0, 0);
				}

				if (pHdr->code == TTN_GETDISPINFO) {
					LPTOOLTIPTEXT pTtt = (LPTOOLTIPTEXT) lParam;
					pTtt->hinst = GetModuleHandle(0);
					pTtt->lpszText = MAKEINTRESOURCE(pTtt->hdr.idFrom);
					return true;
				}

				if (pHdr->idFrom == IDC_DLG_STATUSBAR && (pHdr->code == NM_CLICK || pHdr->code == NM_RCLICK)) {
					NMMOUSE* pm = (NMMOUSE*)lParam;
					int id = pm->dwItemSpec;
					if (id != SB_EDITADATA_TRIGGERS && id != SB_EDITADATA_INDEXES)
						return 0;

					RECT rc, rc2;
					GetWindowRect(pHdr->hwndFrom, &rc);
					SendMessage(pHdr->hwndFrom, SB_GETRECT, id, (LPARAM)&rc2);
					HMENU hMenu = (HMENU)GetProp(hWnd, id == SB_EDITADATA_TRIGGERS ? TEXT("TRIGGERMENU") : id == SB_EDITADATA_INDEXES ? TEXT("INDEXMENU") : 0);

					POINT p = {rc.left + rc2.left, rc.top};
					TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
				}

			}
			break;

			case WM_COMMAND: {
				WORD cmd = LOWORD(wParam);
				bool hasRowid = GetProp(hWnd, TEXT("HASROWID"));
				bool canUpdate = GetProp(hWnd, TEXT("CANUPDATE"));
				char* md5keys8 = (char*)GetProp(hWnd, TEXT("MD5KEYS8"));
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				HWND hHeader = (HWND)ListView_GetHeader(hListWnd);
				int colCount = Header_GetItemCount(hHeader);
				TCHAR*** cache = (TCHAR***)GetProp(hListWnd, TEXT("CACHE"));
				int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
				byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
				unsigned char** blobs = (unsigned char**)GetProp(hListWnd, TEXT("BLOBS"));

				int currRow = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
				int currCol = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN"));

				if (cmd == IDC_DLG_CANCEL || cmd == IDCANCEL)
					SendMessage(hWnd, WM_CLOSE, 0, 0);

				if (cmd == IDM_RESULT_COPY_CELL || cmd == IDM_RESULT_COPY_ROW)
					onListViewMenu(hListWnd, currRow, currCol, cmd, true);

				if (cmd == IDM_VALUE_VIEW) {
					int rowNo = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
					int colNo = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTCOLUMN"));

					TValue value = {0};
					if (ListView_GetItemValue(hListWnd, rowNo, colNo, &value))
						openDialog(IDD_VALUE_VIEWER, (DLGPROC)cbDlgValueViewer, (LPARAM)&value);
				}

				if (cmd == IDM_VALUE_EDIT) {
					int _rowNo = resultset[currRow];
					bool isBLOB = datatypes[currCol + (colCount - 1) * _rowNo] == SQLITE_BLOB;
					if (isBLOB)
						return false;

					TCHAR* buf16 = new TCHAR[_tcslen(cache[_rowNo][currCol]) + 1];
					_tcscpy(buf16, cache[_rowNo][currCol]);
					if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_EDITDATA_VALUE), hWnd, (DLGPROC)cbDlgValueEditor, (LPARAM)buf16)) {
						char* value8 = utils::utf16to8(buf16);
						SendMessage(hWnd, WMU_SET_CURRENT_CELL_VALUE, (WPARAM)value8, 0);
						delete [] value8;
					}
					delete [] buf16;

					return true;
				}


				if (cmd == IDM_VALUE_NULL) {
					SendMessage(hWnd, WMU_SET_CURRENT_CELL_VALUE, 0, SQLITE_NULL);
				}

				if (cmd == IDM_VALUE_FILE_OPEN || cmd == IDM_VALUE_FILE_SAVE || cmd == IDM_VALUE_FILE_SET || cmd == IDM_VALUE_FILE_EDIT) {
					int rowNo = resultset[currRow];
					TCHAR* value16 = cache[rowNo][currCol];
					bool isBlob = datatypes[currCol + (colCount - 1) * rowNo] == SQLITE_BLOB;
					const unsigned char* blob = isBlob ? blobs[currCol + rowNo * (colCount - 1)] : 0;
					int blobSize = blob ? utils::getBlobSize(blob) - 4 : 0;

					TCHAR filepath16[MAX_PATH]{0};
					TCHAR filter16[] = TEXT("Images (*.jpg, *.gif, *.png, *.bmp)\0*.jpg;*.jpeg;*.gif;*.png;*.bmp\0Text file (*.txt)\0*.txt\0Audio (*.mp3, *.ogg, *.wav)\0*.mp3,*.ogg,*.wav\0Binary(*.bin,*.dat)\0*.bin,*.dat\0All\0*.*\0");

					auto saveToFile = [hWnd] (TCHAR* filepath16, TCHAR* value16, const unsigned char* data, int dataSize, bool isBlob) {
						FILE* f = _tfopen (filepath16, TEXT("wb"));
						if (f) {
							if (isBlob) {
								fwrite(data, dataSize, 1, f);
							} else {
								char* value8 = utils::utf16to8(value16);
								fwrite(value8, strlen(value8), 1, f);
								delete [] value8;
							}

							fclose(f);
						} else {
							MessageBox(hWnd, TEXT("Save to file failed."), TEXT("Error"), MB_OK);
						}
					};

					if (cmd == IDM_VALUE_FILE_OPEN || cmd == IDM_VALUE_FILE_EDIT) {
						TCHAR ext16[10] = TEXT("txt");
						if (isBlob)
							utils::detectFileExtension((const char*)blob + 4, blobSize, ext16);

						SYSTEMTIME st;
						GetLocalTime(&st);
						_sntprintf(filepath16, MAX_PATH, TEXT("%ls\\blob-%.2u-%.2u-%.2u.%ls"), TMP_PATH, st.wHour, st.wMinute, st.wSecond, _tcslen(ext16) ? ext16 : TEXT("blob"));
						saveToFile(filepath16, value16, blob + 4, blobSize, isBlob);

						SHELLEXECUTEINFO sei{0};
						sei.cbSize = sizeof(SHELLEXECUTEINFO);
						sei.lpFile = filepath16;
						sei.lpVerb = cmd == IDM_VALUE_FILE_OPEN ? TEXT("open") : TEXT("edit");
						sei.nShow = SW_SHOW;
						ShellExecuteEx(&sei);
					}

					if (cmd == IDM_VALUE_FILE_SAVE) {
						TCHAR ext16[10] = TEXT("txt");

						if (isBlob)
							utils::detectFileExtension((const char*)blob + 4, blobSize, ext16);

						_sntprintf(filepath16, MAX_PATH, TEXT("blob.%ls"), ext16);
						if (utils::saveFile(filepath16, filter16, ext16, hWnd))
							saveToFile(filepath16, value16, blob + 4, blobSize, isBlob);
					}

					if (cmd == IDM_VALUE_FILE_SET || cmd == IDM_VALUE_FILE_EDIT) {
						if ((cmd == IDM_VALUE_FILE_SET && utils::openFile(filepath16, filter16, hWnd)) ||
							(cmd == IDM_VALUE_FILE_EDIT && IDOK == MessageBox(hWnd, TEXT("Press OK when finished."), TEXT("Info"), MB_OKCANCEL))) {
							FILE* f = _tfopen (filepath16, TEXT("rb"));
							if (f) {
								fseek(f, 0L, SEEK_END);
								long fileSize = ftell(f);
								rewind(f);

								unsigned char* data8 = new unsigned char[fileSize + 1];
								fread(data8, fileSize, 1, f);
								fclose(f);
								data8[fileSize] = 0;

								TCHAR ext16[32]{0};
								_tsplitpath(filepath16, NULL, NULL, NULL, ext16);
								bool isTxt = _tcscmp(TEXT(".txt"), ext16) == 0 || _tcscmp(TEXT(".xml"), ext16) == 0 || _tcscmp(TEXT(".json"), ext16) == 0 ||
									_tcscmp(TEXT(".html"), ext16) == 0 || _tcscmp(TEXT(".yaml"), ext16) == 0 || _tcscmp(TEXT(".sql"), ext16) == 0;

								if (isTxt)
									SendMessage(hWnd, WMU_SET_CURRENT_CELL_VALUE, (WPARAM)data8, 0);
								else
									SendMessage(hWnd, WMU_SET_CURRENT_CELL_BLOB, (WPARAM)data8, (LPARAM)fileSize);

								delete [] data8;
							}
						}
					}
				}

				if (cmd == IDM_RESULT_VALUE_FILTER)
					onListViewMenu(hListWnd, currRow, currCol, IDM_RESULT_VALUE_FILTER);

				if (cmd == IDM_ROW_ADD) {
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hWnd, (DLGPROC)&cbDlgRow, MAKELPARAM(ROW_ADD, 0));
					int currRow = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					SendMessage(hWnd, WMU_SET_CURRENT_CELL, currRow, currCol);

					ListView_SetSelectionMark(hListWnd, currRow);
				}

				if (cmd == IDM_ROW_REFRESH) {
					SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
					SendMessage(hWnd, WMU_UPDATE_META, 0, 0);
				}

				if (cmd == IDM_ROW_EDIT) {
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hWnd, (DLGPROC)cbDlgRow, MAKELPARAM(canUpdate ? ROW_EDIT: ROW_VIEW, 0));
					int currRow = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					SendMessage(hWnd, WMU_SET_CURRENT_CELL, currRow, currCol);
				}

				if (cmd == IDM_NEXT_DIALOG || cmd == IDM_PREV_DIALOG) {
					switchDialog(hWnd, cmd == IDM_NEXT_DIALOG);
					return true;
				}

				if (cmd == IDM_ROW_DELETE) {
					int count = ListView_GetSelectedCount(hListWnd);
					if (!count)
						return true;

					if (prefs::get("ask-delete") && MessageBox(hWnd, TEXT("Are you sure you want to delete the row? "), TEXT("Delete confirmation"), MB_OKCANCEL) != IDOK)
						return true;

					char placeholders8[count * 2]{0}; // count = 3 => ?, ?, ?
					for (int i = 0; i < count * 2 - 1; i++)
						placeholders8[i] = i % 2 ? ',' : '?';
					placeholders8[count * 2 - 1] = '\0';

					TCHAR*** cache = (TCHAR***)GetProp(hListWnd, TEXT("CACHE"));
					int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
					char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
					char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));

					char sql8[1024 + strlen(tablename8) + strlen(schema8) + (hasRowid ? 0 : strlen(md5keys8)) + count * 2]{0};
					sprintf(sql8, "delete from \"%s\".\"%s\" where %s in (%s)", schema8, tablename8, hasRowid ? "rowid" : md5keys8,  placeholders8);

					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0)) {
						int pos = -1;

						for (int i = 0; i < count; i++) {
							pos = ListView_GetNextItem(hListWnd, pos, LVNI_SELECTED);
							int _rowNo = resultset[pos];
							TCHAR* rowid16 = cache[_rowNo][colCount - 1];

							char* rowid8 = utils::utf16to8(rowid16);
							sqlite3_bind_text(stmt, i + 1, rowid8, -1, SQLITE_TRANSIENT);
							delete [] rowid8;
						}

						if (SQLITE_DONE != sqlite3_step(stmt))
							showDbError(hWnd);
					}
					sqlite3_finalize(stmt);

					SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
				}

				if (cmd == IDM_ROW_DUPLICATE) {
					int rowCount = ListView_GetSelectedCount(hListWnd);
					int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd));

					TCHAR ids16[rowCount * 12]{0};
					int pos = -1;
					for (int i = 0; i < rowCount; i++) {
						pos = ListView_GetNextItem(hListWnd, pos, LVNI_SELECTED);
						int _rowNo = resultset[pos];
						TCHAR* rowid16 = cache[_rowNo][colCount - 1];
						if (_tcslen(ids16))
							_tcscat(ids16, TEXT(","));
						_tcscat(ids16, rowid16);
					}

					char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
					char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
					char* ids8 = utils::utf16to8(ids16);

					char sql8[1024 + 2 * strlen(schema8) + 2 * strlen(tablename8) + (hasRowid ? 0 : strlen(md5keys8))]{0};
					sprintf(sql8, "select 'insert into \"%s\".\"%s\" (' || group_concat(name, ', ') || ') " \
						"select ' || group_concat(name, ', ') || ' from \"%s\".\"%s\" where rowid in (%s)' " \
						"from pragma_table_info('%s') where schema = '%s' and pk = 0", schema8, tablename8, schema8, tablename8, ids8, tablename8, schema8);
					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0) && SQLITE_ROW == sqlite3_step(stmt)) {
						if (SQLITE_OK != sqlite3_exec(db, (char*)sqlite3_column_text(stmt, 0), 0, 0, 0))
							showDbError(hWnd);
						else
							PostMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
					}
					sqlite3_finalize(stmt);

					delete [] ids8;
				}

				if (cmd == IDM_FILTER_TYPE) {
					HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
					bool isFilter = !Toolbar_GetButtonData(hToolbarWnd, IDM_FILTER_TYPE);
					TCHAR title16[10]{0};
					_tcscpy(title16, isFilter ? TEXT("Filter"): TEXT("Where"));

					TBBUTTONINFO tbi{0};
					tbi.cbSize = sizeof(TBBUTTONINFO);
					tbi.dwMask = TBIF_LPARAM | TBIF_IMAGE | TBIF_TEXT;
					tbi.iImage = 4 + isFilter;
					tbi.pszText = title16;
					tbi.lParam = isFilter;
					SendMessage(hToolbarWnd, TB_SETBUTTONINFO, IDM_FILTER_TYPE, (LPARAM)&tbi);

					prefs::set("edit-data-filter-mode", isFilter);
				}

				if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetDlgItem(hWnd, IDC_DLG_QUERYFILTER) && (HWND)lParam == GetFocus()) {
					KillTimer(hWnd, IDT_EDIT_DATA);
					SetTimer(hWnd, IDT_EDIT_DATA, 300, NULL);
				}

				if ((wParam >= IDM_MENU_TRIGGER && wParam < IDM_MENU_TRIGGER + 50) ||(wParam >= IDM_MENU_INDEX && wParam < IDM_MENU_INDEX + 50)) {
					bool isTrigger = wParam >= IDM_MENU_TRIGGER && wParam < IDM_MENU_TRIGGER + 50;
					HMENU hMenu = (HMENU)GetProp(hWnd, isTrigger ? TEXT("TRIGGERMENU") : TEXT("INDEXMENU"));
					TCHAR name16[255];
					GetMenuString(hMenu, wParam, name16, 255, MF_BYCOMMAND);

					char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
					TCHAR* schema16 = utils::utf8to16(schema8);
					TCHAR* fullname16 = utils::getFullTableName(schema16, name16);
					delete [] schema16;

					int len = _tcslen(fullname16) + 2;
					TCHAR buf16[len + 1];
					_sntprintf(buf16, len, TEXT(" %ls"), fullname16);
					buf16[0] = MAKEWORD(1, isTrigger ? TRIGGER : INDEX); // view
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADDVIEWEDIT), hWnd, (DLGPROC)cbDlgAddViewEdit, (LPARAM)buf16);
					delete [] fullname16;
				}
			}
			break;

			case WM_CLOSE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				HWND hValuesWnd = FindWindowEx(hListWnd, 0, WC_COMBOBOX, 0);
				int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd));

				if (hValuesWnd)
					DestroyWindow(hValuesWnd);

				HWND hValueWnd = FindWindowEx(hListWnd, 0, WC_EDIT, 0);
				if (hValueWnd)
					SetProp(hValueWnd, TEXT("ISCHANGED"), 0);

				char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
				delete [] tablename8;
				RemoveProp(hWnd, TEXT("TABLENAME8"));

				char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
				delete [] schema8;
				RemoveProp(hWnd, TEXT("SCHEMA8"));

				bool* generated = (bool*)GetProp(hWnd, TEXT("GENERATED"));
				if (generated != NULL) {
					delete [] generated;
					RemoveProp(hWnd, TEXT("GENERATED"));
				}

				int* widths = (int*)GetProp(hWnd, TEXT("WIDTHS"));
				if (widths != NULL) {
					delete [] widths;
					RemoveProp(hWnd, TEXT("WIDTHS"));
				}

				BYTE* decltypes = (BYTE*)GetProp(hWnd, TEXT("DECLTYPES"));
				if (decltypes != NULL) {
					delete [] decltypes;
					RemoveProp(hWnd, TEXT("DECLTYPES"));
				}

				int* fkColumns = (int*)GetProp(hWnd, TEXT("FKCOLUMNS"));
				if (fkColumns != NULL) {
					delete [] fkColumns;
					RemoveProp(hWnd, TEXT("FKCOLUMNS"));
				}

				char** fkSelects = (char**)GetProp(hWnd, TEXT("FKSELECTS"));
				if (fkSelects != NULL) {
					for(int colNo = 0; colNo < colCount; colNo++) {
						if (fkSelects[colNo + 1])
							delete [] fkSelects[colNo + 1];
					}
					delete [] fkSelects;
					RemoveProp(hWnd, TEXT("FKSELECTS"));
				}

				DestroyMenu((HMENU)GetProp(hWnd, TEXT("TRIGGERMENU")));
				DestroyMenu((HMENU)GetProp(hWnd, TEXT("INDEXMENU")));

				RemoveProp(hWnd, TEXT("ISTABLE"));
				RemoveProp(hWnd, TEXT("HASROWID"));
				RemoveProp(hWnd, TEXT("CANINSERT"));
				RemoveProp(hWnd, TEXT("CANUPDATE"));
				RemoveProp(hWnd, TEXT("CANDELETE"));
				RemoveProp(hWnd, TEXT("KEYCOUNT"));
				RemoveProp(hWnd, TEXT("FORCEUPDATE"));

				RemoveProp(hWnd, TEXT("CURRENTROW"));
				RemoveProp(hWnd, TEXT("CURRENTCOLUMN"));

				RemoveProp(hWnd, TEXT("TRIGGERMENU"));
				RemoveProp(hWnd, TEXT("INDEXMENU"));

				char* keys = (char*)GetProp(hWnd, TEXT("KEYS8"));
				if (keys)
					delete [] keys;
				RemoveProp(hWnd, TEXT("KEYS8"));

				char* md5keys8 = (char*)GetProp(hWnd, TEXT("MD5KEYS8"));
				if (md5keys8 != NULL) {
					delete [] md5keys8;
					RemoveProp(hWnd, TEXT("MD5KEYS8"));
				}

				EndDialog(hWnd, DLG_CANCEL);
				SendMessage(hMainWnd, WMU_UNREGISTER_DIALOG, (WPARAM)hWnd, 0);
				DestroyWindow(hWnd);
			}
			break;
		}

		return false;
	}

	// lParam, USERDATA = Parent Dlg handle
	BOOL CALLBACK cbDlgFKSelector (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);

				HWND hParentDlg = (HWND)lParam;
				int idc = GetWindowLongPtr(hParentDlg, DWLP_USER);

				HWND hEditDataDlg = idc == IDD_EDITDATA ? hParentDlg : GetWindow(hParentDlg, GW_OWNER);
				if (GetWindowLongPtr(hEditDataDlg, DWLP_USER) != IDD_EDITDATA) {
					MessageBox(hWnd, TEXT("Internal error"), 0, MB_OK);
					return EndDialog(hWnd, DLG_CANCEL);
				}

				int colNo = (int)(LONG_PTR)GetProp(hParentDlg, TEXT("CURRENTCOLUMN"));
				char** fkSelects = (char**)GetProp(hEditDataDlg, TEXT("FKSELECTS"));
				int* fkColumns = (int*)GetProp(hEditDataDlg, TEXT("FKCOLUMNS"));
				if (colNo == -1 || fkSelects[colNo] == 0)
					return EndDialog(hWnd, DLG_CANCEL);

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				SendMessage(hListWnd, WM_SETFONT, (WPARAM)hFont, FALSE);
				SetProp(hWnd, TEXT("KEYCOLUMN"), IntToPtr(fkColumns[colNo] + 1));

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, fkSelects[colNo], -1, &stmt, 0))
					ListView_SetData(hListWnd, stmt);
				sqlite3_finalize(stmt);

				TCHAR value16[1024] {0};
				if (idc == IDD_EDITDATA) {
					HWND hParentListWnd = GetDlgItem(hParentDlg, IDC_DLG_ROWS);
					int rowNo = (int)(LONG_PTR)GetProp(hParentDlg, TEXT("CURRENTROW"));
					ListView_GetItemText(hParentListWnd, rowNo, colNo, value16, 1023);
				}

				if (idc == IDD_ROW) {
					HWND hEdit = (HWND)GetProp(hParentDlg, TEXT("CURRENTEDIT"));
					GetWindowText(hEdit, value16, 1023);
				}

				if (_tcslen(value16)) {
					for (int rowNo = 0; rowNo < ListView_GetItemCount(hListWnd); rowNo++) {
						TCHAR fkValue16[1024];
						ListView_GetItemText(hListWnd, rowNo, fkColumns[colNo] + 1, fkValue16, 1023);

						if (_tcscmp(fkValue16, value16) == 0) {
							ListView_SetItemState (hListWnd, rowNo, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
							break;
						}
					}
				}

				utils::alignDialog(hWnd, hParentDlg, false, true);
				SetFocus(hListWnd);
			}
			break;

			case WM_SIZE: {
				RECT rc = {0};
				GetClientRect(hWnd, &rc);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_ROWS), 0, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER);
			}
			break;

			case WMU_SET_VALUE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);
				int colNo = (int)(LONG_PTR)GetProp(hWnd, TEXT("KEYCOLUMN"));
				int rowNo = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
				if (rowNo == -1) {
					EndDialog(hWnd, DLG_CANCEL);
					return true;
				}

				TCHAR value16[MAX_TEXT_LENGTH + 1];
				ListView_GetItemText(hListWnd, rowNo, colNo, value16, MAX_TEXT_LENGTH);
				HWND hParentDlg = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);

				if (GetWindowLongPtr(hParentDlg, DWLP_USER) == IDD_EDITDATA) {
					char* value8 = utils::utf16to8(value16);
					SendMessage(hParentDlg, WMU_SET_CURRENT_CELL_VALUE, (WPARAM)value8, 0);
					delete [] value8;
				}

				if (GetWindowLongPtr(hParentDlg, DWLP_USER) == IDD_ROW) {
					HWND hEdit = (HWND)GetProp(hParentDlg, TEXT("CURRENTEDIT"));
					SetWindowText(hEdit, value16);
				}

				EndDialog(hWnd, DLG_CANCEL);
				return true;
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDOK) {
					GetDlgItemText(hWnd, IDC_DLG_EDITOR, (TCHAR*)GetWindowLongPtr(hWnd, GWLP_USERDATA), MAX_TEXT_LENGTH);
					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ROWS);

				if (pHdr->code == (UINT)NM_CUSTOMDRAW && pHdr->hwndFrom == hListWnd) {
					NMLVCUSTOMDRAW* pCustomDraw = (LPNMLVCUSTOMDRAW)lParam;

					int result = CDRF_DODEFAULT;
					if (pCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
						result = CDRF_NOTIFYITEMDRAW;

					if (pCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
						result = CDRF_NOTIFYSUBITEMDRAW | CDRF_NEWFONT;

					if (pCustomDraw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
						int colNo = (int)(LONG_PTR)GetProp(hWnd, TEXT("KEYCOLUMN"));
						pCustomDraw->clrTextBk = colNo == pCustomDraw->iSubItem ? RGB(255, 255, 225) : RGB(255, 255, 255);
						result = CDRF_DODEFAULT;
					}

					SetWindowLongPtr(hWnd, DWLP_MSGRESULT, result);
					return true;
				}

				if (pHdr->code == LVN_KEYDOWN && pHdr->hwndFrom == hListWnd) {
					NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;

					if (kd->wVKey == VK_RETURN) {
						PostMessage(hWnd, WMU_SET_VALUE, 0, 0);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, true);
						return true;
					}
				}

				if (pHdr->code == (DWORD)NM_DBLCLK && pHdr->hwndFrom == hListWnd) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;

					if (ia->iItem == -1)
						return 0;

					SendMessage(hWnd, WMU_SET_VALUE, 0, 0);
				}
			}
			break;

			case WM_CLOSE: {
				RemoveProp(hWnd, TEXT("KEYCOLUMN"));
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	// lParam, USERDATA = TDlgValueParam
	BOOL CALLBACK cbDlgValueEditor (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				setEditorFont(hEditorWnd);
				SendMessage(hEditorWnd, WM_SETTEXT, (WPARAM)0, (LPARAM)lParam);
				SendMessage(hEditorWnd, EM_SETEVENTMASK, 0, ENM_KEYEVENTS);
				if (prefs::get("word-wrap"))
					toggleWordWrap(hEditorWnd);

				utils::alignDialog(hWnd, hMainWnd);
			}
			break;

			case WM_SIZE: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				bool isReadOnly = GetWindowLongPtr(hEditorWnd, GWL_STYLE) & ES_READONLY;

				POINTFLOAT s = utils::getDlgScale(hWnd);

				RECT rc;
				GetClientRect(hWnd, &rc);
				if (!isReadOnly) {
					HWND hOkWnd = GetDlgItem(hWnd, IDC_DLG_OK);
					SetWindowPos(hEditorWnd, 0, 5 * s.x, 5 * s.y, rc.right - 2 * 5 * s.x, rc.bottom - (14 + 3 * 5) * s.y, SWP_NOZORDER);
					SetWindowPos(hOkWnd, 0, rc.right - (54 + 5) * s.x, rc.bottom - (14 + 5 - 2) * s.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				} else {
					SetWindowPos(hEditorWnd, 0, 5 * s.x, 5 * s.y, rc.right - 2 * 5 * s.x, rc.bottom - 2 * 5 * s.y, SWP_NOZORDER);
				}
				SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)0, (LPARAM)0);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK || wParam == IDOK) {
					GetDlgItemText(hWnd, IDC_DLG_EDITOR, (TCHAR*)GetWindowLongPtr(hWnd, GWLP_USERDATA), MAX_TEXT_LENGTH);
					EndDialog(hWnd, DLG_OK);
				}
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (wParam == IDC_DLG_EDITOR && pHdr->code == EN_MSGFILTER) {
					MSGFILTER* pF = (MSGFILTER*)lParam;
					int key = pF->wParam;
					bool isKeyDown = pF->lParam & (1U << 31);
					bool isControl = HIWORD(GetKeyState(VK_CONTROL));
					if (key == 0x57 && isControl) { // Ctrl + W
						if (isKeyDown)
							toggleWordWrap(pF->nmhdr.hwndFrom);

						pF->wParam = 0;
						return true;
					}

					if (key == VK_RETURN && isControl)
						SendMessage(hWnd, WM_COMMAND, IDC_DLG_OK, 0);
				}
			}
			break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;
		}

		return false;
	}

	// lParam = TDlgValueParam
	BOOL CALLBACK cbDlgValueViewer (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hPreviewWnd = CreateWindow(WC_STATIC, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_CENTER | SS_CENTERIMAGE, 0, 0, 100, 100, hWnd, (HMENU)IDC_PREVIEW, GetModuleHandle(0), NULL);
				SetProp(hPreviewWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hPreviewWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewValueViewer));
				SetProp(hPreviewWnd, TEXT("INFO"), (HANDLE)(new TCHAR[255]));
				SetProp(hPreviewWnd, TEXT("EXT"), (HANDLE)(new TCHAR[32]));

				TValue* value = (TValue*)lParam;
				SetProp(hPreviewWnd, TEXT("DATA"), (HANDLE)value->data);
				SetProp(hPreviewWnd, TEXT("DATATYPE"), IntToPtr(value->dataType));
				SetProp(hPreviewWnd, TEXT("DATALEN"), IntToPtr(value->dataLen));

				SendMessage(hPreviewWnd, WM_SETFONT, (LPARAM)hFont, 0);
				CreateWindowEx(WS_EX_TOPMOST, WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | SS_NOTIFY, 20, 20, 100, 100, hPreviewWnd, (HMENU)IDC_PREVIEW_INFO, GetModuleHandle(0), NULL);

				SetWindowLongPtr(hPreviewWnd, GWLP_USERDATA, lParam);
				PostMessage(hPreviewWnd, WMU_UPDATE_PREVIEW, 0, 0);

				utils::alignDialog(hWnd, hMainWnd);
			}
			break;

			case WM_SIZE: {
				WORD w = LOWORD(lParam);
				WORD h = HIWORD(lParam);

				HWND hPreviewWnd = GetDlgItem(hWnd, IDC_PREVIEW);
				SetWindowPos(hPreviewWnd, 0, 0, 0, w, h - 1, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDCANCEL)
					SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);

				SendMessage(hMainWnd, WMU_UNREGISTER_DIALOG, (WPARAM)hWnd, 0);
				HWND hPreviewWnd = GetDlgItem(hWnd, IDC_PREVIEW);
				DestroyWindow(hPreviewWnd);
				DestroyWindow((HWND)hWnd);
				break;
		}

		return false;
	}

	// USERDATA = TValue
	LRESULT CALLBACK cbNewValueViewer (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_SIZE: {
				WORD w = LOWORD(lParam);
				WORD h = HIWORD(lParam);

				HWND hPluginWnd = (HWND)GetProp(hWnd, TEXT("PLUGINWND"));
				if (hPluginWnd) {
					SetWindowPos(hPluginWnd, 0, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
					SendMessage(hPluginWnd, WM_SIZE, wParam, lParam);
				}

				HWND hInfoWnd = GetDlgItem(hWnd, IDC_PREVIEW_INFO);
				if (hInfoWnd) {
					SendMessage(hInfoWnd, WM_SETFONT, (LPARAM)hMenuFont, 0);
					TCHAR buf16[255];
					GetWindowText(hInfoWnd, buf16, 255);
					SIZE s = utils::getTextSize(hMenuFont, buf16);
					s.cx += s.cx > 0 ? 10 : 0;
					s.cy += s.cy > 0 ? 4 : 0;
					SetWindowPos(hInfoWnd, 0, 0, h - s.cy, MIN(w, s.cx), s.cy, SWP_NOZORDER);
				}

				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;

			case WM_DESTROY: {
				TCHAR* info16 = (TCHAR*)GetProp(hWnd, TEXT("INFO"));
				delete [] info16;
				RemoveProp(hWnd, TEXT("INFO"));

				TCHAR* ext16 = (TCHAR*)GetProp(hWnd, TEXT("EXT"));
				delete [] ext16;
				RemoveProp(hWnd, TEXT("EXT"));

				SendMessage(hWnd, WMU_RESET_PREVIEW, 0, 0);
			}
			break;

			case WM_COMMAND: {
				int cmd = LOWORD(wParam);

				if (HIWORD(wParam) == STN_CLICKED && (LOWORD(wParam) == IDC_PREVIEW_INFO)) {
					RECT rc;
					HWND hInfoWnd = (HWND)lParam;
					GetWindowRect(hInfoWnd, &rc);

					TrackPopupMenu(hPreviewMenu, TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN, rc.left, rc.top, 0, hWnd, NULL);
				}

				if (cmd == IDM_PREVIEW_TO_FILE || cmd == IDM_PREVIEW_AS_FILE) {
					TCHAR* ext16 = (TCHAR*)GetProp(hWnd, TEXT("EXT"));
					const unsigned char* data = (const unsigned char*)GetProp(hWnd, TEXT("DATA"));
					int dataType = PtrToInt(GetProp(hWnd, TEXT("DATATYPE")));
					int dataLen = PtrToInt(GetProp(hWnd, TEXT("DATALEN")));

					TCHAR path16[MAX_PATH + 1] {0};
					if (cmd == IDM_PREVIEW_TO_FILE) {
						_sntprintf(path16, MAX_PATH, TEXT("file.%ls"), _tcslen(ext16) ? ext16 : TEXT("blob"));
						if (!utils::saveFile(path16, TEXT("All\0*.*\0"), ext16, hWnd))
							return false;
					} else {
						SYSTEMTIME st;
						GetLocalTime(&st);
						_sntprintf(path16, MAX_PATH, TEXT("%ls\\blob-%.2u-%.2u-%.2u.%ls"), TMP_PATH, st.wHour, st.wMinute, st.wSecond, _tcslen(ext16) ? ext16 : TEXT("blob"));
					}

					if (dataType == SQLITE_BLOB) {
						FILE* f = _tfopen(path16, TEXT("wb"));
						fwrite(data, dataLen, 1, f);
						fclose(f);
					} else {
						FILE* f = _tfopen(path16, TEXT("wb, ccs=UTF-8"));
						char* text8 = utils::utf16to8((TCHAR*)data);
						fwrite(text8, strlen(text8), 1, f);
						delete [] text8;
						fclose(f);
					}

					if (cmd == IDM_PREVIEW_AS_FILE) {
						SHELLEXECUTEINFO sei{0};
						sei.cbSize = sizeof(SHELLEXECUTEINFO);
						sei.lpFile = path16;
						sei.nShow = SW_SHOW;
						ShellExecuteEx(&sei);
					}
				}

				if (cmd == IDM_PREVIEW_SWITCH_PLUGIN) {
					SetProp(hWnd, TEXT("SWITCH"), IntToPtr(1));
					SendMessage(hWnd, WMU_UPDATE_PREVIEW, 0, 0);
				}
			}
			break;

			case WMU_RESET_PREVIEW: {
				HWND hPluginWnd = (HWND)GetProp(hWnd, TEXT("PLUGINWND"));
				int pluginNo = PtrToInt(GetProp(hWnd, TEXT("PLUGINNO")));
				if (hPluginWnd)
					plugins[pluginNo].close(hPluginWnd);

				RemoveProp(hWnd, TEXT("PLUGINWND"));
				if(!GetProp(hWnd, TEXT("SWITCH")))
					RemoveProp(hWnd, TEXT("PLUGINNO"));

				HWND hInfoWnd = GetDlgItem(hWnd, IDC_PREVIEW_INFO);
				ShowWindow(hInfoWnd, SW_HIDE);
			}
			break;

			// wParam = cell
			case WMU_UPDATE_PREVIEW: {
				SendMessage(hWnd, WMU_RESET_PREVIEW, 0, 0);


				bool isSwitch = PtrToInt(GetProp(hWnd, TEXT("SWITCH")));
				RemoveProp(hWnd, TEXT("SWITCH"));

				HWND hInfoWnd = GetDlgItem(hWnd, IDC_PREVIEW_INFO);
				ShowWindow(hInfoWnd, SW_HIDE);

				const unsigned char* data = (const unsigned char*)GetProp(hWnd, TEXT("DATA"));
				int dataType = PtrToInt(GetProp(hWnd, TEXT("DATATYPE")));
				int dataLen = PtrToInt(GetProp(hWnd, TEXT("DATALEN")));

				if (dataType == SQLITE_NULL) {
					SetWindowText(hWnd, TEXT("The value is NULL"));
					return 0;
				}

				TCHAR infoText16[255] = {0x25B2, 0};
				TCHAR* info16 = (TCHAR*)GetProp(hWnd, TEXT("INFO"));
				TCHAR* ext16 = (TCHAR*)GetProp(hWnd, TEXT("EXT"));

				int pluginNo = isSwitch ? PtrToInt(GetProp(hWnd, TEXT("PLUGINNO"))) : -1;
				for (int i = pluginNo + 1; i <= MAX_PLUGIN_COUNT + pluginNo; i++) {
					TPlugin plugin = plugins[i % MAX_PLUGIN_COUNT];
					if (plugin.hModule && plugin.type == ADDON_VALUE_VIEWER) {
						ZeroMemory(info16, 255);
						ZeroMemory(ext16, 32);
						HWND hPluginWnd = plugin.view(hWnd, data, dataLen, dataType, info16, ext16);
						if (hPluginWnd) {
							pluginNo = i % MAX_PLUGIN_COUNT;
							SetProp(hWnd, TEXT("PLUGINWND"), hPluginWnd);
							SetProp(hWnd, TEXT("PLUGINNO"), IntToPtr(pluginNo));

							if (_tcslen(info16)) {
								_tcscat(infoText16, TEXT(" "));
								_tcscat(infoText16, info16);
							}
							ShowWindow(hInfoWnd, SW_SHOW);

							break;
						}
					}
				}

				SetWindowText(hWnd, pluginNo == -1 ? TEXT("No matched plugin found") : NULL);
				SetWindowText(hInfoWnd, infoText16);

				RECT rc;
				GetClientRect(hWnd, &rc);
				SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));
				InvalidateRect(hInfoWnd, NULL, TRUE);
			}
		}

		return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
	}

	// USERDATA = (mode, colCount)
	BOOL CALLBACK cbDlgRow (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, DWLP_USER, IDD_ROW);

				int mode = LOWORD(lParam);
				int isResultWnd = HIWORD(lParam);
				HWND hListWnd = isResultWnd ? (HWND)SendMessage(hMainWnd, WMU_GET_CURRENT_RESULTSET, 0, 0) : GetDlgItem(GetWindow(hWnd, GW_OWNER), IDC_DLG_ROWS);
				HWND hHeader = ListView_GetHeader(hListWnd);
				int colCount = Header_GetItemCount(hHeader) - !isResultWnd;

				int currRow = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
				SetProp(hWnd, TEXT("CURRENTROW"), IntToPtr(currRow));

				if (!hHeader || !colCount)
					EndDialog(hWnd, DLG_CANCEL);

				HWND hColumnsWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);
				for (int colNo = 1; colNo < colCount; colNo++) {
					TCHAR colName[256];
					Header_GetItemText(hHeader, colNo, colName, 255);

					CreateWindow(WC_STATIC, colName, WS_VISIBLE | WS_CHILD | SS_RIGHT, 0, 0, 0, 0, hColumnsWnd, (HMENU)IntToPtr(IDC_ROW_LABEL + colNo), GetModuleHandle(0), 0);
					HWND hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS | ES_AUTOHSCROLL | (mode == ROW_VIEW ? ES_READONLY : 0), 0, 0, 0, 0, hColumnsWnd, (HMENU)IntToPtr(IDC_ROW_EDIT + colNo), GetModuleHandle(0), 0);
					CreateWindow(WC_BUTTON, TEXT("..."), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hColumnsWnd, (HMENU)IntToPtr(IDC_ROW_SWITCH + colNo), GetModuleHandle(0), 0);

					SetProp(hEdit, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)cbNewRowEdit));
				}
				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETPARENTFONT);
				SetProp(hColumnsWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hColumnsWnd, GWLP_WNDPROC, (LONG_PTR)cbNewScroll));

				if (mode == ROW_ADD) {
					SetWindowText(hWnd, TEXT("Add row"));
					HWND hParentDlg = GetAncestor(hListWnd, GA_ROOT);
					const char* tablename8 = (const char*)GetProp(hParentDlg, TEXT("TABLENAME8"));
					const char* schema8 = (const char*)GetProp(hParentDlg, TEXT("SCHEMA8"));

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db,
						"with dflt as (select * from pragma_table_info where arg = ?1 and schema = ?2), " \
						"fk as (select * from pragma_foreign_key_list where arg = ?1 and schema = ?2) " \
						"select replace(dflt_value, '\"\"', '\"'), iif(fk.\"table\" is not null, 'Reference to ' || fk.\"table\" || '.' || fk.\"to\", '') " \
						"from dflt left join fk on dflt.name = fk.\"from\" order by dflt.cid",
						-1, &stmt, 0)) {
						sqlite3_bind_text(stmt, 1, tablename8, -1, SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt, 2, schema8, -1, SQLITE_TRANSIENT);

						int colNo = 0;
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							TCHAR* default16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
							int len = _tcslen(default16);
							if (len > 0) {
								bool isQ = default16[0] == TEXT('"') && default16[len - 1] == TEXT('"');
								default16[len - 1] = isQ ? 0 : default16[len - 1];
								Edit_SetCueBannerText(GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + colNo + 1), isQ ? default16 + 1 : default16);
							}
							delete [] default16;

							default16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
							if (len == 0 && _tcslen(default16) > 0)
								Edit_SetCueBannerText(GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + colNo + 1), default16);
							delete [] default16;

							colNo++;
						}
					}

					sqlite3_finalize(stmt);
				}

				HWND hOkBtn = GetDlgItem(hWnd, IDC_DLG_OK);
				SetWindowText(hOkBtn, mode == ROW_ADD ? TEXT("Save and New") : mode == ROW_EDIT ? TEXT("Save and Next") : TEXT("Next"));
				if (mode != ROW_VIEW) {
					POINTFLOAT s = utils::getDlgScale(hWnd);

					RECT rc;
					GetClientRect(hOkBtn, &rc);
					SetWindowPos(hOkBtn, 0, 0, 0, rc.right + 10 * s.x, rc.bottom, SWP_NOMOVE | SWP_NOZORDER);
				}

				HWND hClearValues = GetDlgItem(hWnd, IDC_DLG_CLEAR_VALUES);
				if (mode == ROW_ADD) {
					ShowWindow(hClearValues, SW_SHOW);
					Button_SetCheck(hClearValues, prefs::get("clear-values") ? BST_CHECKED : BST_UNCHECKED);
				}

				SetWindowLongPtr(hWnd, GWLP_USERDATA, MAKELPARAM(mode, colCount));
				SetProp(hWnd, TEXT("LISTVIEW"), hListWnd);

				if (mode == ROW_ADD) {
					bool* generated = (bool*)GetProp(GetParent(hListWnd), TEXT("GENERATED"));
					for (int i = 0; (i < colCount) && (generated != NULL); i++) {
						if(generated[i] != 0) {
							SetDlgItemText(hColumnsWnd, IDC_ROW_EDIT + i, TEXT("(generated)"));
							EnableWindow(GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + i), false);
							EnableWindow(GetDlgItem(hColumnsWnd, IDC_ROW_SWITCH + i), false);
						}
					}
				} else {
					SendMessage(hWnd, WMU_SET_DLG_ROW_DATA, 0, 0);
				}
				SetFocus(GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + 1));

				BYTE* decltypes = (BYTE*)GetProp(GetParent(hListWnd), TEXT("DECLTYPES"));
				if ((mode == ROW_ADD || mode == ROW_EDIT) && decltypes) {
					HBRUSH* brushes = new HBRUSH[colCount + 1]{0};
					for (int colNo = 1; colNo < colCount; colNo++) {
						int type = decltypes[colNo];
						int color = type == SQLITE_TEXT	? prefs::get("color-text") :
							type == SQLITE_NULL ? prefs::get("color-null") :
							type == SQLITE_BLOB ? prefs::get("color-blob") :
							type == SQLITE_INTEGER ? prefs::get("color-integer") :
							type == SQLITE_FLOAT ? prefs::get("color-real") :
							RGB(0, 0, 0);
						brushes[colNo] = CreateSolidBrush(color);
					}
					SetProp(hWnd, TEXT("BRUSHES"), (HANDLE)brushes);
				}

				int lineH = 14;
				SetWindowPos(hWnd, 0, 0, 0, prefs::get("dialog-row-width"), (5 + lineH * (MIN(colCount, 15)) + 40) * utils::getDlgScale(hWnd).y, SWP_NOZORDER | SWP_NOMOVE);
				utils::alignDialog(hWnd, hMainWnd);
				if (prefs::get("dialog-row-maximized"))
					ShowWindow(hWnd, SW_MAXIMIZE);
			}
			break;

			case WM_SYSCOMMAND: {
				if (wParam == SC_MAXIMIZE || wParam == SC_RESTORE)
					prefs::set("dialog-row-maximized", wParam == SC_MAXIMIZE);
			}
			break;

			case WM_GETMINMAXINFO: {
				POINTFLOAT s = utils::getDlgScale(hWnd);

				MINMAXINFO* mm = (MINMAXINFO*)lParam;
				mm->ptMinTrackSize.x = 250 * s.x;
				mm->ptMinTrackSize.y = 90 * s.y;
			}
			break;

			case WM_SIZE: {
				WORD w = LOWORD(lParam);
				WORD h = HIWORD(lParam);

				POINTFLOAT s = utils::getDlgScale(hWnd);
				int editH = utils::getEditHeight(hWnd);
				int lineH = 14;

				HWND hColumnsWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);
				int colCount = HIWORD(GetWindowLongPtr(hWnd, GWLP_USERDATA));
				int mode = LOWORD(GetWindowLongPtr(hWnd, GWLP_USERDATA));

				int buttonY = h - 17 * s.y;
				int sx = buttonY - 10 * s.y <= ((colCount - 1) * lineH) * s.y ? GetSystemMetrics(SM_CXVSCROLL) : 0;
				for (int colNo = 1; colNo < colCount; colNo++) {
					int lineY = (5 + lineH * (colNo - 1)) * s.y;
					SetWindowPos(GetDlgItem(hColumnsWnd, IDC_ROW_LABEL + colNo), 0, 5 * s.x, lineY + 1 * s.y, 80 * s.x, editH, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + colNo), 0, 90 * s.x, lineY, w - editH - 100 * s.x - sx, editH, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hColumnsWnd, IDC_ROW_SWITCH + colNo), 0, w - editH - 5 * s.x - sx, lineY, editH, editH, SWP_NOZORDER);
				}

				SetWindowPos(hColumnsWnd, 0, 0, 5 * s.y, w, buttonY - 10 * s.y, SWP_NOZORDER);
				SendMessage(hColumnsWnd, WMU_SET_SCROLL_HEIGHT, ((colCount - 1) * lineH) * s.y, 0);

				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_CLEAR_VALUES), 0, 5 * s.x, buttonY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_CANCEL), 0, w - 58 * s.x, buttonY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_OK), 0, w - (115 + (mode != ROW_VIEW ? 10 : 0)) * s.x, buttonY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

				WINDOWPLACEMENT wp = {};
				wp.length = sizeof(WINDOWPLACEMENT);
				GetWindowPlacement(hWnd, &wp);
				if (IsWindowVisible(hWnd) && (wp.showCmd != SW_SHOWMAXIMIZED) && (wp.showCmd != SW_SHOWMINIMIZED)) {
					RECT rc;
					GetWindowRect(hWnd, &rc);
					prefs::set("dialog-row-width", rc.right - rc.left);
				}

				UpdateWindow(hWnd);
			}
			break;

			case WM_MOUSEWHEEL: {
				int action = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? SB_LINEUP : SB_LINEDOWN;
				SendMessage(GetDlgItem(hWnd, IDC_DLG_COLUMNS), WM_VSCROLL, MAKELPARAM(action, 0), 0);
			}
			break;

			case WMU_SET_DLG_ROW_DATA: {
				HWND hListWnd = (HWND)GetProp(hWnd, TEXT("LISTVIEW"));
				HWND hColumnsWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);
				int currRow = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
				int mode = LOWORD(GetWindowLongPtr(hWnd, GWLP_USERDATA));
				int colCount = HIWORD(GetWindowLongPtr(hWnd, GWLP_USERDATA));
				bool* generated = (bool*)GetProp(GetParent(hListWnd), TEXT("GENERATED"));

				TCHAR title[256];
				_sntprintf(title, 255, mode == ROW_EDIT ? TEXT("Edit row #%i") : TEXT("View row #%i"), currRow + 1);
				SetWindowText(hWnd, title);

				TCHAR val[MAX_TEXT_LENGTH];
				for (int i = 0; i < colCount; i++) {
					ListView_GetItemText(hListWnd, currRow, i, val, MAX_TEXT_LENGTH);
					HWND hEdit = GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + i);
					SetWindowText(hEdit, val);
					bool isGenerated = generated != NULL && generated[i] != 0;
					bool isEnable = !isGenerated;
					EnableWindow(hEdit, isEnable);
					EnableWindow(GetDlgItem(hColumnsWnd, IDC_ROW_SWITCH + i), isEnable);

					// Reset Edit flag
					SetWindowLongPtr(hEdit, GWLP_USERDATA, 0);
				}
				return true;
			}

			case WMU_SET_VALUE: {
				int colNo = PtrToInt(GetProp(hWnd, TEXT("CURRENTCOLUMN")));
				HWND hEdit = GetDlgItem(GetDlgItem(hWnd, IDC_DLG_COLUMNS), IDC_ROW_EDIT + colNo);
				SetWindowText(hEdit, (TCHAR*)wParam);
				SetFocus(hEdit);
			}
			break;

			case WM_COMMAND: {
				HWND hListWnd = (HWND)GetProp(hWnd, TEXT("LISTVIEW"));
				HWND hColumnsWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);
				int currRow = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
				int mode = LOWORD(GetWindowLongPtr(hWnd, GWLP_USERDATA));
				int colCount = HIWORD(GetWindowLongPtr(hWnd, GWLP_USERDATA));

				if (wParam >= IDC_ROW_SWITCH && (int)wParam < IDC_ROW_SWITCH + colCount) {
					int colNo = wParam - IDC_ROW_SWITCH;
					HWND hEdit = GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + wParam - IDC_ROW_SWITCH);
					TCHAR buf[MAX_TEXT_LENGTH]{0};
					GetWindowText(hEdit, buf, MAX_TEXT_LENGTH);

					if (mode == ROW_VIEW) {
						DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_VALUE_EDITOR), hWnd, (DLGPROC)cbDlgValueEditor, (LPARAM)buf);
					} else {
						byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
						byte* decltypes = (byte*)GetProp(GetParent(hListWnd), TEXT("DECLTYPES"));

						int rc = (datatypes && datatypes[colNo] == SQLITE_BLOB) || (decltypes && decltypes[colNo] == SQLITE_BLOB) ?
							utils::openFile(buf, NULL, hWnd) :
							DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_VALUE_EDITOR), hWnd, (DLGPROC)cbDlgValueEditor, (LPARAM)buf);
						if (rc)
							SetWindowText(hEdit, buf);
					}

					SetFocus(hEdit);
				}

				if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_DLG_CLEAR_VALUES)
					prefs::set("clear-values", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_CLEAR_VALUES)));

				if (HIWORD(wParam) == EN_UPDATE && LOWORD(wParam) >= IDC_ROW_EDIT && LOWORD(wParam) < IDC_ROW_EDIT + colCount)
					SetWindowLongPtr((HWND)lParam, GWLP_USERDATA, 1);

				if (wParam == IDOK) {
					int id = GetDlgCtrlID(GetFocus());
					if (id >= IDC_ROW_EDIT && (int)wParam < IDC_ROW_EDIT + colCount)
						return HIWORD(GetKeyState(VK_CONTROL)) ? SendMessage(hWnd, WM_COMMAND, IDC_DLG_OK, 0) : SendMessage(hWnd, WM_NEXTDLGCTL, 0, 0);
				}

				if (wParam == IDC_DLG_OK) {
					HWND hParentWnd = GetParent(hListWnd);
					bool hasRowid = GetProp(hParentWnd, TEXT("HASROWID"));
					char* md5keys8 = (char*)GetProp(hParentWnd, TEXT("MD5KEYS8"));

					auto changeCurrentItem = [hListWnd, hWnd, mode]() {
						ListView_SetItemState( hListWnd, -1, LVIF_STATE, LVIS_SELECTED);
						int rowCount = ListView_GetItemCount(hListWnd);

						int currRow = (int)(LONG_PTR)GetProp(hWnd, TEXT("CURRENTROW"));
						currRow = mode != ROW_ADD ? (currRow + 1) % rowCount : rowCount - 1;
						ListView_SetItemState (hListWnd, currRow, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
						if (!ListView_EnsureVisible(hListWnd, currRow, false)) {
							RECT rect = {0};
							ListView_GetItemRect(hListWnd, currRow, &rect, LVIR_LABEL);
							ListView_Scroll(hListWnd, 0, rect.bottom);
						}
						SetProp(hWnd, TEXT("CURRENTROW"), IntToPtr(currRow));
					};

					SendMessage(hWnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + 1), true);

					if (mode == ROW_VIEW) {
						changeCurrentItem();
						SendMessage(hWnd, WMU_SET_DLG_ROW_DATA, 0, 0);
						return true;
					}

					if (mode == ROW_EDIT) {
						bool isChanged = false;
						for (int i = 1; i < colCount; i++)
							isChanged = isChanged || GetWindowLongPtr(GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + i), GWLP_USERDATA);

						if (!isChanged) {
							changeCurrentItem();
							if (mode == ROW_EDIT)
								SendMessage(hWnd, WMU_SET_DLG_ROW_DATA, 0, 0);
							return false;
						}
					}

					TCHAR* columns16[colCount]{0};
					char* values8[colCount]{0};
					TCHAR* values16[colCount]{0};
					char* columns8[colCount]{0};

					HWND hDlgWnd = GetWindow(hWnd, GW_OWNER);
					char* schema8 = (char*)GetProp(hDlgWnd, TEXT("SCHEMA8"));
					char* tablename8 = (char*)GetProp(hDlgWnd, TEXT("TABLENAME8"));
					byte* datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));

					int len = 0;
					// A first column in the listview is always a rowno. Should be ignored.
					for (int i = 1; i < colCount; i++) {
						HWND hEdit = GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + i);
						len = IsWindowEnabled(hEdit) ? GetWindowTextLength(hEdit) : 0;

						values16[i] = new TCHAR[len + 1]{0};
						GetWindowText(hEdit, values16[i], len + 1);
						values8[i] = utils::utf16to8(values16[i]);

						HWND hLabel = GetDlgItem(hColumnsWnd, IDC_ROW_LABEL + i);
						len = GetWindowTextLength(hLabel);
						columns16[i] = new TCHAR[len + 1]{0};
						GetWindowText(hLabel, columns16[i], len + 1);
						columns8[i] = utils::utf16to8(columns16[i]);
					}

					char sql8[MAX_TEXT_LENGTH]{0};
					char buf8[256 + strlen(schema8) + strlen(tablename8)];
					sprintf(buf8, mode == ROW_ADD ? "insert into \"%s\".\"%s\" (" : "update \"%s\".\"%s\" set ", schema8, tablename8);
					strcat(sql8, buf8);
					int valCount = 0;
					for (int i = 1; i < colCount; i++) {
						HWND hEdit = GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + i);
						if (!IsWindowEnabled(hEdit))
							continue;

						if (mode == ROW_ADD && !strlen(values8[i]))
							continue;

						if (mode == ROW_EDIT && GetWindowLongPtr(hEdit, GWLP_USERDATA) == 0)
							continue;

						sprintf(buf8, mode == ROW_ADD ? "%s\"%s\"" : "%s\"%s\" = ?", valCount > 0 ? ", " : "", columns8[i]);
						strcat(sql8, buf8);

						valCount++;
					}

					if (mode == ROW_ADD) {
						if (valCount > 0) {
							char placeholders8[(valCount + 1) * 2]{0}; // count = 3 => ?, ?, ?
							for (int i = 0; i < (valCount + 1) * 2 - 3; i++)
								placeholders8[i] = i % 2 ? ',' : '?';
							placeholders8[(valCount + 1) * 2 - 1] = '\0';
							strcat(sql8, ") values (");
							strcat(sql8, placeholders8);
							strcat(sql8, ")");
						} else {
							sprintf(sql8, "insert into \"%s\".\"%s\" default values", schema8, tablename8);
						}
					} else {
						strcat(sql8, " where ");
						strcat(sql8, hasRowid ? " rowid " : md5keys8);
						strcat(sql8, " = ? ");
					}

					struct HookUserData {
						char *table;
						int op;
						sqlite3_int64 rowid;
					};
					HookUserData hud = {tablename8, mode == ROW_ADD ? SQLITE_INSERT : SQLITE_UPDATE, -1};

					auto cbHook = [](void *user_data, int op, char const *dbName, char const *table, sqlite3_int64 rowid) {
						HookUserData* hud = (HookUserData*)user_data;
						if (!stricmp(hud->table, table) && hud->op == op)
							hud->rowid = rowid;
					};
					sqlite3_update_hook(db, cbHook, &hud);

					sqlite3_stmt *stmt;
					bool rc = SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0);
					if (rc) {
						byte* decltypes = (byte*)GetProp(GetParent(hListWnd), TEXT("DECLTYPES"));
						int valNo = 1;
						for (int i = 1; i < colCount; i++) {
							HWND hEdit = GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + i);
							bool isChanged = GetWindowLongPtr(hEdit, GWLP_USERDATA);
							if (IsWindowEnabled(hEdit) &&
								((mode == ROW_EDIT && isChanged) || (mode == ROW_ADD && strlen(values8[i]) > 0))) {

								if (datatypes && datatypes[i] == SQLITE_BLOB && utils::isFileExists(values16[i])) {
									FILE *fp = _tfopen (values16[i], TEXT("rb"));
									if (!fp)
										MessageBox(hWnd, TEXT("Opening the file for reading failed."), TEXT("Info"), MB_OK);

									if (rc && fp) {
										fseek(fp, 0L, SEEK_END);
										long size = ftell(fp);
										rewind(fp);

										char* data8 = new char[size]{0};
										fread(data8, size, 1, fp);
										fclose(fp);
										sqlite3_bind_blob(stmt, valNo, data8, size, SQLITE_TRANSIENT);
										delete [] data8;
									}
								} else
									dbutils::bind_variant(stmt, valNo, values8[i], decltypes != 0 && decltypes[i - 1] == SQLITE_TEXT);
								valNo++;
							}
						}

						if (mode == ROW_EDIT) {
							TCHAR rowid16[64];
							ListView_GetItemText(hListWnd, currRow, colCount, rowid16, 64);
							char* rowid8 = utils::utf16to8(rowid16);
							sqlite3_bind_text(stmt, valNo, rowid8, strlen(rowid8), SQLITE_TRANSIENT);
							delete [] rowid8;
						}

						rc = SQLITE_DONE == sqlite3_step(stmt);
					}
					sqlite3_finalize(stmt);
					sqlite3_update_hook(db, NULL, NULL);

					// Update GUI
					if (rc) {
						char sql8[1024 + 2 * strlen(hasRowid ? "rowid" : md5keys8) + strlen(schema8) + strlen(tablename8)];
						sprintf(sql8, "select *, %s rowid from \"%s\".\"%s\" where %s = ?", hasRowid ? "rowid" : md5keys8, schema8, tablename8, hasRowid ? "rowid" : md5keys8);

						sqlite3_stmt *stmt;
						sqlite3_prepare_v2(db, sql8, -1, &stmt, 0);
						if (hasRowid) {
							sqlite3_bind_int64(stmt, 1, hud.rowid);
						} else {
							char* keys8 = (char*)GetProp(hParentWnd, TEXT("KEYS8"));
							int keyCount = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("KEYCOUNT"));
							char placeholders8[(keyCount + 1) * 2]{0}; // count = 3 => ?, ?, ?
							for (int i = 0; i < (keyCount + 1) * 2 - 3; i++)
								placeholders8[i] = i % 2 ? ',' : '?';
							placeholders8[(keyCount + 1) * 2 - 1] = 0;

							char sub8[0124 + 2 * strlen(md5keys8) + strlen(schema8) + strlen(tablename8)]{0};
							sprintf(sub8, "select %s from \"%s\".\"%s\" where (%s) = (%s)", md5keys8, schema8, tablename8, keys8, placeholders8);

							sqlite3_stmt *stmt2;
							if (SQLITE_OK == sqlite3_prepare_v2(db, sub8, -1, &stmt2, 0)) {
								int valNo = 0;
								for (int i = 1; i < colCount && valNo < keyCount; i++) {
									char column8[strlen(columns8[i]) + 3]{0};
									sprintf(column8, "\"%s\"", columns8[i]);
									if (strstr(keys8, column8)) {
										sqlite3_bind_text(stmt2, valNo + 1, values8[i], strlen(values8[i]), SQLITE_TRANSIENT);
										valNo++;
									}
								}

								if (SQLITE_ROW == sqlite3_step(stmt2)) {
									const char* md5 = (const char*)sqlite3_column_text(stmt2, 0);
									sqlite3_bind_text(stmt, 1, md5, strlen(md5), SQLITE_TRANSIENT);
								}
							} else {
								showDbError(hWnd);
							}
							sqlite3_finalize(stmt2);
						}

						if (SQLITE_ROW == sqlite3_step(stmt)) {
							if (mode == ROW_ADD) {
								SendMessage(hListWnd, WMU_ADD_EMPTY_ROW, 0, 0);
								datatypes = (byte*)GetProp(hListWnd, TEXT("DATATYPES"));
							}

							TCHAR*** cache = (TCHAR***)GetProp(hListWnd, TEXT("CACHE"));
							int* resultset = (int*)GetProp(hListWnd, TEXT("RESULTSET"));
							unsigned char** blobs = (unsigned char**)GetProp(hListWnd, TEXT("BLOBS"));
							int rowNo = resultset[mode == ROW_ADD ? ListView_GetItemCount(hListWnd)  - 1: currRow];
							for (int colNo = 0; colNo < sqlite3_column_count(stmt); colNo++) {
								int colType = sqlite3_column_type(stmt, colNo);

								int idx = colNo + 1 + rowNo * colCount;
								if (cache[rowNo][colNo + 1])
									free(cache[rowNo][colNo + 1]);

								if (blobs[idx])
									delete [] blobs[idx];

								cache[rowNo][colNo + 1] = colType == SQLITE_BLOB ? utils::toBlobSize(sqlite3_column_bytes(stmt, colNo)) :
									utils::utf8to16(colType == SQLITE_NULL ? "" : (char*)sqlite3_column_text(stmt, colNo));

								datatypes[idx] = colType;
								int dataSize = sqlite3_column_bytes(stmt, colNo);
								blobs[idx] = colType == SQLITE_BLOB ? utils::toBlob(dataSize, (const unsigned char*)sqlite3_column_blob(stmt, colNo)) : 0;
							}

							ListView_RedrawItems(hListWnd, rowNo, rowNo);
						}
						sqlite3_finalize(stmt);

						changeCurrentItem();
						if (mode == ROW_EDIT)
							SendMessage(hWnd, WMU_SET_DLG_ROW_DATA, 0, 0);
						SetFocus(GetDlgItem(hColumnsWnd, IDC_ROW_EDIT + 1));

						for (int i = 1; prefs::get("clear-values") && (mode == ROW_ADD) && (i <= colCount); i++)
							SetDlgItemText(hColumnsWnd, IDC_ROW_EDIT + i, TEXT(""));
					} else
						showDbError(hWnd);

					for (int i = 1; i < colCount; i++) {
						if (columns16[i])
							delete [] columns16[i];

						if (columns8[i])
							delete [] columns8[i];

						if (values16[i])
							delete [] values16[i];

						if (values8[i])
							delete [] values8[i];
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WMU_CTLCOLOREDIT: {
				int colCount = HIWORD(GetWindowLongPtr(hWnd, GWLP_USERDATA));
				int id = GetDlgCtrlID((HWND)lParam);
				if (id >= IDC_ROW_EDIT && id < IDC_ROW_EDIT + colCount) {
					HBRUSH* brushes = (HBRUSH*)GetProp(hWnd, TEXT("BRUSHES"));
					if (!brushes) {
						HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (INT_PTR)hBrush);
						return (INT_PTR)hBrush;
					}

					int colNo = id - IDC_ROW_EDIT;
					HBRUSH hBrush = brushes[colNo];
					SetBkColor((HDC)wParam, GetBrushColor(hBrush));
					SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (INT_PTR)hBrush);
					return (INT_PTR)hBrush;
				}
			}
			break;

			case WM_DESTROY: {
				HBRUSH* brushes = (HBRUSH*)GetProp(hWnd, TEXT("BRUSHES"));
				if (brushes) {
					for (int i = 0; i < 5; i++)
						DeleteObject(brushes[i]);
					delete [] brushes;
				}
				RemoveProp(hWnd, TEXT("BRUSHES"));

				RemoveProp(hWnd, TEXT("LISTVIEW"));
				RemoveProp(hWnd, TEXT("CURRENTROW"));
			}
			break;
		}

		return false;
	}

	// lParam - table16
	BOOL CALLBACK cbDlgAddColumn (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TCHAR* table16 = (TCHAR*)lParam;
				TCHAR buf[256];
				_sntprintf(buf, 255, TEXT("Add column to \"%ls\""), table16);
				SetWindowText(hWnd, buf);

				TCHAR* schema16 = utils::getTableName(table16, true);
				TCHAR* tablename16 = utils::getTableName(table16);
				SetProp(hWnd, TEXT("TABLENAME16"), (HANDLE)tablename16);
				SetProp(hWnd, TEXT("SCHEMA16"), (HANDLE)schema16);

				HWND hColType = GetDlgItem(hWnd, IDC_DLG_COLTYPE);
				for (int i = 0; DATATYPES16[i]; i++)
					ComboBox_AddString(hColType, DATATYPES16[i]);
				ComboBox_SetCurSel(hColType, 0);

				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumFixEditHeights, (LPARAM)utils::getEditHeight(hWnd));
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					TCHAR colName16[256] = {0};
					GetDlgItemText(hWnd, IDC_DLG_COLNAME, colName16, 255);

					TCHAR colType16[64] = {0};
					GetDlgItemText(hWnd, IDC_DLG_COLTYPE, colType16, 64);

					TCHAR _check16[256] = {0}, check16[300] = {0};
					GetDlgItemText(hWnd, IDC_DLG_CHECK, _check16, 255);
					_sntprintf(check16, 255, _tcslen(_check16) > 0 ? TEXT("check(%ls)") : TEXT("%ls"), _check16);

					TCHAR _defValue16[256] = {0}, defValue16[300] = {0};
					GetDlgItemText(hWnd, IDC_DLG_DEFVALUE, _defValue16, 255);
					if (_tcslen(_defValue16))
						_sntprintf(defValue16, 255, utils::isNumber(_defValue16, 0) ? TEXT("default %ls") : TEXT("default \"%ls\""), _defValue16);

					bool isNotNull = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISNOTNULL));
					bool isUnique = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISUNIQUE));

					TCHAR* schema16 = (TCHAR*)GetProp(hWnd, TEXT("SCHEMA16"));

					TCHAR* tablename16 = (TCHAR*)GetProp(hWnd, TEXT("TABLENAME16"));
					int len = 2000 + _tcslen(schema16) + _tcslen(tablename16);
					TCHAR query16[len + 1]{0};
					_sntprintf(query16, len, TEXT("alter table \"%ls\".\"%ls\" add column \"%ls\" %ls %ls %ls %ls %ls"),
						schema16, tablename16, colName16, colType16, isNotNull ? TEXT("NOT NULL") : TEXT(""), defValue16, check16, isUnique ? TEXT("UNIQUE") : TEXT(""));

					char* query8 = utils::utf16to8(query16);
					if (SQLITE_OK != sqlite3_exec(db, query8, NULL, NULL, NULL))
						showDbError(hWnd);
					else
						EndDialog(hWnd, DLG_OK);
					delete [] query8;
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_CLOSE: {
				TCHAR* tablename16 = (TCHAR*)GetProp(hWnd, TEXT("TABLENAME16"));
				delete [] tablename16;
				RemoveProp(hWnd, TEXT("TABLENAME16"));

				TCHAR* schema16 = (TCHAR*)GetProp(hWnd, TEXT("SCHEMA16"));
				delete [] schema16;
				RemoveProp(hWnd, TEXT("SCHEMA16"));
			}
			break;
		}

		return false;
	}

	// lParam - fullname16
	BOOL CALLBACK cbDlgAddIndex (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TCHAR* fullname16 = (TCHAR*)lParam;
				TCHAR buf[1024];
				TCHAR* ufullname16 = _tcsdup(fullname16);
				_tcsupr(ufullname16);
				_sntprintf(buf, 1023, TEXT("Add index to %ls"), ufullname16);
				free(ufullname16);
				SetWindowText(hWnd, buf);

				TCHAR* schema16 = utils::getTableName(fullname16, true);
				TCHAR* tablename16 = utils::getTableName(fullname16);

				SetProp(hWnd, TEXT("SCHEMA16"), (HANDLE)schema16);
				SetProp(hWnd, TEXT("TABLENAME16"), (HANDLE)tablename16);

				int len = _tcslen(fullname16) + 10;
				TCHAR idxName16[len + 1];
				_sntprintf(idxName16, len, TEXT("idx_%ls_"), tablename16);
				SetDlgItemText(hWnd, IDC_DLG_IDXNAME, idxName16);

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select name 'Column name', 'asc' 'Order' from pragma_table_info(?2) where schema = ?1", -1, &stmt, 0)) {
					char* schema8 = utils::utf16to8(schema16);
					char* tablename8 = utils::utf16to8(tablename16);

					sqlite3_bind_text(stmt, 1, schema8, -1, SQLITE_TRANSIENT);
					sqlite3_bind_text(stmt, 2, tablename8, -1, SQLITE_TRANSIENT);

					delete [] schema8;
					delete [] tablename8;

					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* name16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
						ListBox_AddString(hListWnd, name16);
						delete [] name16;
					}
				}
				sqlite3_finalize(stmt);

				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumFixEditHeights, (LPARAM)utils::getEditHeight(hWnd));
			}
			break;

			case WM_COMMAND: {
				if (HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_DLG_INDEXED_COLUMNS && HIWORD(GetKeyState(VK_CONTROL))) {
					HWND hListWnd = (HWND)lParam;
					int rowNo = ListBox_GetCurSel(hListWnd);
					int len = ListBox_GetTextLen(hListWnd, rowNo);
					TCHAR name16[len + 10]{0};
					ListBox_GetText(hListWnd, rowNo, name16);
					int isDesc = ListBox_GetItemData(hListWnd, rowNo);
					if (isDesc)
						name16[_tcslen(name16) - 5] = 0;
					else
						_tcscat(name16, TEXT(" desc"));

					ListBox_DeleteString(hListWnd, rowNo);
					ListBox_InsertString(hListWnd, rowNo, name16);
					ListBox_SetItemData(hListWnd, rowNo, !isDesc);
				}

				if (HIWORD(wParam) == LBN_DBLCLK) {
					HWND hListWnd = (HWND)lParam;
					int rowNo = ListBox_GetCurSel(hListWnd);

					int len = ListBox_GetTextLen(hListWnd, rowNo);
					TCHAR name16[len + 1]{0};
					ListBox_GetText(hListWnd, rowNo, name16);
					ListBox_DeleteString(hListWnd, rowNo);

					hListWnd = GetDlgItem(hWnd, LOWORD(wParam) == IDC_DLG_COLUMNS ? IDC_DLG_INDEXED_COLUMNS : IDC_DLG_COLUMNS);
					ListBox_AddString(hListWnd, name16);
				}

				if (wParam == IDC_DLG_OK) {
					TCHAR* schema16 = (TCHAR*)GetProp(hWnd, TEXT("SCHEMA16"));
					TCHAR* tablename16 = (TCHAR*)GetProp(hWnd, TEXT("TABLENAME16"));

					TCHAR idxName16[256] = {0};
					GetDlgItemText(hWnd, IDC_DLG_IDXNAME, idxName16, 255);
					bool isUnique = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISUNIQUE));

					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_INDEXED_COLUMNS);
					int colCount = ListBox_GetCount(hListWnd);

					if (_tcslen(idxName16) == 0) {
						MessageBox(hWnd, TEXT("The index name can't be empty"), NULL, MB_OK);
						return 0;
					}

					if (colCount == 0) {
						MessageBox(hWnd, TEXT("You should specify at least one indexed column"), NULL, MB_OK);
						return 0;
					}

					HWND hWhereWnd = GetDlgItem(hWnd, IDC_DLG_IDXWHERE);
					int wsize = GetWindowTextLength(hWhereWnd);

					int len = colCount * 255 + wsize + _tcslen(tablename16) + 200;
					TCHAR query16[len + 1]{0};
					TCHAR* qschema16 = utils::double_quote(schema16);
					TCHAR* qtablename16 = utils::double_quote(tablename16);
					TCHAR* qidxname16 = utils::double_quote(idxName16);
					_sntprintf(query16, len, TEXT("create%ls index %ls.%ls on %ls ("), isUnique ? TEXT(" unique") : TEXT(""), qschema16, qidxname16, qtablename16);
					delete [] qschema16;
					delete [] qtablename16;
					delete [] qidxname16;

					for (int colNo = 0; colNo < colCount; colNo++) {
						int len = ListBox_GetTextLen(hListWnd, colNo);
						TCHAR name16[len + 1]{0};
						ListBox_GetText(hListWnd, colNo, name16);
						int isDesc = ListBox_GetItemData(hListWnd, colNo);
						if (isDesc)
							name16[_tcslen(name16) - 5] = 0;

						TCHAR buf16[len + 11];
						TCHAR* qname16 = utils::double_quote(name16);
						_sntprintf(buf16, len + 10, TEXT("%ls%ls%ls"), qname16, isDesc ? TEXT(" desc") : TEXT(""), colNo != colCount - 1 ? TEXT(", ") : TEXT(")"));
						delete [] qname16;
						_tcscat(query16, buf16);
					}

					if (wsize > 0) {
						TCHAR where16[wsize + 1];
						GetWindowText(hWhereWnd, where16, wsize + 1);
						_tcscat(query16, TEXT(" where "));
						_tcscat(query16, where16);
					}

					char* query8 = utils::utf16to8(query16);
					if (SQLITE_OK != sqlite3_exec(db, query8, NULL, NULL, NULL))
						showDbError(hWnd);
					else
						EndDialog(hWnd, DLG_OK);
					delete [] query8;
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_CLOSE: {
				TCHAR* tablename16 = (TCHAR*)GetProp(hWnd, TEXT("TABLENAME16"));
				delete [] tablename16;
				RemoveProp(hWnd, TEXT("TABLENAME16"));

				TCHAR* schema16 = (TCHAR*)GetProp(hWnd, TEXT("SCHEMA16"));
				delete [] schema16;
				RemoveProp(hWnd, TEXT("SCHEMA16"));
			}
			break;
		}

		return false;
	}

	// type: 1 - editor find strings, 2 - editor replacements, 3 - result find strings
	void updateSearchSuggestions(HWND hComboWnd, int type) {
		TCHAR what16[256];
		GetWindowText(hComboWnd, what16, 255);

		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "replace into search_history (time, what, type) values (?1, ?2, ?3)", -1, &stmt, 0)) {
			sqlite3_bind_int(stmt, 1, std::time(0));
			char* what8 = utils::utf16to8(what16);
			sqlite3_bind_text(stmt, 2, what8, strlen(what8), SQLITE_TRANSIENT);
			delete [] what8;
			sqlite3_bind_int(stmt, 3, type);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);

		char sql8[512];
		sprintf(sql8, "delete from search_history where type = %i and rowid not in (select rowid from search_history where type = %i order by time desc limit 20)", type, type);
		sqlite3_exec(prefs::db, sql8, 0, 0, 0);

		int pos = ComboBox_FindStringExact(hComboWnd, 0, what16);
		if (pos != -1)
			ComboBox_DeleteString(hComboWnd, pos);
		ComboBox_InsertString(hComboWnd, 0, what16);
	}

	// lParam, USERDATA = hEditorWnd
	BOOL CALLBACK cbDlgFind (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hEditorWnd = (HWND)lParam;

				TCHAR* word = getCurrentText(hEditorWnd);
				SetDlgItemText(hWnd, IDC_DLG_FIND_STRING, word);
				delete [] word;

				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_CASE_SENSITIVE), prefs::get("case-sensitive") ? BST_CHECKED : BST_UNCHECKED);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LPARAM)hEditorWnd);

				EnableWindow(GetAncestor(hEditorWnd, GA_ROOT), false);
				HWND hEdit = GetDlgItem(hWnd, IDC_DLG_FIND_STRING);
				SetProp(hEdit, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)&cbNewEdit));
				hEdit = GetDlgItem(hWnd, IDC_DLG_REPLACE_STRING);
				SetProp(hEdit, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)&cbNewEdit));

				for (int type = 1; type < 3; type++) {
					HWND hComboxWnd	= GetDlgItem(hWnd, type == 1 ? IDC_DLG_FIND_STRING : IDC_DLG_REPLACE_STRING);
					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select what from search_history where type = ?1 order by time desc limit 20", -1, &stmt, 0)) {
						sqlite3_bind_int(stmt, 1, type);
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							TCHAR* what16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
							ComboBox_AddString(hComboxWnd, what16);
							delete [] what16;
						}
					}
					sqlite3_finalize(stmt);
				}

				utils::alignDialog(hWnd, hEditorWnd);
			}
			break;

			case WM_COMMAND: {
				HWND hEditorWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
				if (wParam == IDOK || wParam == IDC_DLG_FIND || wParam == IDC_DLG_REPLACE || wParam == IDC_DLG_REPLACE_ALL)
					prefs::set("case-sensitive", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_CASE_SENSITIVE)));

				TCHAR replaceString[256];
				GetDlgItemText(hWnd, IDC_DLG_FIND_STRING, searchString, 255);
				GetDlgItemText(hWnd, IDC_DLG_REPLACE_STRING, replaceString, 255);

				if (wParam == IDC_DLG_FIND || (wParam == IDOK && GetParent(GetFocus()) == GetDlgItem(hWnd, IDC_DLG_FIND_STRING))) {
					doEditorSearch(hEditorWnd, HIWORD(GetKeyState(VK_SHIFT)));
					updateSearchSuggestions(GetDlgItem(hWnd, IDC_DLG_FIND_STRING), 1);
				}

				if (wParam == IDC_DLG_REPLACE || (wParam == IDOK && GetParent(GetFocus()) == GetDlgItem(hWnd, IDC_DLG_REPLACE_STRING))) {
					if (doEditorSearch(hEditorWnd, HIWORD(GetKeyState(VK_SHIFT)))) {
						int crPos = LOWORD(SendMessage(hEditorWnd, EM_GETSEL, 0, 0));
						SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)replaceString);
						PostMessage(hEditorWnd, EM_SETSEL, crPos, crPos + _tcslen(replaceString));

						updateSearchSuggestions(GetDlgItem(hWnd, IDC_DLG_FIND_STRING), 1);
						updateSearchSuggestions(GetDlgItem(hWnd, IDC_DLG_REPLACE_STRING), 2);
					}
				}

				if (wParam == IDC_DLG_REPLACE_ALL) {
					int len = GetWindowTextLength(hEditorWnd);
					TCHAR text16[len + 1] = {0};
					GetWindowText(hEditorWnd, text16, len + 1);

					SetWindowRedraw(hEditorWnd, false);
					int crPos = LOWORD(SendMessage(hEditorWnd, EM_GETSEL, 0, 0));
					TCHAR* rtext16 = utils::replaceAll(text16, searchString, replaceString, 0, !prefs::get("case-sensitive"));
					SendMessage(hEditorWnd, EM_SETSEL, 0, -1);
					SendMessage(hEditorWnd, EM_REPLACESEL, true, (LPARAM)rtext16);
					SetWindowRedraw(hEditorWnd, true);
					InvalidateRect(hEditorWnd, NULL, true);
					PostMessage(hEditorWnd, EM_SETSEL, crPos, crPos);

					updateSearchSuggestions(GetDlgItem(hWnd, IDC_DLG_FIND_STRING), 1);
					updateSearchSuggestions(GetDlgItem(hWnd, IDC_DLG_REPLACE_STRING), 2);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_CLOSE: {
				HWND hEditorWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
				EnableWindow(GetAncestor(hEditorWnd, GA_ROOT), true);
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	// lParam = out buf
	BOOL CALLBACK cbDlgTableName (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetDlgItemText(hWnd, IDC_DLG_TABLENAME, (TCHAR*)lParam);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);

				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumFixEditHeights, (LPARAM)utils::getEditHeight(hWnd));
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK || wParam == IDOK) {
					GetDlgItemText(hWnd, IDC_DLG_TABLENAME, (TCHAR*)GetWindowLongPtr(hWnd, GWLP_USERDATA), 255);
					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgResultsComparison (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowText(hWnd, (TCHAR*)lParam);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COMPARED);
				SendMessage(hListWnd, WM_SETFONT, (WPARAM)hFont, false);

				char sql8[] = "with " \
					"t as (select '-->' dir, * from temp.result1 except select '-->' dir, * from temp.result2)," \
					"t2 as (select '<--' dir, * from temp.result2 except select '<--' dir, * from temp.result1)" \
					"select * from t union all select * from t2";

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0))
					ListView_SetData(hListWnd, stmt);
				else
					showDbError(hWnd);
				sqlite3_finalize(stmt);

				if (ListView_GetItemCount(hListWnd) == 0) {
					MessageBox(hWnd, TEXT("The results are the same"), TEXT("Info"), MB_OK);
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				}

				utils::alignDialog(hWnd, hMainWnd, false, true);
			}
			break;

			case WM_SIZE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COMPARED);
				RECT rc = {0};
				GetClientRect(hWnd, &rc);
				SetWindowPos(hListWnd, 0, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			break;

			case WM_NOTIFY: {
			    NMHDR* pHdr = (LPNMHDR)lParam;
                if (pHdr->code == LVN_COLUMNCLICK && pHdr->idFrom == IDC_DLG_COMPARED) {
					NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
					return ListView_Sort(pHdr->hwndFrom, pLV->iSubItem);
				}
			}
			break;

			case WM_CLOSE: {
				SendMessage(hMainWnd, WMU_UNREGISTER_DIALOG, (WPARAM)hWnd, 0);
				DestroyWindow(hWnd);
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgShortcuts (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				SetProp(hEditorWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hEditorWnd, GWLP_WNDPROC, (LONG_PTR)cbNewEditor));
				SendMessage(hEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_KEYEVENTS);
				setEditorFont(hEditorWnd);
				if (prefs::get("word-wrap"))
					toggleWordWrap(hEditorWnd);
				SetFocus(hEditorWnd);

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_SHORTCUTS);
				SendMessage(hListWnd, WM_SETFONT, (LPARAM)hFont, true);

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select name, query from shortcuts order by 1", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* name16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
						int no = ListBox_AddString(hListWnd, name16);
						ListBox_SetItemData(hListWnd, no, (LPARAM)utils::utf8to16((const char*)sqlite3_column_text(stmt, 1)));
						delete [] name16;
					}
				}
				sqlite3_finalize(stmt);

				if (ListBox_GetCount(hListWnd) == 0) {
					MessageBox(hWnd, TEXT("Shortcuts table is corrupted.\nOpen prefs.sqlite and drop this\ntable and restart application."), TEXT("Error"), MB_OK);
					EndDialog(hWnd, DLG_CANCEL);
					return false;
				}

				SetWindowLongPtr(hListWnd, GWLP_USERDATA, 0);
				ListBox_SetCurSel(hListWnd, 0);
				SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_DLG_SHORTCUTS, LBN_SELCHANGE), (LPARAM)hListWnd);

				utils::alignDialog(hWnd, hMainWnd, true, false);
			}
			break;

			case WM_SIZE: {
				POINTFLOAT s = utils::getDlgScale(hWnd);

				RECT rc;
				GetClientRect(hWnd, &rc);
				int H = rc.bottom - (14 - 2 + 3 * 5) * s.y;
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_SHORTCUTS), 0, 0, 0, 100 * s.x, H, SWP_NOZORDER | SWP_NOMOVE);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_EDITOR), 0, 0, 0, rc.right - (100 + 3 * 5) * s.x, H, SWP_NOZORDER | SWP_NOMOVE);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_LABEL), 0, 5 * s.x, H + (7 + 5) * s.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_OK), 0, rc.right - (54 + 5) * s.x, H + 9 * s.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			}
			break;

			case WM_COMMAND: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_SHORTCUTS);

				if (LOWORD(wParam) == IDC_DLG_SHORTCUTS && HIWORD(wParam) == LBN_SELCHANGE) {
					int prev = (int)GetWindowLongPtr(hListWnd, GWLP_USERDATA);
					int curr = ListBox_GetCurSel(hListWnd);

					if (prev != curr)
						SendMessage(hWnd, WMU_UPDATE_DATA, prev, 0);

					SetWindowText(hEditorWnd, (TCHAR*)ListBox_GetItemData(hListWnd, curr));
					SetWindowLongPtr(hListWnd, GWLP_USERDATA, curr);
				}

				if (LOWORD(wParam) == IDC_DLG_EDITOR && HIWORD(wParam) == EN_CHANGE)
					SendMessage((HWND)lParam, WMU_TEXT_CHANGED, 0, 0);

				if (wParam == IDM_EDITOR_COMMENT)
					toggleComment(hEditorWnd);

				if (wParam == IDM_EDITOR_CUT)
					SendMessage(hEditorWnd, WM_CUT, 0, 0);

				if (wParam == IDM_EDITOR_COPY)
					SendMessage(hEditorWnd, WM_COPY, 0, 0);

				if (wParam == IDM_EDITOR_PASTE)
					pasteText(hEditorWnd);

				if (wParam == IDM_EDITOR_DELETE)
					SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, 0);

				if (wParam == IDC_DLG_OK) {
					int curr = ListBox_GetCurSel(hListWnd);
					SendMessage(hWnd, WMU_UPDATE_DATA, curr, 0);

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "update shortcuts set query = ?2 where name = ?1", -1, &stmt, 0)) {
						for (int i = 0; i < ListBox_GetCount(hListWnd); i++) {
							TCHAR name16[100]{0};
							ListBox_GetText(hListWnd, i, name16);

							char* name8 = utils::utf16to8(name16);
							char* query8 = utils::utf16to8((TCHAR*)ListBox_GetItemData(hListWnd, i));
							sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
							sqlite3_bind_text(stmt, 2, query8, strlen(query8), SQLITE_TRANSIENT);
							delete [] name8;
							delete [] query8;
							sqlite3_step(stmt);
							sqlite3_reset(stmt);
						}
					}
					sqlite3_finalize(stmt);
					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (wParam == IDC_DLG_EDITOR && pHdr->code == EN_SELCHANGE)
					return SendMessage(pHdr->hwndFrom, WMU_SELECTION_CHANGED, wParam, lParam);

				if (wParam == IDC_DLG_EDITOR && pHdr->code == EN_MSGFILTER)
					return processEditorEvents((MSGFILTER*)lParam);
			}
			break;

			// wParam = row number
			case WMU_UPDATE_DATA: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_SHORTCUTS);
				delete [] (TCHAR*)ListBox_GetItemData(hListWnd, wParam);
				int size = GetWindowTextLength(hEditorWnd);
				TCHAR* data = new TCHAR[size + 1];
				GetWindowText(hEditorWnd, data, size + 1);
				ListBox_SetItemData(hListWnd, wParam, (LONG_PTR)data);
			}
			break;

			case WM_DESTROY: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_SHORTCUTS);
				for (int i = 0; i < ListBox_GetCount(hListWnd); i++)
					delete [] (TCHAR*)ListBox_GetItemData(hListWnd, i);
			}
			break;
		}

		return false;
	}

	// lParam = Alias = TCHAR buffer with alias name.
	// The buffer is empty for new connection.
	BOOL CALLBACK cbDlgAttachODBC (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hDriverWnd = GetDlgItem(hWnd, IDC_DLG_DRIVER);
				ComboBox_AddString(hDriverWnd, TEXT("(none)"));

				TCHAR buf16[1024];
				DWORD bufSize = 1024;
				HKEY hKey;
				if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
					int idx = 0;
					while (RegEnumValue(hKey, idx++, buf16, &bufSize, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS) {
						ComboBox_AddString(hDriverWnd, buf16);

						bufSize = 1024;
						ZeroMemory(buf16, sizeof(TCHAR)* bufSize);
					}
					RegCloseKey(hKey);
				}
				ComboBox_SetCurSel(hDriverWnd, 0);

				setEditorFont(GetDlgItem(hWnd, IDC_DLG_OPTIONS));

				TCHAR* alias16 = (TCHAR*)lParam;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
				SetDlgItemText(hWnd, IDC_DLG_ALIAS, alias16);
				if (_tcslen(alias16) > 0) {
					EnableWindow(GetDlgItem(hWnd, IDC_DLG_ALIAS), FALSE);

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select driver, options from odbc_recents where alias = ?1", -1, &stmt, 0)) {
						char* alias8 = utils::utf16to8(alias16);
						sqlite3_bind_text(stmt, 1, alias8, -1, SQLITE_TRANSIENT);
						delete [] alias8;

						if (SQLITE_ROW == sqlite3_step(stmt)) {
							TCHAR* driver16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
							int pos = ComboBox_FindStringExact(hDriverWnd, 0, driver16);
							ComboBox_SetCurSel(hDriverWnd, pos != -1 ? pos : 0);
							delete [] driver16;

							TCHAR* options16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
							SetDlgItemText(hWnd, IDC_DLG_OPTIONS, options16);
							delete [] options16;
						}
					}
					sqlite3_finalize(stmt);
				}

				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumFixEditHeights, (LPARAM)utils::getEditHeight(hWnd));
				utils::alignDialog(hWnd, hMainWnd, true, false);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_HELP) {
					TCHAR buf[MAX_TEXT_LENGTH];
					LoadString(GetModuleHandle(NULL), IDS_ATTACH_ODBC_HELP, buf, MAX_TEXT_LENGTH);
					MessageBox(hWnd, buf, TEXT("Help"), MB_OK);
				}

				if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_DLG_DRIVER) {
					TCHAR driver16[512]{0};
					GetDlgItemText(hWnd, IDC_DLG_DRIVER, driver16, 512);

					SetDlgItemText(hWnd, IDC_DLG_OPTIONS,
						_tcscmp(driver16, TEXT("(none)")) == 0 ? TEXT("") :
						_tcsstr(driver16, TEXT("Access")) && _tcsstr(driver16, TEXT("*.mdb")) ? TEXT("DBQ=C:\\MyDB.mdb;") :
						_tcsstr(driver16, TEXT("Excel")) && _tcsstr(driver16, TEXT("*.xls")) ? TEXT("DBQ=C:\\MyBook.xlsx;\nHeader=1;") :
						_tcsstr(driver16, TEXT("*.csv")) ? TEXT("DBQ=C:\\CSV;\nExtensions=csv,tab,txt") :
						_tcsstr(driver16, TEXT("SQL Server")) ? TEXT("Server=10.10.0.78,1433;\nDatabase=myDB;\nUser Id=admin;\nPassword=1234;") :
						_tcsstr(driver16, TEXT("*.dbf")) ? TEXT("DBQ=C:\\DbfFiles;") :
						TEXT("")
					);
				}

				if (wParam == IDC_DLG_OK) {
					TCHAR alias16[512]{0};
					GetDlgItemText(hWnd, IDC_DLG_ALIAS, alias16, 512);
					char* alias8 = utils::utf16to8(alias16);

					TCHAR driver16[512]{0};
					GetDlgItemText(hWnd, IDC_DLG_DRIVER, driver16, 512);
					char* driver8 = utils::utf16to8(driver16);

					TCHAR options16[2048]{0};
					GetDlgItemText(hWnd, IDC_DLG_OPTIONS, options16, 2047);
					char* options8 = utils::utf16to8(options16);


					bool isOk = strlen(alias8) > 0;

					// Test connection
					if (isOk) {
						sqlite3_stmt* stmt;
						sqlite3_exec(db, "drop table temp.test_connection", 0, 0, 0);
						if (SQLITE_OK == sqlite3_prepare_v2(db, "select odbc_read(iif(?1 is not null, 'Driver={' || ?1 || '};', '') || coalesce(?2, ''), '$TABLES', 'temp.test_connection')", -1, &stmt, 0)) {
							sqlite3_bind_text(stmt, 1, driver8, -1,  SQLITE_TRANSIENT);
							sqlite3_bind_text(stmt, 2, options8, -1,  SQLITE_TRANSIENT);
							sqlite3_step(stmt);
							isOk = SQLITE_OK == sqlite3_exec(db, "drop table temp.test_connection", 0, 0, 0);
						}
						sqlite3_finalize(stmt);
					}

					// Save connection
					if (isOk) {
						sqlite3_stmt* stmt;
						if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "replace into odbc_recents (alias, driver, options, time) values (?1, ?2, ?3, ?4)", -1, &stmt, 0)) {
							sqlite3_bind_text(stmt, 1, alias8, -1,  SQLITE_TRANSIENT);
							if (strcmp(driver8, "(none)"))
								sqlite3_bind_text(stmt, 2, driver8, -1,  SQLITE_TRANSIENT);
							else
								sqlite3_bind_null(stmt, 2);
							sqlite3_bind_text(stmt, 3, options8, -1,  SQLITE_TRANSIENT);
							sqlite3_bind_int(stmt, 4, std::time(0));
							isOk = SQLITE_DONE == sqlite3_step(stmt);
						}
						sqlite3_finalize(stmt);
					}

					if (!isOk)
						MessageBox(hWnd, strlen(alias8) ? TEXT("Can't connect with provided parameters") : TEXT("Alias is mandatory"), 0, MB_ICONEXCLAMATION);

					delete [] alias8;
					delete [] driver8;
					delete [] options8;

					if (isOk) {
						_tcscpy((TCHAR*)GetWindowLongPtr(hWnd, GWLP_USERDATA), alias16);
						EndDialog(hWnd, DLG_OK);
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}
		return FALSE;
	}

	// Used for plugin and extension managers
	// lParam, USERDATA = addon type
	BOOL CALLBACK cbDlgAddonManager (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ADDON_LIST);
				ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
				SendMessage(hListWnd, WM_SETFONT, (WPARAM)hFont, FALSE);
				HWND hStatusWnd = CreateStatusWindow(WS_CHILD | WS_VISIBLE, NULL, hWnd, IDC_DLG_STATUSBAR);

				int type = lParam;
				const TCHAR* type16 = ADDON_EXTS16[type];
				SetWindowLongPtr(hWnd, GWLP_USERDATA, type);

				sqlite3_exec(prefs::db, "create table if not exists temp.addons (name primary key, enable, version, installed_version, installed, author, homepage, brief, description)", 0, 0, 0);
				sqlite3_exec(prefs::db, "delete from temp.addons", 0, 0, 0);

				// Prepare temporary folder for new downloads
				TCHAR uploadPath16[MAX_PATH + 1];
				GetTempPath(MAX_PATH, uploadPath16);
				_tcscat(uploadPath16, TEXT("sqlite-gui\\update\\"));
				SHFILEOPSTRUCT fo = {NULL, FO_DELETE, uploadPath16, NULL, FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT, false, 0, NULL};
				SHFileOperation(&fo);
				SHCreateDirectoryEx(0, uploadPath16, 0);

				char* repository8 = type == ADDON_SQLITE_EXTENSION ? prefs::get("extension-repository", EXTENSION_REPOSITORY) :
					type == ADDON_COLUMN_MODIFIER ? prefs::get("modifier-repository", MODIFIER_REPOSITORY) :
					prefs::get("viewer-repository", VIEWER_REPOSITORY);
				SetProp(hWnd, TEXT("REPOSITORY8"), repository8);

				// Repo
				char url8[1024];
				snprintf(url8, 1023, "/%s/main/summary.json", repository8);
				char* repo8 = utils::httpRequest("GET", "raw.githubusercontent.com", url8);

				char title8[1024];
				snprintf(title8, 1023, "%s - github.com/%s", type == ADDON_VALUE_VIEWER ? "Value viewer plugin manager" : type == ADDON_COLUMN_MODIFIER ? "Column modifier plugin manager" : "SQLite extension manager", repository8);
				TCHAR* title16 = utils::utf8to16(title8);
				SetWindowText(hWnd, title16);
				delete [] title16;

				sqlite3_stmt* stmt;
				if (repo8 != 0) {
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
						"insert into temp.addons (name, version, author, homepage, brief, description) " \
						"select json_extract(value, '$.name'), json_extract(value, '$.version'), json_extract(value, '$.author'), " \
						"json_extract(value, '$.homepage'), json_extract(value, '$.brief'), json_extract(value, '$.description') " \
						"from json_each(?1)", -1, &stmt, 0)) {
						sqlite3_bind_text(stmt, 1, repo8, strlen(repo8), SQLITE_TRANSIENT);
						if(SQLITE_DONE != sqlite3_step(stmt))
							PostMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)TEXT("The repository is invalid"));
					}
					sqlite3_finalize(stmt);
					delete [] repo8;
				} else {
					PostMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)TEXT("The repository is unavailable"));
				}

				// Drive
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
					"insert into temp.addons (name, enable, installed) values (?1, 1, 1) on conflict (name) do update set enable = 1, installed = 1;",
					-1, &stmt, 0)) {
					TCHAR searchPath[MAX_PATH + 1]{0};
					if (type == ADDON_SQLITE_EXTENSION)
						_sntprintf(searchPath, MAX_PATH, TEXT("%ls" EXTENSION_DIRECTORY "*.%ls"), APP_PATH, type16);
					else
						_sntprintf(searchPath, MAX_PATH, TEXT("%ls" PLUGIN_DIRECTORY "%ls\\*.%ls"), APP_PATH, type16, type16);

					WIN32_FIND_DATA ffd;
					HANDLE hFind = FindFirstFile(searchPath, &ffd);

					if (hFind != INVALID_HANDLE_VALUE) {
						do {
							PathRemoveExtension(ffd.cFileName);
							char* name8 = utils::utf16to8(ffd.cFileName);

							sqlite3_reset(stmt);
							sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
							sqlite3_step(stmt);
							delete [] name8;
						} while (FindNextFile(hFind, &ffd));
					}
					FindClose(hFind);
				}
				sqlite3_finalize(stmt);

				// Prefs
				char query8[1024];
				snprintf(query8, 1023,
					"update temp.addons as ta set (installed_version, enable) = (select version, enable from main.addons a where ta.name = a.name and a.type = %i) "\
					"where name in (select name from main.addons where type = %i)", type, type);
				sqlite3_exec(prefs::db, query8, 0, 0, 0);

				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
						"select coalesce(enable and installed is not null, 0), name, " \
						"iif(installed is not null, iif(coalesce(version, '') <> coalesce(installed_version, ''), 'obsolete', 'installed'), ''), " \
						"coalesce(brief, '(Local file: no information)') from temp.addons order by name", -1, &stmt, 0)) {
					LVCOLUMN lvc = {0};
					lvc.mask = LVCF_SUBITEM | LVCF_TEXT;
					lvc.iSubItem = 0;
					ListView_InsertColumn(hListWnd, 0, &lvc);

					lvc.iSubItem = 1;
					lvc.pszText = TEXT("Name");
					ListView_InsertColumn(hListWnd, 1, &lvc);

					lvc.iSubItem = 2;
					lvc.pszText = TEXT("Status");
					ListView_InsertColumn(hListWnd, 2, &lvc);

					lvc.iSubItem = 3;
					lvc.pszText = TEXT("Info");
					ListView_InsertColumn(hListWnd, 3, &lvc);

					int rowNo = 0;
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						LVITEM lvi = {0};
						lvi.mask = 0;
						lvi.iItem = rowNo;
						ListView_InsertItem(hListWnd, &lvi);
						ListView_SetCheckState(hListWnd, rowNo, sqlite3_column_int(stmt, 0) == 1);

						for (int i = 0; i < 3; i++) {
							TCHAR* val16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, i + 1));
							lvi.iSubItem = i + 1;
							lvi.mask = LVIF_TEXT;
							lvi.pszText = val16;
							ListView_SetItem(hListWnd, &lvi);
							delete [] val16;
						}

						rowNo++;
					}

					for (int i = 0; i < 4; i++)
						ListView_SetColumnWidth(hListWnd, i, LVSCW_AUTOSIZE);

					if (ListView_GetColumnWidth(hListWnd, 2) < 30)
						ListView_SetColumnWidth(hListWnd, 2, 120);

					if (ListView_GetColumnWidth(hListWnd, 3) < 50)
						ListView_SetColumnWidth(hListWnd, 3, 200);
				}

				sqlite3_finalize(stmt);

				RECT rc, rc2;
				GetClientRect(hListWnd, &rc);

				HWND hHeader = ListView_GetHeader(hListWnd);
				SetWindowTheme(hHeader, TEXT(" "), TEXT(" "));
				int colCount = Header_GetItemCount(hHeader);
				Header_GetItemRect(hHeader, colCount - 1, &rc2);

				SetWindowPos(hWnd, 0, 0, 0, rc2.right + GetSystemMetrics(SM_CXVSCROLL) + 2 * GetSystemMetrics(SM_CXSIZEFRAME) + 2 * GetSystemMetrics(SM_CXEDGE), rc.bottom, SWP_NOZORDER | SWP_NOMOVE);
				SendMessage(hStatusWnd, WM_SIZE, 0, 0);

				utils::alignDialog(hWnd, hMainWnd, true);
			}
			break;

			case WM_SIZE: {
				HWND hStatusWnd = GetDlgItem(hWnd, IDC_DLG_STATUSBAR);
				SendMessage(hStatusWnd, WM_SIZE, 0, 0);

				RECT rc;
				GetWindowRect(hStatusWnd, &rc);
				POINT p{rc.right, rc.top};
				ScreenToClient(hWnd, &p);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_ADDON_LIST);
				SetWindowPos(hListWnd, 0, 0, 0, p.x, p.y - 1, SWP_NOZORDER | SWP_NOMOVE);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				int type = GetWindowLongPtr(hWnd, GWLP_USERDATA);
				const TCHAR* type16 = ADDON_EXTS16[type];

				if (pHdr->code == (DWORD)LVN_ITEMCHANGING && pHdr->idFrom == IDC_DLG_ADDON_LIST && IsWindowVisible(hWnd)) {
					NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
					HWND hListWnd = pHdr->hwndFrom;

					if ((pLV->uChanged & LVIF_STATE) && (pLV->uNewState & LVIS_STATEIMAGEMASK)) {
						int rowNo = pLV->iItem;
						ListView_SetItemState(hListWnd, -1, LVIS_SELECTED | LVIS_FOCUSED, 0);
						ListView_SetItemState(hListWnd, rowNo, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

						if (!ListView_GetCheckState(hListWnd, rowNo)) {
							TCHAR name16[512];
							TCHAR status16[32];
							ListView_GetItemText(hListWnd, rowNo, 1, name16, 511);
							ListView_GetItemText(hListWnd, rowNo, 2, status16, 31);
							bool isObsolete = _tcscmp(status16, TEXT("obsolete")) == 0;
							char* repository8 = (char*)GetProp(hWnd, TEXT("REPOSITORY8"));

							if (_tcscmp(status16, TEXT("installed")) != 0) {
								TCHAR msg16[1024];
								if (type == ADDON_SQLITE_EXTENSION) {
									if (isObsolete)
										_sntprintf(msg16, 1023, TEXT("Are you sure you want to update the extension?"));
									else
										_sntprintf(msg16, 1023, TEXT("Are you sure you want to install the extension?\nThe extension will be downloaded to %ls" EXTENSION_DIRECTORY "%ls.dll"), APP_PATH, name16);
								} else {
									if (isObsolete)
										_sntprintf(msg16, 1023, TEXT("Are you sure you want to update the plugin?"));
									else
										_sntprintf(msg16, 1023, TEXT("Are you sure you want to install the plugin?\nThe plugin will be downloaded to %ls" PLUGIN_DIRECTORY "%ls\\%ls.%ls"), APP_PATH, type16, name16, type16);
								}

								if (MessageBox(hWnd, msg16, TEXT("Confirmation"), MB_OKCANCEL) != IDOK) {
									SetWindowLongPtr(hWnd, DWLP_MSGRESULT, !isObsolete);
									return !isObsolete;
								}

								char* name8 = utils::utf16to8(name16);
								char url8[1024];
								snprintf(url8, 1023, "%s/releases/latest/download/%s-x%i.zip", repository8, name8, GUI_PLATFORM);

								bool isDone = false;
								int readBytes = 0;

								unsigned char* buf8 = (unsigned char*)utils::httpRequest("GET", "github.com", url8, 0, &readBytes);
								if (buf8 && (readBytes > 30) && (buf8[0] == 0x50 && buf8[1] == 0x4B && buf8[2] == 0x03 && buf8[3] == 0x04)) {
									// Parse first zip local file header
									unsigned int unpackLen = utils::read_le32(&buf8[22]);
									unsigned int packLen = utils::read_le32(&buf8[18]);
									unsigned int nameLen = utils::read_le16(&buf8[26]);
									unsigned int nameExLen = utils::read_le16(&buf8[28]);

									unsigned char *data8 = new unsigned char[MAX(unpackLen, 1)] {0};
									if (TINF_OK == (tinf_uncompress(data8, &unpackLen, buf8 + 30 + nameLen + nameExLen, packLen))) {
										TCHAR path16[MAX_PATH + 1];
										GetTempPath(MAX_PATH, path16);
										_tcscat(path16, TEXT("sqlite-gui\\update\\"));

										_tcscat(path16, name16);
										_tcscat(path16, TEXT("."));
										_tcscat(path16, type16);

										FILE* f = _tfopen(path16, TEXT("wb"));
										if (f) {
											fwrite(data8, 1, unpackLen, f);
											fclose(f);

											ListView_SetItemText(hListWnd, rowNo, 2, TEXT("installed"));

											sqlite3_stmt* stmt;
											if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
													"with v as (select version from temp.addons where name = ?1) " \
													"insert into main.addons (name, type, version) values (?1, ?2, (select version from v)) " \
													"on conflict (name, type) do update set version = (select version from v);", -1, &stmt, 0)) {

													sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
													sqlite3_bind_int(stmt, 2, type);
													sqlite3_step(stmt);
											}
											sqlite3_finalize(stmt);

											isDone = true;
										}

										delete [] name8;
									}
									delete [] data8;
								}

								if (buf8)
									delete [] buf8;

								// update help
								if (isDone && type == ADDON_SQLITE_EXTENSION)	{
									char* name8 = utils::utf16to8(name16);
									char url8[1024];
									snprintf(url8, 1023, "/%s/main/%s/help.json", repository8, name8);
									int readBytes = 0;
									char* buf8 = utils::httpRequest("GET", "raw.githubusercontent.com", url8, 0, &readBytes);

									if (buf8) {
										if (!prefs::applyHelp(buf8, name8))
											MessageBox(hWnd, TEXT("Can't update the extension help"), 0, MB_OK);

										delete [] buf8;
									}
									delete [] name8;
								}

								if (!isDone) {
									MessageBox(hWnd, type == ADDON_SQLITE_EXTENSION ? TEXT("Can't install the extension") : TEXT("Can't install the plugin"), 0, MB_OK);
									isDone = isObsolete; // Allow to enable the obsolete version
								}

								SetWindowLongPtr(hWnd, DWLP_MSGRESULT, !isDone);
								return !isDone;
							}
						}
					}
				}

				if (pHdr->code == (DWORD)LVN_ITEMCHANGED && pHdr->idFrom == IDC_DLG_ADDON_LIST && IsWindowVisible(hWnd)) {
					NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
					HWND hListWnd = pHdr->hwndFrom;
					int rowNo = pLV->iItem;

					if ((pLV->uChanged & LVIF_STATE) && (pLV->uNewState & LVIS_STATEIMAGEMASK)) {
						TCHAR name16[512];
						ListView_GetItemText(hListWnd, rowNo, 1, name16, 511);

						sqlite3_stmt* stmt;
						if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
							"insert into main.addons (name, type, enable) values (?1, ?2, ?3) " \
							"on conflict (name, type) do update set enable = ?3;", -1, &stmt, 0)) {
							char* name8 = utils::utf16to8(name16);
							sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
							delete [] name8;
							sqlite3_bind_int(stmt, 2, type);
							sqlite3_bind_int(stmt, 3, ListView_GetCheckState(hListWnd, rowNo));

							sqlite3_step(stmt);
						}
						sqlite3_finalize(stmt);

						SetProp(hWnd, TEXT("CHANGED"), IntToPtr(1));
					}

					if ((pLV->uChanged & LVIF_STATE) && (pLV->uNewState & LVIS_SELECTED)) {
						TCHAR name16[512];
						ListView_GetItemText(hListWnd, rowNo, 1, name16, 511);

						HWND hStatusWnd = GetDlgItem(hWnd, IDC_DLG_STATUSBAR);
						SendMessage(hStatusWnd, SB_SETTEXT, 0, 0);

						sqlite3_stmt* stmt;
						if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
								"select printf('%s%s%s', '    Author: ' || author, '    Version: ' || version, '    Homepage: ' || homepage) " \
								"from temp.addons where name = ?1", -1, &stmt, 0)) {
							char* name8 = utils::utf16to8(name16);
							sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
							delete [] name8;

							if (SQLITE_ROW == sqlite3_step(stmt)) {
								TCHAR* info16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
								if (_tcslen(info16))
									SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)info16);
								delete [] info16;
							}
						}
						sqlite3_finalize(stmt);
					}
				}

				if (pHdr->code == (DWORD)LVN_ITEMCHANGED && pHdr->idFrom == IDC_DLG_ADDON_LIST && ListView_GetSelectedCount(pHdr->hwndFrom) == 0) {
					HWND hStatusWnd = GetDlgItem(hWnd, IDC_DLG_STATUSBAR);
					SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)(type == ADDON_SQLITE_EXTENSION ? TEXT(" Re-check an extension to update it") : TEXT(" Re-check a plugin to update it")));
				}

				if (pHdr->code == (DWORD)NM_DBLCLK && pHdr->idFrom == IDC_DLG_ADDON_LIST) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;

					TCHAR name16[512];
					ListView_GetItemText(pHdr->hwndFrom, ia->iItem, 1, name16, 511);

					char url8[1024];
					char* name8 = utils::utf16to8(name16);
					char* repository8 = (char*)GetProp(hWnd, TEXT("REPOSITORY8"));
					snprintf(url8, 1023, "/%s/main/%s/help.json", repository8, name8);
					delete [] name8;

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
							"select printf('%s%s%s%s%s', "\
							"'Author: ' || author || char(10), 'Version: ' || version || char(10), " \
							"'Homepage: ' || homepage || char(10), description || char(10), " \
							"iif(author is null and version is null and homepage is null and description is null, 'No information', ''))" \
							" || char(10) || coalesce(group_concat(" \
							"json_extract(js.value, '$.signature') || char(10) || " \
							"json_extract(js.value, '$.description') || char(10) || " \
							"coalesce(json_extract(js.value, '$.example'), '') " \
							", char(10) || char(10)), '') " \
							"from temp.addons, json_each(coalesce(?2, '[{}]')) js where name = ?1", -1, &stmt, 0)) {

						int readBytes = 0;
						DWORD statusCode = 0;
						char* name8 = utils::utf16to8(name16);
						char* buf8 = utils::httpRequest("GET", "raw.githubusercontent.com", url8, 0, &readBytes, &statusCode);
						sqlite3_bind_text(stmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);

						if (buf8 && strcmp(buf8, "[]") && statusCode == 200 ) {
							sqlite3_bind_text(stmt, 2, buf8, strlen(buf8), SQLITE_TRANSIENT);
						} else {
							sqlite3_bind_null(stmt, 2);
						}
						delete [] name8;
						if (buf8)
							delete [] buf8;

						if (SQLITE_ROW == sqlite3_step(stmt)) {
							TCHAR* info16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));

							TDlgParam dp = {
								info16,
								TEXT("Information")
							};

							DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_INFO), hWnd, (DLGPROC)dialogs::cbDlgInfo, (LPARAM)&dp);
							delete [] info16;
						}
					}

					sqlite3_finalize(stmt);
				}
			}
			break;

			case WM_CLOSE: {
				char* repository8 = (char*)GetProp(hWnd, TEXT("REPOSITORY8"));
				delete [] repository8;
				RemoveProp(hWnd, TEXT("REPOSITORY88"));

				EndDialog(hWnd, GetProp(hWnd, TEXT("CHANGED")) ? DLG_OK : DLG_CANCEL);
				RemoveProp(hWnd, TEXT("CHANGED"));
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgCustomFunctions (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TBBUTTON tbButtons [] = {
					{0, IDM_ADD, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Add")},
					{1, IDM_DELETE, 0, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Delete")},
					{2, IDM_SAVE, 0, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Save")},
					{3, IDM_TEST, 0, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Test")},
					{-1, IDM_LAST_SEPARATOR, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0L, 0},
					{4, IDM_HELP, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Help")},
				};

				HWND hToolbarWnd = CreateToolbarEx (hWnd, WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_LIST, IDC_DLG_TOOLBAR, 0, NULL, 0,
					tbButtons, sizeof(tbButtons)/sizeof(tbButtons[0]), 0, 0, 0, 0, sizeof (TBBUTTON));

				int idc = GetSystemMetrics(SM_CXSMICON) <= 16 ? IDB_TOOLBAR_FUNCTIONS16 : IDB_TOOLBAR_FUNCTIONS24;
				SendMessage(hToolbarWnd, TB_SETIMAGELIST, 0, (LPARAM)ImageList_LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(idc), 0, 0, RGB(255,255,255)));

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_FUNCTIONS);
				SendMessage(hListWnd, WM_SETFONT, (LPARAM)hFont, true);

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select id, name from functions order by name", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* name16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
						int no = ListBox_AddString(hListWnd, name16);
						ListBox_SetItemData(hListWnd, no, sqlite3_column_int(stmt, 0));
						delete [] name16;
					}
				}
				sqlite3_finalize(stmt);
				// USERDATA = prev selection row
				SetWindowLongPtr(hListWnd, GWLP_USERDATA, -1);

				HWND hNameWnd = GetDlgItem(hWnd, IDC_DLG_NAME);
				HWND hCodeWnd = GetDlgItem(hWnd, IDC_DLG_CODE);
				SetProp(hCodeWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hCodeWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewEditor));
				setEditorFont(hCodeWnd);
				SendMessage(hCodeWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_KEYEVENTS);

				HWND hHelpWnd = GetDlgItem(hWnd, IDC_DLG_HELP);
				setEditorFont(hHelpWnd);
				SendMessage(hHelpWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_KEYEVENTS);

				EnableWindow(hNameWnd, FALSE);
				EnableWindow(hCodeWnd, FALSE);
				EnableWindow(hHelpWnd, FALSE);
				utils::alignDialog(hWnd, hMainWnd, true);
			}
			break;

			case WM_SIZE: {
				HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
				SendMessage(hToolbarWnd, WM_SIZE, 0, 0);

				POINTFLOAT s = utils::getDlgScale(hWnd);
				int editH = utils::getEditHeight(hWnd);

				RECT rc, trc;
				GetClientRect(hWnd, &rc);
				GetWindowRect(hToolbarWnd, &trc);
				int th = trc.bottom - trc.top;
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_FUNCTIONS), 0, 5 * s.x, th + 5 * s.y, 150 * s.x, rc.bottom - th - 2 * 5 * s.x, SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_NAME_LABEL), 0, 160 * s.x, th + 6 * s.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_NAME), 0, 185 * s.x, th + 5 * s.y, rc.right - 190 * s.x, editH, SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_CODE_LABEL), 0, 160 * s.x, th + 25 * s.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_CODE), 0, 160 * s.x, th + 35 * s.y, rc.right - 165 * s.x, rc.bottom - th - 118 * s.y, SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_HELP_LABEL), 0, 160 * s.x, rc.bottom - 73 * s.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_HELP), 0, 160 * s.x, rc.bottom - 64 * s.y, rc.right - 165 * s.x, 60 * s.y, SWP_NOZORDER);
			}
			break;

			case WM_CLOSE: {
				HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_FUNCTIONS);

				if (SendMessage(hToolbarWnd, TB_GETSTATE, IDM_SAVE, 0) & TBSTATE_ENABLED &&
					IDYES == MessageBox(hWnd, TEXT("Save changes?"), TEXT("Confirmation"), MB_YESNO)) {
					SendMessage(hWnd, WMU_FUNCTION_SAVE, ListBox_GetCurSel(hListWnd), 0);
				}
			}
			break;

			case WM_CONTEXTMENU: {
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				bool isContextKey = p.x == -1 && p.y == -1;
				if ((HWND)wParam == GetDlgItem(hWnd, IDC_DLG_CODE) && !isContextKey)
					TrackPopupMenu(hEditorMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
			}
			break;

			case WM_COMMAND: {
				HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
				HWND hCodeWnd = GetDlgItem(hWnd, IDC_DLG_CODE);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_FUNCTIONS);

				if (LOWORD(wParam) == IDC_DLG_FUNCTIONS && HIWORD(wParam) == LBN_SELCHANGE) {
					int curr = ListBox_GetCurSel(hListWnd);
					int prev = (int)GetWindowLongPtr(hListWnd, GWLP_USERDATA);

					int currId = ListBox_GetItemData(hListWnd, curr);
					int prevId = ListBox_GetItemData(hListWnd, prev);

					SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);
					if (prev != -1 &&
							prevId != currId && SendMessage(hToolbarWnd, TB_GETSTATE, IDM_SAVE, 0) & TBSTATE_ENABLED &&
							IDYES == MessageBox(hWnd, TEXT("Save changes?"), TEXT("Confirmation"), MB_YESNO)) {
						SendMessage(hWnd, WMU_FUNCTION_SAVE, prev, 0);
						if ((SendMessage(hToolbarWnd, TB_GETSTATE, IDM_SAVE, 0) & TBSTATE_ENABLED) == 0) {
							ListBox_SetCurSel(hListWnd, curr);
							SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_DLG_FUNCTIONS, LBN_SELCHANGE), (LPARAM)hListWnd);
						} else {
							SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
							return 0;
						}
					}
					SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select name, code, help from functions where id = ?1", -1, &stmt, 0)) {
						sqlite3_bind_int(stmt, 1, currId);
						sqlite3_step(stmt);
						TCHAR* name16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
						TCHAR* code16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
						TCHAR* help16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 2));
						SetDlgItemText(hWnd, IDC_DLG_NAME, name16);
						SetDlgItemText(hWnd, IDC_DLG_CODE, code16);
						SetDlgItemText(hWnd, IDC_DLG_HELP, help16);
						delete [] name16;
						delete [] code16;
						delete [] help16;
					}
					sqlite3_finalize(stmt);

					Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, 0);
					Toolbar_SetButtonState(hToolbarWnd, IDM_DELETE, TBSTATE_ENABLED);
					Toolbar_SetButtonState(hToolbarWnd, IDM_TEST, TBSTATE_ENABLED);

					EnableWindow(GetDlgItem(hWnd, IDC_DLG_NAME), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_DLG_CODE), TRUE);
					EnableScrollBar(GetDlgItem(hWnd, IDC_DLG_CODE), SB_BOTH, ESB_ENABLE_BOTH);
					EnableWindow(GetDlgItem(hWnd, IDC_DLG_HELP), TRUE);

					SetWindowLongPtr(hListWnd, GWLP_USERDATA, curr);
				}

				if (wParam == IDM_ADD) {
					int prev = ListBox_GetCurSel(hListWnd);
					if (prev != -1 && (SendMessage(hToolbarWnd, TB_GETSTATE, IDM_SAVE, 0) & TBSTATE_ENABLED)) {
						if (IDYES == MessageBox(hWnd, TEXT("Save changes?"), TEXT("Confirmation"), MB_YESNO)) {
							SendMessage(hWnd, WMU_FUNCTION_SAVE, prev, 0);
							if (SendMessage(hToolbarWnd, TB_GETSTATE, IDM_SAVE, 0) & TBSTATE_ENABLED)
								return false;
						}
					}

					ListBox_SetCurSel(hListWnd, -1);

					SetDlgItemText(hWnd, IDC_DLG_NAME, NULL);
					SetDlgItemText(hWnd, IDC_DLG_CODE, NULL);
					SetDlgItemText(hWnd, IDC_DLG_HELP, NULL);

					EnableWindow(GetDlgItem(hWnd, IDC_DLG_NAME), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_DLG_CODE), TRUE);
					EnableScrollBar(GetDlgItem(hWnd, IDC_DLG_CODE), SB_BOTH, ESB_ENABLE_BOTH);
					EnableWindow(GetDlgItem(hWnd, IDC_DLG_HELP), TRUE);

					Toolbar_SetButtonState(hToolbarWnd, IDM_DELETE, 0);
					Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, 0);
					SetFocus(GetDlgItem(hWnd, IDC_DLG_NAME));
				}

				if (wParam == IDM_DELETE) {
					int pos = ListBox_GetCurSel(hListWnd);
					if (pos == -1)
						return false;

					if (IDYES != MessageBox(hWnd, TEXT("Are you sure you want to delete?"), TEXT("Confirmation"), MB_YESNO))
						return false;


					int id = ListBox_GetItemData(hListWnd, pos);
					ListBox_DeleteString(hListWnd, pos);

					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "delete from functions where id = ?1 returning name", -1, &stmt, 0)) {
						sqlite3_bind_int(stmt, 1, id);
						sqlite3_step(stmt);
					}
					sqlite3_finalize(stmt);

					ListBox_SetCurSel(hListWnd, -1);
					SetWindowLongPtr(hListWnd, GWLP_USERDATA, -1);

					SetDlgItemText(hWnd, IDC_DLG_NAME, NULL);
					SetDlgItemText(hWnd, IDC_DLG_CODE, NULL);
					SetDlgItemText(hWnd, IDC_DLG_HELP, NULL);

					EnableWindow(GetDlgItem(hWnd, IDC_DLG_NAME), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_DLG_CODE), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_DLG_HELP), FALSE);

					Toolbar_SetButtonState(hToolbarWnd, IDM_DELETE, 0);
					Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, 0);
					Toolbar_SetButtonState(hToolbarWnd, IDM_TEST, 0);
				}

				if (wParam == IDM_SAVE) {
					int curr = ListBox_GetCurSel(hListWnd);
					SendMessage(hWnd, WMU_FUNCTION_SAVE, curr, 0);
				}

				if (wParam == IDM_TEST) {
					TCHAR code16[MAX_TEXT_LENGTH + 1]{0};
					GetDlgItemText(hWnd, IDC_DLG_CODE, code16, MAX_TEXT_LENGTH);

					char* code8 = utils::utf16to8(code16);
					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, code8, -1, &stmt, 0)) {
						if (sqlite3_bind_parameter_count(stmt))
							DialogBoxParam (GetModuleHandle(0), MAKEINTRESOURCE(IDD_BIND_PARAMETERS), hWnd, (DLGPROC)cbDlgBindParameters, (LPARAM)stmt);
						int rc = sqlite3_step(stmt);
						if (rc == SQLITE_ROW) {
							TCHAR* value16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
							MessageBox(hWnd, value16, TEXT("Result"), MB_OK);
							delete [] value16;
						} else if (rc == SQLITE_DONE)
							MessageBox(hWnd, TEXT("Done"), TEXT("Result"), MB_OK);
						else {
							showDbError(hWnd);
						}
					} else {
						showDbError(hWnd);
					}
					sqlite3_finalize(stmt);
					delete [] code8;
				}

				if (wParam == IDM_HELP) {
					TCHAR buf[MAX_TEXT_LENGTH];
					LoadString(GetModuleHandle(NULL), IDS_FUNCTIONS_HELP, buf, MAX_TEXT_LENGTH);
					MessageBox(hWnd, buf, TEXT("User-defined functions"), MB_OK);
				}

				if ((LOWORD(wParam) == IDC_DLG_NAME || LOWORD(wParam) == IDC_DLG_CODE || LOWORD(wParam) == IDC_DLG_HELP) && HIWORD(wParam) == EN_CHANGE) {
					Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, TBSTATE_ENABLED);
					Toolbar_SetButtonState(hToolbarWnd, IDM_TEST, TBSTATE_ENABLED);
				}

				if ((LOWORD(wParam) == IDC_DLG_CODE  || LOWORD(wParam) == IDC_DLG_HELP) && HIWORD(wParam) == EN_CHANGE)
					SendMessage((HWND)lParam, WMU_TEXT_CHANGED, 0, 0);

				if (wParam == IDM_EDITOR_COMMENT)
					toggleComment(hCodeWnd);

				if (wParam == IDM_EDITOR_CUT)
					SendMessage(hCodeWnd, WM_CUT, 0, 0);

				if (wParam == IDM_EDITOR_COPY)
					SendMessage(hCodeWnd, WM_COPY, 0, 0);

				if (wParam == IDM_EDITOR_PASTE)
					pasteText(hCodeWnd);

				if (wParam == IDM_EDITOR_DELETE)
					SendMessage(hCodeWnd, EM_REPLACESEL, TRUE, 0);

				if (wParam == IDM_EDITOR_FIND) {
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_FIND), hWnd, (DLGPROC)cbDlgFind, (LPARAM)hCodeWnd);
					SetForegroundWindow(hWnd);
					SetFocus(hCodeWnd);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (wParam == IDC_DLG_CODE && pHdr->code == EN_SELCHANGE)
					return SendMessage(pHdr->hwndFrom, WMU_SELECTION_CHANGED, wParam, lParam);

				if (wParam == IDC_DLG_CODE && pHdr->code == EN_MSGFILTER)
					return processEditorEvents((MSGFILTER*)lParam);
			}
			break;

			// wParam = pos
			case WMU_FUNCTION_SAVE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_FUNCTIONS);

				int pos = wParam;
				int id = pos != -1 ? ListBox_GetItemData(hListWnd, pos) : 0;

				TCHAR name16[256]{0};
				GetDlgItemText(hWnd, IDC_DLG_NAME, name16, 255);
				if (_tcslen(name16) == 0) {
					MessageBox(hWnd, TEXT("The name is missing"), TEXT("Error"), MB_OK);
					return false;
				}

				if (_istdigit(name16[0])) {
					MessageBox(hWnd, TEXT("The name must begin with a letter, _ or $"), TEXT("Error"), MB_OK);
					return false;
				}

				for (int i = 0; i < (int)_tcslen(name16); i++) {
					if (_istalnum(name16[i]) || name16[i] == TEXT('_') || name16[i] == TEXT('$'))
						continue;

					MessageBox(hWnd, TEXT("Only letters, _, 0 - 9 or $ are allowed in the name"), TEXT("Error"), MB_OK);
					return false;
				}

				TCHAR code16[MAX_TEXT_LENGTH + 1]{0};
				GetDlgItemText(hWnd, IDC_DLG_CODE, code16, MAX_TEXT_LENGTH);

				TCHAR help16[MAX_TEXT_LENGTH + 1]{0};
				GetDlgItemText(hWnd, IDC_DLG_HELP, help16, MAX_TEXT_LENGTH);

				sqlite3_stmt* stmt;
				bool isCodeValid = SQLITE_OK == sqlite3_prepare16_v2(db, code16, -1, &stmt, 0);
				sqlite3_finalize(stmt);

				if (!isCodeValid) {
					MessageBox(hWnd, TEXT("The code is invalid"), TEXT("Error"), MB_OK);
					return false;
				}

				bool rc = SQLITE_OK == sqlite3_prepare_v2(prefs::db, "replace into functions (id, name, code, help) values (?1, ?2, ?3, ?4) returning id", -1, &stmt, 0);
				if (rc) {
					char* name8 = utils::utf16to8(name16);
					char* code8 = utils::utf16to8(code16);
					char* help8 = utils::utf16to8(help16);
					if (id)
						sqlite3_bind_int(stmt, 1, id);
					sqlite3_bind_text(stmt, 2, name8, strlen(name8), SQLITE_TRANSIENT);
					sqlite3_bind_text(stmt, 3, code8, strlen(code8), SQLITE_TRANSIENT);
					sqlite3_bind_text(stmt, 4, help8, strlen(help8), SQLITE_TRANSIENT);

					rc = SQLITE_ROW == sqlite3_step(stmt);
					if (rc) {
						HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);

						Toolbar_SetButtonState(hToolbarWnd, IDM_DELETE, TBSTATE_ENABLED);
						Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, 0);
						ListBox_DeleteString(hListWnd, pos);
						pos = ListBox_InsertString(hListWnd, pos, name16);
						ListBox_SetItemData(hListWnd, pos, sqlite3_column_int(stmt, 0));
						ListBox_SetCurSel(hListWnd, pos);
						InvalidateRect(hListWnd, NULL, FALSE);

						SetWindowLongPtr(hListWnd, GWLP_USERDATA, pos);
					} else {
						TCHAR* err16 = utils::utf8to16(sqlite3_errmsg(prefs::db));
						MessageBox(hWnd, err16, NULL, 0);
						delete [] err16;
					}
					delete [] name8;
					delete [] code8;
					delete [] help8;
				}
				sqlite3_finalize(stmt);

				return rc;
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgReferences (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TBBUTTON tbButtons [] = {
					{0, IDM_ADD, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Add")},
					{1, IDM_DELETE, 0, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Delete")},
					{2, IDM_SAVE, 0, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Save")},
					{3, IDM_TEST, 0, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Test")},
					{-1, IDM_LAST_SEPARATOR, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0L, 0},
					{4, IDM_HELP, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Help")},
				};

				HWND hToolbarWnd = CreateToolbarEx (hWnd, WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_LIST, IDC_DLG_TOOLBAR, 0, NULL, 0,
					tbButtons, sizeof(tbButtons)/sizeof(tbButtons[0]), 0, 0, 0, 0, sizeof (TBBUTTON));

				int idc = GetSystemMetrics(SM_CXSMICON) <= 16 ? IDB_TOOLBAR_FUNCTIONS16 : IDB_TOOLBAR_FUNCTIONS24;
				SendMessage(hToolbarWnd, TB_SETIMAGELIST, 0, (LPARAM)ImageList_LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(idc), 0, 0, RGB(255,255,255)));

				char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));
				SetProp(hWnd, TEXT("DBNAME8"), dbname8);

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_REFERENCES);
				HWND hTableWnd = GetDlgItem(hWnd, IDC_DLG_TABLENAME);
				HWND hColumnWnd = GetDlgItem(hWnd, IDC_DLG_COLNAME);
				HWND hTitleWnd = GetDlgItem(hWnd, IDC_DLG_TITLE);
				HWND hQueryWnd = GetDlgItem(hWnd, IDC_DLG_QUERY);

				SendMessage(hListWnd, WM_SETFONT, (LPARAM)hFont, true);
				SetWindowLongPtr(hListWnd, GWLP_USERDATA, -1);
				SendMessage(hWnd, WMU_UPDATE_REFERENCES, 0, 0);

				setEditorFont(hQueryWnd);
				SetProp(hQueryWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hQueryWnd, GWLP_WNDPROC, (LONG_PTR)&cbNewEditor));
				setEditorFont(hQueryWnd);
				SendMessage(hQueryWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_KEYEVENTS);

				EnableWindow(hTableWnd, FALSE);
				EnableWindow(hColumnWnd, FALSE);
				EnableWindow(hTitleWnd, FALSE);
				EnableWindow(hQueryWnd, FALSE);
				utils::alignDialog(hWnd, hMainWnd, true);
			}
			break;

			case WM_SIZE: {
				HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
				SendMessage(hToolbarWnd, WM_SIZE, 0, 0);

				POINTFLOAT s = utils::getDlgScale(hWnd);
				int editH = utils::getEditHeight(hWnd);

				RECT rc, trc;
				GetClientRect(hWnd, &rc);
				GetWindowRect(hToolbarWnd, &trc);
				int th = trc.bottom - trc.top;
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_REFERENCES), 0, 5 * s.x, th + 5 * s.y, 150 * s.x, rc.bottom - th - 2 * 5 * s.x, SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_TABLENAME_LABEL), 0, 160 * s.x, th + 7 * s.y, 25 * s.x, editH, SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_TABLENAME), 0, 185 * s.x, th + 5 * s.y, 100 * s.x, editH, SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_COLNAME_LABEL), 0, 300 * s.x, th + 7 * s.y, 25 * s.x, editH, SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_COLNAME), 0, 330 * s.x, th + 5 * s.y, 100 * s.x, editH, SWP_NOZORDER);

				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_TITLE_LABEL), 0, 160 * s.x, th + 24 * s.y, 25 * s.x, editH, SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_TITLE), 0, 185 * s.x, th + 22 * s.y, rc.right - 190 * s.x, editH, SWP_NOZORDER);

				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_QUERY_LABEL), 0, 160 * s.x, th + 45 * s.y, 25 * s.x, editH, SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_QUERY), 0, 160 * s.x, th + 57 * s.y, rc.right - 165 * s.x, rc.bottom - th - 61 * s.y, SWP_NOZORDER);
			}
			break;

			case WM_CLOSE: {
				HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_REFERENCES);

				if (SendMessage(hToolbarWnd, TB_GETSTATE, IDM_SAVE, 0) & TBSTATE_ENABLED &&
					IDYES == MessageBox(hWnd, TEXT("Save changes?"), TEXT("Confirmation"), MB_YESNO)) {
					SendMessage(hWnd, WMU_FUNCTION_SAVE, ListBox_GetCurSel(hListWnd), 0);
				}

				char* dbname8 = (char*)GetProp(hWnd, TEXT("DBNAME8"));
				delete [] dbname8;
				RemoveProp(hWnd, TEXT("DBNAME8"));
			}
			break;

			case WM_CONTEXTMENU: {
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				bool isContextKey = p.x == -1 && p.y == -1;
				if ((HWND)wParam == GetDlgItem(hWnd, IDC_DLG_CODE) && !isContextKey)
					TrackPopupMenu(hEditorMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
			}
			break;

			case WM_COMMAND: {
				HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_REFERENCES);
				HWND hTableWnd = GetDlgItem(hWnd, IDC_DLG_TABLENAME);
				HWND hColumnWnd = GetDlgItem(hWnd, IDC_DLG_COLNAME);
				HWND hTitleWnd = GetDlgItem(hWnd, IDC_DLG_TITLE);
				HWND hQueryWnd = GetDlgItem(hWnd, IDC_DLG_QUERY);

				if (LOWORD(wParam) == IDC_DLG_REFERENCES && HIWORD(wParam) == LBN_SELCHANGE) {
					int rowNo = ListBox_GetCurSel(hListWnd);
					int prevRowNo = GetWindowLongPtr(hListWnd, GWLP_USERDATA);
					if (prevRowNo != -1 && prevRowNo != rowNo &&
						SendMessage(hToolbarWnd, TB_GETSTATE, IDM_SAVE, 0) & TBSTATE_ENABLED &&
						IDYES == MessageBox(hWnd, TEXT("Save changes?"), TEXT("Confirmation"), MB_YESNO)) {
						if (!SendMessage(hWnd, WMU_SAVE_REFERENCE, prevRowNo, 0))
							return false;
					}

					int len = ListBox_GetTextLen(hListWnd, rowNo);
					TCHAR name16[len + 1]{0};
					ListBox_GetText(hListWnd, rowNo, name16);
					int pos = ListBox_GetItemData(hListWnd, rowNo);
					name16[pos] = 0;

					ComboBox_ResetContent(hTableWnd);
					ComboBox_AddString(hTableWnd, name16);
					ComboBox_SetCurSel(hTableWnd, 0);

					ComboBox_ResetContent(hColumnWnd);
					ComboBox_AddString(hColumnWnd, name16 + pos + 1);
					ComboBox_SetCurSel(hColumnWnd, 0);

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select query, refname from refs where tblname = ?1 and colname = ?2 and dbname = ?3", -1, &stmt, 0)) {
						char* tblname8 = utils::utf16to8(name16);
						sqlite3_bind_text(stmt, 1, tblname8, -1, SQLITE_TRANSIENT);
						delete [] tblname8;

						char* colname8 = utils::utf16to8(name16 + pos + 1);
						sqlite3_bind_text(stmt, 2, colname8, -1, SQLITE_TRANSIENT);
						delete [] colname8;

						sqlite3_bind_text(stmt, 3, (char*)GetProp(hWnd, TEXT("DBNAME8")), -1, SQLITE_TRANSIENT);

						sqlite3_step(stmt);

						TCHAR* query16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
						TCHAR* title16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
						SetDlgItemText(hWnd, IDC_DLG_QUERY, query16);
						SetDlgItemText(hWnd, IDC_DLG_TITLE, title16);
						delete [] query16;
						delete [] title16;
					}
					sqlite3_finalize(stmt);

					Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, 0);
					Toolbar_SetButtonState(hToolbarWnd, IDM_DELETE, TBSTATE_ENABLED);
					Toolbar_SetButtonState(hToolbarWnd, IDM_TEST, TBSTATE_ENABLED);

					EnableWindow(hTableWnd, FALSE);
					EnableWindow(hColumnWnd, FALSE);
					EnableWindow(hTitleWnd, TRUE);
					EnableWindow(hQueryWnd, TRUE);
					EnableScrollBar(hQueryWnd, SB_BOTH, ESB_ENABLE_BOTH);

					SetWindowLongPtr(hListWnd, GWLP_USERDATA, rowNo);
				}

				if (wParam == IDM_ADD) {
					int prevRowNo = ListBox_GetCurSel(hListWnd);
					if (prevRowNo != -1 && (SendMessage(hToolbarWnd, TB_GETSTATE, IDM_SAVE, 0) & TBSTATE_ENABLED)) {
						if (IDYES == MessageBox(hWnd, TEXT("Save changes?"), TEXT("Confirmation"), MB_YESNO)) {
							SendMessage(hWnd, WMU_SAVE_REFERENCE, prevRowNo, 0);
							if (SendMessage(hToolbarWnd, TB_GETSTATE, IDM_SAVE, 0) & TBSTATE_ENABLED)
								return false;
						}
					}

					ListBox_SetCurSel(hListWnd, -1);
					SetWindowLongPtr(hListWnd, GWLP_USERDATA, -1);

					int idcs [] = {IDC_DLG_TABLENAME, IDC_DLG_COLNAME, IDC_DLG_TITLE, IDC_DLG_QUERY};
					for (int idc : idcs) {
						EnableWindow(GetDlgItem(hWnd, idc), TRUE);
						SetDlgItemText(hWnd, idc, NULL);
					}

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, "select name from sqlite_master where type in ('table', 'view') order by type, name", -1, &stmt, 0)) {
						ComboBox_ResetContent(hTableWnd);
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							TCHAR* tblname16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
							ComboBox_AddString(hTableWnd, tblname16);
							delete [] tblname16;
						}
					}
					sqlite3_finalize(stmt);
					ComboBox_SetCurSel(hTableWnd, 0);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_DLG_TABLENAME, CBN_SELCHANGE), (LPARAM)hTableWnd);

					Toolbar_SetButtonState(hToolbarWnd, IDM_DELETE, 0);
					Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, 0);
					SetFocus(GetDlgItem(hWnd, IDC_DLG_TITLE));
				}

				if (wParam == IDM_DELETE) {
					int rowNo = ListBox_GetCurSel(hListWnd);
					if (rowNo == -1)
						return false;

					if (IDYES != MessageBox(hWnd, TEXT("Are you sure you want to delete?"), TEXT("Confirmation"), MB_YESNO))
						return false;

					int len = ListBox_GetTextLen(hListWnd, rowNo);
					TCHAR name16[len + 1]{0};
					ListBox_GetText(hListWnd, rowNo, name16);
					int pos = ListBox_GetItemData(hListWnd, rowNo);
					name16[pos] = 0;

					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "delete from refs where tblname = ?1 and colname = ?2 and dbname = ?3", -1, &stmt, 0)) {
						char* tblname8 = utils::utf16to8(name16);
						sqlite3_bind_text(stmt, 1, tblname8, -1, SQLITE_TRANSIENT);
						delete [] tblname8;

						char* colname8 = utils::utf16to8(name16 + pos + 1);
						sqlite3_bind_text(stmt, 2, colname8, -1, SQLITE_TRANSIENT);
						delete [] colname8;

						sqlite3_bind_text(stmt, 3, (char*)GetProp(hWnd, TEXT("DBNAME8")), -1, SQLITE_TRANSIENT);

						if (SQLITE_DONE == sqlite3_step(stmt) && sqlite3_changes(prefs::db) == 1) {
							ListBox_DeleteString(hListWnd, rowNo);
							int idcs [] = {IDC_DLG_TABLENAME, IDC_DLG_COLNAME, IDC_DLG_TITLE, IDC_DLG_QUERY};
							for (int idc : idcs) {
								EnableWindow(GetDlgItem(hWnd, idc), FALSE);
								SetDlgItemText(hWnd, idc, NULL);
							}

							SetWindowLongPtr(hListWnd, GWLP_USERDATA, -1);
							ListBox_SetCurSel(hListWnd, -1);

							Toolbar_SetButtonState(hToolbarWnd, IDM_DELETE, 0);
							Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, 0);
							Toolbar_SetButtonState(hToolbarWnd, IDM_TEST, 0);
						} else {
							MessageBox(hWnd, TEXT("Can't remove the reference"), NULL, MB_YESNO);
						}
					}
					sqlite3_finalize(stmt);
				}

				if (wParam == IDM_SAVE) {
					int rowNo = ListBox_GetCurSel(hListWnd);
					SendMessage(hWnd, WMU_SAVE_REFERENCE, rowNo, 0);
				}

				if (wParam == IDM_TEST) {
					int len = GetWindowTextLength(hQueryWnd) + 1;
					TCHAR* query16 = new TCHAR[len + 1] {0};
					GetWindowText(hQueryWnd, query16, len + 1);

					char* query8 = utils::utf16to8(query16);
					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
						MessageBox(hWnd, sqlite3_bind_parameter_count(stmt) != 1 ? TEXT("Error: Query have to use one parameter") : TEXT("OK"), TEXT("Result"), MB_OK);
					} else {
						showDbError(hWnd);
					}
					sqlite3_finalize(stmt);

					delete [] query8;
					delete [] query16;
				}

				if (wParam == IDM_HELP) {
					TCHAR buf[MAX_TEXT_LENGTH];
					LoadString(GetModuleHandle(NULL), IDS_REFERENCES_HELP, buf, MAX_TEXT_LENGTH);
					MessageBox(hWnd, buf, TEXT("References"), MB_OK);
				}

				if ((LOWORD(wParam) == IDC_DLG_TITLE || LOWORD(wParam) == IDC_DLG_QUERY) && HIWORD(wParam) == EN_CHANGE) {
					Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, TBSTATE_ENABLED);
					Toolbar_SetButtonState(hToolbarWnd, IDM_TEST, TBSTATE_ENABLED);
				}

				if (LOWORD(wParam) == IDC_DLG_TABLENAME && HIWORD(wParam) == CBN_SELCHANGE) {
					sqlite3_stmt* stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, "select name, ?1 || '.' || name from pragma_table_xinfo(?1) order by cid", -1, &stmt, 0)) {
						int len = GetWindowTextLength(hTableWnd);
						TCHAR tblname16[len + 1]{0};
						GetWindowText(hTableWnd, tblname16, len + 1);
						char* tblname8 = utils::utf16to8(tblname16);
						sqlite3_bind_text(stmt, 1, tblname8, -1, SQLITE_TRANSIENT);
						delete [] tblname8;

						ComboBox_ResetContent(hColumnWnd);
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							TCHAR* ref16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
							if (ListBox_FindStringExact(hListWnd, -1, ref16) == LB_ERR) {
								TCHAR* colname16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
								ComboBox_AddString(hColumnWnd, colname16);
								delete [] colname16;
							}
							delete [] ref16;
						}
					}
					sqlite3_finalize(stmt);
					ComboBox_SetCurSel(hColumnWnd, 0);
				}

				if (LOWORD(wParam) == IDC_DLG_QUERY && HIWORD(wParam) == EN_CHANGE)
					SendMessage((HWND)lParam, WMU_TEXT_CHANGED, 0, 0);

				if (wParam == IDM_EDITOR_COMMENT)
					toggleComment(hQueryWnd);

				if (wParam == IDM_EDITOR_CUT)
					SendMessage(hQueryWnd, WM_CUT, 0, 0);

				if (wParam == IDM_EDITOR_COPY)
					SendMessage(hQueryWnd, WM_COPY, 0, 0);

				if (wParam == IDM_EDITOR_PASTE)
					pasteText(hQueryWnd);

				if (wParam == IDM_EDITOR_DELETE)
					SendMessage(hQueryWnd, EM_REPLACESEL, TRUE, 0);

				if (wParam == IDM_EDITOR_FIND) {
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_FIND), hWnd, (DLGPROC)cbDlgFind, (LPARAM)hQueryWnd);
					SetForegroundWindow(hWnd);
					SetFocus(hQueryWnd);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (wParam == IDC_DLG_QUERY && pHdr->code == EN_SELCHANGE)
					return SendMessage(pHdr->hwndFrom, WMU_SELECTION_CHANGED, wParam, lParam);

				if (wParam == IDC_DLG_QUERY && pHdr->code == EN_MSGFILTER)
					return processEditorEvents((MSGFILTER*)lParam);
			}
			break;

			// wParam = rowNo
			case WMU_SAVE_REFERENCE: {
				bool rc = false;

				if (GetWindowTextLength(GetDlgItem(hWnd, IDC_DLG_TABLENAME)) == 0 ||
					GetWindowTextLength(GetDlgItem(hWnd, IDC_DLG_COLNAME)) == 0) {
					MessageBox(hWnd, TEXT("Table and column are mandatory"), NULL, MB_OK);
					return rc;
				}

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
					"replace into refs (dbname, schema, tblname, colname, refname, query) values (?5, 'main', ?1, ?2, ?3, ?4) " \
					"returning tblname || '.' || colname", -1, &stmt, 0)) {
					int idcs [] = {IDC_DLG_TABLENAME, IDC_DLG_COLNAME, IDC_DLG_TITLE, IDC_DLG_QUERY};
					for (int i = 0; i < 4; i++) {
						HWND hPropWnd = GetDlgItem(hWnd, idcs[i]);
						int len = GetWindowTextLength(hPropWnd);
						TCHAR* value16 = new TCHAR[len + 1]{0};
						GetWindowText(hPropWnd, value16, len + 1);
						char* value8 = utils::utf16to8(value16);
						sqlite3_bind_text(stmt, i + 1, value8, -1, SQLITE_TRANSIENT);
						delete [] value8;
						delete [] value16;
					}
					sqlite3_bind_text(stmt, 5, (char*)GetProp(hWnd, TEXT("DBNAME8")), -1, SQLITE_TRANSIENT);

					if (SQLITE_ROW == sqlite3_step(stmt)) {
						rc = true;

						HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
						Toolbar_SetButtonState(hToolbarWnd, IDM_DELETE, TBSTATE_ENABLED);
						Toolbar_SetButtonState(hToolbarWnd, IDM_SAVE, 0);

						int rowNo = wParam;
						if (rowNo == -1) {
							SendMessage(hWnd, WMU_UPDATE_REFERENCES, 0, 0);

							HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_REFERENCES);
							TCHAR* ref16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
							rowNo = ListBox_FindStringExact(hListWnd, -1, ref16);
							ListBox_SetCurSel(hListWnd, rowNo);
							SetWindowLongPtr(hListWnd, GWLP_USERDATA, rowNo);
							delete [] ref16;

							SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_DLG_REFERENCES, LBN_SELCHANGE), (LPARAM)hListWnd);
						}
					} else {
						TCHAR* err16 = utils::utf8to16(sqlite3_errmsg(prefs::db));
						MessageBox(hWnd, err16, NULL, 0);
						delete [] err16;
					}
				}
				sqlite3_finalize(stmt);

				return rc;
			}
			break;

			case WMU_UPDATE_REFERENCES: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_REFERENCES);
				char* dbname8 = (char*)GetProp(hWnd, TEXT("DBNAME8"));

				ListBox_ResetContent(hListWnd);

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select tblname || '.' || colname, length(tblname), length(tblname) + length(colname), tblname, colname from refs where schema = 'main' and dbname = ?1 order by tblname, colname", -1, &stmt, 0)) {
					sqlite3_bind_text(stmt, 1, dbname8, -1, SQLITE_TRANSIENT);

					while (SQLITE_ROW == sqlite3_step(stmt)) {
						int len = sqlite3_column_int(stmt, 2) + 64;
						char query8[len];
						snprintf(query8, len, "select \"%s\" from \"%s\"", sqlite3_column_text(stmt, 4), sqlite3_column_text(stmt, 3));
						if (SQLITE_OK != sqlite3_exec(db, query8, 0, 0, 0))
							continue;

						TCHAR* name16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
						int rowNo = ListBox_AddString(hListWnd, name16);
						ListBox_SetItemData(hListWnd, rowNo, sqlite3_column_int(stmt, 1));
						delete [] name16;
					}
				}
				sqlite3_finalize(stmt);
			}
			break;
		}

		return false;
	}

	// lParam, USERDATA = hListWnd
	BOOL CALLBACK cbDlgResultFind (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_CASE_SENSITIVE), prefs::get("case-sensitive") ? BST_CHECKED : BST_UNCHECKED);

				HWND hComboxWnd	= GetDlgItem(hWnd, IDC_DLG_FIND_STRING);
				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select what from search_history where type = 3 order by time desc limit 20", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* what16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
						ComboBox_AddString(hComboxWnd, what16);
						delete [] what16;
					}
				}
				sqlite3_finalize(stmt);

				SetFocus(hComboxWnd);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_FIND || (wParam == IDOK && GetParent(GetFocus()) == GetDlgItem(hWnd, IDC_DLG_FIND_STRING))) {
					HWND hListWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
					TCHAR searchString[256];
					GetDlgItemText(hWnd, IDC_DLG_FIND_STRING, searchString, 255);

					prefs::set("case-sensitive", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_CASE_SENSITIVE)));

					SendMessage(hListWnd, WMU_RESULT_SEARCH, (WPARAM)searchString, 0);
					updateSearchSuggestions(GetDlgItem(hWnd, IDC_DLG_FIND_STRING), 3);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_CLOSE: {
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	// lParam, USERDATA = IN-OUT buffer
	BOOL CALLBACK cbDlgUriDbPath (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);

				HWND hComboxWnd	= GetDlgItem(hWnd, IDC_DLG_DATABASE);

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select path from recents where path like 'file:%' order by time desc limit 20", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* path16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
						ComboBox_AddString(hComboxWnd, path16);
						delete [] path16;
					}
				}
				sqlite3_finalize(stmt);

				SetFocus(hComboxWnd);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					TCHAR* path16 = (TCHAR*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

					TCHAR buf16[MAX_PATH + 1];
					HWND hComboxWnd	= GetDlgItem(hWnd, IDC_DLG_DATABASE);
					GetWindowText(hComboxWnd, buf16, MAX_PATH);
					TCHAR* trimmed16 = utils::trim(buf16);
					_sntprintf(path16, MAX_PATH, TEXT("%ls"), _tcslen(trimmed16) ? trimmed16 : TEXT("file::memory:?cache=shared"));
					delete [] trimmed16;
					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_CLOSE: {
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	// LPARAM = TDlgParam
	BOOL CALLBACK cbDlgInfo (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hInfoWnd = GetDlgItem(hWnd, IDC_DLG_INFO);
				TDlgParam* dp = (TDlgParam*)lParam;

				SendMessage(hInfoWnd, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
				SetWindowText(hInfoWnd, dp->s1);
				SetWindowText(hWnd, dp->s2);

				POINTFLOAT s = utils::getDlgScale(hWnd);

				RECT rc;
				GetClientRect(hInfoWnd, &rc);
				InflateRect(&rc, -20 * s.x, -5 * s.y);
				SendMessage(hInfoWnd, EM_SETRECT, 0, (LPARAM)&rc);

				utils::alignDialog(hWnd, hMainWnd);
				SetFocus(hInfoWnd);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK || wParam == IDOK || wParam == IDCANCEL)
					EndDialog(hWnd, 0);
			}
			break;

			case WM_CLOSE: {
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}


	#define CHART_LINES     0
	#define CHART_DOTS      1
	#define CHART_AREAS     2
	#define CHART_HISTOGRAM 3
	#define CHART_BARS      4
	#define CHART_NONE      5

	#define CHART_MAX  1.79769e+308
	#define CHART_NULL 0.00012003

	#define CHART_NUMBER 1
	#define CHART_DATE   2
	#define CHART_TEXT   3

	#define CHART_BORDER 40
	#define CHART_BARS_LEFT 150
	#define CHART_BAR_HEIGHT 20
	#define CHART_BAR_SPACE 3

	COLORREF COLORS[MAX_CHART_COLOR_COUNT] = {RGB(0, 0, 0), RGB(51, 34, 136), RGB(136, 204, 238), RGB(68, 170, 153), RGB(17, 119, 51), RGB(153, 153, 51), RGB(221, 204, 119), RGB(204, 102, 119), RGB(136, 34, 85), RGB(170, 68, 153)};
	HPEN hPens[MAX_CHART_COLOR_COUNT];
	HBRUSH hBrushes[MAX_CHART_COLOR_COUNT];

	struct qsortItem {
		int rowNo;
		double value;
	};

	int qsortComparator (const void *i, const void *j) {
		double s = ((qsortItem*)i)->value - ((qsortItem*)j)->value;
		return s > 0 ? 1 : s < 0 ? -1 : 0;
	}

	double map (double x, double in_min, double in_max, double out_min, double out_max) {
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}

	void drawChart(HWND hChartWnd, HDC hdc, int w, int h, int _scrollY) {
		HWND hWnd = GetParent(hChartWnd);
		HWND hOptionsWnd = GetDlgItem(hWnd, IDC_DLG_CHART_OPTIONS);

		int type = (int)(LONG_PTR)GetProp(hWnd, TEXT("TYPE"));
		double* data = (double*)GetProp(hWnd, TEXT("DATA"));
		int colCount = (int)(LONG_PTR)GetProp(hWnd, TEXT("COLCOUNT"));
		int rowCount = (int)(LONG_PTR)GetProp(hWnd, TEXT("ROWCOUNT"));
		double* minmax = (double*)GetProp(hWnd, TEXT("MINMAX"));
		double minX = minmax[0];
		double maxX = minmax[1];
		double minY = minmax[2];
		double maxY = minmax[3];
		bool* isColumns = (bool*)GetProp(hWnd, TEXT("ISCOLUMNS"));
		int* colTypes = (int*)GetProp(hWnd, TEXT("COLTYPES"));
		BYTE* colColors = (BYTE*)GetProp(hWnd, TEXT("COLCOLORS"));

		int colBase = (int)(LONG_PTR)GetProp(hWnd, TEXT("COLBASE"));
		int colBaseType = colTypes[colBase];
		bool isExport = _scrollY < 0;
		int scrollY = _scrollY < 0 ? 0 : _scrollY;

		if (type == CHART_AREAS)
			minY = MIN(0, minY);

		if (type == CHART_HISTOGRAM)
			minY = 0;

		HWND hListWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		HWND hHeader = ListView_GetHeader(hListWnd);

		SelectFont(hdc, hFont);
		RECT rc = {0, 0, w, h};
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

		int dataColCount = 0;
		int groupColCount = 0;
		for (int colNo = 0; colNo < colCount; colNo++) {
			dataColCount += isColumns[colNo];
			groupColCount += colTypes[colNo] == CHART_DATE || colTypes[colNo] == CHART_TEXT;
		}

		if (dataColCount == 0 || (groupColCount == 0 && type == CHART_BARS))
			type = CHART_NONE;

		if (type == CHART_BARS) {
			HBRUSH hNullBrush = CreateSolidBrush(RGB(200, 200, 200));
			double minV = MIN(0, minY);
			double maxV = maxY;

			int barNo = 0;
			for (int colNo = 1; colNo < colCount; colNo++) {
				if (!isColumns[colNo] || colNo == colBase)
					continue;

				BYTE colorNo = colColors[colNo];

				for (int rowNo = 0; rowNo < rowCount; rowNo++) {
					double val = data[colNo + rowNo * colCount];
					bool isNull = val == CHART_NULL;

					// Attribute bar
					int top = CHART_BORDER/2 + rowNo * dataColCount * (CHART_BAR_HEIGHT + CHART_BAR_SPACE + 10) + (CHART_BAR_HEIGHT + CHART_BAR_SPACE) * barNo;
					int left = CHART_BARS_LEFT + map(0, minV, maxV, 0, w - CHART_BORDER - CHART_BARS_LEFT);
					int right = CHART_BARS_LEFT + (!isNull ? map(val, minV, maxV, 0, w - CHART_BORDER - CHART_BARS_LEFT) : left - CHART_BARS_LEFT + 40);
					if (left == right)
						right += 2;

					if (left > right) {
						int tmp = left;
						left = right;
						right = tmp;
					}

					SelectBrush(hdc, isNull ? hNullBrush : hBrushes[colorNo]);
					Rectangle(hdc, left, top - scrollY, right, top + CHART_BAR_HEIGHT - scrollY);

					// Value and title on bar
					TCHAR val16[64];
					if (isNull)
						_sntprintf(val16, 63, TEXT("N/A"));
					else
						_sntprintf(val16, 63, TEXT("%g"), data[colNo + rowNo * colCount]);
					SIZE s = {0};
					GetTextExtentPoint32(hdc, val16, _tcslen(val16), &s);

					bool isValueInside = (right - left > s.cx + 10) || (!isNull && val < 0);
					SetBkColor(hdc, isNull ? RGB(200, 200, 200) : isValueInside ? COLORS[colorNo] : RGB(255, 255, 255));
					SetTextColor(hdc, isValueInside ? RGB(255, 255, 255) : RGB(0, 0, 0));

					RECT rc {left + 10, top - scrollY, isValueInside ? right - 10 : right + 10 + s.cx, top + CHART_BAR_HEIGHT - scrollY};
					DrawText(hdc, val16, _tcslen(val16), &rc, (val > 0 ? DT_RIGHT : DT_LEFT) | DT_VCENTER | DT_SINGLELINE);

					if ((right - left > 60) && isExport) {
						TCHAR attr[256]{0};
						Header_GetItemText(hHeader, colNo, attr, 255);
						DrawText(hdc, attr, _tcslen(attr), &rc, (val > 0 ? DT_LEFT : DT_RIGHT) | DT_VCENTER | DT_SINGLELINE);
					}
				}
				barNo++;
			}

			// Group title
			for (int rowNo = 0; rowNo < rowCount; rowNo++) {
				TCHAR name[256]{0};
				ListView_GetItemText(hListWnd, rowNo, colBase, name, 255);
				if (_tcslen(name) == 0)
					_sntprintf(name, 10, TEXT("<<Empty>>"));

				SetTextColor(hdc, RGB(0, 0, 0));
				SetBkColor(hdc, RGB(255, 255, 255));

				RECT rc {0, 0, CHART_BARS_LEFT - CHART_BORDER - 10, 0};
				DrawText(hdc, name, _tcslen(name), &rc, DT_RIGHT | DT_MODIFYSTRING | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT);

				int top = CHART_BORDER/2 + rowNo * dataColCount * (CHART_BAR_HEIGHT + CHART_BAR_SPACE + 10) - scrollY;
				int middle = top + ((CHART_BAR_HEIGHT + CHART_BAR_SPACE) * dataColCount - CHART_BAR_SPACE) / 2;
				int h = rc.bottom - rc.top;

				rc = {CHART_BORDER, middle - h/2, CHART_BARS_LEFT - 10, middle + h/2};
				DrawText(hdc, name, _tcslen(name), &rc, DT_RIGHT | DT_MODIFYSTRING | DT_WORDBREAK | DT_NOCLIP);
			}

			DeleteObject(hNullBrush);
		}

		if (type == CHART_LINES || type == CHART_DOTS || type == CHART_AREAS || type == CHART_HISTOGRAM) {
			double zoom = PtrToInt(GetProp(hChartWnd, TEXT("ZOOM"))) / 10.0;
			minX /= zoom;
			maxX /= zoom;

			RECT rc{0};
			GetClientRect(hChartWnd, &rc);
			int w = rc.right;
			double offsetX = (double)PtrToInt(GetProp(hChartWnd, TEXT("POSITION")));
			offsetX = map(offsetX, 0, w - 2 * CHART_BORDER, 0, maxX - minX);

			minX -= offsetX;
			maxX -= offsetX;
		}

		// Grid
		if (type == CHART_LINES || type == CHART_DOTS || type == CHART_AREAS || type == CHART_HISTOGRAM) {
			HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
			SelectObject(hdc, hPen);

			// https://stackoverflow.com/a/18049477/6121703
			auto findDelta = [](float maxvalue, int count) {
				float step = maxvalue/count,
					 order = powf(10, floorf(log10(step))),
					 delta = (int)(step/order + 0.5);

				static float ndex[] = {1, 1.5, 2, 2.5, 5, 10};
				static int ndexLenght = sizeof(ndex)/sizeof(float);
				for (int i = ndexLenght - 2; i > 0; --i)
					if(delta > ndex[i]) return ndex[i + 1] * order;
				return delta * order;
			};

			int x = 0;
			double d = findDelta(maxX - minX, w / prefs::get("chart-grid-size-x"));
			if (colBaseType == CHART_DATE) {
				int DAY = 60 * 60 * 24;
				d = d < DAY / 24 ? DAY / 24 : floor(d / DAY) * DAY;
				for (int i = 0; minX + d * i < maxX + d; i++) {
					double x0 = minX + d * i;
					x = map(x0, minX, maxX, CHART_BORDER, w - CHART_BORDER);
					if (x > w - CHART_BORDER) {
						x0 = maxX;
						x = map(x0, minX, maxX, CHART_BORDER, w - CHART_BORDER);
					}

					MoveToEx(hdc, x, CHART_BORDER, NULL);
					LineTo(hdc, x, h - CHART_BORDER);

					TCHAR val[64];
					time_t rawtime = x0;
					struct tm ts = *localtime(&rawtime);
					_tcsftime(val, 64, d < DAY ? TEXT("%Y-%m-%d %H:%M") : TEXT("%Y-%m-%d"), &ts);

					RECT rc {x - 30, 10, x + 30, CHART_BORDER + 5};
					DrawText(hdc, val, _tcslen(val), &rc, DT_BOTTOM | DT_WORDBREAK | DT_CENTER | DT_CALCRECT);
					DrawText(hdc, val, _tcslen(val), &rc, DT_BOTTOM | DT_WORDBREAK | DT_CENTER);
					RECT rc2 {x - 30, h - CHART_BORDER + 10, x + 30, h};
					DrawText(hdc, val, _tcslen(val), &rc2, DT_TOP | DT_WORDBREAK | DT_CENTER | DT_CALCRECT);
					DrawText(hdc, val, _tcslen(val), &rc2, DT_TOP | DT_WORDBREAK | DT_CENTER);
				}
			} else {
				for (int i = 0; minX + d * i < maxX + d; i++) {
					double x0 = minX + d * i;
					x = map(x0, minX, maxX, CHART_BORDER, w - CHART_BORDER);
					if (x > w - CHART_BORDER) {
						x0 = maxX;
						x = map(x0, minX, maxX, CHART_BORDER, w - CHART_BORDER);
					}

					MoveToEx(hdc, x, CHART_BORDER, NULL);
					LineTo(hdc, x, h - CHART_BORDER);

					TCHAR val[64];
					_sntprintf(val, 63, TEXT("%g"), x0);

					RECT rc {x - 30, 0, x + 30, CHART_BORDER - 5};
					DrawText(hdc, val, _tcslen(val), &rc, DT_BOTTOM | DT_SINGLELINE | DT_CENTER | DT_CALCRECT);
					DrawText(hdc, val, _tcslen(val), &rc, DT_BOTTOM | DT_SINGLELINE | DT_CENTER);

					RECT rc2 {x - 30, h - CHART_BORDER + 5, x + 30, h};
					DrawText(hdc, val, _tcslen(val), &rc2, DT_TOP | DT_SINGLELINE | DT_CENTER | DT_CALCRECT);
					DrawText(hdc, val, _tcslen(val), &rc2, DT_TOP | DT_SINGLELINE | DT_CENTER);
				}
			}

			int y = 0;
			d = findDelta(maxY - minY, h / prefs::get("chart-grid-size-y"));
			for (int i = 0; minY + d * i < maxY + d; i++) {
				double y0 = minY + d * i;
				y = map(y0, minY, maxY, CHART_BORDER, h - CHART_BORDER);
				if (h - y < CHART_BORDER) {
					y0 = maxY;
					y = h - CHART_BORDER;
				}

				MoveToEx(hdc, CHART_BORDER, h - y, NULL);
				LineTo(hdc, x, h - y);

				TCHAR val[64];
				_sntprintf(val, 63, TEXT("%g"), y0);
				RECT rc {0, h - y - 10, CHART_BORDER - 5, h - y + 10};
				DrawText(hdc, val, _tcslen(val), &rc, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
				RECT rc2 {x + 5, h - y - 10, w, h - y + 8};
				DrawText(hdc, val, _tcslen(val), &rc2, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
			}

			DeleteObject(hPen);
		}

		// Graph
		int lineNo = 0;
		if (type == CHART_LINES || type == CHART_DOTS) {
			for (int colNo = 1; colNo < colCount; colNo++) {
				if (colNo == colBase)
					continue;

				BYTE colorNo = colColors[colNo];

				HWND hColumnWnd = GetDlgItem(hOptionsWnd, IDC_DLG_CHART_COLUMN + colNo);
				if (!IsWindowEnabled(hColumnWnd) || Button_GetCheck(hColumnWnd) != BST_CHECKED)
					continue;

				SelectObject(hdc, hPens[colorNo]);
				int pointCount = 0;

				for (int rowNo = 0; rowNo < rowCount; rowNo++) {
					double valX = data[colBase + rowNo * colCount];
					double valY = data[colNo + rowNo * colCount];

					if (valX == CHART_NULL || valY == CHART_NULL)
						continue;

					if (valX < minX || valX > maxX)
						continue;

					double x = map(valX, minX, maxX, CHART_BORDER, w - CHART_BORDER);
					double y = h - map(valY, minY, maxY, CHART_BORDER, h - CHART_BORDER);

					if (type == CHART_LINES) {
						if (!pointCount) {
							MoveToEx(hdc, x, y, NULL);
							pointCount++;
							continue;
						}
						LineTo(hdc, x, y);
						pointCount++;
					}

					if (type == CHART_DOTS) {
						int r = 3;
						SelectObject(hdc, hPens[colorNo]);
						SelectBrush(hdc, hBrushes[colorNo]);
						Ellipse(hdc, x - r, y - r, x + r, y + r);
						pointCount++;
					}
				}

				if (pointCount > 0)
					lineNo++;
			}
		}

		if (type == CHART_AREAS || type == CHART_HISTOGRAM) {
            HDC hTmpDC = CreateCompatibleDC(hdc);
            VOID *pvResBits, *pvTmpBits;
            HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
            HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));

			BITMAPINFO bmi{0};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = w;
			bmi.bmiHeader.biHeight = h;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * 4;

			HBITMAP hResBmp = CreateDIBSection(hTmpDC, &bmi, DIB_RGB_COLORS, &pvResBits, NULL, 0x0);
			SelectObject(hTmpDC, hResBmp);
			RECT r{0, 0, w, h};
			//BitBlt(hTmpDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
			FillRect(hTmpDC, &r, hWhiteBrush);

			HBITMAP hTmpBmp = CreateDIBSection(hTmpDC, &bmi, DIB_RGB_COLORS, &pvTmpBits, NULL, 0x0);
			SelectObject(hTmpDC, hTmpBmp);

			for (int colNo = 1; colNo < colCount; colNo++) {
				if (colNo == colBase)
					continue;

				BYTE colorNo = colColors[colNo];

				HWND hColumnWnd = GetDlgItem(hOptionsWnd, IDC_DLG_CHART_COLUMN + colNo);
				if (!IsWindowEnabled(hColumnWnd) || Button_GetCheck(hColumnWnd) != BST_CHECKED)
					continue;

				FillRect(hTmpDC, &r, hWhiteBrush);
				SelectObject(hTmpDC, hPens[colorNo]);
				SelectObject(hTmpDC, hBrushes[colorNo]);

				int pointCount = 0;
				double prevX = CHART_NULL;
				double prevY = CHART_NULL;
				double y0 = h - map(minY, minY, maxY, CHART_BORDER, h - CHART_BORDER);

				for (int rowNo = 0; rowNo < rowCount; rowNo++) {
					if (data[colNo + rowNo * colCount] == CHART_NULL)
						continue;

					if (data[colBase + rowNo * colCount] == CHART_NULL)
						continue;

					double x = map(data[colBase + rowNo * colCount], minX, maxX, CHART_BORDER, w - CHART_BORDER);
					double y = h - map(data[colNo + rowNo * colCount], minY, maxY, CHART_BORDER, h - CHART_BORDER);

					if (prevX != CHART_NULL) {
						if (type == CHART_AREAS) {
							POINT p[4] = {{(int)prevX, (int)y0}, {(int)prevX, (int)prevY}, {(int)x, (int)y}, {(int)x, (int)y0}};
							Polygon(hTmpDC, p, 4);
						}

						if (type == CHART_HISTOGRAM) {
							Rectangle(hTmpDC, prevX + 1, prevY + 2, x, y0);

							SelectObject(hTmpDC, hGridPen);
							MoveToEx(hTmpDC, x, y0, 0);
							LineTo(hTmpDC, x, y0);
						}
					}

					prevX = x;
					prevY = y;
					pointCount++;
				}

				if (pointCount > 0) {
					COLORREF color = COLORS[colorNo];
					DWORD c = ((BYTE)(GetBValue(color)) | ((BYTE)(GetGValue(color)) << 8) | ((BYTE)(GetRValue(color)) << 16));

					for (int y = 0; y < h; y++) {
						for (int x = 0; x < w; x++) {
							if (((UINT32 *)pvTmpBits)[x + y * w] == 0x00FFFFFF)
								continue;

							((UINT32 *)pvResBits)[x + y * w] =
								(((UINT32 *)pvResBits)[x + y * w] != 0x00FFFFFF ?
								utils::blend(((UINT32 *)pvResBits)[x + y * w], c, 64) : c) | 0xFF000000;
						}
					}
					lineNo++;
				}
			}

			SelectObject(hTmpDC, hResBmp);
			TransparentBlt(hdc, 0, 0, w, h, hTmpDC, 0, 0, w, h, RGB(255, 255, 255));
			//BitBlt(hdc, 0, 0, w, h, hTmpDC, 0, 0, SRCCOPY);

			DeleteObject(hTmpBmp);
			DeleteObject(hResBmp);
			DeleteDC(hTmpDC);

			DeleteObject(hWhiteBrush);
			DeleteObject(hGridPen);
		}

		// Legend
		if (isExport && (type == CHART_LINES || type == CHART_DOTS || type == CHART_AREAS || type == CHART_HISTOGRAM)) {
			HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
			lineNo = 0;
			for (int colNo = 1; colNo < colCount; colNo++) {
				if (colNo == colBase)
					continue;

				BYTE colorNo = colColors[colNo];

				HWND hColumnWnd = GetDlgItem(hOptionsWnd, IDC_DLG_CHART_COLUMN + colNo);
				if (!IsWindowEnabled(hColumnWnd) || Button_GetCheck(hColumnWnd) != BST_CHECKED)
					continue;

				TCHAR name16[256];
				Header_GetItemText(hHeader, colNo, name16, 255);

				int x = w - 100;
				int y = CHART_BORDER + 5 + lineNo * 15;
				SelectObject(hdc, hPen);
				SelectBrush(hdc, hBrushes[colorNo]);
				Ellipse(hdc, x - 16, y + 1, x - 4, y + 13);

				SetTextColor(hdc, RGB(0, 0, 0));
				TextOut(hdc, x, y, name16, _tcslen(name16));

				lineNo++;
			}
			DeleteObject(hPen);

			if (lineNo == 0)
				type = CHART_NONE;
		}

		if (type == CHART_NONE) {
			RECT rc = {0, 0, w, h}, rc2 = {0, 0, w, h};
			TCHAR text[] = TEXT("Not enough data to chart.\nVisit Wiki if you have questions.");
			DrawText(hdc, text, _tcslen(text), &rc2, DT_CALCRECT);
			rc.top = rc.bottom / 2 - rc2.bottom / 2;
			DrawText(hdc, text, _tcslen(text), &rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_EXPANDTABS);
		}
	}

	// lParam = hListWnd = USERDATA
	BOOL CALLBACK cbDlgChart (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
				for (int i = 0; i < MAX_CHART_COLOR_COUNT; i++) {
					hPens[i] = CreatePen(PS_SOLID, 2, COLORS[i]);
					hBrushes[i] = CreateSolidBrush(COLORS[i]);
				}

				HWND hListWnd = (HWND)lParam;
				HWND hHeader = ListView_GetHeader(hListWnd);
				int colCount = Header_GetItemCount(hHeader);
				int rowCount = ListView_GetItemCount(hListWnd);
				int size = colCount * rowCount;

				double* data = new double[size];
				for (int i = 0; i < size; i++)
					data[i] = CHART_NULL;
				for (int rowNo = 0; rowNo < rowCount; rowNo++)
					data[rowNo * colCount] = rowNo; // First column is a rowid and is used to sort a text column

				int* colTypes = new int[colCount]{0};
				for (int colNo = 1; colNo < colCount; colNo++) {
					int nNumber = 0;
					int nDate = 0;
					int nEmpty = 0;
					int nText = 0;
					for (int rowNo = 0; rowNo < MIN(rowCount, 10); rowNo++) {
						TCHAR buf16[256]{0};
						double val;
						ListView_GetItemText(hListWnd, rowNo, colNo, buf16, 255);
						if (_tcslen(buf16) == 0) {
							nEmpty++;
						} else if (utils::isNumber(buf16, &val)) {
							nNumber++;
						} else if (utils::isDate(buf16, &val)) {
							nDate++;
						} else {
							nText++;
						}
					}

					int nMax = MAX(MAX(nNumber, nDate), nText);
					colTypes[colNo] = nMax == nNumber ? CHART_NUMBER : nMax == nDate ? CHART_DATE : CHART_TEXT;
				}

				for (int colNo = 1; colNo < colCount; colNo++) {
					for (int rowNo = 0; rowNo < rowCount; rowNo++) {
						TCHAR buf16[256]{0};
						double val;
						ListView_GetItemText(hListWnd, rowNo, colNo, buf16, 255);
						if (_tcslen(buf16) > 0) {
							bool isNumber =	utils::isNumber(buf16, &val);
							bool isDate = !isNumber && utils::isDate(buf16, &val);
							data[colNo + rowNo * colCount] = isNumber || isDate ? val : CHART_NULL;
						}
					}
				}

				double *minmax = new double[4]{0};
				bool *isColumns = new bool[colCount]{0};
				BYTE *colColors = new BYTE[colCount]{0};

				SetProp(hWnd, TEXT("COLTYPES"), (HANDLE)colTypes);
				SetProp(hWnd, TEXT("COLCOLORS"), (HANDLE)colColors);
				SetProp(hWnd, TEXT("DATA"), (HANDLE)data); // <row><row>...<row>
				SetProp(hWnd, TEXT("MINMAX"), (HANDLE)minmax); // minX, maxX, minY, maxY
				SetProp(hWnd, TEXT("ISCOLUMNS"), (HANDLE)isColumns); // render or not
				SetProp(hWnd, TEXT("COLCOUNT"), IntToPtr(colCount)); // include one hidden
				SetProp(hWnd, TEXT("ROWCOUNT"), IntToPtr(rowCount));
				SetProp(hWnd, TEXT("TYPE"), (HANDLE)CHART_NONE);
				SetProp(hWnd, TEXT("COLBASE"), (HANDLE)0);

				SetProp(hWnd, TEXT("MENU"), (HANDLE)LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDC_MENU_CHART)));

				HWND hChartWnd = GetDlgItem(hWnd, IDC_DLG_CHART);
				SetWindowLongPtr(hChartWnd, GWLP_WNDPROC, (LONG_PTR)cbNewChart);

				HWND hOptionsWnd = GetDlgItem(hWnd, IDC_DLG_CHART_OPTIONS);
				HINSTANCE hInstance = GetModuleHandle(0);
				CreateWindow(WC_STATIC, TEXT("Type"), WS_VISIBLE | WS_CHILD, 10, 13, 50, 20, hOptionsWnd, NULL, hInstance, NULL);

				HWND hTypeWnd = CreateWindow(WC_COMBOBOX, NULL, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 60, 10, 122, 200, hOptionsWnd, (HMENU)IDC_DLG_CHART_TYPE, hInstance, NULL);
				const TCHAR* types[] = {TEXT("Lines"), TEXT("Dots"), TEXT("Areas"), TEXT("Histogram"), TEXT("Bars")};
				for (int i = 0; i < 5; i++) {
					int pos = ComboBox_AddString(hTypeWnd, types[i]);
					ComboBox_SetItemData(hTypeWnd, pos, i);
				}

				CreateWindow(WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ, 10, 40, 174, 1, hOptionsWnd, NULL, hInstance, NULL);
				CreateWindow(WC_STATIC, TEXT(""), WS_VISIBLE | WS_CHILD, 10, 53, 50, 20, hOptionsWnd, (HMENU)IDC_DLG_CHART_BASE_LABEL, hInstance, NULL);
				CreateWindow(WC_COMBOBOX, NULL, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 60, 50, 122, 200, hOptionsWnd, (HMENU)IDC_DLG_CHART_BASE, hInstance, NULL);

				for (int colNo = 1; colNo < colCount; colNo++) {
					TCHAR buf[256];
					Header_GetItemText(hHeader, colNo, buf, 255);
					HWND hBtnWnd = CreateWindow(WC_BUTTON, buf, BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 10, 60 + colNo * 25, 172, 19, hOptionsWnd, (HMENU)IntToPtr(IDC_DLG_CHART_COLUMN + colNo), hInstance, NULL);
					Button_SetCheck(hBtnWnd, BST_CHECKED);
				}

				EnumChildWindows(hOptionsWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETPARENTFONT);
				cbOldChartOptions = (WNDPROC)SetWindowLongPtr(hOptionsWnd, GWLP_WNDPROC, (LONG_PTR)cbNewChartOptions);

				ComboBox_SetCurSel(hTypeWnd, colTypes[1] == CHART_TEXT ? CHART_BARS : CHART_LINES);
				PostMessage(hOptionsWnd, WM_COMMAND, MAKEWPARAM(IDC_DLG_CHART_TYPE, CBN_SELCHANGE), (LPARAM)hTypeWnd);
			}
			break;

			case WM_DESTROY: {
				double* data = (double*)GetProp(hWnd, TEXT("DATA"));
				delete [] data;
				RemoveProp(hWnd, TEXT("DATA"));

				int* isColumns = (int*)GetProp(hWnd, TEXT("ISCOLUMNS"));
				delete [] isColumns;
				RemoveProp(hWnd, TEXT("ISCOLUMNS"));

				int* colTypes = (int*)GetProp(hWnd, TEXT("COLTYPES"));
				delete [] colTypes;
				RemoveProp(hWnd, TEXT("COLTYPES"));

				BYTE* colColors = (BYTE*)GetProp(hWnd, TEXT("COLCOLORS"));
				delete [] colColors;
				RemoveProp(hWnd, TEXT("COLCOLORS"));

				double* minmax = (double*)GetProp(hWnd, TEXT("MINMAX"));
				delete [] minmax;
				RemoveProp(hWnd, TEXT("MINMAX"));

				RemoveProp(hWnd, TEXT("COLCOUNT"));
				RemoveProp(hWnd, TEXT("ROWCOUNT"));
				RemoveProp(hWnd, TEXT("COLBASE"));
				RemoveProp(hWnd, TEXT("TYPE"));

				DestroyMenu((HMENU)GetProp(hWnd, TEXT("MENU")));
				RemoveProp(hWnd, TEXT("MENU"));

				for (int i = 0; i < MAX_CHART_COLOR_COUNT; i++) {
					DeleteObject(hPens[i]);
					DeleteObject(hBrushes[i]);
				}

				RemoveProp(hWnd, TEXT("SCROLLY"));
			}
			break;

			case WM_SIZE: {
				HWND hChartWnd = GetDlgItem(hWnd, IDC_DLG_CHART);
				HWND hOptionsWnd = GetDlgItem(hWnd, IDC_DLG_CHART_OPTIONS);
				int w = 190;
				SetWindowPos(hChartWnd, 0, 0, 0, LOWORD(lParam) - w, HIWORD(lParam), SWP_NOMOVE | SWP_NOZORDER);
				SetWindowPos(hOptionsWnd, 0, LOWORD(lParam) - w, 0, w, HIWORD(lParam), SWP_NOZORDER);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WMU_RESORT_DATA: {
				int colCount = (int)(LONG_PTR)GetProp(hWnd, TEXT("COLCOUNT"));
				int rowCount = (int)(LONG_PTR)GetProp(hWnd, TEXT("ROWCOUNT"));
				int size = colCount * rowCount;

				double* data = new double[size];
				for (int i = 0; i < size; i++)
					data[i] = CHART_NULL;

				double* srcData = (double*)GetProp(hWnd, TEXT("DATA"));
				int* colTypes = (int*)GetProp(hWnd, TEXT("COLTYPES"));
				int colBase = (int)(LONG_PTR)GetProp(hWnd, TEXT("COLBASE"));
				if (colTypes[colBase] == CHART_TEXT)
					colBase = 0;

				qsortItem axisX[rowCount];
				for (int rowNo = 0; rowNo < rowCount; rowNo++)
					axisX[rowNo] = {(int)srcData[rowNo * colCount + 0], srcData[rowNo * colCount + colBase]};

				qsort(axisX, rowCount, sizeof(qsortItem), qsortComparator);

				for (int rowNo = 0; rowNo < rowCount; rowNo++) {
					int rowIdx = 0;
					for (; (rowIdx < rowCount) && (srcData[rowIdx * colCount + 0] != axisX[rowNo].rowNo); rowIdx++);
					for (int barNo = 0; barNo < colCount; barNo++)
						data[barNo + rowNo * colCount] = srcData[barNo + rowIdx * colCount];
				}

				delete [] srcData;
				SetProp(hWnd, TEXT("DATA"), data);
			}
			break;

			case WMU_UPDATE_MINMAX: {
				int colCount = (int)(LONG_PTR)GetProp(hWnd, TEXT("COLCOUNT"));
				int rowCount = (int)(LONG_PTR)GetProp(hWnd, TEXT("ROWCOUNT"));

				bool* isColumns = (bool*)GetProp(hWnd, TEXT("ISCOLUMNS"));
				double* minmax = (double*)GetProp(hWnd, TEXT("MINMAX"));
				double* data = (double*)GetProp(hWnd, TEXT("DATA"));


				int colBase = (int)(LONG_PTR)GetProp(hWnd, TEXT("COLBASE"));
				double minX = CHART_MAX;
				double maxX = -CHART_MAX;
				double minY = CHART_MAX;
				double maxY = -CHART_MAX;

				int* colTypes = (int*)GetProp(hWnd, TEXT("COLTYPES"));
				for (int colNo = 0; colNo < colCount; colNo++) {
					if (colTypes[colNo] == CHART_TEXT)
							continue;

					if ((colNo != colBase) && !isColumns[colNo])
						continue;

					for (int rowNo = 0; rowNo < rowCount; rowNo++) {
						double val = data[rowNo * colCount + colNo];
						if (val != CHART_NULL) {
							minX = colNo == colBase ? MIN(val, minX) : minX;
							maxX = colNo == colBase ? MAX(val, maxX) : maxX;
							minY = colNo != colBase ? MIN(val, minY) : minY;
							maxY = colNo != colBase ? MAX(val, maxY) : maxY;
						}
					}
				}

				minmax[0] = minX;
				minmax[1] = maxX;

				float d = ceil(log10(maxY - minY));
				minmax[2] = floor(minY/d) * d;
				minmax[3] = ceil(maxY/d) * d;

				SendMessage(GetDlgItem(hWnd, IDC_DLG_CHART), WM_COMMAND, IDM_CHART_RESET, 0);
			}
			break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;
		}

		return false;
	}

	// lParam and USERDATA are sqlite3 statement handle
	BOOL CALLBACK cbDlgBindParameters (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)lParam);

				sqlite3_stmt* stmt = (sqlite3_stmt*)lParam;
				sqlite3_stmt* stmt2;
				sqlite3_prepare_v2(prefs::db, "select value from query_params where lower(dbname) = lower(?1) and lower(name) = lower(?2)", -1, &stmt2, 0);
				char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));

				POINTFLOAT s = utils::getDlgScale(hWnd);
				int editH = utils::getEditHeight(hWnd);
				int lineH = 15;

				int paramCount = sqlite3_bind_parameter_count(stmt);
				for (int i = 1; i < paramCount + 1; i++) {
					const char* name8 = (char*)sqlite3_bind_parameter_name(stmt, i);
					TCHAR* name16 = utils::utf8to16(name8);

					CreateWindow(WC_STATIC, _tcslen(name16) ? name16 : TEXT("(Unnamed)"), WS_VISIBLE | WS_CHILD | SS_RIGHT, 5 * s.x, 6 * s.y + lineH * (i - 1) * s.y, 95 * s.x, 12 * s.y, hWnd, (HMENU)IntToPtr(IDC_ROW_LABEL +  i), GetModuleHandle(0), 0);

					if (_tcslen(name16)) {
						HWND hComboWnd = CreateWindow(WC_COMBOBOX, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_TABSTOP | CBS_DROPDOWN, 105 * s.x, 5 * s.y + lineH * (i - 1) * s.y, 185 * s.x, 200 * s.y, hWnd, (HMENU)IntToPtr(IDC_ROW_EDIT + i), GetModuleHandle(0), 0);

						sqlite3_reset(stmt2);
						sqlite3_bind_text(stmt2, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt2, 2, name8, strlen(name8), SQLITE_TRANSIENT);

						while(SQLITE_ROW == sqlite3_step(stmt2)) {
							TCHAR* value16 = utils::utf8to16((char*)sqlite3_column_text(stmt2, 0));
							ComboBox_AddString(hComboWnd, value16);
							delete [] value16;
						}
					} else {
						CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | ES_AUTOHSCROLL, 105 * s.x, 5 * s.y + lineH * (i - 1) * s.y, 185 * s.x, editH, hWnd, (HMENU)IntToPtr(IDC_ROW_EDIT + i), GetModuleHandle(0), 0);
					}
					delete [] name16;
				}
				sqlite3_finalize(stmt2);
				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETPARENTFONT);

				SetWindowPos(hWnd, 0, 0, 0, 300 * s.x, (paramCount + 1) * lineH * s.y + 32 * s.y, SWP_NOMOVE | SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_OK), 0, 237 * s.x, paramCount * lineH * s.y + 9 * s.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				delete [] dbname8;

				SetFocus(GetDlgItem(hWnd, IDC_ROW_EDIT + 1));
				utils::alignDialog(hWnd, hMainWnd);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					sqlite3_stmt* stmt = (sqlite3_stmt*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
					sqlite3_stmt* stmt2;
					sqlite3_prepare_v2(prefs::db, "replace into query_params (dbname, name, value) values (?1, ?2, ?3);", -1, &stmt2, 0);
					char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));

					int paramCount = sqlite3_bind_parameter_count(stmt);
					for (int i = 1; i <= paramCount; i++) {
						TCHAR name16[1024];
						GetDlgItemText(hWnd, IDC_ROW_LABEL + i, name16, 1023);
						char* name8 = utils::utf16to8(name16);

						TCHAR value16[1024];
						GetDlgItemText(hWnd, IDC_ROW_EDIT + i, value16, 1023);
						char* value8 = utils::utf16to8(value16);
						dbutils::bind_variant(stmt, i, value8);

						sqlite3_reset(stmt2);
						sqlite3_bind_text(stmt2, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt2, 2, name8, strlen(name8), SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt2, 3, value8, strlen(value8), SQLITE_TRANSIENT);
						sqlite3_step(stmt2);

						delete [] name8;
						delete [] value8;
					}
					sqlite3_finalize(stmt2);

					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	// lParam, USERDATA is in/out buffer: in - schema.tblname, out - a generated statement.
	BOOL CALLBACK cbDlgDrop (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)lParam);
				HWND hTypeWnd = GetDlgItem(hWnd, IDC_DLG_TYPE);
				ComboBox_AddString(hTypeWnd, TEXT("SELECT"));
				ComboBox_AddString(hTypeWnd, TEXT("UPDATE"));
				ComboBox_AddString(hTypeWnd, TEXT("INSERT"));
				ComboBox_AddString(hTypeWnd, TEXT("DELETE"));
				ComboBox_SetCurSel(hTypeWnd, 0);

				POINTFLOAT s = utils::getDlgScale(hWnd);
				int lineH = 14;

				HWND hColumnsWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);
				SetFocus(hColumnsWnd);

				TCHAR* schema16 = utils::getTableName((TCHAR*)lParam, true);
				TCHAR* tablename16 = utils::getTableName((TCHAR*)lParam);

				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select name from pragma_table_info(?2) where schema = ?1 order by cid", -1, &stmt, 0)) {
					char* schema8 = utils::utf16to8(schema16);
					char* tablename8 = utils::utf16to8(tablename16);
					sqlite3_bind_text(stmt, 1, schema8, strlen(schema8), SQLITE_TRANSIENT);
					sqlite3_bind_text(stmt, 2, tablename8, strlen(tablename8), SQLITE_TRANSIENT);
					delete [] schema8;
					delete [] tablename8;

					int colNo = 0;
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* name16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
						CreateWindow(WC_BUTTON, name16, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 5 * s.x, lineH * colNo * s.y, 210 * s.x, 12 * s.y, hColumnsWnd, (HMENU)IntToPtr(IDC_ROW_SWITCH + colNo), GetModuleHandle(0), 0);
						delete [] name16;
						colNo++;
					}
					EnumChildWindows(hColumnsWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETPARENTFONT);

					SetProp(hColumnsWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hColumnsWnd, GWLP_WNDPROC, (LONG_PTR)cbNewScroll));
					SendMessage(hColumnsWnd, WMU_SET_SCROLL_HEIGHT, colNo * lineH * s.y, 0);

				}
				sqlite3_finalize(stmt);

				delete [] schema16;
				delete [] tablename16;


				utils::alignDialog(hWnd, hMainWnd);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					TCHAR* res = (TCHAR*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

					TCHAR* fullname16 = _tcsdup(res);
					int type = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_TYPE));

					TCHAR columns[MAX_TEXT_LENGTH]{0};
					HWND hColumnsWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);

					BYTE OP_SELECT = 0;
					BYTE OP_UPDATE = 1;
					BYTE OP_INSERT = 2;
					BYTE OP_DELETE = 3;

					int colNo = 0;
					int valCount = 0;
					while (1) {
						HWND hColumnWnd = GetDlgItem(hColumnsWnd, IDC_ROW_SWITCH + colNo);
						colNo++;

						if (!IsWindow(hColumnWnd))
							break;

						if (Button_GetCheck(hColumnWnd) != BST_CHECKED)
							continue;

						TCHAR colName[255];
						GetWindowText(hColumnWnd, colName, 255);

						if (_tcslen(columns) > 0)
							_tcscat(columns, type == OP_SELECT ? TEXT(",\n       ") : type == OP_DELETE ? TEXT(" and ") : TEXT(", "));
						_tcscat(columns, colName);
						if (type == OP_UPDATE || type == OP_DELETE)
							_tcscat(columns, TEXT(" = ?"));

						valCount++;
					}

					if (type == OP_SELECT)
						_sntprintf(res, 255, TEXT("select %ls\n  from %ls"), columns, fullname16);
					if (type == OP_UPDATE)
						_sntprintf(res, 255, TEXT("update %ls set %ls where"), fullname16, columns);
					if (type == OP_INSERT) {
						TCHAR placeholders[valCount * 3 + 1]{0};
						for (int i= 0; i < valCount; i++)
							_tcscat(placeholders, i == 0 ? TEXT("?") : TEXT(", ?"));
						_sntprintf(res, 255, TEXT("insert into %ls (%ls) values (%ls)"), fullname16, columns, placeholders);
					}
					if (type == OP_DELETE)
						_sntprintf(res, 255, TEXT("delete from %ls where %ls"), fullname16, columns);

					free(fullname16);

					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_MOUSEWHEEL: {
				int action = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? SB_LINEUP : SB_LINEDOWN;
				SendMessage(GetDlgItem(hWnd, IDC_DLG_COLUMNS), WM_VSCROLL, MAKELPARAM(action, 0), 0);
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbEnumFont(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD fontType, LPVOID hWnd)  {
		if (fontType & TRUETYPE_FONTTYPE && lplf->lfFaceName[0] != TEXT('@'))
			ComboBox_AddString((HWND)hWnd, lplf->lfFaceName);
		return true;
	}

	COLORREF GetBrushColor(HBRUSH brush) {
		LOGBRUSH lbr;
		if (GetObject(brush, sizeof(lbr), &lbr) != sizeof(lbr))
			return CLR_NONE;

		return lbr.lbColor;
	}

	const int SETTING_COLOR_COUNT = 12;

	bool CALLBACK cbEnumChildrenReparent (HWND hWnd, HWND hParentWnd) {
		if (hWnd != hParentWnd) {
			int id = GetDlgCtrlID(hWnd);
			if (id == IDC_DLG_OK || id == IDC_DLG_CANCEL)
				return true;

			RECT rc;
			GetWindowRect(hWnd, &rc);
			POINT p = {rc.left, rc.top};
			ScreenToClient(hParentWnd, &p);

			GetWindowRect(hParentWnd, &rc);
			TabCtrl_AdjustRect(hParentWnd, FALSE, &rc);
			POINT p2 = {rc.left, rc.top};
			ScreenToClient(hParentWnd, &p2);

			GetWindowRect(hParentWnd, &rc);
			MapWindowPoints(HWND_DESKTOP, GetParent(hParentWnd), (LPPOINT) &rc, 2);

			SetParent(hWnd, hParentWnd);
			SetWindowPos(hWnd, HWND_BOTTOM, rc.left + p2.x + p.x, rc.top + p2.y + p.y, 0, 0, SWP_NOSIZE);
		}

		return true;
	}

	LRESULT CALLBACK cbNewTabSettings(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_GETDLGCODE: {
				return DLGC_WANTALLKEYS | CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
			}
			break;

			case WM_KEYDOWN: {
				if (wParam == VK_ESCAPE)
					SendMessage(GetParent(hWnd), WM_COMMAND, IDC_DLG_CANCEL, 0);

			}
			break;

			case WM_COMMAND: {
				if (HIWORD(wParam) == STN_CLICKED && (LOWORD(wParam) >= IDC_DLG_COLOR && LOWORD(wParam) < IDC_DLG_COLOR + SETTING_COLOR_COUNT)) {
					COLORREF colors[16] = {GetSysColor(COLOR_BTNFACE)}; // Fix bug: sometimes dialog has filled background as the first custom color.

					HBRUSH* brushes = (HBRUSH*)GetProp(GetParent(hWnd), TEXT("BRUSHES"));
					int brushNo = LOWORD(wParam) - IDC_DLG_COLOR;
					CHOOSECOLOR cc = {0};
					cc.lStructSize = sizeof(cc);
					cc.hwndOwner = hWnd;
					cc.lpCustColors = colors;
					cc.rgbResult = GetBrushColor(brushes[brushNo]);
					cc.lpTemplateName = MAKEINTRESOURCE(IDD_COLOR_PICKER);
					cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ENABLETEMPLATE;

					if (ChooseColor(&cc)) {
						DeleteObject(brushes[brushNo]);
						brushes[brushNo] = CreateSolidBrush(cc.rgbResult);
						InvalidateRect((HWND)lParam, NULL, TRUE);
					}
				}

				if (HIWORD(wParam) == STN_CLICKED && LOWORD(wParam) == IDC_DLG_HTTP_SERVER)
					EnableWindow(GetDlgItem(hWnd, IDC_DLG_HTTP_SERVER_PORT), Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_HTTP_SERVER)));

				if (HIWORD(wParam) == STN_CLICKED && LOWORD(wParam) == IDC_DLG_USE_LOGGER) {
					TCHAR loggerPath16[MAX_PATH + 1];
					_sntprintf(loggerPath16, MAX_PATH, TEXT("%ls\\logger.exe"), APP_PATH);
					if (Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_USE_LOGGER)) && !utils::isFileExists(loggerPath16) && !IsWindow(hLoggerWnd)) {
						TCHAR msg16[2048];
						_sntprintf(msg16, 1023, TEXT("The special logger.exe is required. Do you want to download it to %ls? Check Wiki if you have questions about it."), loggerPath16);

						if (IDOK == MessageBox(hWnd, msg16, TEXT("Confirmation"), MB_OKCANCEL)) {
								char url8[1024];
								snprintf(url8, 1023, "/little-brother/logger/releases/latest/download/logger-x%i.zip", GUI_PLATFORM);

								bool isDone = false;
								int readBytes = 0;

								unsigned char* buf8 = (unsigned char*)utils::httpRequest("GET", "github.com", url8, 0, &readBytes);
								if (buf8 && (readBytes > 30) && (buf8[0] == 0x50 && buf8[1] == 0x4B && buf8[2] == 0x03 && buf8[3] == 0x04)) {
									// Parse first zip local file header
									unsigned int unpackLen = utils::read_le32(&buf8[22]);
									unsigned int packLen = utils::read_le32(&buf8[18]);
									unsigned int nameLen = utils::read_le16(&buf8[26]);
									unsigned int nameExLen = utils::read_le16(&buf8[28]);

									unsigned char* data8 = new unsigned char[MAX(unpackLen, 1)] {0};
									if (TINF_OK == (tinf_uncompress(data8, &unpackLen, buf8 + 30 + nameLen + nameExLen, packLen))) {
										FILE* f = _tfopen(loggerPath16, TEXT("wb"));
										if (f) {
											fwrite(data8, 1, unpackLen, f);
											fclose(f);
											isDone = true;
										}
									}
									delete [] data8;
								}

								if (buf8)
									delete [] buf8;

								if (!isDone) {
									CheckDlgButton(hWnd, IDC_DLG_USE_LOGGER, BST_UNCHECKED);
									MessageBox(hWnd, TEXT("Can't download and run logger.exe.\nCheck an ethernet connection."), TEXT("Error"), MB_OK);
								}
						} else {
							CheckDlgButton(hWnd, IDC_DLG_USE_LOGGER, BST_UNCHECKED);
						}
					}
				}
			}
			break;

			case WM_CTLCOLORSTATIC: {
				int id = GetDlgCtrlID((HWND)lParam);
				if (id >= IDC_DLG_COLOR && id < IDC_DLG_COLOR + SETTING_COLOR_COUNT) {
					HBRUSH* brushes = (HBRUSH*)GetProp(GetParent(hWnd), TEXT("BRUSHES"));
					int brushNo = id - IDC_DLG_COLOR;
					HBRUSH hBrush = brushes[brushNo];
					HDC hDC = (HDC)wParam;

					if (brushNo < 6) {
						SetTextColor(hDC, GetBrushColor(brushes[brushNo]));
						SetBkColor(hDC, RGB(255, 255, 255));
						return (INT_PTR)GetStockObject(WHITE_BRUSH);
					} else {
						SetBkColor(hDC, GetBrushColor(hBrush));
						return (INT_PTR)hBrush;
					}
				}
			}
			break;
		}

		return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
	}


	// lParam = init tabNo
	BOOL CALLBACK cbDlgSettings (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hTabWnd = GetDlgItem(hWnd, IDC_DLG_SETTING_TAB);
				TCITEM tci;
				tci.mask = TCIF_TEXT | TCIF_IMAGE;
				tci.iImage = 0;
				tci.pszText = TEXT("General");
				tci.cchTextMax = 10;
				TabCtrl_InsertItem(hTabWnd, 0, &tci);
				tci.pszText = TEXT("Appearance");
				tci.cchTextMax = 20;
				TabCtrl_InsertItem(hTabWnd, 1, &tci);
				tci.pszText = TEXT("Extra");
				tci.cchTextMax = 20;
				TabCtrl_InsertItem(hTabWnd, 2, &tci);
				SetWindowTheme(hTabWnd, TEXT(" "), TEXT(" "));

				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumFixEditHeights, (LPARAM)utils::getEditHeight(hWnd));
				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildrenReparent, (LPARAM)hTabWnd);
				SetProp(hTabWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hTabWnd, GWLP_WNDPROC, (LONG_PTR)cbNewTabSettings));

				// There is a some issue for edit after the combobox reparenting
				// So just recreate it. Use for font family and size
 				auto recreateDropDown = [hTabWnd](int idc) {
 					HWND hWnd = GetDlgItem(hTabWnd, idc);
 					HWND hPrevWnd = GetNextWindow(hWnd, GW_HWNDPREV);

					RECT rc;
					GetWindowRect(hWnd, &rc);
					POINT pos{rc.left, rc.top}, dims {rc.right - rc.left, rc.bottom - rc.top};
					ScreenToClient(hTabWnd, &pos);
					HFONT hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
					DestroyWindow(hWnd);

					hWnd = CreateWindow(WC_COMBOBOX, NULL, CBS_DROPDOWN | CBS_HASSTRINGS | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
						pos.x, pos.y, dims.x, 200, hTabWnd, (HMENU)(LONG_PTR)idc, GetModuleHandle(0), NULL);
					SetWindowPos(hWnd, hPrevWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); // Restore Z-index to keep TAB-navigation correct
					SendMessage(hWnd, WM_SETFONT, (LPARAM)hFont, TRUE);

					return hWnd;
				};

				HWND hFontSize = recreateDropDown(IDC_DLG_FONT_SIZE);
				int fontSize = prefs::get("font-size");
				int sizes[] = {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24};
				int idx = -1;
				for (int i = 0; i < 11; i++) {
					idx = fontSize == sizes[i] ? i : idx;
					TCHAR buf[4];
					_sntprintf(buf, 3, TEXT("%i"), sizes[i]);
					ComboBox_AddString(hFontSize, buf);
				}
				if (idx != -1) {
					ComboBox_SetCurSel(hFontSize, idx);
				} else {
					TCHAR fs[10];
					_sntprintf(fs, 9, TEXT("%i"), fontSize);
					ComboBox_SetText(hFontSize, fs);
				}

				HWND hExitByEscape = GetDlgItem(hTabWnd, IDC_DLG_EXIT_BY_ESCAPE);
				ComboBox_AddString(hExitByEscape, TEXT("Do nothing"));
				ComboBox_AddString(hExitByEscape, TEXT("Close application"));
				ComboBox_AddString(hExitByEscape, TEXT("Ask before closing the app"));
				ComboBox_SetCurSel(hExitByEscape, prefs::get("exit-by-escape"));

				HWND hFontFamily = recreateDropDown(IDC_DLG_FONT_FAMILY);
				HDC hDC = GetDC(hMainWnd);
				LOGFONT lf = {0};
				lf.lfFaceName[0] = TEXT('\0');
				lf.lfCharSet = GetTextCharset(hDC);
				EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC) cbEnumFont, (LPARAM)hFontFamily, 0);
				ReleaseDC(hMainWnd, hDC);
				char* fontFamily8 = prefs::get("font-family", "Courier New");
				TCHAR* fontFamily16 = utils::utf8to16(fontFamily8);
				ComboBox_SetCurSel(hFontFamily, ComboBox_FindStringExact(hFontFamily, -1, fontFamily16));
				delete [] fontFamily16;
				delete [] fontFamily8;

				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_RESTORE_DB), prefs::get("restore-db") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_RESTORE_EDITOR), prefs::get("restore-editor") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_ASK_DELETE), prefs::get("ask-delete") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_AUTO_FILTERS), prefs::get("auto-filters") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_WORD_WRAP), prefs::get("word-wrap") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_HTTP_SERVER), prefs::get("http-server") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_CHECK_UPDATES), prefs::get("check-update") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_USE_LOGGER), prefs::get("use-logger") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_TAB_AUTOCOMPLETE), prefs::get("autocomplete-by-tab") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_DISABLE_HELP), prefs::get("disable-autocomplete-help") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_SYNC_OFF), prefs::get("synchronous-off") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_RETAIN_PASSPHRASE), prefs::get("retain-passphrase") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_FOREIGN_KEYS), prefs::get("use-foreign-keys") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hTabWnd, IDC_DLG_LEGACY_RENAME), prefs::get("use-legacy-rename") ? BST_CHECKED : BST_UNCHECKED);

				TCHAR buf[256];
				_sntprintf(buf, 255, TEXT("%i"), prefs::get("row-limit"));
				SetDlgItemText(hTabWnd, IDC_DLG_ROW_LIMIT, buf);

				_sntprintf(buf, 255, TEXT("%i"), prefs::get("cli-row-limit"));
				SetDlgItemText(hTabWnd, IDC_DLG_CLI_ROW_LIMIT, buf);

				_sntprintf(buf, 255, TEXT("%i"), prefs::get("beep-query-duration"));
				SetDlgItemText(hTabWnd, IDC_DLG_BEEP_ON_QUERY_END, buf);

				_sntprintf(buf, 255, TEXT("%i"), prefs::get("http-server-port"));
				SetDlgItemText(hTabWnd, IDC_DLG_HTTP_SERVER_PORT, buf);
				EnableWindow(GetDlgItem(hTabWnd, IDC_DLG_HTTP_SERVER_PORT), prefs::get("http-server"));

				char* startup8 = prefs::get("startup", "");
				TCHAR* startup16 = utils::utf8to16(startup8);
				SetDlgItemText(hTabWnd, IDC_DLG_STARTUP, startup16);
				delete [] startup16;
				delete [] startup8;

				char* googleApiKey8 = prefs::get("google-api-key", "");
				TCHAR* googleApiKey16 = utils::utf8to16(googleApiKey8);
				SetDlgItemText(hTabWnd, IDC_DLG_GOOGLE_KEY, googleApiKey16);
				delete [] googleApiKey16;
				delete [] googleApiKey8;

				char* viewerRepository8 = prefs::get("viewer-repository", VIEWER_REPOSITORY);
				TCHAR* viewerRepository16 = utils::utf8to16(viewerRepository8);
				SetDlgItemText(hTabWnd, IDC_DLG_VIEWER_REPOSITORY, viewerRepository16);
				Edit_SetCueBannerText(GetDlgItem(hTabWnd, IDC_DLG_VIEWER_REPOSITORY), TEXT(VIEWER_REPOSITORY));
				delete [] viewerRepository16;
				delete [] viewerRepository8;

				char* modifierRepository8 = prefs::get("modifier-repository", MODIFIER_REPOSITORY);
				TCHAR* modifierRepository16 = utils::utf8to16(modifierRepository8);
				SetDlgItemText(hTabWnd, IDC_DLG_MODIFIER_REPOSITORY, modifierRepository16);
				Edit_SetCueBannerText(GetDlgItem(hTabWnd, IDC_DLG_MODIFIER_REPOSITORY), TEXT(MODIFIER_REPOSITORY));
				delete [] modifierRepository16;
				delete [] modifierRepository8;

				char* extensionRepository8 = prefs::get("extension-repository", EXTENSION_REPOSITORY);
				TCHAR* extensionRepository16 = utils::utf8to16(extensionRepository8);
				SetDlgItemText(hTabWnd, IDC_DLG_EXTENSION_REPOSITORY, extensionRepository16);
				Edit_SetCueBannerText(GetDlgItem(hTabWnd, IDC_DLG_EXTENSION_REPOSITORY), TEXT(EXTENSION_REPOSITORY));
				delete [] extensionRepository16;
				delete [] extensionRepository8;

				HWND hIndent = GetDlgItem(hTabWnd, IDC_DLG_INDENT);
				for (int i = 0; i < 3; i++)
					ComboBox_AddString(hIndent, INDENT_LABELS[i]);
				ComboBox_SetCurSel(hIndent, prefs::get("editor-indent"));

				HWND hDelimiter = GetDlgItem(hTabWnd, IDC_DLG_DELIMITER);
				for (int i = 0; i < 4; i++)
					ComboBox_AddString(hDelimiter, i != 2 ? tools::DELIMITERS[i] : TEXT("Tab"));
				ComboBox_SetCurSel(hDelimiter, prefs::get("copy-to-clipboard-delimiter"));

				HBRUSH* brushes = new HBRUSH[SETTING_COLOR_COUNT]{0};
				brushes[0] = CreateSolidBrush(prefs::get("color-keyword"));
				brushes[1] = CreateSolidBrush(prefs::get("color-function"));
				brushes[2] = CreateSolidBrush(prefs::get("color-quoted"));
				brushes[3] = CreateSolidBrush(prefs::get("color-comment"));
				brushes[4] = CreateSolidBrush(prefs::get("color-parenthesis"));
				brushes[5] = CreateSolidBrush(prefs::get("color-pragma"));

				brushes[6] = CreateSolidBrush(prefs::get("color-text"));
				brushes[7] = CreateSolidBrush(prefs::get("color-null"));
				brushes[8] = CreateSolidBrush(prefs::get("color-blob"));
				brushes[9] = CreateSolidBrush(prefs::get("color-integer"));
				brushes[10] = CreateSolidBrush(prefs::get("color-real"));
				brushes[11] = CreateSolidBrush(prefs::get("color-current-cell"));
				SetProp(hWnd, TEXT("BRUSHES"), (HANDLE)brushes);

				SendMessage(hWnd, WMU_TAB_CHANGED, lParam < TabCtrl_GetItemCount(hTabWnd) ? lParam : 0, 0);
				utils::alignDialog(hWnd, hMainWnd);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					HWND hTabWnd = GetDlgItem(hWnd, IDC_DLG_SETTING_TAB);

					TCHAR buf[255];
					GetDlgItemText(hTabWnd, IDC_DLG_FONT_FAMILY, buf, 255);
					char* fontFamily8 = utils::utf16to8(buf);
					prefs::set("font-family", fontFamily8);
					delete [] fontFamily8;

					GetDlgItemText(hTabWnd, IDC_DLG_FONT_SIZE, buf, 255);
					prefs::set("font-size", MAX(8, _tcstol(buf, NULL, 10)));
					prefs::set("restore-db", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_RESTORE_DB)));
					prefs::set("restore-editor", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_RESTORE_EDITOR)));
					prefs::set("ask-delete", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_ASK_DELETE)));
					prefs::set("auto-filters", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_AUTO_FILTERS)));
					prefs::set("word-wrap", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_WORD_WRAP)));
					prefs::set("http-server", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_HTTP_SERVER)));
					prefs::set("check-update", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_CHECK_UPDATES)));
					prefs::set("use-logger", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_USE_LOGGER)));
					prefs::set("autocomplete-by-tab", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_TAB_AUTOCOMPLETE)));
					prefs::set("disable-autocomplete-help", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_DISABLE_HELP)));
					prefs::set("retain-passphrase", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_RETAIN_PASSPHRASE)));
					prefs::set("exit-by-escape", ComboBox_GetCurSel(GetDlgItem(hTabWnd, IDC_DLG_EXIT_BY_ESCAPE)));
					prefs::set("synchronous-off", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_SYNC_OFF)));
					prefs::set("editor-indent", ComboBox_GetCurSel(GetDlgItem(hTabWnd, IDC_DLG_INDENT)));
					prefs::set("use-foreign-keys", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_FOREIGN_KEYS)));
					prefs::set("use-legacy-rename", Button_GetCheck(GetDlgItem(hTabWnd, IDC_DLG_LEGACY_RENAME)));
					prefs::set("copy-to-clipboard-delimiter", ComboBox_GetCurSel(GetDlgItem(hTabWnd, IDC_DLG_DELIMITER)));

					GetDlgItemText(hTabWnd, IDC_DLG_ROW_LIMIT, buf, 255);
					prefs::set("row-limit", (int)_tcstod(buf, NULL));

					GetDlgItemText(hTabWnd, IDC_DLG_CLI_ROW_LIMIT, buf, 255);
					prefs::set("cli-row-limit", (int)_tcstod(buf, NULL));

					GetDlgItemText(hTabWnd, IDC_DLG_BEEP_ON_QUERY_END, buf, 255);
					prefs::set("beep-query-duration", (int)_tcstod(buf, NULL));

					GetDlgItemText(hTabWnd, IDC_DLG_HTTP_SERVER_PORT, buf, 255);
					prefs::set("http-server-port", (int)_tcstod(buf, NULL));

					TCHAR startup16[MAX_TEXT_LENGTH]{0};
					GetDlgItemText(hTabWnd, IDC_DLG_STARTUP, startup16, MAX_TEXT_LENGTH - 1);
					char* startup8 = utils::utf16to8(startup16);
					prefs::set("startup", startup8);
					delete [] startup8;

					GetDlgItemText(hTabWnd, IDC_DLG_GOOGLE_KEY, buf, 254);
					char* googleApiKey8 = utils::utf16to8(buf);
					prefs::set("google-api-key", googleApiKey8);
					delete [] googleApiKey8;

					GetDlgItemText(hTabWnd, IDC_DLG_VIEWER_REPOSITORY, buf, 254);
					char* viewerRepository8 = utils::utf16to8(_tcslen(buf) ? buf : TEXT(VIEWER_REPOSITORY));
					prefs::set("viewer-repository", viewerRepository8);
					delete [] viewerRepository8;

					GetDlgItemText(hTabWnd, IDC_DLG_MODIFIER_REPOSITORY, buf, 254);
					char* modifierRepository8 = utils::utf16to8(_tcslen(buf) ? buf : TEXT(MODIFIER_REPOSITORY));
					prefs::set("modifier-repository", modifierRepository8);
					delete [] modifierRepository8;

					GetDlgItemText(hTabWnd, IDC_DLG_EXTENSION_REPOSITORY, buf, 254);
					char* extensionRepository8 = utils::utf16to8(_tcslen(buf) ? buf : TEXT(EXTENSION_REPOSITORY));
					prefs::set("extension-repository", extensionRepository8);
					delete [] extensionRepository8;

					HBRUSH* brushes = (HBRUSH*)GetProp(hWnd, TEXT("BRUSHES"));
					prefs::set("color-keyword", GetBrushColor(brushes[0]));
					prefs::set("color-function", GetBrushColor(brushes[1]));
					prefs::set("color-quoted", GetBrushColor(brushes[2]));
					prefs::set("color-comment", GetBrushColor(brushes[3]));
					prefs::set("color-parenthesis", GetBrushColor(brushes[4]));
					prefs::set("color-pragma", GetBrushColor(brushes[5]));

					prefs::set("color-text", GetBrushColor(brushes[6]));
					prefs::set("color-null", GetBrushColor(brushes[7]));
					prefs::set("color-blob", GetBrushColor(brushes[8]));
					prefs::set("color-integer", GetBrushColor(brushes[9]));
					prefs::set("color-real", GetBrushColor(brushes[10]));

					COLORREF cellColor = GetBrushColor(brushes[11]);
					prefs::set("color-current-cell", cellColor);

					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;

				if (pHdr->idFrom == IDC_DLG_SETTING_TAB && pHdr->code == TCN_SELCHANGE)
					SendMessage(hWnd, WMU_TAB_CHANGED, TabCtrl_GetCurSel(pHdr->hwndFrom), 0);
			}
			break;

			// wParam = tabNo
			case WMU_TAB_CHANGED: {
				int idcs[3][30] = {
					{
						IDC_DLG_RESTORE_DB, IDC_DLG_RESTORE_EDITOR, IDC_DLG_WORD_WRAP, IDC_DLG_TAB_AUTOCOMPLETE,
						IDC_DLG_DISABLE_HELP, IDC_DLG_ASK_DELETE, IDC_DLG_AUTO_FILTERS, IDC_DLG_SYNC_OFF, IDC_DLG_RETAIN_PASSPHRASE,
						IDC_DLG_HTTP_SERVER, IDC_DLG_HTTP_SERVER_PORT, IDC_DLG_CHECK_UPDATES, IDC_DLG_USE_LOGGER
					},
					{
						IDC_DLG_FONT_FAMILY, IDC_DLG_FONT_LABEL, IDC_DLG_FONT_SIZE, IDC_DLG_EDITOR_LABEL, IDC_DLG_COLOR, IDC_DLG_GRID_LABEL,
						IDC_DLG_COLOR + 1, IDC_DLG_COLOR + 2, IDC_DLG_COLOR + 3, IDC_DLG_COLOR + 4, IDC_DLG_COLOR + 5,
						IDC_DLG_COLOR + 6, IDC_DLG_COLOR + 7, IDC_DLG_COLOR + 8, IDC_DLG_COLOR + 9, IDC_DLG_COLOR + 10,
						IDC_DLG_COLOR + 11, IDC_DLG_ESCAPE_LABEL, IDC_DLG_EXIT_BY_ESCAPE, IDC_DLG_INDENT_LABEL, IDC_DLG_INDENT,
						IDC_DLG_ROW_LIMIT_LABEL, IDC_DLG_ROW_LIMIT, IDC_DLG_CLI_ROW_LIMIT_LABEL, IDC_DLG_CLI_ROW_LIMIT,
						IDC_DLG_BEEP_LABEL, IDC_DLG_BEEP_ON_QUERY_END,
						IDC_DLG_DELIMITER_LABEL, IDC_DLG_DELIMITER
					},
					{
						IDC_DLG_STARTUP, IDC_DLG_STARTUP_LABEL, IDC_DLG_FOREIGN_KEYS, IDC_DLG_LEGACY_RENAME,
						IDC_DLG_GOOGLE_KEY_LABEL, IDC_DLG_GOOGLE_KEY,
						IDC_DLG_VIEWER_REPOSITORY, IDC_DLG_VIEWER_REPOSITORY_LABEL,
						IDC_DLG_MODIFIER_REPOSITORY, IDC_DLG_MODIFIER_REPOSITORY_LABEL,
						IDC_DLG_EXTENSION_REPOSITORY, IDC_DLG_EXTENSION_REPOSITORY_LABEL
					}
				};

				HWND hTabWnd = GetDlgItem(hWnd, IDC_DLG_SETTING_TAB);
				int tabNo = (int)wParam;
				for (int i = 0; i < 3; i++) {
					for (int idc : idcs[i]) {
						if (idc)
							ShowWindow(GetDlgItem(hTabWnd, idc), i == tabNo ? SW_SHOW : SW_HIDE);
					}
				}
			}
			break;

			case WM_CLOSE: {
				HBRUSH* brushes = (HBRUSH*)GetProp(hWnd, TEXT("BRUSHES"));
				for (int i = 0; i < SETTING_COLOR_COUNT; i++)
					DeleteObject(brushes[i]);
				delete [] brushes;
				RemoveProp(hWnd, TEXT("BRUSHES"));
			}
			break;
		}

		return false;
	}

	#define CIPHER_NONE               0
	#define CIPHER_WXSQLITE128        1
	#define CIPHER_WXSQLITE256        2
	#define CIPHER_SQLEET             3
	#define CIPHER_SQLCIPHER          4
	#define CIPHER_SYSTEM_DATA        5
	#define CIPHER_ASCON128           6

	#define ENCRYPT_MODE_OPEN         0
	#define ENCRYPT_MODE_REKEY        1
	#define ENCRYPT_MODE_ATTACH       2

	bool getAttachString(const char* path8, const char* alias8, const char* key8, char* result8) {
		result8[0] = 0;

		sqlite3_stmt* stmt;
		bool rc = SQLITE_OK == sqlite3_prepare_v2(prefs::db,
			"with params (param, value) as (select param, value " \
			"from (select param, value, no from main.encryption where dbpath = ?1 union "\
			"select param, value, no from temp.encryption where dbpath = ?1 union " \
			"select 'noop', 'novalue', 100) order by no)"\
			"select 'attach ''file:' || ?1 || '?' || group_concat(param || '=' || value, '&') || ''' as ' || ?2 || " \
			"' key ' || quote((select coalesce(?3, max(value), '') from params where param = 'key')) " \
			"from params where param <> 'key'", -1, &stmt, 0);

		if (rc) {
			sqlite3_bind_text(stmt, 1, path8, -1, SQLITE_TRANSIENT);

			if (alias8) {
				sqlite3_bind_text(stmt, 2, alias8, -1, SQLITE_TRANSIENT);
			} else {
				char* alias8 = utils::getFileName(path8, true);
				sqlite3_bind_text(stmt, 2, alias8, -1, SQLITE_TRANSIENT);
				delete [] alias8;
			}

			if (key8) {
				sqlite3_bind_text(stmt, 3, key8, -1, SQLITE_TRANSIENT);
			} else {
				sqlite3_bind_null(stmt, 3);
			}

			rc = SQLITE_ROW == sqlite3_step(stmt);
			if (rc)
				strcpy(result8, (const char*)sqlite3_column_text(stmt, 0));
		}
		sqlite3_finalize(stmt);

		return rc;
	}

	LRESULT CALLBACK cbNewKeyIcon(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) {
			HWND hKeyWnd = GetDlgItem(GetParent(hWnd), IDC_DLG_CIPHER_KEY);
			SendMessage(hKeyWnd, EM_SETPASSWORDCHAR, (WPARAM)((msg == WM_LBUTTONUP) ? TEXT('\U000025CF') : 0), 0);
			InvalidateRect(hKeyWnd, NULL, TRUE);

			return 0;
		}

		return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
	}

	// USERDATA = lParam
	/*
		There are 3 dialog modes:
			open database - db can't execute query, lParam = sqlite3 db handle
			encrypt database - db can execute query, lParam is not set, lParam = 0
			attach database - db can execute query, lParam is set as "attach database as ...", lParam = dbpath8
	*/
	const TCHAR* ciphers[7] = {TEXT(""), TEXT("aes128cbc"), TEXT("aes256cbc"), TEXT("chacha20"), TEXT("sqlcipher"), TEXT("rc4"), TEXT("ascon128")};
	BOOL CALLBACK cbDlgEncryption (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);

				HWND hCombo = GetDlgItem(hWnd, IDC_DLG_CIPHER);
				ComboBox_AddString(hCombo, TEXT("None"));
				ComboBox_AddString(hCombo, TEXT("wxSQLite3: AES 128 Bit CBC"));
				ComboBox_AddString(hCombo, TEXT("wxSQLite3: AES 256 Bit CBC"));
				ComboBox_AddString(hCombo, TEXT("sqleet: ChaCha20"));
				ComboBox_AddString(hCombo, TEXT("SQLCipher: AES 256 Bit CBC"));
				ComboBox_AddString(hCombo, TEXT("System.Data.SQLite: RC4"));
				ComboBox_AddString(hCombo, TEXT("Ascon: Ascon-128 v1.2"));
				ComboBox_SetCurSel(hCombo, 0);

				hCombo = GetDlgItem(hWnd, IDC_DLG_CIPHER_PAGESIZE);
				ComboBox_AddString(hCombo, TEXT("0"));
				ComboBox_AddString(hCombo, TEXT("512"));
				ComboBox_AddString(hCombo, TEXT("1024"));
				ComboBox_AddString(hCombo, TEXT("2048"));
				ComboBox_AddString(hCombo, TEXT("4096"));
				ComboBox_AddString(hCombo, TEXT("8192"));
				ComboBox_AddString(hCombo, TEXT("16384"));
				ComboBox_AddString(hCombo, TEXT("32768"));
				ComboBox_AddString(hCombo, TEXT("65536"));
				ComboBox_SetCurSel(hCombo, 0);

				hCombo = GetDlgItem(hWnd, IDC_DLG_CIPHER_KDF_ALGORITHM);
				ComboBox_AddString(hCombo, TEXT("SHA1"));
				ComboBox_AddString(hCombo, TEXT("SHA256"));
				ComboBox_AddString(hCombo, TEXT("SHA512"));

				hCombo = GetDlgItem(hWnd, IDC_DLG_CIPHER_HMAC_ALGORITHM);
				ComboBox_AddString(hCombo, TEXT("SHA1"));
				ComboBox_AddString(hCombo, TEXT("SHA256"));
				ComboBox_AddString(hCombo, TEXT("SHA512"));

				hCombo = GetDlgItem(hWnd, IDC_DLG_CIPHER_HMAC_PGNO);
				ComboBox_AddString(hCombo, TEXT("native"));
				ComboBox_AddString(hCombo, TEXT("little endian"));
				ComboBox_AddString(hCombo, TEXT("big-endian"));

				hCombo = GetDlgItem(hWnd, IDC_DLG_CIPHER_HMAC_USE);
				ComboBox_AddString(hCombo, TEXT("no"));
				ComboBox_AddString(hCombo, TEXT("yes"));

				hCombo = GetDlgItem(hWnd, IDC_DLG_CIPHER_PROFILE);
				ComboBox_AddString(hCombo, TEXT("Custom"));
				ComboBox_AddString(hCombo, TEXT("v1"));
				ComboBox_AddString(hCombo, TEXT("v2"));
				ComboBox_AddString(hCombo, TEXT("v3"));
				ComboBox_AddString(hCombo, TEXT("v4"));
				ComboBox_SetCurSel(hCombo, 0);

				bool isOpen = SQLITE_OK == sqlite3_exec(db, "select * from sqlite_master limit 1", 0, 0, 0);
				int mode = !isOpen ? ENCRYPT_MODE_OPEN : isOpen && !lParam ? ENCRYPT_MODE_REKEY : ENCRYPT_MODE_ATTACH;
				SetProp(hWnd, TEXT("MODE"), IntToPtr(mode));

				const char* dbpath8 = mode == ENCRYPT_MODE_OPEN ? sqlite3_db_filename((sqlite3*)lParam, 0) :
					mode == ENCRYPT_MODE_REKEY ? sqlite3_db_filename(db, 0) :
					(const char*) lParam;
				SetProp(hWnd, TEXT("DBPATH8"), (HANDLE)dbpath8);

				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(prefs::db,
						"select idc, value, stored " \
						"from (select idc, value, 1 stored, no from main.encryption where dbpath = ?1 union select idc, value, 0 stored, no from temp.encryption where dbpath = ?1)" \
						"order by no", -1, &stmt, 0)) {
					sqlite3_bind_text(stmt, 1, dbpath8, strlen(dbpath8), SQLITE_TRANSIENT);
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						int idc = sqlite3_column_int(stmt, 0);
						if (idc == IDC_DLG_CIPHER_KEY) {
							TCHAR* key16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
							SetDlgItemText(hWnd, idc, key16);
							delete [] key16;

							Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_CIPHER_STORE_KEY), sqlite3_column_int(stmt, 2) == 1 ? BST_CHECKED : BST_UNCHECKED);
						} else if (idc == IDC_DLG_CIPHER) {
							TCHAR* cipher16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));
							for (int i = CIPHER_WXSQLITE128; i <= CIPHER_ASCON128; i++) {
								if (_tcscmp(cipher16, ciphers[i]) == 0)
									SendMessage(hWnd, WMU_SET_VALUE, idc, i);
							}

							delete [] cipher16;
						} else {
							SendMessage(hWnd, WMU_SET_VALUE, idc, sqlite3_column_int(stmt, 1));
						}
						SetProp(GetDlgItem(hWnd, idc), TEXT("PROTECTED"), IntToPtr(1));
					}
				}
				sqlite3_finalize(stmt);

				SendMessage(hWnd, WMU_CIPHER_CHANGED, 0, 0);
				for (int idc = IDC_DLG_CIPHER_KEY; idc <= IDC_DLG_CIPHER_HEADER_SIZE; idc++)
					RemoveProp(GetDlgItem(hWnd, idc), TEXT("PROTECTED"));

				HICON hIcon = ImageList_GetIcon(hIconsImageList, 1, 0);
				HWND hKeyIconWnd = GetDlgItem(hWnd, IDC_DLG_CIPHER_SHOW_KEY);
				SendMessage(hKeyIconWnd, STM_SETICON, (LPARAM)hIcon, 0);
				SetProp(hKeyIconWnd, TEXT("WNDPROC"), (HANDLE)SetWindowLongPtr(hKeyIconWnd, GWLP_WNDPROC, (LONG_PTR)cbNewKeyIcon));

				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumFixEditHeights, (LPARAM)utils::getEditHeight(hWnd));
			}
			break;

			case WM_COMMAND: {
				if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_DLG_CIPHER)
					return SendMessage(hWnd, WMU_CIPHER_CHANGED, 0, 0);

				if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_DLG_CIPHER_PROFILE) {
					int iProfile = ComboBox_GetCurSel((HWND)lParam);
					for (int id = IDC_DLG_CIPHER_KDF_ITER; id <= IDC_DLG_CIPHER_HEADER_SIZE; id += 2)
						EnableWindow(GetDlgItem(hWnd, id), iProfile == 0);

					const int profiles[5][9] = {
						{0, 256000, 2, 1, 1, 0x3a, 2, 2, 0},    // default
						{1024, 4000, 2, -1, -1, 0, 0, 0, -1},   // v1
						{1024, 4000, 2, 1, 1, 0x3a, 0, 0, -1},  // v2
						{1024, 64000,2, 1, 1, 0x3a, 0, 0, -1},  // v3
						{4096, 256000, 2, 1, 1, 0x3a, 2, 2, 0}  // v4
					};

					int idcs [] = {
						IDC_DLG_CIPHER_PAGESIZE, IDC_DLG_CIPHER_KDF_ITER, IDC_DLG_CIPHER_FAST_KDF_ITER,
						IDC_DLG_CIPHER_HMAC_USE, IDC_DLG_CIPHER_HMAC_PGNO, IDC_DLG_CIPHER_HMAC_SALT,
						IDC_DLG_CIPHER_KDF_ALGORITHM, IDC_DLG_CIPHER_HMAC_ALGORITHM, IDC_DLG_CIPHER_HEADER_SIZE
					};
					for (int i = 0; i < 9; i++)
						SendMessage(hWnd, WMU_SET_VALUE, idcs[i], profiles[iProfile][i]);

					EnableWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_PAGESIZE), iProfile == 0);
				}

				if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_DLG_CIPHER_LEGACY) {
					bool isLegacy = Button_GetCheck((HWND)lParam) == BST_CHECKED;
					prefs::set("cipher-legacy", isLegacy);
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_PAGESIZE), isLegacy ? SW_SHOW : SW_HIDE);
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_PAGESIZE_LABEL), isLegacy ? SW_SHOW : SW_HIDE);

					int iCipher = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_CIPHER));
					if (iCipher == CIPHER_SQLCIPHER) {
						HWND hCtrl = GetDlgItem(hWnd, IDC_DLG_CIPHER_PROFILE);
						EnableWindow(hCtrl, isLegacy);
						SendMessage(hWnd, WMU_SET_VALUE, IDC_DLG_CIPHER_PROFILE, isLegacy ? 4 : 0);

						SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_DLG_CIPHER_PROFILE, CBN_SELCHANGE), (LPARAM)hCtrl);
					}
				}

				if (wParam == IDC_DLG_HELP)
					ShellExecute(0, 0, TEXT("https://utelle.github.io/SQLite3MultipleCiphers/docs/ciphers/cipher_legacy_mode/"), 0, 0 , SW_SHOW);

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);

				if (wParam == IDC_DLG_OK || wParam == IDOK) {
					sqlite3* prefsDb = prefs::db;
					int mode = PtrToInt(GetProp(hWnd, TEXT("MODE")));
					const char* dbpath8 = (const char*)GetProp(hWnd, TEXT("DBPATH8"));
					int iCipher = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_CIPHER));

					bool isLegacy = Button_GetState(GetDlgItem(hWnd, IDC_DLG_CIPHER_LEGACY)) == BST_CHECKED;

					// Fix issue #165
					// pragma page_size = N throws error: Rekeying failed. Pagesize cannot be changed for an encrypted database.
					// Page size has to be changed independently
					int pageSizeFrom = 0, pageSizeTo = 0;
					if (iCipher > 0 && isLegacy && mode == ENCRYPT_MODE_REKEY) {
						sqlite3_stmt* stmt;
						if (SQLITE_OK == sqlite3_prepare_v2(db, "pragma page_size", -1, &stmt, 0) && SQLITE_ROW == sqlite3_step(stmt))
							pageSizeFrom = sqlite3_column_int(stmt, 0);
						sqlite3_finalize(stmt);

						TCHAR buf16[64];
						GetDlgItemText(hWnd, IDC_DLG_CIPHER_PAGESIZE, buf16, 64);
						pageSizeTo = _ttoi(buf16);

						if (GetProp(hMainWnd, TEXT("ISENCRYPT")) && pageSizeTo != 0 && pageSizeFrom != pageSizeTo) {
							MessageBox(hWnd, TEXT("New parameters require to change a page size. It doesn't supported for an encrypted database. Disable the current cipher and apply new parameters over the unencrypted database."), 0, MB_OK);
							return false;
						}

						if (pageSizeTo != 0) {
							char query8[256];
							snprintf(query8, 256, "pragma page_size = %i; vacuum;", pageSizeTo);
							if (SQLITE_OK != sqlite3_exec(db, query8, 0, 0, 0)) {
								MessageBox(hWnd, TEXT("Error: Can't change page size before rekey"), 0, MB_OK);
								return false;
							}
						}
					}

					auto setParam = [prefsDb, hWnd, mode](const char* dbpath, const char* param, int idc, int no) {
						TCHAR buf16[64];
						GetDlgItemText(hWnd, idc, buf16, 64);
						HWND hCtrl = GetDlgItem(hWnd, idc);

						if (idc == IDC_DLG_CIPHER)
							_sntprintf(buf16, 63, TEXT("%ls"), ciphers[ComboBox_GetCurSel(hCtrl)]);

						if (idc == IDC_DLG_CIPHER_LEGACY)
							_sntprintf(buf16, 63, TEXT("%i"), Button_GetState(hCtrl) == BST_CHECKED);

						if (idc == IDC_DLG_CIPHER_PROFILE)
							_sntprintf(buf16, 63, TEXT("%i"), ComboBox_GetCurSel(hCtrl));

						if (idc == IDC_DLG_CIPHER_KDF_ALGORITHM || idc == IDC_DLG_CIPHER_HMAC_ALGORITHM ||
							idc == IDC_DLG_CIPHER_HMAC_PGNO || idc == IDC_DLG_CIPHER_HMAC_USE)
							_sntprintf(buf16, 63, TEXT("%i"), ComboBox_GetCurSel(hCtrl));

						char* buf8 = utils::utf16to8(buf16);
						char sql8[1024];
						sprintf(sql8, idc == IDC_DLG_CIPHER || idc == IDC_DLG_CIPHER_KEY ? "pragma %s = '%s'" : "pragma %s = %s", param, buf8);
						int rc = true;
						if (mode != ENCRYPT_MODE_ATTACH)
							rc = SQLITE_OK == sqlite3_exec(db, sql8, 0, 0, 0);

						bool isKey = idc == IDC_DLG_CIPHER_KEY;
						bool isStoredKey = isKey && Button_GetState(GetDlgItem(hWnd, IDC_DLG_CIPHER_STORE_KEY)) == BST_CHECKED;
						if (isKey)
							sqlite3_exec(prefsDb, isStoredKey ? "delete from temp.encryption where dbpath = ?1 and param = ?2" : "delete from main.encryption where dbpath = ?1 and param = ?2", 0, 0, 0);

						sqlite3_stmt* stmt;
						if (SQLITE_OK == sqlite3_prepare_v2(prefsDb,
							isKey && isStoredKey ? "replace into main.encryption (dbpath, param, value, idc, no) values (?1, ?2, ?3, ?4, ?5)" :
							isKey && !isStoredKey ? "replace into temp.encryption (dbpath, param, value, idc, no) values (?1, ?2, ?3, ?4, ?5)" :
							"replace into main.encryption (dbpath, param, value, idc, no) values (?1, ?2, ?3, ?4, ?5)", -1, &stmt, 0)) {
							sqlite3_bind_text(stmt, 1, dbpath, strlen(dbpath), SQLITE_TRANSIENT);
							sqlite3_bind_text(stmt, 2, param, strlen(param), SQLITE_TRANSIENT);
							sqlite3_bind_text(stmt, 3, buf8, strlen(buf8), SQLITE_TRANSIENT);
							sqlite3_bind_int(stmt, 4, idc);
							sqlite3_bind_int(stmt, 5, no);
							rc = rc && SQLITE_OK == sqlite3_step(stmt);
						}
						sqlite3_finalize(stmt);

						delete [] buf8;
						return rc;
					};

					auto resetParams = [prefsDb](const char* dbpath8) {
						sqlite3_stmt* stmt;
						sqlite3_prepare_v2(prefsDb, "delete from main.encryption where dbpath = ?1", -1, &stmt, 0);
						sqlite3_bind_text(stmt, 1, dbpath8, strlen(dbpath8), SQLITE_TRANSIENT);
						sqlite3_step(stmt);
						sqlite3_finalize(stmt);

						sqlite3_prepare_v2(prefsDb, "delete from temp.encryption where dbpath = ?1", -1, &stmt, 0);
						sqlite3_bind_text(stmt, 1, dbpath8, strlen(dbpath8), SQLITE_TRANSIENT);
						sqlite3_step(stmt);
						sqlite3_finalize(stmt);
					};

					sqlite3_exec(prefs::db, "begin transaction;", 0, 0, 0);
					resetParams(dbpath8);

					if (iCipher > 0) {
						setParam(dbpath8, "cipher", IDC_DLG_CIPHER, 1);
						setParam(dbpath8, "legacy", IDC_DLG_CIPHER_LEGACY, 2);

						if (iCipher == CIPHER_WXSQLITE256 || iCipher == CIPHER_SQLEET || iCipher == CIPHER_ASCON128)
							setParam(dbpath8, "kdf_iter", IDC_DLG_CIPHER_KDF_ITER, 4);

						if (iCipher == CIPHER_SQLCIPHER) {
							int profile = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_CIPHER_PROFILE));
							if (profile > 0) {
								setParam(dbpath8, "legacy", IDC_DLG_CIPHER_LEGACY, 2);
								setParam(dbpath8, "profile", IDC_DLG_CIPHER_PROFILE, 3);
							} else  {
								setParam(dbpath8, "kdf_iter", IDC_DLG_CIPHER_KDF_ITER, 4);
								setParam(dbpath8, "fast_kdf_iter", IDC_DLG_CIPHER_FAST_KDF_ITER, 5);
								setParam(dbpath8, "kdf_algorithm", IDC_DLG_CIPHER_KDF_ALGORITHM, 6);
								setParam(dbpath8, "hmac_use", IDC_DLG_CIPHER_HMAC_USE, 7);
								setParam(dbpath8, "hmac_algorithm", IDC_DLG_CIPHER_HMAC_ALGORITHM, 8);
								setParam(dbpath8, "hmac_pgno", IDC_DLG_CIPHER_HMAC_PGNO, 9);
								setParam(dbpath8, "hmac_salt_mask", IDC_DLG_CIPHER_HMAC_SALT, 10);
								setParam(dbpath8, "plaintext_header_size", IDC_DLG_CIPHER_HEADER_SIZE, 11);
							}
						}
					}

					TCHAR key16[255];
					GetDlgItemText(hWnd, IDC_DLG_CIPHER_KEY, key16, 255);
					char* key8 = utils::utf16to8(key16);
					int rc = 0;
					if (mode == ENCRYPT_MODE_OPEN) {
						setParam(dbpath8, "key", IDC_DLG_CIPHER_KEY, 12);
						rc = sqlite3_exec(db, "select * from sqlite_master limit 1", 0, 0, 0);
					}

					if (mode == ENCRYPT_MODE_REKEY) {
						int rcBackup = MessageBox(hWnd, TEXT("Encryption can damage the database. Create a backup?"), TEXT("Alert"), MB_ICONASTERISK | MB_YESNOCANCEL);
						rc = rcBackup == IDCANCEL ? SQLITE_ABORT : SQLITE_OK;

						if (rcBackup == IDYES) {
							rc = SQLITE_ABORT;
							TCHAR path16[MAX_PATH + 1];
							char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));
							TCHAR* dbname16 = utils::utf8to16(dbname8);
							_sntprintf(path16, MAX_PATH, dbname16);
							delete [] dbname8;
							delete [] dbname16;

							if (utils::saveFile(path16, TEXT("Databases (*.sqlite, *.sqlite3, *.db, *.db3)\0*.sqlite;*.sqlite3;*.db;*.db3\0All\0*.*\0"), TEXT("sqlite"), hWnd)) {
								char* path8 = utils::utf16to8(path16);
								char query8[strlen(path8) + 256];
								sprintf(query8, "vacuum main into '%s'", path8);
								delete [] path8;
								rc = sqlite3_exec(db, query8, 0, 0, 0);
								if (rc != SQLITE_OK)
									showDbError(hWnd);
							}
						}

						if (rc == SQLITE_OK) {
							char query8[256 + strlen(key8)];
							sprintf(query8, "pragma main.rekey = '%s'", iCipher > 0 ? key8 : "");
							rc = sqlite3_exec(db, query8, 0, 0, 0);

							if (rc == SQLITE_OK) {
								if (strlen(key8) > 0 && iCipher > 0) {
									setParam(dbpath8, "key", IDC_DLG_CIPHER_KEY, 12);
									SendMessage(hMainWnd, WMU_SET_ECRYPT_FLAG, 0, 1);
								} else {
									resetParams(dbpath8);
									SendMessage(hMainWnd, WMU_SET_ECRYPT_FLAG, 0, 0);
								}
							}
						}
					}

					if (mode == ENCRYPT_MODE_ATTACH) {
						char attach8[2 * strlen(dbpath8) + strlen(key8) + 2048]{0};
						rc = getAttachString(dbpath8, 0, key8, attach8) ? sqlite3_exec(db, attach8, 0, 0, 0) : SQLITE_ERROR;
						if (rc == SQLITE_OK)
							setParam(dbpath8, "key", IDC_DLG_CIPHER_KEY, 12);
					}

					delete [] key8;
					if (rc == SQLITE_OK) {
						sqlite3_exec(prefs::db, "commit;", 0, 0, 0);
						EndDialog(hWnd, DLG_OK);
					} else {
						sqlite3_exec(prefs::db, "rollback;", 0, 0, 0);

						if (pageSizeTo != 0) {
							char query8[256];
							snprintf(query8, 256, "pragma page_size = %i; vacuum;", pageSizeFrom);
							sqlite3_exec(db, query8, 0, 0, 0);
						}
						MessageBeep(0);
					}
				}
			}
			break;

			// wParam = idc, lParam = value
			case WMU_SET_VALUE: {
				TCHAR buf[64];
				HWND hCtrl = GetDlgItem(hWnd, wParam);
				if (GetProp(hCtrl, TEXT("PROTECTED")))
					return false;

				GetClassName((HWND)hCtrl, buf, 64);
				if (wParam == IDC_DLG_CIPHER_PAGESIZE) {
					_sntprintf(buf, 63, TEXT("%i"), lParam);
					int pos = ComboBox_FindStringExact(hCtrl, 0, buf);
					ComboBox_SetCurSel(hCtrl, MAX(0, pos));
				} else if (_tcscmp(buf, WC_COMBOBOX) == 0) {
					ComboBox_SetCurSel(hCtrl, lParam);
				} else if ((_tcscmp(buf, WC_BUTTON) == 0) && (GetWindowLongPtr(hCtrl, GWL_STYLE) & BS_CHECKBOX) == BS_CHECKBOX) {
					Button_SetCheck(hCtrl, lParam > 0 ? BST_CHECKED : BST_UNCHECKED);
				} else {
					_sntprintf(buf, 63, TEXT("%i"), lParam);
					SetWindowText(hCtrl, lParam == -1 ? TEXT("") : buf);
				}
			}
			break;

			case WMU_CIPHER_CHANGED: {
				int iCipher = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_CIPHER));
				EnableWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_KEY), iCipher > 0);
				EnableWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_STORE_KEY), iCipher > 0);
				EnableWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_PAGESIZE), TRUE);

				for (int id = IDC_DLG_CIPHER_LEGACY; id <= IDC_DLG_CIPHER_HEADER_SIZE; id++)
					ShowWindow(GetDlgItem(hWnd, id), SW_HIDE);

				if (iCipher == CIPHER_NONE)
					return true;

				HWND hLegacyBox = GetDlgItem(hWnd, IDC_DLG_CIPHER_LEGACY);
				bool isInit = GetProp(hLegacyBox, TEXT("PROTECTED"));
				bool isLegacy = isInit ? Button_GetCheck(hLegacyBox) == BST_CHECKED : prefs::get("cipher-legacy") || iCipher == CIPHER_SYSTEM_DATA;

				ShowWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_LEGACY), SW_SHOW);
				SendMessage(hWnd, WMU_SET_VALUE, IDC_DLG_CIPHER_PAGESIZE, iCipher == CIPHER_SQLEET ? 4096 : 0);
				ShowWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_PAGESIZE_LABEL), isLegacy ? SW_SHOW : SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_PAGESIZE), isLegacy ? SW_SHOW : SW_HIDE);
				EnableWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_LEGACY), TRUE);

				if (iCipher == CIPHER_WXSQLITE256 || iCipher == CIPHER_SQLEET || iCipher == CIPHER_ASCON128) {
					HWND hLabel = GetDlgItem(hWnd, IDC_DLG_CIPHER_KDF_ITER_LABEL);
					HWND hCtrl = GetDlgItem(hWnd, IDC_DLG_CIPHER_KDF_ITER);

					ShowWindow(hLabel, SW_SHOW);
					ShowWindow(hCtrl, SW_SHOW);
					EnableWindow(hLabel, TRUE);
					EnableWindow(hCtrl, TRUE);
					int kdf_iter = iCipher == CIPHER_WXSQLITE256 ? 4001 :
						iCipher == CIPHER_SQLEET ? 64007 :
						iCipher == CIPHER_ASCON128 ? 64007 : 0;
					SendMessage(hWnd, WMU_SET_VALUE, IDC_DLG_CIPHER_KDF_ITER, kdf_iter);

					RECT rc{5, 41 + 15 * 1, 165, 41 + 15 * 2};
					MapDialogRect(hWnd, &rc);
					SetWindowPos(hLabel, 0, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
					SetWindowPos(hCtrl, 0, rc.right, rc.top - 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				}

				if (iCipher == CIPHER_SQLCIPHER) {
					RECT rc{5, 41 + 15 * 1, 165, 41 + 15 * 2};
					MapDialogRect(hWnd, &rc);
					SetWindowPos(GetDlgItem(hWnd, IDC_DLG_CIPHER_KDF_ITER_LABEL), 0, rc.left, rc.bottom, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
					SetWindowPos(GetDlgItem(hWnd, IDC_DLG_CIPHER_KDF_ITER), 0, rc.right, rc.bottom - 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

					for (int id = IDC_DLG_CIPHER_PROFILE_LABEL; id <= IDC_DLG_CIPHER_HEADER_SIZE; id++)
						ShowWindow(GetDlgItem(hWnd, id), SW_SHOW);

					for (int id = IDC_DLG_CIPHER_KDF_ITER; id <= IDC_DLG_CIPHER_HEADER_SIZE; id += 2)
						EnableWindow(GetDlgItem(hWnd, id), FALSE);

					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_DLG_CIPHER_LEGACY, BN_CLICKED), (LPARAM)GetDlgItem(hWnd, IDC_DLG_CIPHER_LEGACY));
				}

				if (iCipher == CIPHER_SYSTEM_DATA) {
					// CIPHER_SYSTEM_DATA supports only legacy mode
					EnableWindow(GetDlgItem(hWnd, IDC_DLG_CIPHER_LEGACY), TRUE);
					Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_CIPHER_LEGACY), BST_CHECKED);
				}
			}
			break;

			case WM_CLOSE: {
				RemoveProp(hWnd, TEXT("MODE"));
				RemoveProp(hWnd, TEXT("DBPATH8"));
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	LRESULT CALLBACK cbNewEditDataEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_GETDLGCODE: {
				return DLGC_WANTALLKEYS | CallWindowProc(cbOldEditDataEdit, hWnd, msg, wParam, lParam);
			}
			break;

			case WM_DESTROY: {
				HWND hListWnd = GetParent(hWnd);
				BOOL isChanged = !!GetProp(hWnd, TEXT("ISCHANGED"));
				if (isChanged) {
					int len = GetWindowTextLength(hWnd);
					TCHAR* value16 = new TCHAR[len + 1]{0};
					GetWindowText(hWnd, value16, len + 1);
					char* value8 = utils::utf16to8(value16);
					SendMessage(GetParent(GetParent(hWnd)), WMU_SET_CURRENT_CELL_VALUE, (WPARAM)value8, 0);
					delete [] value16;
					delete [] value8;
				}

				if (!FindWindowExW(hListWnd, 0, WC_COMBOBOX, NULL))
					SetFocus(hListWnd);

				RemoveProp(hWnd, TEXT("ISCHANGED"));
			}
			break;

			case WM_KILLFOCUS: {
				DestroyWindow(hWnd);
			}
			break;

			case WM_KEYDOWN: {
				SetProp(hWnd, TEXT("ISCHANGED"), (HANDLE)1);
				if (wParam == VK_SPACE && HIWORD(GetKeyState(VK_CONTROL))) {
					SetProp(hWnd, TEXT("ISCHANGED"), 0);

					int size = GetWindowTextLength(hWnd);
					TCHAR* value16 = new TCHAR[size + 1]{0};
					GetWindowText(hWnd, value16, size + 1);
					SendMessage(GetAncestor(hWnd, GA_ROOT), WMU_CREATE_VALUE_SELECTOR, (WPARAM)value16, 0);
					delete [] value16;
					return true;
				}

				if (wParam == VK_RETURN) {
					DestroyWindow(hWnd);
					return true;
				}

				if (wParam == VK_ESCAPE) {
					SetProp(hWnd, TEXT("ISCHANGED"), 0);
					DestroyWindow(hWnd);
					return true;
				}

				if (wParam == 0x41 && HIWORD(GetKeyState(VK_CONTROL))) { // Ctrl + A
					SendMessage(hWnd, EM_SETSEL, 0, -1);
					return true;
				}
			}
			break;

			case WM_PASTE:
			case WM_CUT: {
				SetProp(hWnd, TEXT("ISCHANGED"), (HANDLE)1);
			}
			break;

			case WM_LBUTTONDBLCLK: {
				if (GetWindowTextLength(hWnd) == 0) {
					return SendMessage(GetAncestor(hWnd, GA_ROOT), WMU_CREATE_VALUE_SELECTOR, 0, 0);
				}
			}
			break;
		}

		return CallWindowProc(cbOldEditDataEdit, hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK cbNewAddTableCell(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_GETDLGCODE)
			return (DLGC_WANTALLKEYS | CallWindowProc(cbOldAddTableCell, hWnd, msg, wParam, lParam));

		switch(msg){
			case WM_DESTROY: {
				int data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
				if (!data) // Exit by Esc
					return 0;

				HWND hListWnd = GetParent(hWnd);
				int size = GetWindowTextLength(hWnd) + 1;
				TCHAR value16[size]{0};
				GetWindowText(hWnd, value16, size);
				ListView_SetItemText(hListWnd, LOWORD(data), HIWORD(data), value16);

				HWND parent = GetParent(hWnd);
				InvalidateRect(parent, 0, TRUE);
			}
			break;

			case WM_KEYDOWN: {
				if (wParam == VK_RETURN) {
					DestroyWindow(hWnd);
				}

				if (wParam == VK_ESCAPE){
					SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
					DestroyWindow(hWnd);
				}
			}
			break;
		}

		return CallWindowProc(cbOldAddTableCell, hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK cbNewAddTableComboboxEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_GETDLGCODE)
			return (DLGC_WANTALLKEYS | CallWindowProc(cbOldAddTableComboboxEdit, hWnd, msg, wParam, lParam));

		if (msg == WM_CHAR && (wParam == VK_RETURN || wParam == VK_ESCAPE))
			SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, lParam);

		return CallWindowProc(cbOldAddTableComboboxEdit, hWnd, msg, wParam, lParam);
	}

	// USERDATA = the last column under cursor
	LRESULT CALLBACK cbNewAddTableHeader(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_MOUSEMOVE) {
			LONG x = GET_X_LPARAM(lParam);
			LONG y = GET_Y_LPARAM(lParam);
			HD_HITTESTINFO hi{0};
			hi.pt = {x, y};

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);

			SendMessage(hWnd, HDM_HITTEST, 0, (LPARAM)&hi);
			if (hi.iItem == -1) {
				hideTooltip();
				return true;
			}

			if (GetWindowLongPtr(hWnd, GWLP_USERDATA) == hi.iItem)
				return true;

			RECT rc;
			SendMessage(hWnd, HDM_GETITEMRECT, hi.iItem, (LPARAM)&rc);
			POINT p{(rc.right + rc.left)/2, rc.bottom + 5};
			ClientToScreen(hWnd, &p);

			TCHAR text16[1024];
			_sntprintf(text16, 1023, TEXT("%s"), tooltips[hi.iItem]);
			showTooltip(p.x, p.y, text16);

			SetWindowLongPtr(hWnd, GWLP_USERDATA, hi.iItem);
		}

		if ((msg == WM_MOUSELEAVE)) {
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 1000);
			hideTooltip();
		}

		return CallWindowProc(cbOldAddTableHeader, hWnd, msg, wParam, lParam);
	}


	LRESULT CALLBACK cbNewFilterEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch(msg){
			case WM_GETDLGCODE: {
				return (DLGC_WANTALLKEYS | CallWindowProc(cbOldHeaderEdit, hWnd, msg, wParam, lParam));
			}
			break;

			case WM_PAINT: {
				cbOldHeaderEdit(hWnd, msg, wParam, lParam);
				if (GetDlgCtrlID(hWnd) == IDC_DLG_FILTER)
					return TRUE;

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
					HWND hDlgWnd = GetAncestor(hWnd, GA_ROOT);
					if (wParam == VK_RETURN)
						SendMessage(hDlgWnd, WMU_UPDATE_DATA, 0, 0);

					if (wParam == VK_ESCAPE)
						SendMessage(hDlgWnd, WM_CLOSE, 0, 0);

					if (wParam == VK_TAB) {
						HWND hListWnd = GetDlgItem(hDlgWnd, IDC_DLG_ROWS);
						HWND hHeader = ListView_GetHeader(hListWnd);

						HWND hFocusWnd = GetDlgItem(hHeader, IDC_HEADER_EDIT + 1);
						if (GetParent(hWnd) == hHeader)	{
							int colCount = Header_GetItemCount(hHeader);
							int colNo = GetDlgCtrlID(hWnd) - IDC_HEADER_EDIT;
							hFocusWnd = colNo < colCount - 2 ?
								GetDlgItem(hHeader, IDC_HEADER_EDIT + colNo + 1) :
								GetDlgItem(GetDlgItem(hDlgWnd, IDC_DLG_TOOLBAR), IDC_DLG_FILTER);
						}
						SetFocus(hFocusWnd);
					}

					return 0;
				}
			}
			break;
		}

		if (processEditKeys(hWnd, msg, wParam, lParam))
			return 0;

		return CallWindowProc(cbOldHeaderEdit, hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK cbNewRowEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_GETDLGCODE)
			return DLGC_WANTTAB | DLGC_WANTCHARS | DLGC_WANTARROWS;

		if (msg == WM_CHAR) {
			bool isControl = HIWORD(GetKeyState(VK_CONTROL));
			if (wParam == VK_TAB || (wParam == VK_SPACE && isControl))
				return true;
		}

		if (msg == WM_KEYDOWN) {
			HWND hDlg = GetAncestor(hWnd, GA_ROOT);
			HWND hParentWnd = GetParent(hWnd);
			bool isControl = HIWORD(GetKeyState(VK_CONTROL));

			// Move focus
			if (wParam == VK_TAB) {
				if (HIWORD(GetKeyState(VK_CONTROL))) {
					int id = IDC_ROW_SWITCH + GetDlgCtrlID(hWnd) - IDC_ROW_EDIT;
					SendMessage(hParentWnd, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)GetDlgItem(hParentWnd, id));
					return 0;
				} if (HIWORD(GetKeyState(VK_SHIFT))) {
					int colCount = HIWORD(GetWindowLongPtr(hDlg, GWLP_USERDATA));
					int id = GetDlgCtrlID(hWnd) - 1;
					id = id <= IDC_ROW_EDIT ? IDC_ROW_EDIT + colCount - 1 : id;
					SetFocus(GetDlgItem(hParentWnd, id));
					return 0;
				} else {
					SendMessage(hDlg, WM_NEXTDLGCTL, 0, 0);
				}
			}

			if (wParam == VK_SPACE && isControl) {
				if (GetWindowLong(hWnd, GWL_STYLE) & ES_READONLY)
					return 0;

				HWND hDlg = GetAncestor(hWnd, GA_ROOT);
				int colNo = GetDlgCtrlID(hWnd) - IDC_ROW_EDIT;
				SetProp(hDlg, TEXT("CURRENTEDIT"), hWnd);
				SetProp(hDlg, TEXT("CURRENTCOLUMN"), IntToPtr(colNo));

				HWND hEditDataDlg = GetWindow(hDlg, GW_OWNER);
				if (GetWindowLongPtr(hEditDataDlg, DWLP_USER) != IDD_EDITDATA) {
					MessageBox(hWnd, TEXT("Internal error"), 0, MB_OK);
					return 0;
				}
				char** fkSelects = (char**)GetProp(hEditDataDlg, TEXT("FKSELECTS"));
				bool hasFK = fkSelects && colNo != -1 && fkSelects[colNo];
				if (hasFK) {
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_FK_SELECTOR), hWnd, (DLGPROC)cbDlgFKSelector, (LPARAM)hDlg);
				} else {
					RECT rc;
					GetWindowRect(hWnd, &rc);
					MapWindowPoints(HWND_DESKTOP, hDlg, (POINT*)&rc, 2);

					char* tablename8 = (char*)GetProp(hEditDataDlg, TEXT("TABLENAME8"));
					char* schema8 = (char*)GetProp(hEditDataDlg, TEXT("SCHEMA8"));

					TCHAR column16[256]{0};
					GetDlgItemText(GetParent(hWnd), IDC_ROW_LABEL + colNo, column16, 255);
					char* column8 = utils::utf16to8(column16);

					TCHAR value16[1024]{0};
					GetWindowText(hWnd, value16, 1023);

					createValueSelector(hDlg, &rc, value16, schema8, tablename8, column8);
					delete [] column8;
				}
				return 0;
			}
		}

		if (processEditKeys(hWnd, msg, wParam, lParam))
			return 0;

		return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK cbNewScroll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_COMMAND: {
				SendMessage(GetParent(hWnd), WM_COMMAND, wParam, lParam);
			}
			break;

			case WM_VSCROLL: {
				WORD action = LOWORD(wParam);
				int scrollY = (int)(LONG_PTR)GetProp(hWnd, TEXT("SCROLLY"));

				int pos = action == SB_THUMBPOSITION ? HIWORD(wParam) :
					action == SB_THUMBTRACK ? HIWORD(wParam) :
					action == SB_LINEDOWN ? scrollY + 30 :
					action == SB_LINEUP ? scrollY - 30 :
					-1;

				if (pos == -1)
					break;

				SCROLLINFO si{0};
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_POS;
				si.nPos = pos;
				si.nTrackPos = 0;
				SetScrollInfo(hWnd, SB_VERT, &si, true);
				GetScrollInfo(hWnd, SB_VERT, &si);
				pos = si.nPos;
				POINT p{0, pos - scrollY};
				HDC hdc = GetDC(hWnd);
				LPtoDP(hdc, &p, 1);
				ReleaseDC(hWnd, hdc);
				ScrollWindow(hWnd, 0, -p.y, NULL, NULL);
				SetProp(hWnd, TEXT("SCROLLY"), IntToPtr(pos));

				return 0;
			}
			break;

			case WMU_SET_SCROLL_HEIGHT: {
				RECT rc = { 0 };
				GetWindowRect(hWnd, &rc);
				SCROLLINFO si = { 0 };
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_ALL;
				si.nMin = 0;
				si.nMax = wParam;
				si.nPage = rc.bottom - rc.top;
				si.nPos = 0;
				si.nTrackPos = 0;

				int pos = GetScrollPos(hWnd, SB_VERT);
				ScrollWindow(hWnd, 0, -GetScrollPos(hWnd, SB_VERT), NULL, NULL);
				SetScrollInfo(hWnd, SB_VERT, &si, FALSE);
				SetScrollPos(hWnd, SB_VERT, pos, TRUE);
			}
			break;

			case WM_DESTROY: {
				RemoveProp(hWnd, TEXT("SCROLLY"));
			}
			break;

			case WM_CTLCOLOREDIT:
			case WM_CTLCOLORSTATIC: {
				LRESULT res = SendMessage(GetParent(hWnd), WMU_CTLCOLOREDIT, wParam, lParam);
				if (res)
					return res;
			}
			break;

			case WM_NCMOUSEMOVE:
			case WM_NCLBUTTONDOWN:
			case WM_NCLBUTTONUP:
			case WM_NCLBUTTONDBLCLK:
			case WM_NCRBUTTONDOWN:
			case WM_NCRBUTTONUP:
			case WM_NCRBUTTONDBLCLK:
			case WM_NCMBUTTONDOWN:
			case WM_NCMBUTTONUP :
			case WM_NCMBUTTONDBLCLK:
			case WM_NCHITTEST: {
				return CallWindowProc(DefWindowProc, hWnd, msg, wParam, lParam);
			}
			break;
		}

		return CallWindowProc((WNDPROC)GetProp(hWnd, TEXT("WNDPROC")), hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK cbNewChart(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_DESTROY: {
				RemoveProp(hWnd, TEXT("SCROLLY"));
				RemoveProp(hWnd, TEXT("ZOOM"));
				RemoveProp(hWnd, TEXT("POSITION"));
			}
			break;

			case WM_PAINT : {
				RECT rc{0};
				GetClientRect(hWnd, &rc);
				int w = rc.right;
				int h = rc.bottom;

				PAINTSTRUCT ps{0};
				ps.fErase = FALSE;
				HDC hdc = BeginPaint(hWnd, &ps);

				// Double buffering https://stackoverflow.com/a/25461603/6121703
				HDC memDC = CreateCompatibleDC(hdc);
				HBITMAP hBmp = CreateCompatibleBitmap(hdc, w, h);
				HBITMAP hOldBmp = (HBITMAP)SelectObject(memDC, hBmp);

				drawChart(hWnd, memDC, w, h, (int)(LONG_PTR)GetProp(hWnd, TEXT("SCROLLY")));
				BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

				SelectObject(memDC, hOldBmp);
				DeleteObject(hBmp);
				DeleteDC(memDC);

				EndPaint(hWnd, &ps);

				return true;
			}
			break;

			case WM_CONTEXTMENU: {
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				HMENU hMenu = (HMENU)GetProp(GetParent(hWnd), TEXT("MENU"));
				TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDM_EXPORT_PNG || wParam == IDM_EXPORT_CLIPBOARD) {
					RECT rc;
					GetClientRect(hWnd, &rc);

					SCROLLINFO si{0};
					si.cbSize = sizeof(si);
					si.fMask = SIF_RANGE;
					GetScrollInfo(hWnd, SB_VERT, &si);
					rc.bottom = MAX(rc.bottom, si.nMax);

					HDC hDC = GetDC(hWnd);
					HDC hCompatDC = CreateCompatibleDC(hDC);
					HBITMAP hBitmap = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
					SelectObject(hCompatDC, hBitmap);

					drawChart(hWnd, hCompatDC, rc.right, rc.bottom, -1);

					TCHAR path[MAX_PATH + 1];
					_sntprintf(path, MAX_PATH, TEXT("chart.png"));
					if (wParam == IDM_EXPORT_PNG && utils::saveFile(path, TEXT("PNG files\0*.png\0All\0*.*\0"), TEXT("png"), hWnd)) {
						const CLSID pngClsid = { 0x557cf406, 0x1a04, 0x11d3, {0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};

						Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(hBitmap, NULL);
						bitmap->Save(path, &pngClsid, NULL);
						delete bitmap;
					}

					if (wParam == IDM_EXPORT_CLIPBOARD) {
						OpenClipboard(hWnd);
						EmptyClipboard();
						SetClipboardData(CF_BITMAP, hBitmap);
						CloseClipboard();
					}

					DeleteDC(hCompatDC);
					ReleaseDC(hWnd, hDC);
					DeleteObject(hBitmap);
				}

				if (wParam == IDM_CHART_RESET) {
					SetProp(hWnd, TEXT("ZOOM"), IntToPtr(10));
					SetProp(hWnd, TEXT("POSITION"), 0);
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}
			break;

			case WM_LBUTTONDOWN: {
				SetProp(hWnd, TEXT("LAST_POSITION"), IntToPtr(GET_X_LPARAM(lParam)));
				SetCapture(hWnd);
				SetFocus(hWnd);
				return true;
			}
			break;

			case WM_LBUTTONUP: {
				ReleaseCapture();
				return true;
			}
			break;

			case WM_MOUSEMOVE: {
				HWND hParentWnd = GetParent(hWnd);
				int type = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("TYPE"));

				if (wParam & MK_LBUTTON) {
					int pos = PtrToInt(GetProp(hWnd, TEXT("POSITION")));
					int delta = PtrToInt(GetProp(hWnd, TEXT("LAST_POSITION"))) - GET_X_LPARAM(lParam);
					SetProp(hWnd, TEXT("POSITION"), IntToPtr(pos - delta));
					SetProp(hWnd, TEXT("LAST_POSITION"), IntToPtr(GET_X_LPARAM(lParam)));
					InvalidateRect(hWnd, NULL, TRUE);
				}

				if (type == CHART_LINES || type == CHART_DOTS || type == CHART_AREAS || type == CHART_HISTOGRAM) {
					int* colTypes = (int*)GetProp(hParentWnd, TEXT("COLTYPES"));
					int colBase = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("COLBASE"));

					double* minmax = (double*)GetProp(hParentWnd, TEXT("MINMAX"));
					double minX = minmax[0];
					double maxX = minmax[1];
					double minY = minmax[2];
					double maxY = minmax[3];
					bool isDate = colTypes[colBase] == CHART_DATE;

					RECT rc{0};
					GetClientRect(hWnd, &rc);
					int w = rc.right;
					int h = rc.bottom;

					double zoom = PtrToInt(GetProp(hWnd, TEXT("ZOOM"))) / 10.0;
					minX /= zoom;
					maxX /= zoom;
					int offsetX = PtrToInt(GetProp(hWnd, TEXT("POSITION")));

					TCHAR title[255]{0};
					double x = GET_X_LPARAM(lParam) - offsetX;
					double y = h - GET_Y_LPARAM(lParam);
					x = map(x, CHART_BORDER, w - CHART_BORDER, minX, maxX);
					y = map(y, CHART_BORDER, h - CHART_BORDER, minY, maxY);

					if (isDate) {
						TCHAR val[64];
						time_t rawtime = x;
						struct tm ts = *localtime(&rawtime);
						_tcsftime(val, 64, TEXT("%Y-%m-%d %H:%M"), &ts);
						_sntprintf(title, 63, TEXT("X: %ls, Y: %g"), val, y);
					} else {
						_sntprintf(title, 63, TEXT("X: %g, Y: %g"), x, y);
					}
					SetWindowText(hParentWnd, title);
				}
			}
			break;

			case WM_MOUSEWHEEL: {
				HWND hParentWnd = GetParent(hWnd);

				int type = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("TYPE"));
				bool isUp = GET_WHEEL_DELTA_WPARAM(wParam) > 0;
				if (type == CHART_BARS) {
					int action = isUp ? SB_LINEUP : SB_LINEDOWN;
					SendMessage(hWnd, WM_VSCROLL, MAKELPARAM(action, 0), 0);
				} else {
					RECT rc{0};
					GetClientRect(hWnd, &rc);
					int w = rc.right;

					POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
					ScreenToClient(hWnd, &p);
					double aspect = (double)(p.x - CHART_BORDER) / (w - 2.0 * CHART_BORDER);

					int zoom = PtrToInt(GetProp(hWnd, TEXT("ZOOM")));
					int pos = PtrToInt(GetProp(hWnd, TEXT("POSITION")));

					int prevZoom = zoom;
					zoom = MAX(zoom + (isUp ? 2 : -2), 10);
					pos -= (w - 2.0 * CHART_BORDER) * (zoom - prevZoom) / 10. * aspect;

					SetProp(hWnd, TEXT("ZOOM"), IntToPtr(zoom));
					SetProp(hWnd, TEXT("POSITION"), IntToPtr(pos));

					InvalidateRect(hWnd, NULL, TRUE);
				}
			}
			break;

			case WM_VSCROLL: {
				WORD action = LOWORD(wParam);
				int scrollY = (int)(LONG_PTR)GetProp(hWnd, TEXT("SCROLLY"));

				int pos = action == SB_THUMBPOSITION ? HIWORD(wParam) :
					action == SB_THUMBTRACK ? HIWORD(wParam) :
					action == SB_LINEDOWN ? scrollY + 30 :
					action == SB_LINEUP ? scrollY - 30 :
					-1;

				if (pos == -1)
					break;

				SCROLLINFO si{0};
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_POS;
				si.nPos = pos;
				si.nTrackPos = 0;
				SetScrollInfo(hWnd, SB_VERT, &si, true);
				GetScrollInfo(hWnd, SB_VERT, &si);
				pos = si.nPos;
				POINT p{0, pos - scrollY};
				HDC hdc = GetDC(hWnd);
				LPtoDP(hdc, &p, 1);
				ReleaseDC(hWnd, hdc);
				ScrollWindow(hWnd, 0, -p.y, NULL, NULL);
				SetProp(hWnd, TEXT("SCROLLY"), IntToPtr(pos));
				SetFocus(hWnd);

				return 0;
			}
			break;

			case WM_SIZE: {
				HWND hParentWnd = GetParent(hWnd);

				int type = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("TYPE"));
				if (type == CHART_BARS) {
					int colCount = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("COLCOUNT"));
					int rowCount = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("ROWCOUNT"));
					int dataColCount = 0;
					HWND hOptionsWnd = GetDlgItem(hParentWnd, IDC_DLG_CHART_OPTIONS);
					for (int colNo = 0; colNo < colCount; colNo++) {
						HWND hColumnWnd = GetDlgItem(hOptionsWnd, IDC_DLG_CHART_COLUMN + colNo);
						dataColCount += IsWindowEnabled(hColumnWnd) && Button_GetCheck(hColumnWnd) == BST_CHECKED;
					}

					SCROLLINFO si{0};
					si.cbSize = sizeof(SCROLLINFO);
					si.fMask = SIF_ALL;
					si.nMin = 0;
					si.nMax = CHART_BORDER/2 + rowCount * dataColCount * (CHART_BAR_HEIGHT + CHART_BAR_SPACE + 10); // scrollable height
					si.nPage = HIWORD(lParam); // window height
					si.nPos = 0;
					si.nTrackPos = 0;
					SetScrollInfo(hWnd, SB_VERT, &si, true);
					SetProp(hWnd, TEXT("SCROLLY"), 0);
				} else {
					PostMessage(hWnd, WM_COMMAND, IDM_CHART_RESET, 0);
				}

				InvalidateRect(hWnd, 0, TRUE);

				return true;
			}
			break;
		}

		return CallWindowProc(DefWindowProc, hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK cbNewChartOptions(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_MOUSEMOVE: {
				SetWindowText(GetParent(hWnd), TEXT(""));
			}
			break;

			case WM_CTLCOLORSTATIC: {
				HWND hParentWnd = GetParent(hWnd);
				int colCount = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("COLCOUNT"));
				BYTE* colColors = (BYTE*)GetProp(hParentWnd, TEXT("COLCOLORS"));

				for (int colNo = 1; colNo < colCount; colNo++) {
					HWND hColumnWnd = GetDlgItem(hWnd, IDC_DLG_CHART_COLUMN + colNo);
					bool isChecked = Button_GetCheck(hColumnWnd) == BST_CHECKED;
					BYTE colorNo = colColors[colNo];
					if (hColumnWnd == (HWND)lParam && isChecked) {
						HDC hDC = (HDC)wParam;
						SetTextColor(hDC, RGB(255, 255, 255));
						SetBkColor(hDC, COLORS[colorNo]);

						return (LRESULT)hBrushes[colorNo];
					}
				}
			}
			break;

			case WM_COMMAND: {
				HWND hParentWnd = GetParent(hWnd);
				HWND hChartWnd = GetDlgItem(hParentWnd, IDC_DLG_CHART);
				int colCount = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("COLCOUNT"));

				if (LOWORD(wParam) == IDC_DLG_CHART_TYPE && HIWORD(wParam) == CBN_SELCHANGE) {
					HWND hTypeWnd = GetDlgItem(hWnd, IDC_DLG_CHART_TYPE);
					HWND hBaseLabel = GetDlgItem(hWnd, IDC_DLG_CHART_BASE_LABEL);
					HWND hBaseWnd = GetDlgItem(hWnd, IDC_DLG_CHART_BASE);

					int type = ComboBox_GetItemData(hTypeWnd, ComboBox_GetCurSel(hTypeWnd));
					SetProp(hParentWnd, TEXT("TYPE"), IntToPtr(type));

					SetWindowText(hBaseLabel, type == CHART_BARS ? TEXT("Group by") : TEXT("Axis-X"));
					ComboBox_ResetContent(hBaseWnd);

					int* colTypes = (int*)GetProp(hParentWnd, TEXT("COLTYPES"));

					bool isFirstColumn = true;
					for (int colNo = 0; colNo < colCount; colNo++) {
						HWND hColumnWnd = GetDlgItem(hWnd, IDC_DLG_CHART_COLUMN + colNo);

						if (((type == CHART_LINES || type == CHART_DOTS || type == CHART_AREAS || type == CHART_HISTOGRAM) &&
							(colTypes[colNo] == CHART_NUMBER || colTypes[colNo] == CHART_DATE)) ||
							(type == CHART_BARS && (colTypes[colNo] == CHART_TEXT || colTypes[colNo] == CHART_DATE))) {
							TCHAR buf[256];
							GetWindowText(hColumnWnd, buf, 255);
							int pos = ComboBox_AddString(hBaseWnd, buf);
							ComboBox_SetItemData(hBaseWnd, pos, colNo);

							if (isFirstColumn) {
								isFirstColumn = false;
								SetWindowLongPtr(hBaseWnd, GWLP_USERDATA, colNo);
							}
						}

						EnableWindow(hColumnWnd, colTypes[colNo] == CHART_NUMBER);
					}
					ComboBox_SetCurSel(hBaseWnd, 0);
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_DLG_CHART_BASE, CBN_SELCHANGE), (LPARAM)hBaseWnd);

					if (type == CHART_BARS) {
						RECT rc{0};
						GetClientRect(hChartWnd, &rc);
						PostMessage(hChartWnd, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
					} else {
						SCROLLINFO si{0};
						si.cbSize = sizeof(SCROLLINFO);
						si.fMask = SIF_ALL;
						SetScrollInfo(hChartWnd, SB_VERT, &si, true);
					}
				}

				if (LOWORD(wParam) == IDC_DLG_CHART_BASE && HIWORD(wParam) == CBN_SELCHANGE) {
					HWND hParentWnd = GetParent(hWnd);
					int* colTypes = (int*)GetProp(hParentWnd, TEXT("COLTYPES"));
					BYTE* colColors = (BYTE*)GetProp(hParentWnd, TEXT("COLCOLORS"));
					bool* isColumns = (bool*)GetProp(hParentWnd, TEXT("ISCOLUMNS"));

					HWND hBaseWnd = GetDlgItem(hWnd, IDC_DLG_CHART_BASE);
					int colBase = ComboBox_GetItemData(hBaseWnd, ComboBox_GetCurSel(hBaseWnd));
					SetWindowLongPtr(hBaseWnd, GWLP_USERDATA, colBase);
					SetProp(hParentWnd, TEXT("COLBASE"), IntToPtr(colBase));

					int no = 1;
					for (int colNo = 1; colNo < colCount; colNo++) {
						colColors[colNo] = 0;

						bool isVisible = (colTypes[colNo] == CHART_NUMBER) && (colNo != colBase);
						HWND hColumnWnd = GetDlgItem(hWnd, IDC_DLG_CHART_COLUMN + colNo);
						if (isVisible) {
							SetWindowPos(hColumnWnd, 0, 10, 60 + no * 25, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
							colColors[colNo] = no % MAX_CHART_COLOR_COUNT;
							no++;
						}
						EnableWindow(hColumnWnd, isVisible);
						ShowWindow(hColumnWnd, isVisible ? SW_SHOW : SW_HIDE);
						isColumns[colNo] = isVisible && Button_GetCheck(hColumnWnd) == BST_CHECKED;
					}

					SendMessage(hParentWnd, WMU_RESORT_DATA, 0, 0);
					SendMessage(hParentWnd, WMU_UPDATE_MINMAX, 0, 0);
					SendMessage(hChartWnd, WM_PAINT, 0, 0);
				}

				if (LOWORD(wParam) > IDC_DLG_CHART_COLUMN && (LOWORD(wParam) < IDC_DLG_CHART_COLUMN + colCount) && HIWORD(wParam) == BN_CLICKED) {
					bool* isColumns = (bool*)GetProp(hParentWnd, TEXT("ISCOLUMNS"));
					isColumns[wParam - IDC_DLG_CHART_COLUMN] = Button_GetCheck((HWND)lParam) == BST_CHECKED;

					SendMessage(hParentWnd, WMU_UPDATE_MINMAX, 0, 0);
					SendMessage(hChartWnd, WM_PAINT, 0, 0);
					InvalidateRect(hWnd, NULL, true);

					int type = (int)(LONG_PTR)GetProp(hParentWnd, TEXT("TYPE"));
					if (type == CHART_BARS) {
						RECT rc{0};
						GetClientRect(hChartWnd, &rc);
						PostMessage(hChartWnd, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
					}
				}
			}
			break;
		}

		return CallWindowProc(cbOldChartOptions, hWnd, msg, wParam, lParam);
	}
}
