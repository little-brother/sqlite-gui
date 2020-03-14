#include"resource.h"
#include"global.h"
#include "prefs.h"
#include "utils.h"
#include"dialogs.h"

namespace dialogs {
	BOOL CALLBACK cbDlgAdd (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TV_ITEM tv;
				tv.mask = TVIF_HANDLE | TVIF_PARAM;
				tv.hItem = treeItems[0];

				if(!TreeView_GetItem(hTreeWnd, &tv) || !tv.lParam)
					return EndDialog(hWnd, DLG_CANCEL);

				int type = abs(tv.lParam);
				HWND hDlgEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				TCHAR buf[256];
				_stprintf(buf, TEXT("Add %s"), TYPES16[type]);
				SetWindowText(hWnd, buf);

				SendMessage(hDlgEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_UPDATE | ENM_KEYEVENTS);
				SetParent(hAutoComplete, hDlgEditorWnd);

				setEditorFont(hDlgEditorWnd);
				LoadString(GetModuleHandle(NULL), IDS_CREATE_DDL + type, buf, 255);
				SetWindowText(hDlgEditorWnd, buf);
				SetWindowLong(hDlgEditorWnd, GWL_USERDATA, type);
				SetFocus(hDlgEditorWnd);
			}
			break;

			case WM_COMMAND: {
				if (LOWORD(wParam) == IDC_DLG_EDITOR && HIWORD(wParam) == EN_CHANGE)
					updateHighlighting((HWND)lParam);

				if (wParam == IDC_DLG_OK) {
					HWND hDlgEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
					int size = GetWindowTextLength(hDlgEditorWnd) + 1;
					TCHAR* text = new TCHAR[size]{0};
					GetWindowText(hDlgEditorWnd, text, size);
					bool isOk = executeCommandQuery(text);
					delete [] text;

					if (isOk) {
						EndDialog(hWnd, GetWindowLong(hDlgEditorWnd, GWL_USERDATA));
					} else {
						SetFocus(hDlgEditorWnd);
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				if (wParam == IDC_DLG_EDITOR) {
					NMHDR* pHdr = (LPNMHDR)lParam;
					MSGFILTER * pF = (MSGFILTER*)lParam;
					if (pHdr->code == EN_MSGFILTER) { // KEYDOWN
						if (pF->wParam == VK_RETURN && GetAsyncKeyState(VK_CONTROL)) {
							PostMessage(hWnd, WM_COMMAND, IDC_DLG_OK, 0);
							pF->wParam = 0;
							return true;
						}

						return processAutoComplete(pF);
					}
				}
			}
			break;

			case WM_DESTROY:
				SetParent(hAutoComplete, hEditorWnd);
				break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;

		}

		return false;
	}

	BOOL CALLBACK cbDlgEdit (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TCHAR name16[256] = {0};
				TV_ITEM tv;
				tv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
				tv.hItem = treeItems[0];
				tv.pszText = name16;
				tv.cchTextMax = 256;

				if(!TreeView_GetItem(hTreeWnd, &tv) || !tv.lParam)
					return EndDialog(hWnd, -1);

				int type = abs(tv.lParam);

				HWND hDlgEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
				TCHAR buf[65546];
				_stprintf(buf, TEXT("Edit %s \"%s\""), TYPES16[type], name16);
				SetWindowText(hWnd, buf);

				SendMessage(hDlgEditorWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_UPDATE | ENM_KEYEVENTS);
				SetParent(hAutoComplete, hDlgEditorWnd);

				setEditorFont(hDlgEditorWnd);
				SetWindowLong(hDlgEditorWnd, GWL_USERDATA, type);
				SetFocus(hDlgEditorWnd);

				TCHAR* sql16 = getDDL(name16, type);
				if (sql16) {
					TCHAR buf[_tcslen(sql16) + 128];
					_stprintf(buf, TEXT("drop %s if exists \"%s\";\n\n%s;"), TYPES16[type], name16, sql16);
					SetWindowText(hDlgEditorWnd, buf);
				} else {
					SetWindowText(hDlgEditorWnd, TEXT("Error to get SQL"));
				}
				delete [] sql16;
			}
			break;

