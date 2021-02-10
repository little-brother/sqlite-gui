#include "resource.h"
#include "global.h"
#include "prefs.h"
#include "utils.h"
#include "dialogs.h"
#include "tools.h"

namespace dialogs {
	WNDPROC cbOldEditDataEdit, cbOldAddTableCell, cbOldHeaderEdit;
	LRESULT CALLBACK cbNewEditDataEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewAddTableCell(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK cbNewFilterEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgEditDataValue (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool ListView_UpdateCell(HWND hListWnd, int rowNo, int colNo, TCHAR* value16);

	const TCHAR* DATATYPES16[] = {TEXT("integer"), TEXT("real"), TEXT("text"), TEXT("null"), TEXT("blob"), TEXT("json"), 0};
	const TCHAR* INDENT_LABELS[] = {TEXT("Tab"), TEXT("2 spaces"), TEXT("4 spaces"), 0};
	const TCHAR* INDENTS[] = {TEXT("\t"), TEXT("  "), TEXT("    ")};

	bool isRequireHighligth = false;
	bool isRequireParenthesisHighligth = false;

	BOOL CALLBACK cbDlgAddEdit (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				bool isEdit = lParam == IDM_EDIT;

				TCHAR name16[256] = {0};
				TV_ITEM tv;
				tv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
				tv.hItem = treeItems[0];
				tv.pszText = name16;
				tv.cchTextMax = 256;

				if(!TreeView_GetItem(hTreeWnd, &tv) || !tv.lParam || tv.lParam == COLUMN)
					return EndDialog(hWnd, -1);

				int type = abs(tv.lParam);
				SetWindowLong(hWnd, GWL_USERDATA, type);

				HWND hDlgEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				TCHAR buf[512];
				_stprintf(buf, isEdit ? TEXT("Edit %s \"%s\"") : TEXT("Add %s"), TYPES16[type], name16);
				SetWindowText(hWnd, buf);

				SendMessage(hDlgEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_KEYEVENTS);
				setEditorFont(hDlgEditorWnd);

				SetFocus(hDlgEditorWnd);

				if (isEdit) {
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_EXAMPLE), SW_HIDE);
					TCHAR* sql16 = getDDL(name16, type, true);
					if (sql16) {
						SetWindowText(hDlgEditorWnd, sql16);
						delete [] sql16;
					} else {
						SetWindowText(hDlgEditorWnd, TEXT("Error to get DDL"));
					}
				}
			}
			break;

			case WM_CONTEXTMENU: {
				POINT p = {LOWORD(lParam), HIWORD(lParam)};
				bool isContextKey = p.x == 65535 && p.y == 65535;
				if ((HWND)wParam == GetDlgItem(hWnd, IDC_DLG_EDITOR) && !isContextKey)
					TrackPopupMenu(hEditorMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
			}
			break;

			case WM_COMMAND: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);

				if (LOWORD(wParam) == IDC_DLG_EDITOR && HIWORD(wParam) == EN_CHANGE && prefs::get("use-highlight") && !isRequireHighligth) {
					PostMessage(hWnd, WMU_HIGHLIGHT, 0, 0);
					isRequireHighligth = true;
				}

				if (wParam == IDC_DLG_EXAMPLE) {
					TCHAR buf[1024];
					int type = GetWindowLong(hWnd, GWL_USERDATA);
					LoadString(GetModuleHandle(NULL), IDS_CREATE_DDL + type, buf, 1024);
					SetWindowText(GetDlgItem(hWnd, IDC_DLG_EDITOR), buf);
				}

				if (wParam == IDM_EDITOR_CUT)
					SendMessage(hEditorWnd, WM_CUT, 0, 0);

				if (wParam == IDM_EDITOR_COPY)
					SendMessage(hEditorWnd, WM_COPY, 0, 0);

				if (wParam == IDM_EDITOR_PASTE)
					SendMessage(hEditorWnd, WM_PASTE, 0, 0);