			case WM_COMMAND: {
				if (LOWORD(wParam) == IDC_DLG_EDITOR && HIWORD(wParam) == EN_CHANGE)
					updateHighlighting((HWND)lParam);

				if (wParam == IDC_DLG_OK) {
					HWND hDlgEditorWnd = GetDlgItem(hWnd, IDC_DLG_EDITOR);
					int size = GetWindowTextLength(hDlgEditorWnd) + 1;
					TCHAR* text = new TCHAR[size]{0};
					GetWindowText(hDlgEditorWnd, text, size);
					bool isOk = executeCommandQuery(text);
					delete [] text;

					if (isOk) {
						EndDialog(hWnd, GetWindowLong(hDlgEditorWnd, GWL_USERDATA));
					} else {
						SetFocus(hDlgEditorWnd);
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				if (wParam == IDC_DLG_EDITOR) {
					NMHDR* pHdr = (LPNMHDR)lParam;
					MSGFILTER * pF = (MSGFILTER*)lParam;
					if (pHdr->code == EN_MSGFILTER) { // KEYDOWN
						if (pF->wParam == VK_RETURN && GetAsyncKeyState(VK_CONTROL)) {
							PostMessage(hWnd, WM_COMMAND, IDC_DLG_OK, 0);
							pF->wParam = 0;
							return true;
						}

						return processAutoComplete(pF);
					}
				}
			}
			break;

			case WM_DESTROY:
				SetParent(hAutoComplete, hEditorWnd);
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
				SetWindowText(hWnd, lParam == IDM_HISTORY ? TEXT("Execution history queries") : TEXT("Gists"));

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				SetFocus(hListWnd);

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

				SendMessage(hWnd, WM_UPDATE_QUERYLIST, 0, 0);
				ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | 0x10000000);
				SendMessage(hWnd, WM_SIZE, 0, 0);
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
						TCHAR query16[MAX_QUERY_LENGTH];
						ListView_GetItemText(hListWnd, pos, 1, query16, MAX_QUERY_LENGTH);

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
				if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_DLG_QUERYFILTER)
					SendMessage(hWnd, WM_UPDATE_QUERYLIST, 0, 0);

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);

				if (wParam == IDOK) {
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
					int iPos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					if (iPos == -1)
						break;

					TCHAR buf[MAX_QUERY_LENGTH];
					ListView_GetItemText(hListWnd, iPos, 1, buf, MAX_QUERY_LENGTH);
					SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (LPARAM)buf);
					SendMessage(hEditorWnd, EM_REPLACESEL, TRUE, (LPARAM)(buf[_tcslen(buf) - 1] != TEXT(';') ? TEXT(";\n") : TEXT("\n")));
					EndDialog(hWnd, DLG_OK);
				}
			}
			break;

			case WM_UPDATE_QUERYLIST: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				HWND hFilterWnd = GetDlgItem(hWnd, IDC_DLG_QUERYFILTER);

				int size = GetWindowTextLength(hFilterWnd);
				TCHAR* filter16 = new TCHAR[size + 1]{0};
				GetWindowText(hFilterWnd, filter16, size + 1);

				int idx = GetWindowLong(hWnd, GWL_USERDATA);
				char* filter8 = utils::utf16to8(filter16);

				char * queries[prefs::get("max-query-count")];
				int count = prefs::getQueries(idx == IDM_HISTORY ? "history" : "gists", filter8, queries);

				ListView_DeleteAllItems(hListWnd);
				for (int i = 0; i < count; i++) {
					TCHAR* text16 = utils::utf8to16(queries[i]);
					TCHAR* col16 = _tcstok (text16, TEXT("\t"));

					time_t time = (time_t) _tcstol(col16, NULL, 10);
					struct tm *local  = localtime(&time);
					TCHAR tbuf[64] = {0};
					_tcsftime(tbuf, 64, TEXT("%d-%m-%Y %H:%M"), local);

					LVITEM  lvi = {0};
					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 0;
					lvi.iItem = i;
					lvi.pszText = tbuf;
					lvi.cchTextMax = 64;
					ListView_InsertItem(hListWnd, &lvi);

					col16 = _tcstok (NULL, TEXT("\t"));
					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 1;
					lvi.iItem = i;
					lvi.pszText = col16;
					lvi.cchTextMax = _tcslen(col16) + 1;
					ListView_SetItem(hListWnd, &lvi);

					delete [] text16;
					delete queries[i];
				}

				if (count > 0)
					ListView_SetItemState (hListWnd, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);

				delete [] filter8;
				delete [] filter16;
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
				SetWindowPos(hWnd, 0, prefs::get("x") + 40, prefs::get("y") + 80, prefs::get("width") - 80, prefs::get("height") - 120,  SWP_NOZORDER);
				ShowWindow (hWnd, prefs::get("maximized") == 1 ? SW_MAXIMIZE : SW_SHOW);
				TCHAR buf[256];
				_stprintf(buf, TEXT("Editing data of \"%s\""), editTableData16);
				SetWindowText(hWnd, buf);

				HWND hFilterWnd = GetDlgItem(hWnd, IDC_DLG_QUERYFILTER);
				SetWindowText(hFilterWnd, TEXT("where 1 = 1 limit 1000"));
				SendMessage(hWnd, WM_UPDATE_TABLE_DATA, 0 , 0);

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				LONG style = GetWindowLong(hListWnd, GWL_STYLE);
				style &= ~LVS_SINGLESEL;
				SetWindowLong(hListWnd, GWL_STYLE, style);
				SetFocus(hListWnd);

				ShowWindow(GetDlgItem(hWnd, IDC_DLG_QUERYADD), SW_SHOW);
			}
			break;

			case WM_SIZE: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				HWND hFilterWnd = GetDlgItem(hWnd, IDC_DLG_QUERYFILTER);

				RECT rc;
				GetClientRect(hWnd, &rc);
				SetWindowPos(hFilterWnd, 0, 40, 0, rc.right - rc.left - 0 - 40, 20, SWP_NOZORDER);
				SetWindowPos(hListWnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top - 21, SWP_NOZORDER | SWP_NOMOVE);
			}
			break;

			case WM_UPDATE_TABLE_DATA: {
				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
				HWND hFilterWnd = GetDlgItem(hWnd, IDC_DLG_QUERYFILTER);
				int size = GetWindowTextLength(hFilterWnd);
				TCHAR* filter16 = new TCHAR[size + 1]{0};
				GetWindowText(hFilterWnd, filter16, size + 1);

				char* table8 = utils::utf16to8(editTableData16);
				char* filter8 = utils::utf16to8(filter16);

				char query8[MAX_QUERY_LENGTH];
				sprintf(query8, "select *, rowid rowid from %s %s", table8, filter8 && strlen(filter8) ? filter8 : "1 = 1");

				sqlite3_stmt *stmt;
				bool rc = sqlite3_prepare_v2(db, query8, -1, &stmt, 0) == SQLITE_OK;
				if (rc) {
					int colCount = sqlite3_column_count(stmt);
					setListViewData(hListWnd, stmt);
					ListView_SetColumnWidth(hListWnd, colCount - 1, 0); // last column is rowid
				} else {
					sqlite3_finalize(stmt);
				}

				delete [] table8;
				delete [] filter8;
				delete [] filter16;

				return rc;
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (pHdr->code == (DWORD)NM_DBLCLK && pHdr->hwndFrom == GetDlgItem(hWnd, IDC_DLG_QUERYLIST)) {
					HWND hListWnd = pHdr->hwndFrom;
					POINT p;
					GetCursorPos(&p);
					ScreenToClient(hListWnd, &p);

					LVHITTESTINFO hti;
					hti.pt.x = p.x;
					hti.pt.y = p.y;
					int lResult = ListView_SubItemHitTest (hListWnd, &hti);

					if (lResult!=-1){
						RECT rect;
						ListView_GetSubItemRect(hListWnd, hti.iItem, hti.iSubItem, LVIR_BOUNDS, &rect);
						int h = rect.bottom - rect.top;
						int w = rect.right - rect.left;

						TCHAR buf[MAX_QUERY_LENGTH];
						ListView_GetItemText(hListWnd, hti.iItem, hti.iSubItem, buf, MAX_QUERY_LENGTH);

						bool isRichEdit = GetAsyncKeyState(VK_CONTROL) || _tcslen(buf) > 100 || _tcschr(buf, TEXT('\n'));
						HWND hEdit = isRichEdit ?
							CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("RICHEDIT50W"), buf, WS_CHILD | WS_VISIBLE | ES_WANTRETURN | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL, rect.left, rect.top - 2, 300, 150, hListWnd, 0, GetModuleHandle(NULL), NULL):
							CreateWindowEx(0, WC_EDIT, buf, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, rect.left, rect.top, w < 100 ? 100 : w, h, hListWnd, 0, GetModuleHandle(NULL), NULL);

						SetWindowLong(hEdit, GWL_USERDATA, hti.iSubItem);
						int end = GetWindowTextLength(hEdit);
						SendMessage(hEdit, EM_SETSEL, end, end);
						if (isRichEdit)
							setEditorFont(hEdit);
						else
							SendMessage(hEdit, WM_SETFONT, (LPARAM)hDefFont, true);
						SetFocus(hEdit);

						cbOldListItemEdit = (WNDPROC)SetWindowLong(hEdit, GWL_WNDPROC, (LONG)cbNewListItemEdit);
					}
				}

				if (pHdr->code == LVN_KEYDOWN && pHdr->hwndFrom == GetDlgItem(hWnd, IDC_DLG_QUERYLIST)) {
					NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;
					if (kd->wVKey == VK_DELETE)
						PostMessage(hWnd, WM_COMMAND, IDM_ROW_DELETE, 0);
				}

				if (pHdr->code == (DWORD)NM_RCLICK && pHdr->hwndFrom == GetDlgItem(hWnd, IDC_DLG_QUERYLIST) && ListView_GetSelectedCount(pHdr->hwndFrom) == 1) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					currCell = {ia->hdr.hwndFrom, ia->iItem, ia->iSubItem};

					POINT p;
					GetCursorPos(&p);
					TrackPopupMenu(hEditDataMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
				}
			}
			break;

			case WM_COMMAND: {

				if (wParam == IDOK) { // User push Enter
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
					int pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
					if (hListWnd == GetFocus() && pos != -1) {
						currCell = {hListWnd, pos, 0};
						PostMessage(hWnd, WM_COMMAND, IDM_ROW_EDIT, 0);
					}
				}

				if (wParam == IDC_DLG_QUERYADD)
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hWnd, (DLGPROC)&cbDlgRow, (LPARAM)ROW_ADD);

				if (wParam == IDM_ROW_EDIT)
					DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ROW), hWnd, (DLGPROC)&cbDlgRow, (LPARAM)ROW_EDIT);

				if (wParam == IDM_ROW_DELETE) {
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_QUERYLIST);
					int count = ListView_GetSelectedCount(hListWnd);
					if (!count)
						return true;

					char* placeholders8 = new char[count * 2]{0}; // count = 3 => ?, ?, ?
					for (int i = 0; i < count * 2 - 1; i++)
						placeholders8[i] = i % 2 ? ',' : '?';
					placeholders8[count * 2 - 1] = '\0';

					char* sql8 = new char[128 + count * 2]{0};
					char* table8 = utils::utf16to8(editTableData16);
					sprintf(sql8, "delete from \"%s\" where rowid in (%s)", table8, placeholders8);
					delete [] placeholders8;
					delete [] table8;

					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0)) {
						int pos = -1;
						TCHAR buf16[64];
						for (int i = 0; i < count; i++) {
							pos = ListView_GetNextItem(hListWnd, pos, LVNI_SELECTED);
							ListView_GetItemText(hListWnd, pos, 0, buf16, 128);
							sqlite3_bind_int64(stmt, i + 1, _tcstol(buf16, NULL, 10));
						}

						if (SQLITE_DONE == sqlite3_step(stmt)) {
							pos = -1;
							while((pos = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED)) != -1)
								ListView_DeleteItem(hListWnd, pos);
							ListView_SetItemState (hListWnd, pos, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
						} else {
							char *err8 = (char*)sqlite3_errmsg(db);
							TCHAR* err16 = utils::utf8to16(err8);
							MessageBox(hMainWnd, err16, TEXT("Error"), MB_OK);
							sqlite3_free(err8);
							delete [] err16;
						}
						sqlite3_finalize(stmt);
					}
					delete [] sql8;
				}

				if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetDlgItem(hWnd, IDC_DLG_QUERYFILTER))
					SendMessage(hWnd, WM_UPDATE_TABLE_DATA, 0 , 0);

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

	BOOL CALLBACK cbDlgRow (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				int mode = lParam;
				HWND hListWnd = mode != ROW_VIEW ? GetDlgItem(GetWindow(hWnd, GW_OWNER), IDC_DLG_QUERYLIST) : (HWND)GetWindowLong(hTabWnd, GWL_USERDATA);
				HWND hHeader = ListView_GetHeader(hListWnd);
				int colCount = Header_GetItemCount(hHeader) - (mode != ROW_VIEW);

				if (!hHeader || !colCount)
					EndDialog(hWnd, DLG_CANCEL);

				for (int colNo = 0; colNo < colCount; colNo++) {
					TCHAR colName[255];
					HDITEM hdi = { 0 };
					hdi.mask = HDI_TEXT;
					hdi.pszText = colName;
					hdi.cchTextMax = 255;
					Header_GetItem(hHeader, colNo, &hdi);

					CreateWindow(WC_STATIC, colName, WS_VISIBLE | WS_CHILD | SS_RIGHT, 5, 5 + 20 * colNo, 70, 18, hWnd, (HMENU)(IDC_ROW_LABEL +  colNo), GetModuleHandle(0), 0);
					CreateWindow(WC_EDIT, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_TABSTOP | ES_AUTOHSCROLL | (mode == ROW_VIEW ? ES_READONLY : 0), 80, 3 + 20 * colNo, 285, 18, hWnd, (HMENU)(IDC_ROW_EDIT + colNo), GetModuleHandle(0), 0);
					CreateWindow(WC_BUTTON, TEXT(">"), WS_VISIBLE | WS_CHILD | BS_FLAT, 370, 3 + 20 * colNo, 18, 18, hWnd, (HMENU)(IDC_ROW_SWITCH + colNo), GetModuleHandle(0), 0);
				}
				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETDEFFONT);
				SetWindowPos(hWnd, 0, 0, 0, 400, colCount * 20 + 140, SWP_NOMOVE | SWP_NOZORDER);

				SetWindowText(hWnd, mode == ROW_ADD ? TEXT("New row") : mode == ROW_EDIT ? TEXT("Edit row") : TEXT("View row"));

				HWND hOkBtn = GetDlgItem(hWnd, IDC_DLG_OK);
				HWND hCancelBtn = GetDlgItem(hWnd, IDC_DLG_CANCEL);
				SetWindowText(hOkBtn, mode == ROW_ADD ? TEXT("Save and New") : mode == ROW_EDIT ? TEXT("Save and Next") : TEXT("Next"));
				SetWindowPos(hOkBtn, 0, 202, colCount * 20 + 86, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				SetWindowPos(hCancelBtn, 0, 297, colCount * 20 + 86, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				SetWindowLong(hWnd, GWL_USERDATA, MAKELPARAM(mode, colCount));
				SetWindowLong(GetDlgItem(hWnd, IDC_DLG_USERDATA), GWL_USERDATA, (LONG)hListWnd);

				if (mode != ROW_ADD)
					SendMessage(hWnd, WM_SET_DLG_ROW_DATA, 0, 0);
				SetFocus(GetDlgItem(hWnd, IDC_ROW_EDIT));
			}
			break;

			case WM_SET_DLG_ROW_DATA: {
				HWND hListWnd = (HWND)GetWindowLong(GetDlgItem(hWnd, IDC_DLG_USERDATA), GWL_USERDATA);
				int colCount = HIWORD(GetWindowLong(hWnd, GWL_USERDATA));

				TCHAR val[MAX_QUERY_LENGTH];
				for (int i = 0; i < colCount; i++) {
					ListView_GetItemText(hListWnd, currCell.iItem, i, val, MAX_QUERY_LENGTH);
					HWND hEdit = GetDlgItem(hWnd, IDC_ROW_EDIT + i);
					SetWindowText(hEdit, val);
				}
				return true;
			}

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;

			case WM_COMMAND: {
				if (wParam >= IDC_ROW_SWITCH && wParam < IDC_ROW_SWITCH + 100) {
					int no = wParam - IDC_ROW_SWITCH;
					HWND hEdit = GetDlgItem(hWnd, IDC_ROW_EDIT + no);
					int size = GetWindowTextLength(hEdit);
					TCHAR* text = new TCHAR[size + 1]{0};
					GetWindowText(hEdit, text, size + 1);

					RECT rect;
					GetWindowRect(hEdit, &rect);

					POINT p = {rect.left, rect.top};
					ScreenToClient(hWnd, &p);

					TCHAR cls[255];
					GetClassName(hEdit, cls, 255);

					int readable =  GetWindowLong(hEdit, GWL_STYLE) & ES_READONLY ? ES_READONLY : 0;
					DestroyWindow(hEdit);

					bool isEdit = !_tcscmp(WC_EDIT, cls);
					hEdit = CreateWindow(
						isEdit ? TEXT("RICHEDIT50W") : WC_EDIT,
						text,
						isEdit ?
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_TABSTOP | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN | readable:
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_TABSTOP | ES_AUTOHSCROLL | readable,
						p.x, p.y, rect.right - rect.left, isEdit ? 100 : 18, hWnd, (HMENU)(IDC_ROW_EDIT + no), GetModuleHandle(0), 0);
					SetWindowPos(hEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					SendMessage(hEdit, WM_SETFONT, (LPARAM)hDefFont, true);
					SetFocus(hEdit);
					SetWindowText(GetDlgItem(hWnd, IDC_ROW_SWITCH + no), isEdit ? TEXT("V") : TEXT(">"));

					delete [] text;
				}

				if (wParam == IDOK) {
					int id = GetDlgCtrlID(GetFocus());
					if (id >= IDC_ROW_EDIT && wParam < IDC_ROW_EDIT + 100)
						SendMessage(hWnd, WM_NEXTDLGCTL, 0, 0);
				}

				if (wParam == IDC_DLG_OK) {
					HWND hListWnd = (HWND)GetWindowLong(GetDlgItem(hWnd, IDC_DLG_USERDATA), GWL_USERDATA);
					int mode = LOWORD(GetWindowLong(hWnd, GWL_USERDATA));
					int colCount = HIWORD(GetWindowLong(hWnd, GWL_USERDATA));

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
						SendMessage(hWnd, WM_SET_DLG_ROW_DATA, 0, 0);
						return true;
					}

					TCHAR* columns16[colCount];
					char* values8[colCount];
					TCHAR* values16[colCount];
					char* columns8[colCount];
					char* table8 = utils::utf16to8(editTableData16);

					int len = 0;
					for (int i = 0; i < colCount; i++) {
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

					char* sql8 = new char[MAX_QUERY_LENGTH]{0};
					char buf8[256];
					sprintf(buf8, mode == ROW_ADD ? "insert into \"%s\" (" : "update \"%s\" set ", table8);
					strcat(sql8, buf8);
					for (int i = 0; i < colCount; i++) {
						sprintf(buf8, mode == ROW_ADD ? "\"%s\"" : "\"%s\" = ?", columns8[i]);
						strcat(sql8, buf8);
						if (i != colCount - 1)
							strcat(sql8, ", ");
					}

					if (mode == ROW_ADD) {
						char* placeholders8 = new char[colCount * 2]; // count = 3 => ?, ?, ?
						for (int i = 0; i < colCount * 2 - 1; i++)
							placeholders8[i] = i % 2 ? ',' : '?';
						placeholders8[colCount * 2 - 1] = '\0';

						strcat(sql8, ") values (");
						strcat(sql8, placeholders8);
						strcat(sql8, ")");
						delete [] placeholders8;
					} else {
						strcat(sql8, " where rowid = ?");
					}

					struct HookUserData {
						sqlite3 *db;
						HWND hWnd;
						int iItem;
					};
					HookUserData hud = {db, hListWnd, currCell.iItem};
					auto cbHook = [](void *user_data, int operation_type, char const *dbName, char const *table, sqlite3_int64 rowid) {
						HookUserData* hud = (HookUserData*)user_data;
						char sql8[255];
						sprintf(sql8, "select *, rowid from %s where rowid = ?", table);
						sqlite3_stmt *stmt;
						sqlite3_prepare_v2(hud->db, sql8, -1, &stmt, 0);
						sqlite3_bind_int64(stmt, 1, rowid);

						if (SQLITE_ROW == sqlite3_step(stmt)) {
							int iItem = operation_type == SQLITE_INSERT ? ListView_GetItemCount(hud->hWnd) : hud->iItem;
							for (int i = 0; i < sqlite3_column_count(stmt); i++) {
								char* value8 = (char *) sqlite3_column_text(stmt, i);
								TCHAR* value16 = utils::utf8to16(value8);

								LVITEM  lvi = {0};
								lvi.mask = LVIF_TEXT;
								lvi.iSubItem = i;
								lvi.iItem = iItem;
								lvi.pszText = value16;
								lvi.cchTextMax = _tcslen(value16) + 1;
								if (operation_type == SQLITE_INSERT && i == 0)
									ListView_InsertItem(hud->hWnd, &lvi);
								else
									ListView_SetItem(hud->hWnd, &lvi);
								delete [] value16;
							}
						}
						sqlite3_finalize(stmt);

					};
					sqlite3_update_hook(db, cbHook, &hud);

					sqlite3_stmt *stmt;
					bool rc = SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0);
					if (rc) {
						for (int i = 0; i < colCount; i++)
							utils::sqlite3_bind_variant(stmt, i + 1, values8[i]);

						if (mode == ROW_EDIT) {
							TCHAR rowid[64];
							ListView_GetItemText(hListWnd, currCell.iItem, colCount, rowid, 64);
							sqlite3_bind_int64(stmt, colCount + 1, _tcstol(rowid, NULL, 10));
						}

						rc = SQLITE_DONE == sqlite3_step(stmt);
						sqlite3_finalize(stmt);
						sqlite3_update_hook(db, NULL, NULL);
					}

					if (rc) {
						changeCurrentItem();
						if (mode == ROW_EDIT)
							SendMessage(hWnd, WM_SET_DLG_ROW_DATA, 0, 0);
						SetFocus(GetDlgItem(hWnd, IDC_ROW_EDIT));
					} else {
						char* err8 = (char*)sqlite3_errmsg(db);
						TCHAR* err16 = utils::utf8to16(err8);
						MessageBox(hWnd, err16, NULL, 0);
						delete [] err16;
						sqlite3_free(err8);
					}

					for (int i = 0; i < colCount; i++) {
						delete [] columns16[i];
						delete [] columns8[i];
						delete [] values16[i];
						delete [] values8[i];
					}

					delete [] table8;
					delete [] sql8;
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
				ComboBox_AddString(hColType, TEXT("INTEGER"));
				ComboBox_AddString(hColType, TEXT("REAL"));
				ComboBox_AddString(hColType, TEXT("TEXT"));
				ComboBox_AddString(hColType, TEXT("NULL"));
				ComboBox_AddString(hColType, TEXT("BLOB"));
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
					TCHAR query16[2000] = {0};
					_stprintf(query16, TEXT("alter table \"%s\" add column \"%s\" %s %s %s %s %s"),
						editTableData16, colName16, colType16, isNotNull ? TEXT("NOT NULL") : TEXT(""), defValue16, check16, isUnique ? TEXT("UNIQUE") : TEXT(""));

					char* err8 = 0;
					char* query8 = utils::utf16to8(query16);
					bool isOk = SQLITE_OK == sqlite3_exec(db, query8, NULL, NULL, &err8);
					if (!isOk) {
						TCHAR* err16 = utils::utf8to16(err8);
						MessageBox(hMainWnd, err16, TEXT("Error"), MB_OK);
						delete [] err16;
					}

					sqlite3_free(err8);
					delete [] query8;

					if (isOk) {
						EndDialog(hWnd, 0);
					}
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
				LOGFONT  lf = {0};
				lf.lfFaceName[0] = TEXT('\0');
				lf.lfCharSet = GetTextCharset(hDC);
				EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC) cbEnumFont, (LPARAM)hFontFamily, 0);
				ReleaseDC(hMainWnd, hDC);
				char* fontFamily8 = prefs::get("font-family", "Arial");
				TCHAR* fontFamily16 = utils::utf8to16(fontFamily8);
				ComboBox_SetCurSel(hFontFamily, ComboBox_FindString(hFontFamily, -1, fontFamily16));
				delete [] fontFamily16;
				delete [] fontFamily8;

				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_AUTOLOAD), prefs::get("autoload-extensions") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_RESTORE_DB), prefs::get("restore-db") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_RESTORE_EDITOR), prefs::get("restore-editor") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_USE_HIGHLIGHT), prefs::get("use-highlight") ? BST_CHECKED : BST_UNCHECKED);
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_USE_LEGACY), prefs::get("use-legacy-rename") ? BST_CHECKED : BST_UNCHECKED);
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

					setEditorFont(hEditorWnd);
					setTreeFont(hEditorWnd);
					sqlite3_exec(db, prefs::get("use-legacy-rename") ? "pragma legacy_alter_table = 1" : "pragma legacy_alter_table = 0", 0, 0, 0);

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
}