				if (wParam == IDM_EDITOR_DELETE)
					SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, 0);

				if (wParam == IDC_DLG_OK) {
					int size = GetWindowTextLength(hEditorWnd) + 1;
					TCHAR query16[size]{0};
					GetWindowText(hEditorWnd, query16, size);
					char* query8 = utils::utf16to8(query16);
					int rc = sqlite3_exec(db, query8, NULL, 0 , 0);
					delete [] query8;

					SetFocus(hEditorWnd);
					if (SQLITE_OK == rc) {
						EndDialog(hWnd, DLG_OK);
					} else {
						showDbError(hMainWnd);
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (wParam == IDC_DLG_EDITOR && pHdr->code == EN_SELCHANGE && !isRequireParenthesisHighligth) {
					SELCHANGE *pSc = (SELCHANGE *)lParam;
					if (pSc->seltyp > 0)
						return 1;

					PostMessage(hWnd, WMU_HIGHLIGHT, 0, 0);
					isRequireParenthesisHighligth = true;
				}

				if (wParam == IDC_DLG_EDITOR && pHdr->code == EN_MSGFILTER) {
					return processEditorEvents((MSGFILTER*)lParam);
				}
			}
			break;

			case WMU_HIGHLIGHT: {
				processHightlight(GetDlgItem(hWnd, IDC_DLG_EDITOR), isRequireHighligth, isRequireParenthesisHighligth);
				isRequireHighligth = false;
				isRequireParenthesisHighligth = false;
			}
			break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgAddTable (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);

				const TCHAR* colNames[] = {TEXT("#"), TEXT("Name"), TEXT("Type"), TEXT("PK"), TEXT("NN"), TEXT("UQ"), TEXT("Default"), TEXT("Check"), 0};
				int colWidths[] = {30, 145, 60, 30, 30, 30, 0, 0, 0};

				for (int i = 0; colNames[i]; i++) {
					LVCOLUMN lvc = {0};
					lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
					lvc.iSubItem = i;
					lvc.pszText = (TCHAR*)colNames[i];
					lvc.cchTextMax = _tcslen(colNames[i]) + 1;
					lvc.cx = colWidths[i];
					lvc.fmt = colWidths[i] < 80 ? LVCFMT_CENTER : LVCFMT_LEFT;
					ListView_InsertColumn(hListWnd, i, &lvc);
				}

				LVCOLUMN lvc = {mask: LVCF_FMT, fmt: LVCFMT_RIGHT};
				ListView_SetColumn(hListWnd, 0, &lvc);

				SendMessage(hWnd, WMU_ADD_ROW, 0, 0);
				ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
			}
			break;

			case WM_COMMAND: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_COLUMNS);
				if (wParam == IDC_DLG_OK) {
					int rowCount = ListView_GetItemCount(hListWnd);
					bool isWithoutRowid = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISWITHOUT_ROWID));
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
					TCHAR columns16[MAX_TEXT_LENGTH] = {0};
					for (int rowNo = 0; rowNo < rowCount; rowNo++) {
						TCHAR* row[8] = {0};
						for (int colNo = 0; colNo < 8; colNo++) {
							row[colNo] = new TCHAR[512]{0};
							ListView_GetItemText(hListWnd, rowNo, colNo, row[colNo], 512);
						}

						if (!_tcslen(row[1]))
							continue;

						TCHAR colDefinition16[2048];
						// name type [NOT NULL] [DEFAULT ...] [CHECK(...)] [PRIMARY KEY] [AUTOINCREMENT] [UNIQUE]
						_stprintf(colDefinition16, TEXT("\"%s\" %s%s%s%s%s%s%s%s%s%s"),
							row[1], // name
							row[2], // type
							_tcslen(row[4]) ? TEXT(" not null") : TEXT(""),
							_tcslen(row[6]) ? TEXT(" default \"") : TEXT(""),
							_tcslen(row[6]) ? row[6] : TEXT(""),
							_tcslen(row[6]) ? TEXT("\"") : TEXT(""),
							_tcslen(row[7]) ? TEXT(" check(") : TEXT(""),
							_tcslen(row[7]) ? row[7] : TEXT(""),
							_tcslen(row[7]) ? TEXT(")") : TEXT(""),
							!_tcslen(row[3]) || pkCount > 1 ? TEXT("") : !_tcscmp(TEXT("integer"), row[2]) && !isWithoutRowid ? TEXT(" primary key autoincrement")	: TEXT(" primary key"),
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

					TCHAR query16[MAX_TEXT_LENGTH] = {0};
					_stprintf(query16, TEXT("create table \"%s\" (\n%s%s%s%s\n)%s"),
						tblName16,
						columns16,
						pkCount > 1 ? TEXT(", primary key(\"") : TEXT(""),
						pkCount > 1 ? pk16 : TEXT(""),
						pkCount > 1 ? TEXT("\")") : TEXT(""),
						isWithoutRowid ? TEXT(" without rowid") : TEXT("")
					);

					char* query8 = utils::utf16to8(query16);
					int rc = sqlite3_exec(db, query8, NULL, 0 , 0);
					delete [] query8;

					if (SQLITE_OK == rc) {
						EndDialog(hWnd, DLG_OK);
					} else {
						showDbError(hMainWnd);
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);

				if (wParam == IDC_DLG_MORE) {
					HWND hBtn = GetDlgItem(hWnd, IDC_DLG_MORE);
					bool isOpen = GetWindowLong(hBtn, GWL_USERDATA);
					SetWindowLong(hBtn, GWL_USERDATA, !isOpen);
					SetWindowText(hBtn, isOpen ? TEXT(">>") : TEXT("<<"));
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_ISWITHOUT_ROWID), isOpen ? SW_HIDE : SW_SHOW);

					RECT rc;
					GetWindowRect(hWnd, &rc);
					SetWindowPos(hWnd, 0, 0, 0, rc.right - rc.left + (isOpen ? -250 : 250), rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

					GetWindowRect(hListWnd, &rc);
					SetWindowPos(hListWnd, 0, 0, 0, rc.right - rc.left + (isOpen ? -250 : 250), rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

					LVCOLUMN lvc = {mask: LVCF_WIDTH, fmt: 0, cx: + (isOpen ? 0 : 125)};
					ListView_SetColumn(hListWnd, 6, &lvc);
					ListView_SetColumn(hListWnd, 7, &lvc);
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

					if (ia->iItem == -1)
						return true;

					RECT rect;
					ListView_GetSubItemRect(hListWnd, ia->iItem, ia->iSubItem, LVIR_BOUNDS, &rect);
					int h = rect.bottom - rect.top;
					int w = ListView_GetColumnWidth(hListWnd, ia->iSubItem);

					TCHAR buf[1024];
					ListView_GetItemText(hListWnd, ia->iItem, ia->iSubItem, buf, MAX_TEXT_LENGTH);

					if (ia->iSubItem > 0 && w < 60) {
						ListView_SetItemText(hListWnd, ia->iItem, ia->iSubItem, (TCHAR*)(_tcslen(buf) ? TEXT("") : TEXT("v")));
						return true;
					}

					HWND hCell = 0;
					if (ia->iSubItem == 2) {
						hCell = CreateWindow(WC_COMBOBOX, buf, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VISIBLE | WS_CHILD | WS_TABSTOP, rect.left, rect.top - 4, w + 18, 200, hListWnd, NULL, GetModuleHandle(0), NULL);
						ComboBox_AddString(hCell, TEXT(""));
						for (int i = 0; DATATYPES16[i]; i++)
							ComboBox_AddString(hCell, DATATYPES16[i]);
						ComboBox_SetCurSel(hCell, 0);
					}

					if (ia->iSubItem == 1 || ia->iSubItem == 6 || ia->iSubItem == 7) {
						hCell = CreateWindowEx(0, WC_EDIT, buf, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, rect.left, rect.top, w, h - 1, hListWnd, 0, GetModuleHandle(NULL), NULL);
						int end = GetWindowTextLength(hCell);
						SendMessage(hCell, EM_SETSEL, end, end);
					}

					if (hCell) {
						SetWindowLong(hCell, GWL_USERDATA, MAKELPARAM(ia->iItem, ia->iSubItem));
						cbOldAddTableCell = (WNDPROC)SetWindowLong(hCell, GWL_WNDPROC, (LONG)cbNewAddTableCell);
						SendMessage(hCell, WM_SETFONT, (LPARAM)hDefFont, true);
						SetFocus(hCell);
					}
				}

				if (pHdr->code == (DWORD)NM_DBLCLK && pHdr->idFrom == IDC_DLG_COLUMNS) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					if (ia->iItem == -1)
						SendMessage(hWnd, WM_COMMAND, IDC_DLG_ROW_ADD, 0);
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

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgQueryList (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLong(hWnd, GWL_USERDATA, lParam);
				SetWindowPos(hWnd, 0, prefs::get("x") + 40, prefs::get("y") + 80, prefs::get("width") - 80, prefs::get("height") - 120,  SWP_NOZORDER);
				ShowWindow (hWnd, prefs::get("maximized") == 1 ? SW_MAXIMIZE : SW_SHOW);
				SetWindowText(hWnd, lParam == IDM_HISTORY ? TEXT("Query history") : TEXT("Saved queries"));

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);

				LVCOLUMN lvc;
				lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
				lvc.iSubItem = 0;
				lvc.pszText = (TCHAR*)TEXT("Date");
				lvc.cx = 110;
				ListView_InsertColumn(hListWnd, 0, &lvc);

				lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
				lvc.iSubItem = 1;
				lvc.pszText = (TCHAR*)TEXT("Query");
				ListView_InsertColumn(hListWnd, 1, &lvc);

				SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
				ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | 0x10000000);
				SendMessage(hWnd, WM_SIZE, 0, 0);

				SetFocus(GetDlgItem(hWnd, IDC_DLG_QUERYFILTER));
			}
			break;

			case WM_TIMER: {
				KillTimer(hWnd, IDT_EDIT_DATA);
				SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);
			}
			break;

			case WM_SIZE: {
				HWND hFilterWnd = GetDlgItem(hWnd, IDC_DLG_QUERYFILTER);
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);

				RECT rc;
				GetClientRect(hWnd, &rc);
				SetWindowPos(hFilterWnd, 0, 0, 0, rc.right - rc.left - 0, 20, SWP_NOZORDER | SWP_NOMOVE);
				SetWindowPos(hListWnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top - 21, SWP_NOZORDER | SWP_NOMOVE);

				LVCOLUMN lvc;
				lvc.mask = LVCF_WIDTH;
				lvc.iSubItem = 1;
				lvc.cx = rc.right - rc.left - 130;
				ListView_SetColumn(hListWnd, 1, &lvc);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (pHdr->code == (DWORD)NM_DBLCLK)
					PostMessage(hWnd, WM_COMMAND, IDOK, 0);

				if (pHdr->code == LVN_KEYDOWN) {
					HWND hListWnd = pHdr->hwndFrom;
					NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;
					int pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					if (kd->wVKey == VK_DELETE && pos != -1) {
						int idx = GetWindowLong(hWnd, GWL_USERDATA);
						TCHAR query16[MAX_TEXT_LENGTH];
						ListView_GetItemText(hListWnd, pos, 1, query16, MAX_TEXT_LENGTH);

						char* query8 = utils::utf16to8(query16);
						prefs::deleteQuery(idx == IDM_HISTORY ? "history" : "gists", query8);
						ListView_DeleteItem(hListWnd, pos);
						ListView_SetItemState (hListWnd, pos, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
						delete [] query8;
					}
				}
			}
			break;

			case WM_COMMAND: {
				if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetDlgItem(hWnd, IDC_DLG_QUERYFILTER) && (HWND)lParam == GetFocus()) {
					KillTimer(hWnd, IDT_EDIT_DATA);
					SetTimer(hWnd, IDT_EDIT_DATA, 300, NULL);
					return true;
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);

				if (wParam == IDOK) {
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
					int iPos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					if (iPos == -1)
						break;

					TCHAR buf[MAX_TEXT_LENGTH];
					ListView_GetItemText(hListWnd, iPos, 1, buf, MAX_TEXT_LENGTH);

					int crPos;
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

				int idx = GetWindowLong(hWnd, GWL_USERDATA);
				char* filter8 = utils::utf16to8(filter16);

				char* queries[prefs::get("max-query-count")];
				int count = prefs::getQueries(idx == IDM_HISTORY ? "history" : "gists", filter8, queries);

				ListView_DeleteAllItems(hListWnd);
				for (int i = 0; i < count; i++) {
					TCHAR* text16 = utils::utf8to16(queries[i]);
					TCHAR* q = _tcschr(text16, TEXT('\t'));
					if (q != NULL) {
						q += 1;
						int len = _tcslen(text16);
						int len1 = _tcslen(q);

						TCHAR time[len - len1 + 1]{0};
						_tcsncpy(time, text16, len - len1);
						LVITEM  lvi = {0};
						lvi.mask = LVIF_TEXT;
						lvi.iSubItem = 0;
						lvi.iItem = i;
						lvi.pszText = time;
						lvi.cchTextMax = len - len1 + 1;
						ListView_InsertItem(hListWnd, &lvi);

						lvi.mask = LVIF_TEXT;
						lvi.iSubItem = 1;
						lvi.iItem = i;
						lvi.pszText = q;
						lvi.cchTextMax = len1 + 1;
						ListView_SetItem(hListWnd, &lvi);
					}
					delete [] text16;
					delete queries[i];
				}

				if (count > 0)
					ListView_SetItemState (hListWnd, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);

				delete [] filter8;
				return true;
			}
			break;

			case WM_CLOSE: {
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgEditData (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TCHAR* schema16 = utils::getName(editTableData16, true);
				TCHAR* tablename16 = utils::getName(editTableData16);

				char* tablename8 = utils::utf16to8(tablename16);
				SetProp(hWnd, TEXT("TABLENAME8"), (HANDLE)tablename8);

				char* schema8 = utils::utf16to8(schema16);
				SetProp(hWnd, TEXT("SCHEMA8"), (HANDLE)schema8);

				delete [] tablename16;
				delete [] schema16;

				sqlite3_stmt *stmt;
				char query8[MAX_TEXT_LENGTH]{0};
				sprintf(query8, "select rowid from \"%s\".\"%s\" limit 1", schema8, tablename8);
				bool hasRowid = SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0);
				sqlite3_finalize(stmt);

				SetProp(hWnd, TEXT("HASROWID"), (HANDLE)hasRowid);
				if (!hasRowid) {
					sprintf(query8,
						"select '\"' || group_concat(name, '\",\"') || '\"', " \
						"'md5(\"' || group_concat(name, '\" || ''***'' || \"') || '\")', " \
						"count(1) "
						"from pragma_table_info('%s') where pk > 0 and schema = '%s' order by pk ", tablename8, schema8);
					if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0) && SQLITE_ROW == sqlite3_step(stmt)) {
						SetProp(hWnd, TEXT("KEYS8"), (HANDLE)strdup((char*)sqlite3_column_text(stmt, 0)));
						SetProp(hWnd, TEXT("MD5KEYS8"), (HANDLE)strdup((char*)sqlite3_column_text(stmt, 1)));
						SetProp(hWnd, TEXT("KEYCOUNT"), (HANDLE)sqlite3_column_int(stmt, 2));
					} else {
						showDbError(hWnd);
					}
					sqlite3_finalize(stmt);
				}
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				HWND hHeader = ListView_GetHeader(hListWnd);
				LONG_PTR styles = GetWindowLongPtr(hHeader, GWL_STYLE);
				SetWindowLongPtr(hHeader, GWL_STYLE, styles | HDS_FILTERBAR);
				CreateWindow(WC_LISTBOX, NULL, WS_CHILD, 300, 0, 400, 100, hListWnd, (HMENU)IDC_REFLIST, GetModuleHandle(0), 0);

				bool isTable = false;
				sprintf(query8, "select lower(type) = 'table' from %s.sqlite_master where tbl_name = \"%s\" and type in ('view', 'table')", schema8, tablename8);
				if ((SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) && (SQLITE_ROW == sqlite3_step(stmt)))
					isTable = sqlite3_column_int(stmt, 0);
				sqlite3_finalize(stmt);
				SetWindowLong(hWnd, GWL_USERDATA, +isTable);

				SendMessage(hWnd, WMU_UPDATE_DATA, 0 , 0);

				TBBUTTON tbTableButtons [] = {
					{2, IDM_ROW_REFRESH, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Refresh")},
					{0, IDM_ROW_ADD, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Add")},
					{1, IDM_ROW_DELETE, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Delete")},
					{3, IDM_GENERATE_DATA, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Generate data")},
					{-1, IDM_LAST_SEPARATOR, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0L, 0}
				};

				TBBUTTON tbViewButtons [] = {
					{2, IDM_ROW_REFRESH, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0}, 0L, (INT_PTR)TEXT("Refresh")},
					{-1, IDM_LAST_SEPARATOR, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0L, 0}
				};

				int btnCount = isTable ? sizeof(tbTableButtons)/sizeof(tbTableButtons[0]) : sizeof(tbViewButtons)/sizeof(tbViewButtons[0]);
				HWND hToolbarWnd = CreateToolbarEx (hWnd, WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_LIST, IDC_DLG_TOOLBAR, 0, NULL, 0,
					isTable ? tbTableButtons : tbViewButtons, btnCount,
					0, 0, 0, 0, sizeof (TBBUTTON));
				SendMessage(hToolbarWnd, TB_SETIMAGELIST,0, (LPARAM)ImageList_LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_TOOLBAR_DATA), 0, 0, RGB(255,255,255)));

				RECT rc{0};
				SendMessage(hToolbarWnd, TB_GETRECT, IDM_LAST_SEPARATOR, (LPARAM)&rc);
				HWND hFilterWnd = CreateWindowEx(0L, WC_EDIT, NULL, WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, rc.right, 2, 180, 19, hToolbarWnd, (HMENU) IDC_DLG_FILTER, GetModuleHandle(0), 0);
				SendMessage(hFilterWnd, WM_SETFONT, (LPARAM)SendMessage(hToolbarWnd, WM_GETFONT, 0, 0), true);
				cbOldHeaderEdit = (WNDPROC)SetWindowLong(hFilterWnd, GWL_WNDPROC, (LONG)cbNewFilterEdit);

				int colCount = Header_GetItemCount(hHeader);
				HFONT hFont = (HFONT)SendMessage(hListWnd, WM_GETFONT, 0, 0);
				for (int i = 0; i < colCount; i++) {
					RECT rc;
					Header_GetItemRect(hHeader, i, &rc);
					HWND hEdit = CreateWindowEx(WS_EX_TOPMOST, WC_EDIT, NULL, ES_CENTER | ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hHeader, (HMENU)(IDC_HEADER_EDIT + i), GetModuleHandle(0), NULL);
					SendMessage(hEdit, WM_SETFONT, (LPARAM)hFont, true);
					cbOldHeaderEdit = (WNDPROC)SetWindowLong(hEdit, GWL_WNDPROC, (LONG)cbNewFilterEdit);
					CreateWindowEx(WS_EX_TOPMOST, WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | SS_WHITEFRAME, 0, 0, 0, 0, hHeader, (HMENU)(IDC_HEADER_STATIC + i), GetModuleHandle(0), NULL);
				}

				int* widths = new int[colCount]{0};
				for (int i = 0; i < colCount; i++)
					widths[i] = ListView_GetColumnWidth(hListWnd, i);
				SetProp(hWnd, TEXT("WIDTHS"), (HANDLE)widths);

				SetWindowPos(hWnd, 0, prefs::get("x") + 40, prefs::get("y") + 80, prefs::get("width") - 80, prefs::get("height") - 120,  SWP_NOZORDER);
				ShowWindow (hWnd, prefs::get("maximized") == 1 ? SW_MAXIMIZE : SW_SHOW);
			}
			break;

			case WM_SIZE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				HWND hToolbarWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);

				SendMessage(hToolbarWnd, WM_SIZE, 0, 0);
				RECT rc, rc2, rc3;
				GetClientRect(hWnd, &rc);
				GetClientRect(hToolbarWnd, &rc2);
				SetWindowPos(hListWnd, 0, 0, rc2.bottom + 2, rc.right - rc.left, rc.bottom - rc.top - 28, SWP_NOZORDER);

				HWND hFilterWnd = GetDlgItem(hToolbarWnd, IDC_DLG_FILTER);
				GetWindowRect(hFilterWnd, &rc3);
				POINT p{rc3.left, rc3.bottom};
				ScreenToClient(hToolbarWnd, &p);
				SetWindowPos(hFilterWnd, 0, 0, 0, rc.right - p.x, 19, SWP_NOZORDER | SWP_NOMOVE);
			}
			break;

			case WMU_UPDATE_DATA: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				HWND hHeader = ListView_GetHeader(hListWnd);
				bool isTable = GetWindowLong(hWnd, GWL_USERDATA) == 1;
				bool hasRowid = GetProp(hWnd, TEXT("HASROWID"));
				HWND hFilterWnd = GetDlgItem(GetDlgItem(hWnd, IDC_DLG_TOOLBAR),IDC_DLG_FILTER);

				int size = GetWindowTextLength(hFilterWnd);
				TCHAR filter16[size + 1]{0};
				GetWindowText(hFilterWnd, filter16, size + 1);

				SetWindowText(hWnd, TEXT("Fetching data..."));

				char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
				char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
				char* md5keys = (char*)GetProp(hWnd, TEXT("MD5KEYS8"));

				TCHAR where16[MAX_TEXT_LENGTH]{0};
				_tcscat(where16, TEXT("where ("));
				_tcscat(where16, _tcslen(filter16) ? filter16 : TEXT("1 = 1"));
				_tcscat(where16, TEXT(") "));

				for (int colNo = 1; colNo < Header_GetItemCount(hHeader); colNo++) {
					HWND hEdit = GetDlgItem(hHeader, IDC_HEADER_EDIT + colNo);
					if (GetWindowTextLength(hEdit) > 0) {
						TCHAR colname16[256]{0};
						Header_GetItemText(hHeader, colNo, colname16, 255);
						_tcscat(where16, TEXT(" and \""));
						_tcscat(where16, colname16);

						TCHAR buf16[2]{0};
						GetWindowText(hEdit, buf16, 2);
						_tcscat(where16,
							buf16[0] == TCHAR('=') ? TEXT("\" = ? ") :
							buf16[0] == TCHAR('/') ? TEXT("\" regexp ? ") :
							TEXT("\" like '%' || ? || '%' "));
					}

				}
				char* where8 = utils::utf16to8(where16);

				char query8[MAX_TEXT_LENGTH]{0};
				sprintf(query8, "select *, %s rowid from \"%s\".\"%s\" t %s", hasRowid ? "rowid" : md5keys, schema8, tablename8, where8 && strlen(where8) ? where8 : "");

				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
					int colCount = sqlite3_column_count(stmt);
					if (GetProp(hWnd, TEXT("BLOBS")) == NULL) {
						bool* blobs = new bool[colCount]{0};
						for (int i = 0; i < colCount; i++)
							blobs[i] = sqlite3_column_decltype(stmt, i) != 0 && stricmp(sqlite3_column_decltype(stmt, i), "blob") == 0;
						SetProp(hWnd, TEXT("BLOBS"), (HANDLE)blobs);
					}

					int bindNo = 0;
					for (int colNo = 1; (colNo < colCount) && strlen(where8); colNo++) {
						HWND hEdit = GetDlgItem(hHeader, IDC_HEADER_EDIT + colNo);
						int size = GetWindowTextLength(hEdit);
						if (size > 0) {
							TCHAR value16[size + 1]{0};
							GetWindowText(hEdit, value16, size + 1);
							char* value8 = utils::utf16to8(value16[0] == TEXT('=') || value16[0] == TEXT('/') ? value16 + 1 : value16);
							utils::sqlite3_bind_variant(stmt, bindNo + 1, value8);
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

					TCHAR buf[256]{0};
					TCHAR* tablename16 = utils::utf8to16(tablename8);
					_stprintf(buf, TEXT("%s \"%s\" [%s%i rows]"), isTable ? TEXT("Table") : TEXT("View"), tablename16, rowCount < 0 ? TEXT("Show only first ") : TEXT(""), abs(rowCount));
					delete [] tablename16;
					SetWindowText(hWnd, buf);
				} else {
					showDbError(hWnd);
					sqlite3_finalize(stmt);
				}

				ListView_SetItemState(hListWnd, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				SendMessage(hWnd, WMU_SET_CURRENT_CELL, 0, 1);

				delete [] where8;
				PostMessage(hWnd, WMU_UPDATE_COLSIZE, 0, 0);
				InvalidateRect(hHeader, NULL, true);
				return true;
			}
			break;

			case WMU_UPDATE_COLSIZE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				HWND hHeader = ListView_GetHeader(hListWnd);
				SendMessage(hHeader, WM_SIZE, 0, 0);
				for (int i = 0; i < Header_GetItemCount(hHeader); i++) {
					RECT rc;
					Header_GetItemRect(hHeader, i, &rc);
					SetWindowPos(GetDlgItem(hHeader, IDC_HEADER_STATIC + i), 0, rc.left + 1, rc.top + 20, rc.right - rc.left - 2, 2, SWP_NOZORDER);
					SetWindowPos(GetDlgItem(hHeader, IDC_HEADER_EDIT + i), 0, rc.left + 1, rc.top + 20 + 2, rc.right - rc.left - 2, rc.bottom - rc.top - 21 - 2, SWP_NOZORDER);
				}
			}
			break;

			case WMU_SET_CURRENT_CELL: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				RECT rect;
				ListView_GetSubItemRect(hListWnd, currCell.iItem, currCell.iSubItem, LVIR_BOUNDS, &rect);
				InvalidateRect(hListWnd, &rect, true);

				currCell = {hListWnd, (int)wParam, (int)lParam};

				ListView_GetSubItemRect(hListWnd, currCell.iItem, currCell.iSubItem, LVIR_BOUNDS, &rect);
				InvalidateRect(hListWnd, &rect, true);
				SetFocus(hListWnd);
			}
			break;

			case WMU_SYNC_CURRENT_CELL: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				int rowNo = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
				SendMessage(hWnd, WMU_SET_CURRENT_CELL, rowNo, currCell.iSubItem);
			}
			break;

			case WMU_EDIT_VALUE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				bool withText = wParam != 0;

				RECT rect;
				ListView_GetSubItemRect(hListWnd, currCell.iItem, currCell.iSubItem, LVIR_BOUNDS, &rect);
				int h = rect.bottom - rect.top;
				int w = ListView_GetColumnWidth(hListWnd, currCell.iSubItem);

				TCHAR buf[MAX_TEXT_LENGTH];
				ListView_GetItemText(hListWnd, currCell.iItem, currCell.iSubItem, buf, MAX_TEXT_LENGTH);

				if (_tcscmp(buf, TEXT("(BLOB)")) == 0 || currCell.iSubItem < 1)
					return true;

				HWND hEdit = CreateWindowEx(0, WC_EDIT, withText ? buf : NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, rect.left, rect.top, w, h, hListWnd, 0, GetModuleHandle(NULL), NULL);
				SetWindowLong(hEdit, GWL_USERDATA, MAKELPARAM(currCell.iItem, currCell.iSubItem));
				int end = GetWindowTextLength(hEdit);
				SendMessage(hEdit, EM_SETSEL, end, end);
				SendMessage(hEdit, WM_SETFONT, (LPARAM)hDefFont, true);
				cbOldEditDataEdit = (WNDPROC)SetWindowLong(hEdit, GWL_WNDPROC, (LONG)cbNewEditDataEdit);
				SetFocus(hEdit);

				if (!withText)
					keybd_event(lParam, 0, 0, 0);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				bool isTable = GetWindowLong(hWnd, GWL_USERDATA) == 1;
				bool hasRowid = GetProp(hWnd, TEXT("HASROWID"));
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);

				if (pHdr->code == LVN_COLUMNCLICK) {
					NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
					return ListView_Sort(pHdr->hwndFrom, pLV->iSubItem);
				}

				if (pHdr->code == (DWORD)NM_RCLICK && pHdr->hwndFrom == hListWnd) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					SendMessage(hWnd, WMU_SET_CURRENT_CELL, ia->iItem, ia->iSubItem);

					POINT p;
					GetCursorPos(&p);

					TCHAR buf[10];
					ListView_GetItemText(hListWnd, ia->iItem, ia->iSubItem, buf, 10);

					bool* blobs = (bool*)GetProp(hWnd, TEXT("BLOBS"));
					HMENU hMenu = !isTable ? hResultMenu : blobs[ia->iSubItem - 1] || !_tcscmp(buf, TEXT("(BLOB)")) ? hBlobMenu : hEditDataMenu;

					if (hMenu == hEditDataMenu)
						EnableMenuItem(hMenu, IDM_ROW_EDIT, ListView_GetSelectedCount(hListWnd) == 1 ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

					EnableMenuItem(hMenu, IDM_ROW_DUPLICATE, hasRowid ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

					TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
				}

				if (pHdr->code == (DWORD)NM_CLICK && GetAsyncKeyState(VK_MENU)) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					SendMessage(hWnd, WMU_SET_CURRENT_CELL, ia->iItem, ia->iSubItem);
					return ListView_ShowRef(hListWnd, ia->iItem, ia->iSubItem);
				}

				if (!isTable && pHdr->code == (DWORD)NM_DBLCLK && pHdr->hwndFrom == hListWnd) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					if (ia->iItem != -1)
						SendMessage(hWnd, WM_COMMAND, IDM_ROW_EDIT, 0);
				}

				if (pHdr->code == (DWORD)NM_CLICK && pHdr->hwndFrom == hListWnd) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					return SendMessage(hWnd, WMU_SET_CURRENT_CELL, ia->iItem, ia->iSubItem);
				}

				if (isTable && pHdr->hwndFrom == hListWnd && pHdr->code == (UINT)NM_CUSTOMDRAW) {
					NMLVCUSTOMDRAW* pCustomDraw = (LPNMLVCUSTOMDRAW)lParam;

					int result = CDRF_DODEFAULT;
					if (pCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
						result = CDRF_NOTIFYITEMDRAW;

					if (pCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
						result = CDRF_NOTIFYSUBITEMDRAW | CDRF_NEWFONT;

					if ((pCustomDraw->nmcd.dwDrawStage == CDDS_POSTPAINT) | CDDS_ITEM) {
						if (pCustomDraw->nmcd.dwItemSpec == (DWORD)currCell.iItem) {
							RECT rect;
							ListView_GetSubItemRect(hListWnd, currCell.iItem, currCell.iSubItem, LVIR_BOUNDS, &rect);

							HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
							HDC hdc = pCustomDraw->nmcd.hdc;
							SelectObject(hdc, hPen);

							RECT rc {0};
							ListView_GetSubItemRect(hListWnd, currCell.iItem, currCell.iSubItem, LVIR_BOUNDS, &rc);
							MoveToEx(hdc, rc.left - 2, rc.top, 0);
							LineTo(hdc, rc.right - 1, rc.top);
							LineTo(hdc, rc.right - 1, rc.bottom - 2);
							LineTo(hdc, rc.left + 1, rc.bottom - 2);
							LineTo(hdc, rc.left + 1, rc.top);
							DeleteObject(hPen);
						}
					}

					SetWindowLongPtr(hWnd, DWLP_MSGRESULT, result);
					return true;
				}

				if (pHdr->code == LVN_KEYDOWN && pHdr->hwndFrom == hListWnd) {
					NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;

					bool isControl = GetAsyncKeyState(VK_CONTROL);
					if (kd->wVKey == VK_F5) {
						PostMessage(hWnd, WM_COMMAND, IDM_ROW_REFRESH, 0);
						return true;
					}

					if (kd->wVKey == 0x43 && isControl) { // Ctrl + C
						currCell = {hListWnd, 0, 0};
						PostMessage(hWnd, WM_COMMAND, IDM_RESULT_COPY_ROW, 0);
						return true;
					}

					if (kd->wVKey == 0x41 && isControl) { // Ctrl + A
						ListView_SetItemState(hListWnd, -1, LVIS_SELECTED, LVIS_SELECTED);
						return true;
					}

					if (kd->wVKey == 0x56 && isControl && currCell.iSubItem > 0) { // Ctrl + V
						TCHAR* clipboard = utils::getClipboardText();
						ListView_UpdateCell(hListWnd, currCell.iItem, currCell.iSubItem, clipboard);
						delete [] clipboard;
						return true;
					}

					if (isTable && kd->wVKey == VK_DELETE) {
						PostMessage(hWnd, WM_COMMAND, IDM_ROW_DELETE, 0);
						return true;
					}

					if (isTable && (kd->wVKey == VK_NEXT || kd->wVKey == VK_PRIOR || kd->wVKey == VK_HOME || kd->wVKey == VK_END)) {
						PostMessage(hWnd, WMU_SYNC_CURRENT_CELL, 0, 0);
						return true;
					}

					if (isTable && (kd->wVKey == VK_UP || kd->wVKey == VK_DOWN)) {
						int rowCount = ListView_GetItemCount(hListWnd);
						int rowNo = (currCell.iItem + rowCount + (kd->wVKey == VK_UP ? -1 : 1)) % rowCount;
						SendMessage(hWnd, WMU_SET_CURRENT_CELL, rowNo, currCell.iSubItem);
						ListView_SetItemState(hListWnd, -1, 0, LVIS_SELECTED);
						ListView_SetItemState(hListWnd, rowNo, LVIS_SELECTED, LVIS_SELECTED);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, true);
						return true;
					}

					if (isTable && (kd->wVKey == VK_LEFT || kd->wVKey == VK_RIGHT)) {
						int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd));
						int colNo = (currCell.iSubItem + colCount + (kd->wVKey == VK_LEFT ? -1 : 1)) % colCount;
						colNo = colNo == 0 && kd->wVKey == VK_LEFT ? colCount - 2 : colNo == colCount - 1 && kd->wVKey == VK_RIGHT ? 1 : colNo;
						SendMessage(hWnd, WMU_SET_CURRENT_CELL, currCell.iItem, colNo);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, true);
						return true;
					}

					bool isNum = kd->wVKey >= 0x31 && kd->wVKey <= 0x39;
					bool isNumPad = kd->wVKey >= 0x61 && kd->wVKey <= 0x69;
					if ((isNum || isNumPad) && isControl) {// Ctrl + 1-9
						ListView_Sort(pHdr->hwndFrom, kd->wVKey - (isNum ? 0x31 : 0x61) + 1);
						if (isTable)
							PostMessage(hWnd, WMU_SYNC_CURRENT_CELL, 0, 0);
						return true;
					}

					if (isTable && !isControl && kd->wVKey >= 0x30 && kd->wVKey <= 0x5A) { // 0, 1, ..., y, z
						SendMessage(hWnd, WMU_EDIT_VALUE, 0, kd->wVKey);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, true);
						return true;
					}

					if (isTable && kd->wVKey == VK_INSERT)
						SendMessage(hWnd, WM_COMMAND, IDM_ROW_ADD, 0);
				}

				if (isTable && pHdr->code == (DWORD)NM_DBLCLK && pHdr->hwndFrom == hListWnd) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;

					if (ia->iItem == -1)
						return SendMessage(hWnd, WM_COMMAND, IDC_DLG_ROW_ADD, 0);

					if (ia->iSubItem == 0 || ia->iSubItem == Header_GetItemCount(ListView_GetHeader(hListWnd)) - 1)
						return true;

					currCell = {hListWnd, ia->iItem, ia->iSubItem};
				}

				if (isTable && pHdr->code == (DWORD)NM_DBLCLK && pHdr->hwndFrom == hListWnd)
					return SendMessage(hWnd, WMU_EDIT_VALUE, 1, 0);

				// This event is triggered on ListView_SetData too. So ListView is hiding to prevent processing of the notification.
				if (pHdr->code == HDN_ITEMCHANGED && pHdr->hwndFrom == ListView_GetHeader(hListWnd) && IsWindowVisible(hListWnd)) {
					int* widths = (int*)GetProp(hWnd, TEXT("WIDTHS"));
					int colNo = ((LPNMHEADER)lParam)->iItem;
					widths[colNo] = ListView_GetColumnWidth(hListWnd, colNo);
					SendMessage(hWnd, WMU_UPDATE_COLSIZE, 0, 0);
				}
			}
			break;

			case WM_COMMAND: {
				WORD cmd = LOWORD(wParam);
				bool isTable = GetWindowLong(hWnd, GWL_USERDATA) == 1;
				bool hasRowid = GetProp(hWnd, TEXT("HASROWID"));
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);

				if (wParam == IDOK) { // User push Enter
					int pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					if (hListWnd == GetFocus() && pos != -1) {
						currCell = {hListWnd, pos, currCell.iSubItem};
						PostMessage(hWnd, WM_COMMAND, IDM_ROW_EDIT, 0);
					}
				}

				if (cmd == IDC_DLG_CANCEL || cmd == IDCANCEL)
					SendMessage(hWnd, WM_CLOSE, 0, 0);

				if (cmd == IDM_RESULT_CHART || cmd == IDM_RESULT_COPY_CELL || cmd == IDM_RESULT_COPY_ROW || cmd == IDM_RESULT_EXPORT)
					onListViewMenu(cmd, true);

				if (cmd == IDM_ROW_ADD)
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hWnd, (DLGPROC)&cbDlgRow, MAKELPARAM(ROW_ADD, 0));

				if (cmd == IDM_ROW_REFRESH)
					SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);

				if (cmd == IDM_VALUE_EDIT) {
					TCHAR buf[MAX_TEXT_LENGTH]{0};
					ListView_GetItemText(hListWnd, currCell.iItem, currCell.iSubItem, buf, MAX_TEXT_LENGTH);

					if (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_EDITDATA_VALUE), hWnd, (DLGPROC)&cbDlgEditDataValue, (LPARAM)buf))
						ListView_UpdateCell(hListWnd, currCell.iItem, currCell.iSubItem, buf);
				}

				if (cmd == IDM_ROW_EDIT) {
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hWnd, (DLGPROC)&cbDlgRow, MAKELPARAM(isTable ? ROW_EDIT: ROW_VIEW, 0));
					SendMessage(hWnd, WMU_SET_CURRENT_CELL, currCell.iItem, currCell.iSubItem);
				}

				if (cmd == IDM_ROW_DELETE) {
					int count = ListView_GetSelectedCount(hListWnd);
					if (!count)
						return true;

					char* placeholders8 = new char[count * 2]{0}; // count = 3 => ?, ?, ?
					for (int i = 0; i < count * 2 - 1; i++)
						placeholders8[i] = i % 2 ? ',' : '?';
					placeholders8[count * 2 - 1] = '\0';

					char sql8[1024 + count * 2]{0};
					char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
					char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));

					sprintf(sql8, "delete from \"%s\".\"%s\" where %s in (%s)", schema8, tablename8, hasRowid ? "rowid" : (char*)GetProp(hWnd, TEXT("MD5KEYS8")),  placeholders8);
					delete [] placeholders8;

					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0)) {
						int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd));
						int pos = -1;
						TCHAR buf16[64];
						for (int i = 0; i < count; i++) {
							pos = ListView_GetNextItem(hListWnd, pos, LVNI_SELECTED);
							ListView_GetItemText(hListWnd, pos, colCount - 1, buf16, 128);
							char* buf8 = utils::utf16to8(buf16);
							sqlite3_bind_text(stmt, i + 1, buf8, strlen(buf8), SQLITE_TRANSIENT);
							delete [] buf8;
						}

						if (SQLITE_DONE == sqlite3_step(stmt)) {
							pos = -1;
							while((pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED)) != -1)
								ListView_DeleteItem(hListWnd, pos);
							ListView_SetItemState (hListWnd, pos, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
						} else {
							showDbError(hWnd);
						}
					}
					sqlite3_finalize(stmt);
				}

				if (cmd == IDM_ROW_DUPLICATE) {
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
					int count = ListView_GetSelectedCount(hListWnd);
					int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd));

					TCHAR ids16[count * 12]{0};
					TCHAR buf16[128];
					int pos = -1;
					for (int i = 0; i < count; i++) {
						pos = ListView_GetNextItem(hListWnd, pos, LVNI_SELECTED);
						ListView_GetItemText(hListWnd, pos, colCount - 1, buf16, 128);
						if (_tcslen(ids16))
							_tcscat(ids16, TEXT(","));
						_tcscat(ids16, buf16);
					}

					char sql8[1024]{0};
					char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
					char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
					char* ids8 = utils::utf16to8(ids16);
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

				if (cmd == IDM_BLOB_NULL || cmd == IDM_BLOB_IMPORT || cmd == IDM_BLOB_EXPORT) {
					TCHAR path16[MAX_PATH]{0};
					TCHAR filter16[] = TEXT("Images (*.jpg, *.gif, *.png, *.bmp)\0*.jpg;*.jpeg;*.gif;*.png;*.bmp\0Binary(*.bin,*.dat)\0*.bin,*.dat\0All\0*.*\0");
					bool isOK = (cmd == IDM_BLOB_IMPORT && utils::openFile(path16, filter16)) ||
						(cmd == IDM_BLOB_EXPORT && utils::saveFile(path16, filter16)) ||
						(cmd == IDM_BLOB_NULL && MessageBox(hWnd, TEXT("Are you sure to reset the cell?"), TEXT("Erase confirmation"), MB_OKCANCEL) == IDOK);

					if (!isOK)
						return 1;

					int rowNo = currCell.iItem;
					int colNo = currCell.iSubItem;
					HWND hListWnd = currCell.hListWnd;
					HWND hHeader = (HWND)ListView_GetHeader(hListWnd);

					TCHAR column16[256];
					if (!hHeader || !Header_GetItemText(hHeader, colNo, column16, 255))
						return 1;

					TCHAR rowid16[65] = {0};
					int colCount = Header_GetItemCount(hHeader);
					ListView_GetItemText(hListWnd, rowNo, colCount - 1, rowid16, 64);

					char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
					char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
					char* column8 = utils::utf16to8(column16);

					char query8[1024] = {0};
					if (cmd == IDM_BLOB_EXPORT) {
						sprintf(query8, "select %s from \"%s\".\"%s\" where %s = ?1", column8, schema8, tablename8, hasRowid ? "rowid" : (char*)GetProp(hWnd, TEXT("MD5KEYS8")));
					} else {
						sprintf(query8, "update \"%s\".\"%s\" set \"%s\" = ?2 where %s = ?1", schema8, tablename8, column8, hasRowid ? "rowid" : (char*)GetProp(hWnd, TEXT("MD5KEYS8")));
					}

					char* path8 = utils::utf16to8(path16);
					sqlite3_stmt *stmt;

					int	rc = SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0);
					if (rc) {
						char* rowid8 = utils::utf16to8(rowid16);
						sqlite3_bind_text(stmt, 1, rowid8, strlen(rowid8), SQLITE_TRANSIENT);
						delete [] rowid8;
					}

					if (rc && (cmd == IDM_BLOB_NULL)) {
						sqlite3_bind_null(stmt, 2);
						rc = SQLITE_DONE == sqlite3_step(stmt);
						ListView_SetItemText(hListWnd, rowNo, colNo, TEXT(""));
					}

					if (rc && (cmd == IDM_BLOB_IMPORT)) {
						FILE *fp = fopen (path8 , "rb");
						if (!fp)
							MessageBox(hWnd, TEXT("Opening the file for reading failed."), TEXT("Info"), MB_OK);

						if (rc && fp) {
							fseek(fp, 0L, SEEK_END);
							long size = ftell(fp);
							rewind(fp);

							char* data8 = new char[size]{0};
							fread(data8, size, 1, fp);
							fclose(fp);

							sqlite3_bind_blob(stmt, 2, data8, size, SQLITE_TRANSIENT);
							rc = SQLITE_DONE == sqlite3_step(stmt);
							delete [] data8;

							ListView_SetItemText(hListWnd, rowNo, colNo, TEXT("(BLOB)"));
						}
					}

					if (rc && (cmd == IDM_BLOB_EXPORT)) {
						rc = SQLITE_ROW == sqlite3_step(stmt);
						FILE *fp = fopen (path8 , "wb");
						if (!fp)
							MessageBox(hWnd, TEXT("Opening the file for writing failed."), TEXT("Info"), MB_OK);
						if (rc && fp) {
							fwrite(sqlite3_column_blob(stmt, 0), sqlite3_column_bytes(stmt, 0), 1, fp);
							fclose(fp);
						}
					}

					sqlite3_finalize(stmt);
					if (!rc)
						showDbError(hWnd);

					delete [] column8;
					delete [] path8;
				}

				if (cmd == IDM_GENERATE_DATA && (DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_TOOL_GENERATE_DATA), hMainWnd, (DLGPROC)&tools::cbDlgDataGenerator, (LPARAM)editTableData16)))
					SendMessage(hWnd, WMU_UPDATE_DATA, 0, 0);

				if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetDlgItem(hWnd, IDC_DLG_QUERYFILTER) && (HWND)lParam == GetFocus()) {
					KillTimer(hWnd, IDT_EDIT_DATA);
					SetTimer(hWnd, IDT_EDIT_DATA, 300, NULL);
				}
			}
			break;

			case WM_CLOSE: {
				char* tablename8 = (char*)GetProp(hWnd, TEXT("TABLENAME8"));
				delete [] tablename8;
				RemoveProp(hWnd, TEXT("TABLENAME8"));

				char* schema8 = (char*)GetProp(hWnd, TEXT("SCHEMA8"));
				delete [] schema8;
				RemoveProp(hWnd, TEXT("SCHEMA8"));

				bool* blobs = (bool*)GetProp(hWnd, TEXT("BLOBS"));
				delete [] blobs;
				RemoveProp(hWnd, TEXT("BLOBS"));

				int* widths = (int*)GetProp(hWnd, TEXT("WIDTHS"));
				delete [] widths;
				RemoveProp(hWnd, TEXT("WIDTHS"));

				RemoveProp(hWnd, TEXT("HASROWID"));
				RemoveProp(hWnd, TEXT("KEYCOUNT"));

				char* keys = (char*)GetProp(hWnd, TEXT("KEYS8"));
				if (keys)
					delete [] keys;
				RemoveProp(hWnd, TEXT("KEYS8"));

				char* md5keys = (char*)GetProp(hWnd, TEXT("MD5KEYS8"));
				if (md5keys)
					delete [] md5keys;
				RemoveProp(hWnd, TEXT("MD5KEYS8"));

				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	// USERDATA - buffer with text
	BOOL CALLBACK cbDlgEditDataValue (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLong(hWnd, GWL_USERDATA, lParam);
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				setEditorFont(hEditorWnd);
				SendMessage(hEditorWnd, WM_SETTEXT, (WPARAM)0, (LPARAM)lParam);
			}
			break;

			case WM_SIZE: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				HWND hOkWnd = GetDlgItem(hWnd, IDC_DLG_OK);
				RECT rc = {0};
				GetClientRect(hWnd, &rc);
				SetWindowPos(hEditorWnd, 0, 0, 0, rc.right, rc.bottom - 40, SWP_NOMOVE | SWP_NOZORDER);
				SetWindowPos(hOkWnd, 0, rc.right - 82, rc.bottom - 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)0, (LPARAM)0);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK || wParam == IDOK) {
					GetDlgItemText(hWnd, IDC_DLG_EDITOR, (TCHAR*)GetWindowLong(hWnd, GWL_USERDATA), MAX_TEXT_LENGTH);
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

	// USERDATA - buffer with text
	BOOL CALLBACK cbDlgViewDataValue (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLong(hWnd, GWL_USERDATA, lParam);
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				setEditorFont(hEditorWnd);
				SendMessage(hEditorWnd, WM_SETTEXT, (WPARAM)0, (LPARAM)lParam);
			}
			break;

			case WM_SIZE: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				RECT rc = {0};
				GetClientRect(hWnd, &rc);
				SetWindowPos(hEditorWnd, 0, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER);
				SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)0, (LPARAM)0);
			}
			break;

			case WM_COMMAND: {
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

	BOOL CALLBACK cbDlgRow (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				int mode = LOWORD(lParam);
				int isResultWnd = HIWORD(lParam);
				HWND hListWnd = isResultWnd ? (HWND)GetWindowLong(hTabWnd, GWL_USERDATA) : GetDlgItem(GetWindow(hWnd, GW_OWNER), IDC_DLG_QUERYLIST);
				HWND hHeader = ListView_GetHeader(hListWnd);
				int colCount = Header_GetItemCount(hHeader) - !isResultWnd;

				if (!hHeader || !colCount)
					EndDialog(hWnd, DLG_CANCEL);

				for (int colNo = 1; colNo < colCount; colNo++) {
					TCHAR colName[256];
					Header_GetItemText(hHeader, colNo, colName, 255);

					CreateWindow(WC_STATIC, colName, WS_VISIBLE | WS_CHILD | SS_RIGHT, 5, 5 + 20 * (colNo - 1), 120, 18, hWnd, (HMENU)(IDC_ROW_LABEL +  colNo), GetModuleHandle(0), 0);
					CreateWindow(WC_EDIT, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_TABSTOP | ES_AUTOHSCROLL | (mode == ROW_VIEW ? ES_READONLY : 0), 130, 3 + 20 * (colNo - 1), 234, 18, hWnd, (HMENU)(IDC_ROW_EDIT + colNo), GetModuleHandle(0), 0);
					CreateWindow(WC_BUTTON, TEXT("..."), WS_VISIBLE | WS_CHILD | BS_FLAT, 365, 3 + 20 * (colNo - 1), 18, 18, hWnd, (HMENU)(IDC_ROW_SWITCH + colNo), GetModuleHandle(0), 0);
				}
				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETDEFFONT);
				SetWindowPos(hWnd, 0, 0, 0, 400, colCount * 20 + 53, SWP_NOMOVE | SWP_NOZORDER);

				SetWindowText(hWnd, mode == ROW_ADD ? TEXT("New row") : mode == ROW_EDIT ? TEXT("Edit row") : TEXT("View row"));

				HWND hOkBtn = GetDlgItem(hWnd, IDC_DLG_OK);
				HWND hCancelBtn = GetDlgItem(hWnd, IDC_DLG_CANCEL);
				SetWindowText(hOkBtn, mode == ROW_ADD ? TEXT("Save and New") : mode == ROW_EDIT ? TEXT("Save and Next") : TEXT("Next"));
				SetWindowPos(hOkBtn, 0, 200, colCount * 20 - 4, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				SetWindowPos(hCancelBtn, 0, 295, colCount * 20 - 4, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				SetWindowLong(hWnd, GWL_USERDATA, MAKELPARAM(mode, colCount));
				SetWindowLong(GetDlgItem(hWnd, IDC_DLG_USERDATA), GWL_USERDATA, (LONG)hListWnd);

				if (mode != ROW_ADD)
					SendMessage(hWnd, WMU_SET_DLG_ROW_DATA, 0, 0);
				SetFocus(GetDlgItem(hWnd, IDC_ROW_EDIT + 1));
			}
			break;

			case WMU_SET_DLG_ROW_DATA: {
				HWND hListWnd = (HWND)GetWindowLong(GetDlgItem(hWnd, IDC_DLG_USERDATA), GWL_USERDATA);
				int colCount = HIWORD(GetWindowLong(hWnd, GWL_USERDATA));

				TCHAR val[MAX_TEXT_LENGTH];
				for (int i = 0; i < colCount; i++) {
					ListView_GetItemText(hListWnd, currCell.iItem, i, val, MAX_TEXT_LENGTH);
					HWND hEdit = GetDlgItem(hWnd, IDC_ROW_EDIT + i);
					SetWindowText(hEdit, val);
					bool isBlob = _tcscmp(val, TEXT("(BLOB)")) == 0;
					EnableWindow(hEdit, !isBlob);
					EnableWindow(GetDlgItem(hWnd, IDC_ROW_SWITCH + i), !isBlob);
				}
				return true;
			}

			case WM_CLOSE: {
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_COMMAND: {
				if (wParam >= IDC_ROW_SWITCH && wParam < IDC_ROW_SWITCH + 100) {
					int no = wParam - IDC_ROW_SWITCH;
					HWND hEdit = GetDlgItem(hWnd, IDC_ROW_EDIT + no);
					TCHAR buf[MAX_TEXT_LENGTH]{0};
					GetWindowText(hEdit, buf, MAX_TEXT_LENGTH);

					int mode = LOWORD(GetWindowLong(hWnd, GWL_USERDATA));

					if (mode != ROW_VIEW && DLG_OK == DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_EDITDATA_VALUE), hWnd, (DLGPROC)&cbDlgEditDataValue, (LPARAM)buf))
						SetWindowText(hEdit, buf);

					if (mode == ROW_VIEW)
						DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_VIEWDATA_VALUE), hWnd, (DLGPROC)&cbDlgViewDataValue, (LPARAM)buf);

					SetFocus(hEdit);
				}

				if (wParam == IDOK) {
					int id = GetDlgCtrlID(GetFocus());
					if (id >= IDC_ROW_EDIT && wParam < IDC_ROW_EDIT + 100)
						return GetAsyncKeyState(VK_CONTROL) ? SendMessage(hWnd, WM_COMMAND, IDC_DLG_OK, 0) : SendMessage(hWnd, WM_NEXTDLGCTL, 0, 0);
				}

				if (wParam == IDC_DLG_OK) {
					HWND hListWnd = (HWND)GetWindowLong(GetDlgItem(hWnd, IDC_DLG_USERDATA), GWL_USERDATA);
					int mode = LOWORD(GetWindowLong(hWnd, GWL_USERDATA));
					int colCount = HIWORD(GetWindowLong(hWnd, GWL_USERDATA));

					HWND hParentWnd = GetParent(hListWnd);
					bool hasRowid = GetProp(hParentWnd, TEXT("HASROWID"));
					char* md5keys8 = (char*)GetProp(hParentWnd, TEXT("MD5KEYS8"));

					auto changeCurrentItem = [hListWnd, mode]() {
						ListView_SetItemState( hListWnd, -1, LVIF_STATE, LVIS_SELECTED);

						int colCount = ListView_GetItemCount(hListWnd);
						currCell.iItem = mode != ROW_ADD ? (currCell.iItem + 1) % colCount : colCount - 1;
						ListView_SetItemState (hListWnd, currCell.iItem, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
						if (!ListView_EnsureVisible(hListWnd, currCell.iItem, false)) {
							RECT rect = {0};
							ListView_GetItemRect(hListWnd, currCell.iItem, &rect, LVIR_LABEL);
							ListView_Scroll(hListWnd, 0, rect.bottom);
						}
					};

					if (mode == ROW_VIEW) {
						changeCurrentItem();
						SendMessage(hWnd, WMU_SET_DLG_ROW_DATA, 0, 0);
						return true;
					}

					TCHAR* columns16[colCount];
					char* values8[colCount];
					TCHAR* values16[colCount];
					char* columns8[colCount];

					TCHAR* schema16 = utils::getName(editTableData16, true);
					TCHAR* tablename16 = utils::getName(editTableData16);

					char* schema8 = utils::utf16to8(schema16);
					char* tablename8 = utils::utf16to8(tablename16);
					delete [] schema16;
					delete [] tablename16;

					int len = 0;
					// A first column in the listview is always a rowno. Should be ignored.
					for (int i = 1; i < colCount; i++) {
						if (!IsWindowEnabled(GetDlgItem(hWnd, IDC_ROW_EDIT + i)))
							continue;

						HWND hLabel = GetDlgItem(hWnd, IDC_ROW_LABEL + i);
						len = GetWindowTextLength(hLabel);
						columns16[i] = new TCHAR[len + 1]{0};
						GetWindowText(hLabel, columns16[i], len + 1);
						columns8[i] = utils::utf16to8(columns16[i]);

						HWND hEdit = GetDlgItem(hWnd, IDC_ROW_EDIT + i);
						len = GetWindowTextLength(hEdit);
						values16[i] = new TCHAR[len + 1]{0};
						GetWindowText(hEdit, values16[i], len + 1);
						values8[i] = utils::utf16to8(values16[i]);
					}

					char sql8[MAX_TEXT_LENGTH]{0};
					char buf8[256];
					sprintf(buf8, mode == ROW_ADD ? "insert into \"%s\".\"%s\" (" : "update \"%s\".\"%s\" set ", schema8, tablename8);
					strcat(sql8, buf8);
					int valCount = 0;
					for (int i = 1; i < colCount; i++) {
						if (!IsWindowEnabled(GetDlgItem(hWnd, IDC_ROW_EDIT + i)))
							continue;

						sprintf(buf8, mode == ROW_ADD ? "%s\"%s\"" : "%s\"%s\" = ?", valCount > 0 ? ", " : "", columns8[i]);
						strcat(sql8, buf8);

						valCount++;
					}

					if (mode == ROW_ADD) {
						char placeholders8[(valCount + 1) * 2]{0}; // count = 3 => ?, ?, ?
						for (int i = 0; i < (valCount + 1) * 2 - 3; i++)
							placeholders8[i] = i % 2 ? ',' : '?';
						placeholders8[(valCount + 1) * 2 - 1] = '\0';
						strcat(sql8, ") values (");
						strcat(sql8, placeholders8);
						strcat(sql8, ")");
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
						int valNo = 1;
						for (int i = 1; i < colCount; i++)
							if (IsWindowEnabled(GetDlgItem(hWnd, IDC_ROW_EDIT + i))) {
								utils::sqlite3_bind_variant(stmt, valNo, values8[i]);
								valNo++;
							}

						if (mode == ROW_EDIT) {
							TCHAR rowid16[64];
							ListView_GetItemText(hListWnd, currCell.iItem, colCount, rowid16, 64);
							char* rowid8 = utils::utf16to8(rowid16);
							sqlite3_bind_text(stmt, valNo, rowid8, strlen(rowid8), SQLITE_TRANSIENT);
							delete [] rowid8;
						}

						rc = SQLITE_DONE == sqlite3_step(stmt);
					}
					sqlite3_finalize(stmt);
					sqlite3_update_hook(db, NULL, NULL);

					if (rc) {
						char sql8[255 + strlen(hasRowid ? "rowid" : md5keys8)];
						sprintf(sql8, "select *, %s rowid from \"%s\".\"%s\" where %s = ?", hasRowid ? "rowid" : md5keys8, schema8, tablename8, hasRowid ? "rowid" : md5keys8);

						sqlite3_stmt *stmt;
						sqlite3_prepare_v2(db, sql8, -1, &stmt, 0);
						if (hasRowid) {
							sqlite3_bind_int64(stmt, 1, hud.rowid);
						} else {
							char* keys8 = (char*)GetProp(hParentWnd, TEXT("KEYS8"));
							int keyCount = (int)GetProp(hParentWnd, TEXT("KEYCOUNT"));
							char placeholders8[(keyCount + 1) * 2]{0}; // count = 3 => ?, ?, ?
							for (int i = 0; i < (keyCount + 1) * 2 - 3; i++)
								placeholders8[i] = i % 2 ? ',' : '?';
							placeholders8[(keyCount + 1) * 2 - 1] = 0;

							char sub8[2 * strlen(keys8) + 256]{0};
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
							int iItem = mode == ROW_ADD ? ListView_GetItemCount(hListWnd) : currCell.iItem;
							for (int i = 0; i < sqlite3_column_count(stmt); i++) {
								int colType = sqlite3_column_type(stmt, i);
								TCHAR* value16 = utils::utf8to16(
									colType == SQLITE_NULL ? "" :
									colType == SQLITE_BLOB ? "(BLOB)" :
									(char *) sqlite3_column_text(stmt, i));

								LVITEM  lvi = {0};
								if (mode == ROW_ADD && i == 0) {
									lvi.mask = 0;
									lvi.iSubItem = 0;
									lvi.iItem = iItem;
									ListView_InsertItem(hListWnd, &lvi);
								}

								lvi.mask = LVIF_TEXT;
								lvi.iSubItem = i + 1;
								lvi.iItem = iItem;
								lvi.pszText = value16;
								lvi.cchTextMax = _tcslen(value16) + 1;

								ListView_SetItem(hListWnd, &lvi);
								delete [] value16;
							}
						}
						sqlite3_finalize(stmt);

						changeCurrentItem();
						if (mode == ROW_EDIT)
							SendMessage(hWnd, WMU_SET_DLG_ROW_DATA, 0, 0);
						SetFocus(GetDlgItem(hWnd, IDC_ROW_EDIT + 1));
					} else
						showDbError(hWnd);

					for (int i = 1; i < colCount; i++) {
						delete [] columns16[i];
						delete [] columns8[i];
						delete [] values16[i];
						delete [] values8[i];
					}

					delete [] tablename8;
					delete [] schema8;
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgAddColumn (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TCHAR buf[256];
				_stprintf(buf, TEXT("Add column to \"%s\""), editTableData16);
				SetWindowText(hWnd, buf);

				HWND hColType = GetDlgItem(hWnd, IDC_DLG_COLTYPE);
				for (int i = 0; DATATYPES16[i]; i++)
					ComboBox_AddString(hColType, DATATYPES16[i]);
				ComboBox_SetCurSel(hColType, 0);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					TCHAR colName16[255] = {0};
					GetDlgItemText(hWnd, IDC_DLG_COLNAME, colName16, 255);

					TCHAR colType16[64] = {0};
					GetDlgItemText(hWnd, IDC_DLG_COLTYPE, colType16, 64);

					TCHAR _check16[255] = {0}, check16[300] = {0};
					GetDlgItemText(hWnd, IDC_DLG_CHECK, _check16, 255);
					_stprintf(check16, _tcslen(_check16) > 0 ? TEXT("check(%s)") : TEXT("%s"), _check16);

					TCHAR _defValue16[255] = {0}, defValue16[300] = {0};
					GetDlgItemText(hWnd, IDC_DLG_DEFVALUE, _defValue16, 255);
					_stprintf(defValue16, _tcslen(_defValue16) > 0 ? TEXT("default \"%s\"") : TEXT("%s"), _defValue16);

					bool isNotNull = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISNOTNULL));
					bool isUnique = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISUNIQUE));
					TCHAR query16[2000 + _tcslen(editTableData16)] = {0};

					TCHAR* schema16 = utils::getName(editTableData16, true);
					TCHAR* tablename16 = utils::getName(editTableData16);
					_stprintf(query16, TEXT("alter table \"%s\".\"%s\" add column \"%s\" %s %s %s %s %s"),
						schema16, tablename16, colName16, colType16, isNotNull ? TEXT("NOT NULL") : TEXT(""), defValue16, check16, isUnique ? TEXT("UNIQUE") : TEXT(""));
					delete [] schema16;
					delete [] tablename16;

					char* query8 = utils::utf16to8(query16);
					if (SQLITE_OK != sqlite3_exec(db, query8, NULL, NULL, NULL))
						showDbError(hWnd);
					else
						EndDialog(hWnd, 0);
					delete [] query8;
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

	BOOL CALLBACK cbDlgFind (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hEditorWnd = (HWND)lParam;
				int start, end;
				SendMessage(hEditorWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
				TCHAR* word;
				if (start != end) {
					word = new TCHAR[end - start + 1]{0};
					TEXTRANGE tr{{start, end}, word};
					SendMessage(hEditorWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
				} else {
					word = getWordFromCursor(hEditorWnd, false);
				}
				SetDlgItemText(hWnd, IDC_DLG_FIND, word);
				delete [] word;
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK || wParam == IDOK) {
					GetDlgItemText(hWnd, IDC_DLG_FIND, searchString, 255);
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

	BOOL CALLBACK cbDlgDDL (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				setEditorFont(hEditorWnd);
				SendMessage(hEditorWnd, WM_SETTEXT, (WPARAM)0, (LPARAM)lParam);

				processHightlight(hEditorWnd, true, false);
			}
			break;

			case WM_SIZE: {
				HWND hEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				RECT rc = {0};
				GetClientRect(hWnd, &rc);
				SetWindowPos(hEditorWnd, 0, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER);
				SendMessage(hEditorWnd, EM_SETSEL, (WPARAM)0, (LPARAM)0);
			}
			break;

			case WM_COMMAND: {
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



	#define CHART_NONE 0
	#define CHART_PLOT 1
	#define CHART_BARS 2

	#define CHART_MAX  1.79769e+308
	#define CHART_NULL 0.00012003

	#define CHART_BORDER 40
	#define CHART_GRID 5
	#define CHART_BARS_LEFT 180
	#define CHART_BAR_HEIGHT 20
	#define CHART_BAR_SPACE 3

	COLORREF COLORS[MAX_CHART_COLOR_COUNT] = {RGB(51, 34, 136), RGB(136, 204, 238), RGB(68, 170, 153), RGB(17, 119, 51), RGB(153, 153, 51), RGB(221, 204, 119), RGB(204, 102, 119), RGB(136, 34, 85), RGB(170, 68, 153)};
	HPEN hPens[MAX_CHART_COLOR_COUNT];
	HBRUSH hBrushes[MAX_CHART_COLOR_COUNT];

	int qsortComparator (const double *i, const double *j) {
		double s = *i - *j;
		return s > 0 ? 1 : s < 0 ? -1 : 0;
	}

	double map (double x, double in_min, double in_max, double out_min, double out_max) {
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}

	BOOL CALLBACK cbDlgChart (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLong(hWnd, GWL_USERDATA, lParam);
				for (int i = 0; i < MAX_CHART_COLOR_COUNT; i++) {
					hPens[i] = CreatePen(PS_SOLID, 2, COLORS[i]);
					hBrushes[i] = CreateSolidBrush(COLORS[i]);
				}

				HWND hListWnd = (HWND)lParam;
				int colCount = Header_GetItemCount(ListView_GetHeader(hListWnd));
				int rowCount = ListView_GetItemCount(hListWnd);
				int size = colCount * rowCount;
				double *rawdata = new double[size];
				double *data = new double[size];

				double minX = CHART_MAX;
				double maxX = -CHART_MAX;
				double minY = CHART_MAX;
				double maxY = -CHART_MAX;

				for (int i = 0; i < size; i++) {
					rawdata[i] = CHART_NULL;
					data[i] = CHART_NULL;
				}

				for (int colNo = 1; colNo < colCount; colNo++) {
					for (int rowNo = 0; rowNo < rowCount; rowNo++) {
						TCHAR buf16[256]{0};
						double val;
						ListView_GetItemText(hListWnd, rowNo, colNo, buf16, 255);
						if (_tcslen(buf16) && utils::isNumber(buf16, &val)) {
							rawdata[colNo * rowCount + rowNo] = val;
							minX = colNo == 1 ? MIN(val, minX) : minX;
							maxX = colNo == 1 ? MAX(val, maxX) : maxX;
							minY = colNo > 1 ? MIN(val, minY) : minY;
							maxY = colNo > 1 ? MAX(val, maxY) : maxY;
						}
					}
				}

				int type = CHART_BARS;
				for (int i = 0; i < rowCount; i++) {
					if (rawdata[rowCount + i] != CHART_NULL)
						type = CHART_PLOT;
				}

				int dataColCount = 0;
				if (type == CHART_BARS) {
					for (int i = 0; i < size; i++)
						data[i] = rawdata[i];

					for (int colNo = 2; colNo < colCount; colNo++) {
						int dataRowCount = 0;
						for (int rowNo = 0; rowNo < rowCount; rowNo++)
							dataRowCount += data[rowCount * colNo + rowNo] != CHART_NULL;
						dataColCount += dataRowCount > 0;
					}
					if (dataColCount == 0)
						type = CHART_NONE;
				}

				// Sort by axis-X data
				if (type == CHART_PLOT) {
					double axisX[rowCount];
					for (int i = 0; i < rowCount; i++)
						axisX[i] = rawdata[rowCount + i];
					qsort(axisX, rowCount, sizeof(double), (int(*) (const void *, const void *)) qsortComparator);

					for (int i = 0; i < rowCount; i++)
						data[rowCount + i] = axisX[i];

					for (int rowNo = 0; rowNo < rowCount; rowNo++) {
						if (axisX[rowNo] == CHART_NULL)
							continue;

						int idx;
						for (idx = 0; idx < rowCount && (rawdata[rowCount + idx] != axisX[rowNo]); idx++);
						for (int barNo = 2; barNo < colCount; barNo++)
							data[rowCount * barNo + rowNo] = rawdata[rowCount * barNo + idx];
					}
				}

				delete [] rawdata;

				double *minmax = new double[4];
				minmax[0] = minX;
				minmax[1] = maxX;
				minmax[2] = minY;
				minmax[3] = maxY;

				SetProp(hWnd, TEXT("TYPE"), (HANDLE)type);
				SetProp(hWnd, TEXT("DATA"), (HANDLE)data);
				SetProp(hWnd, TEXT("MINMAX"), (HANDLE)minmax);
				SetProp(hWnd, TEXT("COLCOUNT"), (HANDLE)colCount);
				SetProp(hWnd, TEXT("ROWCOUNT"), (HANDLE)rowCount);
				SetProp(hWnd, TEXT("DATACOLCOUNT"), (HANDLE)dataColCount);
				SendMessage(hWnd, WM_PAINT, 0, 0);
			}
			break;

			case WM_DESTROY: {
				RemoveProp(hWnd, TEXT("TYPE"));
				RemoveProp(hWnd, TEXT("COLCOUNT"));
				RemoveProp(hWnd, TEXT("ROWCOUNT"));
				RemoveProp(hWnd, TEXT("DATACOLCOUNT"));

				double* data = (double*)GetProp(hWnd, TEXT("DATA"));
				delete [] data;
				RemoveProp(hWnd, TEXT("DATA"));

				double* minmax = (double*)GetProp(hWnd, TEXT("MINMAX"));
				delete [] minmax;
				RemoveProp(hWnd, TEXT("MINMAX"));

				for (int i = 0; i < MAX_CHART_COLOR_COUNT; i++) {
					DeleteObject(hPens[i]);
					DeleteObject(hBrushes[i]);
				}
			}
			break;

			case WM_ERASEBKGND: {
				RECT rc{0};
				GetClientRect(hWnd, &rc);
				FillRect((HDC)wParam, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

				return true;
			}
			break;

			case WM_PAINT : {
				InvalidateRect(hWnd, NULL, true);

				int type = (int)GetProp(hWnd, TEXT("TYPE"));
				double* data = (double*)GetProp(hWnd, TEXT("DATA"));
				int colCount = (int)GetProp(hWnd, TEXT("COLCOUNT"));
				int rowCount = (int)GetProp(hWnd, TEXT("ROWCOUNT"));
				int dataColCount = (int)GetProp(hWnd, TEXT("DATACOLCOUNT"));

				double* minmax = (double*)GetProp(hWnd, TEXT("MINMAX"));
				double minX = minmax[0];
				double maxX = minmax[1];
				double minY = minmax[2];
				double maxY = minmax[3];

				HWND hListWnd = (HWND)GetWindowLong(hWnd, GWL_USERDATA);
				HWND hHeader = ListView_GetHeader(hListWnd);

				RECT rc{0};
				GetClientRect(hWnd, &rc);
				int w = rc.right;
				int h = rc.bottom;

				PAINTSTRUCT ps{0};
				ps.fErase = true;
				HDC hdc = BeginPaint(hWnd, &ps);
				SelectFont(hdc, hDefFont);

				if (type == CHART_NONE) {
					RECT rc = {0, 0, w, h}, rc2 = {0, 0, w, h};
					TCHAR text[] = TEXT("Not enough data to chart.\nVisit Wiki if you have questions.");
					DrawText(hdc, text, _tcslen(text), &rc2, DT_CALCRECT);
					rc.top = rc.bottom / 2 - rc2.bottom / 2;
					DrawText(hdc, text, _tcslen(text), &rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_EXPANDTABS);
				}

				if (type == CHART_BARS) {
					double minV = MIN(minX, minY);
					minV = MIN(0, minV);
					double maxV = MAX(maxX, maxY);

					HBRUSH hNullBrush = CreateSolidBrush(RGB(200, 200, 200));

					int barNo = 0;
					for (int colNo = 2; colNo < colCount; colNo++) {
						bool isEmpty = true;
						for (int rowNo = 0; rowNo < rowCount; rowNo++)
							isEmpty = isEmpty && (data[colNo * rowCount + rowNo] == CHART_NULL);

						if (isEmpty)
							continue;

						for (int rowNo = 0; rowNo < rowCount; rowNo++) {
							double val = data[colNo * rowCount + rowNo];
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

							SelectBrush(hdc, isNull ? hNullBrush : hBrushes[barNo % MAX_CHART_COLOR_COUNT]);
							Rectangle(hdc, left, top, right, top + CHART_BAR_HEIGHT);

							// Value and title on bar
							bool isValueInside = (right - left > 30) || (!isNull && val < 0);
							RECT rc {left + 10, top, isValueInside ? right - 10 : right + 30, top + CHART_BAR_HEIGHT};
							TCHAR val16[64];
							if (isNull)
								_stprintf(val16, TEXT("N/A"));
							else
								_stprintf(val16, TEXT("%g"), data[colNo * rowCount + rowNo]);
							SetBkColor(hdc, isNull ? RGB(200, 200, 200) : isValueInside ? COLORS[barNo % MAX_CHART_COLOR_COUNT] : RGB(255, 255, 255));
							SetTextColor(hdc, isValueInside ? RGB(255, 255, 255) : RGB(0, 0, 0));
							DrawText(hdc, val16, _tcslen(val16), &rc, (val > 0 ? DT_RIGHT : DT_LEFT) | DT_VCENTER | DT_SINGLELINE);

							if (right - left > 60) {
								TCHAR attr[256]{0};
								Header_GetItemText(hHeader, colNo, attr, 255);
								DrawText(hdc, attr, _tcslen(attr), &rc, (val > 0 ? DT_LEFT : DT_RIGHT) | DT_VCENTER | DT_SINGLELINE);
							}
						}
						barNo++;
					}

					// Group title
					for (int rowNo = 0; rowNo < rowCount; rowNo++) {
						int top = CHART_BORDER/2 + rowNo * dataColCount * (CHART_BAR_HEIGHT + CHART_BAR_SPACE + 10);
						RECT rc = {CHART_BORDER, top, CHART_BARS_LEFT - 10, top + (CHART_BAR_HEIGHT + CHART_BAR_SPACE) * dataColCount - CHART_BAR_SPACE};
						TCHAR name[256]{0};
						ListView_GetItemText(hListWnd, rowNo, 1, name, 255);

						SetTextColor(hdc, RGB(0, 0, 0));
						SetBkColor(hdc, RGB(255, 255, 255));
						DrawText(hdc, name, _tcslen(name), &rc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
					}

					DeleteObject(hNullBrush);
				}

				if (type == CHART_PLOT) {
					HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
					SelectObject(hdc, hPen);

					// Grid
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

					double d = findDelta(maxX - minX, CHART_GRID);
					int x = 0, y = 0;
					for (int i = 0; minX + d * i < maxX + d; i++) {
						x = map(minX + d * i, minX, maxX, CHART_BORDER, w - CHART_BORDER);
						MoveToEx(hdc, x, CHART_BORDER, NULL);
						LineTo(hdc, x, h - CHART_BORDER);

						TCHAR val[64];
						_stprintf(val, TEXT("%g"), minX + d * i);
						RECT rc {x - 30, 0, x + 30, CHART_BORDER - 5};
						DrawText(hdc, val, _tcslen(val), &rc, DT_BOTTOM | DT_SINGLELINE | DT_CENTER);
						RECT rc2 {x - 30, h - CHART_BORDER + 5, x + 30, h};
						DrawText(hdc, val, _tcslen(val), &rc2, DT_TOP | DT_SINGLELINE | DT_CENTER);
					}

					d = findDelta(maxY - minY, CHART_GRID);
					for (int i = 0; minY + d * i < maxY + d; i++) {
						y = map(minY + d * i, minY, maxY, CHART_BORDER, h - CHART_BORDER);
						MoveToEx(hdc, CHART_BORDER, y, NULL);
						LineTo(hdc, x, y);

						TCHAR val[64];
						_stprintf(val, TEXT("%g"), minY + d * i);
						RECT rc {0, h - y - 10, CHART_BORDER - 5, h - y + 10};
						DrawText(hdc, val, _tcslen(val), &rc, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
						RECT rc2 {x + 5, h - y - 10, w, h - y + 8};
						DrawText(hdc, val, _tcslen(val), &rc2, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
					}

					// Lines
					int lineNo = 0;
					for (int colNo = 2; colNo < colCount; colNo++) {
						SelectObject(hdc, hPens[lineNo % MAX_CHART_COLOR_COUNT]);
						int pointCount = 0;

						for (int rowNo = 0; rowNo < rowCount; rowNo++) {
							if (data[rowCount + rowNo] == CHART_NULL || data[colNo * rowCount + rowNo] == CHART_NULL)
								continue;

							if (!pointCount) {
								double x = map(data[rowCount + rowNo], minX, maxX, CHART_BORDER, w - CHART_BORDER);
								double y = h - map(data[rowCount * colNo + rowNo], minY, maxY, CHART_BORDER, h - CHART_BORDER);
								MoveToEx(hdc, x, y, NULL);
								pointCount++;
								continue;
							}

							double x = map(data[rowCount + rowNo], minX, maxX, CHART_BORDER, w - CHART_BORDER);
							double y = h - map(data[rowCount * colNo + rowNo], minY, maxY, CHART_BORDER, h - CHART_BORDER);
							LineTo(hdc, x, y);

							pointCount++;
						}

						if (pointCount > 1) {
							TCHAR name16[256];
							Header_GetItemText(hHeader, colNo, name16, 255);

							int x = w - 100;
							int y = CHART_BORDER + 5 + lineNo * 15;
							SelectBrush(hdc, hBrushes[lineNo % MAX_CHART_COLOR_COUNT]);
							Ellipse(hdc, x - 15, y + 2, x - 5, y + 12);

							TextOut(hdc, x, y, name16, _tcslen(name16));
							lineNo++;
						}
					}

					if (lineNo == 0) {
						SetProp(hWnd, TEXT("TYPE"), (HANDLE)CHART_NONE);
						PostMessage(hWnd, WM_PAINT, 0, 0);
					}

					DeleteObject(hPen);
				}

				EndPaint(hWnd, &ps);
			}
			break;

			case WM_MOUSEMOVE: {
				int type = (int)GetProp(hWnd, TEXT("TYPE"));
				if (type != CHART_PLOT)
					return true;

				double* minmax = (double*)GetProp(hWnd, TEXT("MINMAX"));
				double minX = minmax[0];
				double maxX = minmax[1];
				double minY = minmax[2];
				double maxY = minmax[3];

				RECT rc{0};
				GetClientRect(hWnd, &rc);
				int w = rc.right;
				int h = rc.bottom;

				TCHAR title[255]{0};
				double x = LOWORD(lParam);
				double y = h - HIWORD(lParam);
				x = map(x, CHART_BORDER, w - CHART_BORDER, minX, maxX);
				y = map(y, CHART_BORDER, h - CHART_BORDER, minY, maxY);

				_stprintf(title, TEXT("X: %g, Y: %g"), x, y);
				SetWindowText(hWnd, title);
			}
			break;

			case WM_SIZE: {
				SendMessage(hWnd, WM_PAINT, 0, 0);
			}
			break;

			case WM_COMMAND: {
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

	// lParam and USERDATA are sqlite3 statement handle
	BOOL CALLBACK cbDlgBindParameters (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);

				sqlite3_stmt* stmt = (sqlite3_stmt*)lParam;
				sqlite3_stmt* stmt2;
				sqlite3_prepare_v2(db, "select value from preferences.query_params where lower(dbname) = lower(?1) and lower(name) = lower(?2)", -1, &stmt2, 0);
				char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));

				int paramCount = sqlite3_bind_parameter_count(stmt);
				for (int i = 1; i < paramCount + 1; i++) {
					const char* name8 = (char*)sqlite3_bind_parameter_name(stmt, i);
					TCHAR* name16 = utils::utf8to16(name8);
					if (_tcslen(name16)) {
						CreateWindow(WC_STATIC, name16, WS_VISIBLE | WS_CHILD | SS_RIGHT, 5, 10 + 30 * (i - 1), 100, 18, hWnd, (HMENU)(IDC_ROW_LABEL +  i), GetModuleHandle(0), 0);
						HWND hComboWnd = CreateWindow(WC_COMBOBOX, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_TABSTOP | CBS_DROPDOWN, 110, 7 + 30 * (i - 1), 184, 200, hWnd, (HMENU)(IDC_ROW_EDIT + i), GetModuleHandle(0), 0);

						sqlite3_reset(stmt2);
						sqlite3_bind_text(stmt2, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt2, 2, name8, strlen(name8), SQLITE_TRANSIENT);

						while(SQLITE_ROW == sqlite3_step(stmt2)) {
							TCHAR* value16 = utils::utf8to16((char*)sqlite3_column_text(stmt2, 0));
							ComboBox_AddString(hComboWnd, value16);
							delete [] value16;
						}
					} else {
						CreateWindow(WC_STATIC, TEXT("(Unnamed)"), WS_VISIBLE | WS_CHILD | SS_RIGHT, 5, 10 + 30 * (i - 1), 100, 18, hWnd, (HMENU)(IDC_ROW_LABEL +  i), GetModuleHandle(0), 0);
						CreateWindow(WC_EDIT, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_TABSTOP | CBS_DROPDOWN, 110, 7 + 30 * (i - 1), 184, 20, hWnd, (HMENU)(IDC_ROW_EDIT + i), GetModuleHandle(0), 0);
					}
					delete [] name16;
				}
				sqlite3_finalize(stmt2);
				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETDEFFONT);

				SetWindowPos(hWnd, 0, 0, 0, 305, paramCount * 30 + 62, SWP_NOMOVE | SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWnd, IDC_DLG_OK), 0, 204, paramCount * 30 + 8, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				delete [] dbname8;

				SetFocus(GetDlgItem(hWnd, IDC_ROW_EDIT + 1));
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					sqlite3_stmt* stmt = (sqlite3_stmt*)GetWindowLong(hWnd, GWL_USERDATA);;
					sqlite3_stmt* stmt2;
					sqlite3_prepare_v2(db, "replace into preferences.query_params (dbname, name, value) values (?1, ?2, ?3);", -1, &stmt2, 0);
					char* dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));

					int paramCount = sqlite3_bind_parameter_count(stmt);
					for (int i = 1; i <= paramCount; i++) {
						TCHAR name16[1024];
						GetDlgItemText(hWnd, IDC_ROW_LABEL + i, name16, 1023);
						char* name8 = utils::utf16to8(name16);

						TCHAR value16[1024];
						GetDlgItemText(hWnd, IDC_ROW_EDIT + i, value16, 1023);
						char* value8 = utils::utf16to8(value16);
						utils::sqlite3_bind_variant(stmt, i, value8);

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

	BOOL CALLBACK cbEnumFont(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD fontType, LPVOID hWnd)  {
		if (fontType & TRUETYPE_FONTTYPE && lplf->lfFaceName[0] != TEXT('@'))
			ComboBox_AddString((HWND)hWnd, lplf->lfFaceName);
		return true;
	}

	BOOL CALLBACK cbDlgSettings (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hFontSize = GetDlgItem(hWnd, IDC_DLG_FONT_SIZE);
				int fontSize = prefs::get("font-size");
				int sizes[] = {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24};
				int idx = 0;
				for (int i = 0; i < 11; i++) {
					idx = fontSize == sizes[i] ? i : idx;
					TCHAR buf[3];
					_stprintf(buf, TEXT("%i"), sizes[i]);
					ComboBox_AddString(hFontSize, buf);
				}
				ComboBox_SetCurSel(hFontSize, idx);

				HWND hFontFamily = GetDlgItem(hWnd, IDC_DLG_FONT_FAMILY);
				HDC hDC = GetDC(hMainWnd);
				LOGFONT lf = {0};
				lf.lfFaceName[0] = TEXT('\0');
				lf.lfCharSet = GetTextCharset(hDC);
				EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC) cbEnumFont, (LPARAM)hFontFamily, 0);
				ReleaseDC(hMainWnd, hDC);
				char* fontFamily8 = prefs::get("font-family", "Courier New");
				TCHAR* fontFamily16 = utils::utf8to16(fontFamily8);
				ComboBox_SetCurSel(hFontFamily, ComboBox_FindString(hFontFamily, -1, fontFamily16));
				delete [] fontFamily16;
				delete [] fontFamily8;

				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_AUTOLOAD), prefs::get("autoload-extensions") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_RESTORE_DB), prefs::get("restore-db") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_RESTORE_EDITOR), prefs::get("restore-editor") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_USE_HIGHLIGHT), prefs::get("use-highlight") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_USE_LEGACY), prefs::get("use-legacy-rename") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_SYNC_OFF), prefs::get("synchronous-off") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_EXIT_BY_ESCAPE), prefs::get("exit-by-escape") ? BST_CHECKED : BST_UNCHECKED);

				TCHAR buf[255];
				_stprintf(buf, TEXT("%i"), prefs::get("row-limit"));
				SetDlgItemText(hWnd, IDC_DLG_ROW_LIMIT, buf);

				_stprintf(buf, TEXT("%i"), prefs::get("cli-row-limit"));
				SetDlgItemText(hWnd, IDC_DLG_CLI_ROW_LIMIT, buf);

				_stprintf(buf, TEXT("%i"), prefs::get("beep-query-duration"));
				SetDlgItemText(hWnd, IDC_DLG_BEEP_ON_QUERY_END, buf);

				char* startup8 = prefs::get("startup", "");
				TCHAR* startup16 = utils::utf8to16(startup8);
				SetDlgItemText(hWnd, IDC_DLG_STARTUP, startup16);
				delete [] startup16;
				delete [] startup8;

				HWND hIndent = GetDlgItem(hWnd, IDC_DLG_INDENT);
				for (int i = 0; i < 3; i++)
					ComboBox_AddString(hIndent, INDENT_LABELS[i]);
				ComboBox_SetCurSel(hIndent, prefs::get("editor-indent"));
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					TCHAR buf[255];
					GetDlgItemText(hWnd, IDC_DLG_FONT_FAMILY, buf, 255);
					char* fontFamily8 = utils::utf16to8(buf);
					prefs::set("font-family", fontFamily8);
					delete [] fontFamily8;
					GetDlgItemText(hWnd, IDC_DLG_FONT_SIZE, buf, 255);
					prefs::set("font-size", _tcstol(buf, NULL, 10));
					prefs::set("autoload-extensions", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_AUTOLOAD)));
					prefs::set("restore-db", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_RESTORE_DB)));
					prefs::set("restore-editor", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_RESTORE_EDITOR)));
					prefs::set("use-highlight", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_USE_HIGHLIGHT)));
					prefs::set("use-legacy-rename", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_USE_LEGACY)));
					prefs::set("exit-by-escape", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_EXIT_BY_ESCAPE)));
					prefs::set("synchronous-off", Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_SYNC_OFF)));
					prefs::set("editor-indent", ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_INDENT)));

					GetDlgItemText(hWnd, IDC_DLG_ROW_LIMIT, buf, 255);
					prefs::set("row-limit", (int)_tcstod(buf, NULL));

					GetDlgItemText(hWnd, IDC_DLG_CLI_ROW_LIMIT, buf, 255);
					prefs::set("cli-row-limit", (int)_tcstod(buf, NULL));

					GetDlgItemText(hWnd, IDC_DLG_BEEP_ON_QUERY_END, buf, 255);
					prefs::set("beep-query-duration", (int)_tcstod(buf, NULL));

					setEditorFont(hEditorWnd);
					setTreeFont(hTreeWnd);
					sqlite3_exec(db, prefs::get("use-legacy-rename") ? "pragma legacy_alter_table = 1" : "pragma legacy_alter_table = 0", 0, 0, 0);

					TCHAR startup16[MAX_TEXT_LENGTH]{0};
					GetDlgItemText(hWnd, IDC_DLG_STARTUP, startup16, MAX_TEXT_LENGTH - 1);
					char* startup8 = utils::utf16to8(startup16);
					prefs::set("startup", startup8);
					delete [] startup8;

					if (!prefs::get("use-highlight")) {
						CHARFORMAT cf = {0};
						cf.cbSize = sizeof(CHARFORMAT2) ;
						SendMessage(hEditorWnd, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM) &cf);
						cf.dwMask = CFM_COLOR;
						cf.dwEffects = 0;
						cf.crTextColor = RGB(0, 0, 0);

						SendMessage(hEditorWnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &cf);
					}

					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	// USERDATA = MAKELPARAM(iItem, iSubItem)
	LRESULT CALLBACK cbNewEditDataEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_GETDLGCODE)
			return (DLGC_WANTALLKEYS | CallWindowProc(cbOldEditDataEdit, hWnd, msg, wParam, lParam));

		switch(msg){
			case WM_DESTROY: {
				HWND hListWnd = GetParent(hWnd);
				HWND hBtn = FindWindowExW(hListWnd, 0, WC_BUTTON, NULL);
				DestroyWindow(hBtn);

				int data = GetWindowLong(hWnd, GWL_USERDATA);
				if (data) {
					int size = GetWindowTextLength(hWnd);
					TCHAR value16[size + 1]{0};
					GetWindowText(hWnd, value16, size + 1);
					ListView_UpdateCell(hListWnd, LOWORD(data), HIWORD(data), value16);
				}
				SetFocus(hListWnd);
			}
			break;

			case WM_KILLFOCUS: {
				DestroyWindow(hWnd);
			}
			break;

			case WM_KEYDOWN: {
				if (wParam == VK_RETURN) {
					DestroyWindow(hWnd);
				}

				if (wParam == VK_ESCAPE) {
					SetWindowLong(hWnd, GWL_USERDATA, 0);
					DestroyWindow(hWnd);
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
				int data = GetWindowLong(hWnd, GWL_USERDATA);
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

			case WM_KILLFOCUS: {
				DestroyWindow(hWnd);
			}
			break;

			case WM_KEYDOWN: {
				if (wParam == VK_RETURN) {
					DestroyWindow(hWnd);
				}

				if (wParam == VK_ESCAPE){
					SetWindowLong(hWnd, GWL_USERDATA, 0);
					DestroyWindow(hWnd);
				}
			}
			break;
		}

		return CallWindowProc(cbOldAddTableCell, hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK cbNewFilterEdit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_GETDLGCODE)
			return (DLGC_WANTALLKEYS | CallWindowProc(cbOldHeaderEdit, hWnd, msg, wParam, lParam));

		switch(msg){
			// Prevent beep
			case WM_CHAR: {
				if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == VK_TAB)
					return 0;
			}
			break;

			case WM_KEYDOWN: {
				if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == VK_TAB) {
					HWND hDlgWnd = GetAncestor(hWnd, GA_ROOT);
					if (wParam == VK_RETURN) {
						SendMessage(hDlgWnd, WMU_UPDATE_DATA, 0, 0);
						SetFocus(hWnd);
					}

					if (wParam == VK_ESCAPE)
						SendMessage(hDlgWnd, WM_CLOSE, 0, 0);

					if (wParam == VK_TAB) {
						HWND hListWnd = GetDlgItem(hDlgWnd, IDC_DLG_QUERYLIST);
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

		return CallWindowProc(cbOldHeaderEdit, hWnd, msg, wParam, lParam);
	}

	bool ListView_UpdateCell(HWND hListWnd, int rowNo, int colNo, TCHAR* value16) {
		HWND hHeader = (HWND)ListView_GetHeader(hListWnd);
		TCHAR column16[256]{0};
		if (hHeader != NULL && Header_GetItemText(hHeader, colNo, column16, 255)) {
			TCHAR* schema16 = utils::getName(editTableData16, true);
			TCHAR* tablename16 = utils::getName(editTableData16);

			HWND hParentWnd = GetParent(hListWnd);
			bool hasRowid = GetProp(hParentWnd, TEXT("HASROWID"));
			TCHAR* md5keys16 = utils::utf8to16((char*)GetProp(hParentWnd, TEXT("MD5KEYS8")));

			TCHAR query16[256 + _tcslen(editTableData16)];
			_stprintf(query16, TEXT("update \"%s\".\"%s\" set %s = ?1 where %s = ?2"), schema16, tablename16, column16, hasRowid ? TEXT("rowid") : md5keys16);
			delete [] schema16;
			delete [] tablename16;
			delete [] md5keys16;

			int colCount = Header_GetItemCount(hHeader);
			ListView_GetItemText(hListWnd, rowNo, colCount - 1, column16, 64);

			char* query8 = utils::utf16to8(query16);
			char* value8 = utils::utf16to8(value16);
			char* column8 = utils::utf16to8(column16);

			sqlite3_stmt *stmt;
			if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
				sqlite3_bind_text(stmt, 1, value8, strlen(value8), SQLITE_TRANSIENT);
				sqlite3_bind_text(stmt, 2, column8, strlen(column8), SQLITE_TRANSIENT);
				if (SQLITE_DONE == sqlite3_step(stmt))
					ListView_SetItemText(hListWnd, rowNo, colNo, value16);

			}
			sqlite3_finalize(stmt);

			delete [] query8;
			delete [] value8;
			delete [] column8;
		}

		return true;
	}
}
