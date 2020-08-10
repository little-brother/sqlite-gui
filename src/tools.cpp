#include "global.h"
#include "resource.h"
#include "tools.h"
#include "utils.h"
#include "prefs.h"

namespace tools {
	const TCHAR* DELIMITERS[5] = {TEXT(";"), TEXT(","), TEXT("\t"), TEXT("|"), TEXT("|||")};

	BOOL CALLBACK cbDlgExportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hTable = GetDlgItem(hWnd, IDC_DLG_TABLENAME);
				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select name from sqlite_master where type in ('table', 'view') order by 1", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* name16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
						ComboBox_AddString(hTable, name16);
						delete [] name16;
					}
				}
				sqlite3_finalize(stmt);
				ComboBox_SetCurSel(hTable, 0);

				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_ISCOLUMNS), BST_CHECKED);

				HWND hDelimiter = GetDlgItem(hWnd, IDC_DLG_DELIMITER);
				for (int i = 0; i < 5; i++)
					ComboBox_AddString(hDelimiter, i != 2 ? DELIMITERS[i] : TEXT("Tab"));
				ComboBox_SetCurSel(hDelimiter, prefs::get("csv-export-delimiter"));

				HWND hNewLine = GetDlgItem(hWnd, IDC_DLG_NEWLINE);
				ComboBox_AddString(hNewLine, TEXT("Windows"));
				ComboBox_AddString(hNewLine, TEXT("Unix"));
				ComboBox_SetCurSel(hNewLine, prefs::get("csv-export-is-unix-line"));

				SetFocus(hTable);
			}
			break;
			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					TCHAR path16[MAX_PATH] = {0};
					if (!utils::saveFile(path16, TEXT("CSV files\0*.csv\0All\0*.*\0")))
						return true;

					FILE* f = _tfopen(path16, TEXT("w, ccs=UTF-8"));
					if (f == NULL) {
						MessageBox(hWnd, TEXT("Error to open file"), NULL, MB_OK);
						return true;
					}

					TCHAR table16[256] = {0};
					GetDlgItemText(hWnd, IDC_DLG_TABLENAME, table16, 256);

					bool isColumns = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISCOLUMNS));
					int iDelimiter = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_DELIMITER));
					const TCHAR* delimiter16 = DELIMITERS[iDelimiter];
					bool isUnixNewLine = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_NEWLINE));

					TCHAR sql16[256] = {0};
					_stprintf(sql16, TEXT("select * from \"%s\""), table16);

					char* sql8 = utils::utf16to8(sql16);
					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0)) {
						while (isColumns || (SQLITE_ROW == sqlite3_step(stmt))) {
							TCHAR* line16 = new TCHAR[MAX_TEXT_LENGTH]{0};
							for(int i = 0; i < sqlite3_column_count(stmt); i++) {
								if (i != 0)
									_tcscat(line16, delimiter16);
								TCHAR* value16 = utils::utf8to16(isColumns ? (char *)sqlite3_column_name(stmt, i) : (char *)sqlite3_column_text(stmt, i));
								_tcscat(line16, value16);
								delete [] value16;
							}
							_tcscat(line16, isUnixNewLine ? TEXT("\n") : TEXT("\r\n"));
							_ftprintf(f, line16);
							delete [] line16;
							isColumns = false;
						}
					}
					sqlite3_finalize(stmt);
					fclose(f);
					delete [] sql8;


					prefs::set("csv-export-delimiter", iDelimiter);
					prefs::set("csv-export-is-unix-line", +isUnixNewLine);

					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgExportSQL (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_DATADDL), BST_CHECKED);

				HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_OBJECTLIST);
				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select rowid, type, name from sqlite_master where sql is not null order by case when type = 'table' then 0 when type = 'view' then 1 when type = 'trigger' then 2 else 4 end, name", -1, &stmt, 0)) {
					setListViewData(hListWnd, stmt);
					ListView_SetColumnWidth(hListWnd, 0, 0);
					ListView_SetColumnWidth(hListWnd, 2, LVSCW_AUTOSIZE_USEHEADER);
				} else {
					sqlite3_finalize(stmt);
				}
				SetFocus(hListWnd);
			}
			break;

			case WM_CLOSE: {
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_OK) {
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_OBJECTLIST);
					if (!ListView_GetSelectedCount(hListWnd)) {
						MessageBox(hWnd, TEXT("Please select an object to export"), NULL, MB_OK);
						return true;
					}

					TCHAR path16[MAX_PATH];
					if (!utils::saveFile(path16, TEXT("SQL files\0*.sql\0All\0*.*\0")))
						return true;

					FILE* f = _tfopen(path16, TEXT("w, ccs=UTF-8"));
					if (f == NULL) {
						MessageBox(hWnd, TEXT("Error to open file"), NULL, MB_OK);
						return true;
					}

					bool isDDL = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_DATADDL)) || Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_DDLONLY));
					bool isData = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_DATADDL)) || Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_DATAONLY));

					if (isDDL) {
						int count = ListView_GetSelectedCount(hListWnd);
						char* placeholders8 = new char[count * 2]{0}; // count = 3 => ?, ?, ?
						for (int i = 0; i < count * 2 - 1; i++)
							placeholders8[i] = i % 2 ? ',' : '?';
						placeholders8[count * 2 - 1] = '\0';

						char* sql8 = new char[128 + count * 2]{0};
						sprintf(sql8, "select sql from sqlite_master where rowid in (%s)", placeholders8);
						delete [] placeholders8;

						sqlite3_stmt *stmt;
						if (SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0)) {
							TCHAR buf16[64]{0};
							int pos = -1;
							for (int i = 0; i < count; i++) {
								pos = ListView_GetNextItem(hListWnd, pos, LVNI_SELECTED);
								ListView_GetItemText(hListWnd, pos, 0, buf16, 128);
								sqlite3_bind_int64(stmt, i + 1, _tcstol(buf16, NULL, 10));
							}

							while (SQLITE_ROW == sqlite3_step(stmt)) {
								TCHAR* line16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
								_ftprintf(f, TEXT("%s;\r\n\r\n"), line16);
								delete [] line16;
							}
						}
						sqlite3_finalize(stmt);
						delete [] sql8;
					}

					if (isData) {
						int pos = -1;
						while((pos = ListView_GetNextItem(hListWnd, pos, LVNI_SELECTED)) != -1) {
							TCHAR type16[64];
							TCHAR table16[64];
							ListView_GetItemText(hListWnd, pos, 1, type16, 64);
							ListView_GetItemText(hListWnd, pos, 2, table16, 64);
							if (_tcscmp(type16, TEXT("table")) && _tcscmp(type16, TEXT("view")))
								continue;

							sqlite3_stmt *stmt;
							char* table8 = utils::utf16to8(table16);
							char* sql8 = new char[256 + strlen(table8)]{0};
							sprintf(sql8, "select * from \"%s\"", table8);

							if (SQLITE_OK == sqlite3_prepare_v2(db, sql8, -1, &stmt, 0)) {
								_ftprintf(f, TEXT("-- %s\r\n"), table16);

								TCHAR header16[MAX_TEXT_LENGTH]{0};
								_stprintf(header16, TEXT("insert into \"%s\" ("), table16);

								int colCount = sqlite3_column_count(stmt);
								for (int i = 0; i < colCount; i++) {
									if (i != 0)
										_tcscat(header16, TEXT(", "));
									TCHAR* column16 = utils::utf8to16((char*)sqlite3_column_name(stmt, i));
									_tcscat(header16, column16);
									delete [] column16;
								}
								_tcscat(header16, TEXT(") values ("));

								while(SQLITE_ROW == sqlite3_step(stmt)) {
									TCHAR line16[MAX_TEXT_LENGTH]{0};
									_tcscpy(line16, header16);

									for (int i = 0; i < colCount; i++) {
										if (i != 0)
											_tcscat(line16, TEXT(", "));

										TCHAR* value16 = utils::utf8to16((char*)sqlite3_column_text(stmt, i));
										int type = sqlite3_column_type(stmt, i);

										if (type == SQLITE_BLOB)
											_tcscat(line16, TEXT("\"BLOB (unsupported)\""));
										else if (type == SQLITE_NULL)
											_tcscat(line16, TEXT("null"));
										else if (type == SQLITE_TEXT) {
											_tcscat(line16, TEXT("\""));
											TCHAR* qvalue16 = utils::maskQuotes(value16);
											_tcscat(line16, qvalue16);
											_tcscat(line16, TEXT("\""));
											delete [] qvalue16;
										} else
											_tcscat(line16, value16);
										delete [] value16;
									}
									_tcscat(line16, TEXT(");\n"));
									_ftprintf(f, line16);
								}
								_ftprintf(f, TEXT("\r\n\r\n"));
							}

							sqlite3_finalize(stmt);
							delete [] table8;
							delete [] sql8;
						}
					}
					fclose(f);

					EndDialog(hWnd, DLG_OK);
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgImportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_ISCOLUMNS), prefs::get("csv-import-is-columns") ? BST_CHECKED : BST_UNCHECKED);

				HWND hDelimiter = GetDlgItem(hWnd, IDC_DLG_DELIMITER);
				for (int i = 0; i < 5; i++)
					ComboBox_AddString(hDelimiter, i != 2 ? DELIMITERS[i] : TEXT("Tab"));
				ComboBox_SetCurSel(hDelimiter, prefs::get("csv-import-delimiter"));

				HWND hEncoding = GetDlgItem(hWnd, IDC_DLG_ENCODING);
				ComboBox_AddString(hEncoding, TEXT("UTF-8")); // CP_UTF8
				ComboBox_AddString(hEncoding, TEXT("ANSI")); // CP_ACP
				ComboBox_SetCurSel(hEncoding, prefs::get("csv-import-encoding"));

				TCHAR name16[256];
				_tsplitpath((TCHAR*)lParam, NULL, NULL, name16, NULL);
				for(int i = 0; name16[i]; i++)
					name16[i] = _totlower(name16[i]);

				_tcscat(name16, TEXT("_tmp"));
				SetDlgItemText(hWnd, IDC_DLG_TABLENAME, name16);
				SetWindowLong(hWnd, GWL_USERDATA, lParam);

				SendMessage(hWnd, WMU_SOURCE_UPDATED, 0, 0);
				SetFocus(GetDlgItem(hWnd, IDC_DLG_TABLENAME));
			}
			break;

			case WMU_SOURCE_UPDATED: {
				const TCHAR* delimiter = DELIMITERS[ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_DELIMITER))];
				int isUTF8 = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_ENCODING)) == 0;
				bool isColumns = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISCOLUMNS));
				HWND hPreviewWnd = GetDlgItem(hWnd, IDC_DLG_PREVIEW);

				TCHAR* path16 = (TCHAR*)GetWindowLong(hWnd, GWL_USERDATA);
				HANDLE hFile = CreateFile(path16, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				DWORD size = GetFileSize(hFile, NULL);
				CloseHandle(hFile);

				FILE* f = _tfopen(path16, isUTF8 ? TEXT("r, ccs=UTF-8") : TEXT("r"));
				if (f == NULL || size == INVALID_FILE_SIZE) {
					MessageBox(hWnd, TEXT("Error to open file"), NULL, MB_OK);
					return true;
				}

				ListView_DeleteAllItems(hPreviewWnd);
				HWND hHeader = ListView_GetHeader(hPreviewWnd);
				int cnt = Header_GetItemCount(hHeader);
				for (int i = 0; i < cnt; i++) {
					ListView_DeleteColumn(hPreviewWnd, cnt - 1 - i);
					Header_DeleteItem(hHeader, cnt - 1 - i);
				}

				TCHAR* line = new TCHAR[size + 1]{0};
				int lineNo = 0;
				while(!feof (f) && lineNo < 5) {
					if (_fgetts(line, size, f)) {
						int colNo = 0;
						TCHAR* column = _tcstok (line, delimiter);
						while (column != NULL) {
							if (lineNo == 0) {
								LVCOLUMN lvc;
								lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
								lvc.iSubItem = colNo;
								if (isColumns) {
									lvc.pszText = (TCHAR*)column;
									lvc.cchTextMax = _tcslen(column) + 1;
								} else {
									TCHAR name[64];
									_stprintf(name, TEXT("Column%i"), colNo);
									lvc.pszText = name;
									lvc.cchTextMax = 64;
								}
								lvc.cx = 50;
								ListView_InsertColumn(hPreviewWnd, colNo, &lvc);
							}

							if ((isColumns && lineNo > 0) || !isColumns) {
								LVITEM  lvi = {0};
								lvi.mask = LVIF_TEXT;
								lvi.iSubItem = colNo;
								lvi.iItem = isColumns ? lineNo - 1 : lineNo;
								lvi.pszText = column;
								lvi.cchTextMax = _tcslen(column) + 1;
								if (colNo == 0)
									ListView_InsertItem(hPreviewWnd, &lvi);
								else
									ListView_SetItem(hPreviewWnd, &lvi);
							}
							colNo++;
							column = _tcstok (NULL, delimiter);
						}
					}
					lineNo++;
				}
				delete [] line;
				fclose(f);
				ListView_SetExtendedListViewStyle(hPreviewWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | 0x10000000 /*LVS_EX_AUTOSIZECOLUMNS*/);
			}
			break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;

			case WM_COMMAND: {
				WORD id = LOWORD(wParam);
				WORD cmd = HIWORD(wParam);
				if ((cmd == CBN_SELCHANGE && id == IDC_DLG_ENCODING) ||
					(cmd == CBN_SELCHANGE && id == IDC_DLG_DELIMITER) ||
					(cmd == BN_CLICKED && id == IDC_DLG_ISCOLUMNS))
					SendMessage(hWnd, WMU_SOURCE_UPDATED, 0, 0);

				if (wParam == IDC_DLG_OK) {
					int iDelimiter = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_DELIMITER));
					const TCHAR* delimiter = DELIMITERS[iDelimiter];
					int iEncoding = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_ENCODING));
					int isUTF8 = iEncoding == 0;
					bool isColumns = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_ISCOLUMNS));
					HWND hPreviewWnd = GetDlgItem(hWnd, IDC_DLG_PREVIEW);
					HWND hHeader = ListView_GetHeader(hPreviewWnd);
					int colCount = Header_GetItemCount(hHeader);

					TCHAR buf16[256]{0};
					TCHAR create16[MAX_TEXT_LENGTH]{0};
					TCHAR insert16[MAX_TEXT_LENGTH]{0};
					GetDlgItemText(hWnd, IDC_DLG_TABLENAME, buf16, 255);
					_stprintf(create16, TEXT("create table \"%s\" ("), buf16);
					_stprintf(insert16, TEXT("insert into \"%s\" ("), buf16);

					auto catQuotted = [](TCHAR* a, TCHAR* b) {
						_tcscat(a, TEXT("\""));
						TCHAR* tb = utils::trim(b);
						TCHAR* qb = utils::maskQuotes(tb);
						_tcscat(a, qb);
						_tcscat(a, TEXT("\""));
						delete [] qb;
						delete [] tb;
					};

					HD_ITEM hdi = {0};
					hdi.mask = HDI_TEXT;
					hdi.pszText = buf16;
					hdi.cchTextMax = 255;
					for (int i = 0; i < colCount; i++) {
						if (i != 0) {
							_tcscat(create16, TEXT(", "));
							_tcscat(insert16, TEXT(", "));
						}

						Header_GetItem(hHeader, i, &hdi);
						catQuotted(create16, buf16);
						catQuotted(insert16, buf16);
					}
					_tcscat(create16, TEXT(");"));
					_tcscat(insert16, TEXT(") values ("));
					for (int i = 0; i < colCount; i++)
						_tcscat(insert16, i != colCount - 1 ? TEXT("?, ") : TEXT("?);"));

					TCHAR* path16 = (TCHAR*)GetWindowLong(hWnd, GWL_USERDATA);
					HANDLE hFile = CreateFile(path16, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
					DWORD size = GetFileSize(hFile, NULL);
					CloseHandle(hFile);

					FILE* f = _tfopen(path16, isUTF8 ? TEXT("r, ccs=UTF-8") : TEXT("r"));
					if (f == NULL || size == INVALID_FILE_SIZE) {
						MessageBox(hWnd, TEXT("Error to open file"), NULL, MB_OK);
						return true;
					}

					bool isAutoTransaction = sqlite3_get_autocommit(db) > 0;
					if (isAutoTransaction)
						sqlite3_exec(db, "begin", NULL, 0, NULL);

					char* create8 = utils::utf16to8(create16);
					char* insert8 = utils::utf16to8(insert16);
					sqlite3_stmt *stmt;
					bool rc = (SQLITE_OK == sqlite3_exec(db, create8, NULL, 0, NULL)) && (SQLITE_OK == sqlite3_prepare_v2(db, insert8, -1, &stmt, 0));

					TCHAR* line16 = new TCHAR[size + 1]{0};
					int lineNo = 0;
					while(!feof (f) && rc) {
						if (_fgetts(line16, size, f)) {
							if (lineNo == 0 && isColumns) {
								lineNo++;
								continue;
							}

							int colNo = 0;
							TCHAR* pos = line16;
							TCHAR* value16 = _tcstok (pos, delimiter);
							while (value16 != NULL && colNo < colCount) {

								TCHAR* tvalue16 = utils::trim(value16);
								char* value8 = utils::utf16to8(tvalue16);
								utils::sqlite3_bind_variant(stmt, colNo + 1, value8);
								delete [] value8;
								delete [] tvalue16;
								value16 = _tcstok (NULL, delimiter);
								colNo++;
							}
							for (int i = colNo; i < colCount; i++)
								sqlite3_bind_null(stmt, i + 1);
							rc = sqlite3_step(stmt) == SQLITE_DONE;
							sqlite3_reset(stmt);
							lineNo++;
						}
					}

					delete [] line16;
					delete [] create8;
					delete [] insert8;
					sqlite3_finalize(stmt);
					fclose(f);

					if (!rc) {
						char *err8 = (char*)sqlite3_errmsg(db);
						TCHAR* err16 = utils::utf8to16(err8);
						MessageBox(hWnd, err16, NULL, MB_OK);
						delete [] err16;
					}

					if (isAutoTransaction)
						sqlite3_exec(db, rc ? "commit" : "rollback", NULL, 0, NULL);

					if (rc) {
						prefs::set("csv-import-encoding", iEncoding);
						prefs::set("csv-import-delimiter", iDelimiter);
						prefs::set("csv-import-is-columns", +isColumns);
						EndDialog(hWnd, DLG_OK);
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}


	const TCHAR* GENERATOR_TYPE[1024] = {0};
	const TCHAR* SOURCES[1024] = {0};

	bool execute(const char* query8) {
		int rc = sqlite3_exec(db, query8, NULL, 0, NULL);
		bool res = rc == SQLITE_OK || rc == SQLITE_DONE;
		if (!res)
			printf("\nQuery: %s\nError: %s\n", query8, sqlite3_errmsg(db));

		return res;
	}

	int getDlgItemTextAsNumber(HWND hWnd, int id) {
		TCHAR buf16[32]{0};
		GetDlgItemText(hWnd, id, buf16, 31);
		return _ttoi(buf16);
	}

	WNDPROC cbOldCombobox;
	LRESULT CALLBACK cbNewType(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_COMMAND && HIWORD(wParam) == CBN_SELCHANGE)
			SendMessage(GetAncestor(hWnd, GA_ROOT), WMU_TYPE_CHANGED, (WPARAM)GetParent(hWnd), (LPARAM)hWnd);

		return CallWindowProc(cbOldCombobox, hWnd, msg, wParam, lParam);
	}
	LRESULT CALLBACK cbNewRefTable(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_COMMAND && HIWORD(wParam) == CBN_SELCHANGE)
			SendMessage(GetAncestor(hWnd, GA_ROOT), WMU_REFTABLE_CHANGED, (WPARAM)GetParent(hWnd), (LPARAM)hWnd);

		return CallWindowProc(cbOldCombobox, hWnd, msg, wParam, lParam);
	}

	BOOL CALLBACK cbDlgDataGenerator (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hTable = GetDlgItem(hWnd, IDC_DLG_TABLENAME);
				cbOldCombobox = (WNDPROC)GetWindowLong(hTable, GWL_WNDPROC);

				int rowCount = prefs::get("data-generator-row-count");
				TCHAR rowCount16[32]{0};
				_itot(rowCount, rowCount16, 10);
				SetDlgItemText(hWnd, IDC_DLG_GEN_ROW_COUNT, rowCount16);

				if (prefs::get("data-generator-truncate"))
					Button_SetCheck(GetDlgItem(hWnd, IDC_DLG_GEN_ISTRUNCATE), BST_CHECKED);

				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select name from sqlite_master where type = 'table' and name <> 'sqlite_sequence' order by 1", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* name16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
						ComboBox_AddString(hTable, name16);
						delete [] name16;
					}
				}
				sqlite3_finalize(stmt);
				ComboBox_SetCurSel(hTable, 0);

				if (!GENERATOR_TYPE[0]) {
					int i = 0;
					sqlite3_stmt *stmt;
					while (GENERATOR_TYPE[i] && i < 1024) {
						delete [] GENERATOR_TYPE[i];
						GENERATOR_TYPE[i] = 0;
						i++;
					}

					execute("attach database \"prefs.sqlite\" as prefs1234567890");
					execute("drop table if exists temp.generators");
					execute("create table temp.generators as select * from prefs1234567890.generators");
					execute("detach database prefs1234567890");

					if (SQLITE_OK == sqlite3_prepare_v2(db, "select distinct type from temp.generators order by 1", -1, &stmt, 0)) {
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							GENERATOR_TYPE[i] = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
							i++;
						}
					}
					sqlite3_finalize(stmt);

					i = 0;
					while (SOURCES[i] && i < 1024) {
						delete [] SOURCES[i];
						SOURCES[i] = 0;
						i++;
					}

					if (SQLITE_OK == sqlite3_prepare_v2(db, "select name from sqlite_master where type in ('table', 'view') and name <> 'sqlite_sequence' order by 1", -1, &stmt, 0)) {
						int i = 0;
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							SOURCES[i] = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
							i++;
						}
					}
					sqlite3_finalize(stmt);
				}
				SendMessage(hWnd, WMU_TARGET_CHANGED, 0, 0);
				SetFocus(hTable);
			}
			break;

			case WM_CLOSE: {
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WMU_TARGET_CHANGED: {
				HWND hColumnsWnd = GetDlgItem(hWnd, IDC_DLG_GEN_COLUMNS);
				EnumChildWindows(hColumnsWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_DESTROY);

				TCHAR buf16[255]{0};
				TCHAR query16[MAX_TEXT_LENGTH]{0};
				GetDlgItemText(hWnd, IDC_DLG_TABLENAME, buf16, 255);

				_stprintf(query16, TEXT("select name from pragma_table_info(\"%s\") order by cid"), buf16);
				char* query8 = utils::utf16to8(query16);

				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
					int rowNo = 0;
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						HWND hColumnWnd = CreateWindow(WC_STATIC, NULL, WS_VISIBLE | WS_CHILD, 5, 5 + 30 * rowNo, 470, 23, hColumnsWnd, (HMENU)IDC_DLG_GEN_COLUMN, GetModuleHandle(0), 0);

						TCHAR* colname16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
						CreateWindow(WC_STATIC, colname16, WS_VISIBLE | WS_CHILD, 0, 3, 70, 23, hColumnWnd, (HMENU)IDC_DLG_GEN_COLUMN_NAME, GetModuleHandle(0), 0);
						delete [] colname16;

						HWND hTypeWnd = CreateWindow(WC_COMBOBOX, NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 70, 0, 100, 200, hColumnWnd, (HMENU)IDC_DLG_GEN_COLUMN_TYPE, GetModuleHandle(0), 0);
						ComboBox_AddString(hTypeWnd, TEXT("none"));
						ComboBox_AddString(hTypeWnd, TEXT("sequence"));
						ComboBox_AddString(hTypeWnd, TEXT("number"));
						ComboBox_AddString(hTypeWnd, TEXT("date"));
						ComboBox_AddString(hTypeWnd, TEXT("reference to"));

						if (GENERATOR_TYPE[0])
							ComboBox_AddString(hTypeWnd, TEXT(""));

						for (int i = 0; GENERATOR_TYPE[i]; i++)
							ComboBox_AddString(hTypeWnd, GENERATOR_TYPE[i]);
						ComboBox_SetCurSel(hTypeWnd, 0);
						SetWindowLong(hTypeWnd, GWL_WNDPROC, (LONG)cbNewType);

						CreateWindow(WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP, 180, 0, 210, 23, hColumnWnd, (HMENU)IDC_DLG_GEN_OPTION, GetModuleHandle(0), 0);

						rowNo++;
					}
				}
				sqlite3_finalize(stmt);
				delete [] query8;

				EnumChildWindows(hColumnsWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETDEFFONT);
			}
			break;

			case WMU_TYPE_CHANGED: {
				HWND hColumnWnd = (HWND)wParam;
				HWND hTypeWnd = (HWND)lParam;
				HWND hOptionWnd = GetDlgItem(hColumnWnd, IDC_DLG_GEN_OPTION);
				EnumChildWindows(hOptionWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_DESTROY);

				TCHAR buf16[64];
				GetWindowText(hTypeWnd, buf16, 63);

				if (_tcscmp(buf16, TEXT("sequence")) == 0) {
					CreateWindow(WC_STATIC, TEXT("Start"), WS_VISIBLE | WS_CHILD, 0, 3, 35, 23, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_LABEL, GetModuleHandle(0), 0);
					CreateWindow(WC_EDIT, TEXT("1"), WS_VISIBLE | WS_CHILD | ES_NUMBER | WS_BORDER | ES_CENTER | WS_TABSTOP, 40, 1, 40, 18, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_START, GetModuleHandle(0), 0);
				}

				if (_tcscmp(buf16, TEXT("number")) == 0) {
					CreateWindow(WC_EDIT, TEXT("1"), WS_VISIBLE | WS_CHILD | ES_NUMBER | WS_BORDER | ES_CENTER | WS_TABSTOP, 0, 1, 40, 18, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_START, GetModuleHandle(0), 0);
					CreateWindow(WC_STATIC, TEXT("-"), WS_VISIBLE | WS_CHILD, 45, 3, 10, 23, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_LABEL, GetModuleHandle(0), 0);
					CreateWindow(WC_EDIT, TEXT("100"), WS_VISIBLE | WS_CHILD | ES_NUMBER | WS_BORDER | ES_CENTER | WS_TABSTOP, 53, 1, 40, 18, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_END, GetModuleHandle(0), 0);
					CreateWindow(WC_STATIC, TEXT("x"), WS_VISIBLE | WS_CHILD, 100, 3, 10, 23, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_LABEL, GetModuleHandle(0), 0);
					CreateWindow(WC_EDIT, TEXT("100"), WS_VISIBLE | WS_CHILD | ES_NUMBER | WS_BORDER | ES_CENTER | WS_TABSTOP, 110, 1, 40, 18, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_MULTIPLIER, GetModuleHandle(0), 0);
				}

				if (_tcscmp(buf16, TEXT("date")) == 0) {
					CreateWindowEx(0, DATETIMEPICK_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 80, 23, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_START, GetModuleHandle(0), 0);
					CreateWindow(WC_STATIC, TEXT("-"), WS_VISIBLE | WS_CHILD, 85, 3, 10, 23, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_LABEL, GetModuleHandle(0), 0);
					CreateWindowEx(0, DATETIMEPICK_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP, 95, 0, 80, 23, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_END, GetModuleHandle(0), 0);
				}

				if (_tcscmp(buf16, TEXT("reference to")) == 0) {
					HWND hRefTableWnd = CreateWindow(WC_COMBOBOX, NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 0, 0, 90, 200, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_TABLE, GetModuleHandle(0), 0);
					for (int i = 0; SOURCES[i]; i++)
						ComboBox_AddString(hRefTableWnd, SOURCES[i]);
					SetWindowLong(hRefTableWnd, GWL_WNDPROC, (LONG)cbNewRefTable);

					CreateWindow(WC_COMBOBOX, NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 90, 0, 86, 200, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_COLUMN, GetModuleHandle(0), 0);
					ComboBox_SetCurSel(hRefTableWnd, 0);
					SendMessage(hWnd, WMU_REFTABLE_CHANGED, (WPARAM)hOptionWnd, (LPARAM)hRefTableWnd);
				}

				EnumChildWindows(hOptionWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETDEFFONT);
			}
			break;

			case WMU_REFTABLE_CHANGED: {
				HWND hOptionWnd = (HWND)wParam;
				HWND hRefTableWnd = (HWND)lParam;
				HWND hRefColumnWnd = GetDlgItem(hOptionWnd, IDC_DLG_GEN_OPTION_COLUMN);
				ComboBox_ResetContent(hRefColumnWnd);

				TCHAR buf16[255]{0};
				TCHAR query16[MAX_TEXT_LENGTH]{0};
				GetWindowText(hRefTableWnd, buf16, 255);

				_stprintf(query16, TEXT("select name from pragma_table_info(\"%s\") order by cid"), buf16);
				char* query8 = utils::utf16to8(query16);

				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* colname16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
						ComboBox_AddString(hRefColumnWnd, colname16);
						delete [] colname16;
					}
				}
				sqlite3_finalize(stmt);
				delete [] query8;

				ComboBox_SetCurSel(hRefColumnWnd, 0);
			}
			break;

			case WM_COMMAND: {
				WORD id = LOWORD(wParam);
				WORD cmd = HIWORD(wParam);
				if (cmd == CBN_SELCHANGE && id == IDC_DLG_TABLENAME)
					SendMessage(hWnd, WMU_TARGET_CHANGED, 0, 0);

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL) {
					EndDialog(hWnd, DLG_CANCEL);
				}

				if (wParam == IDC_DLG_OK || wParam == IDOK)	{
					execute("drop table if exists temp.data_generator");

					TCHAR table16[128]{0};
					GetDlgItemText(hWnd, IDC_DLG_TABLENAME, table16, 127);

					char* table8 = utils::utf16to8(table16);
					char query8[MAX_TEXT_LENGTH]{0};
					sprintf(query8, "create table temp.data_generator as select null rownum, t.* from \"%s\" t where 1 = 2", table8);
					execute(query8);

					int rowCount = getDlgItemTextAsNumber(hWnd, IDC_DLG_GEN_ROW_COUNT);

					sprintf(query8, "insert into temp.data_generator (rownum) select value from generate_series(1, %i, 1)", rowCount);
					execute(query8);

					HWND hColumnWnd = GetWindow(GetDlgItem(hWnd, IDC_DLG_GEN_COLUMNS), GW_CHILD);
					char columns8[MAX_TEXT_LENGTH]{0};

					while(IsWindow(hColumnWnd)){
						TCHAR name16[128]{0};
						GetDlgItemText(hColumnWnd, IDC_DLG_GEN_COLUMN_NAME, name16, 127);
						char* name8 = utils::utf16to8(name16);
						if (strlen(columns8) > 0)
							strcat(columns8, ", ");
						strcat(columns8, name8);
						delete [] name8;

						HWND hTypeWnd = GetDlgItem(hColumnWnd, IDC_DLG_GEN_COLUMN_TYPE);
						HWND hOptionWnd = GetDlgItem(hColumnWnd, IDC_DLG_GEN_OPTION);
						TCHAR type16[128]{0};
						GetWindowText(hTypeWnd, type16, 127);
						TCHAR query16[MAX_TEXT_LENGTH]{0};

						if (_tcscmp(type16, TEXT("sequence")) == 0) {
							int start = getDlgItemTextAsNumber(hOptionWnd, IDC_DLG_GEN_OPTION_START);
							_stprintf(query16, TEXT("update temp.data_generator set \"%s\" = rownum + %i"), name16, start);
						}

						if (_tcscmp(type16, TEXT("number")) == 0) {
							int start = getDlgItemTextAsNumber(hOptionWnd, IDC_DLG_GEN_OPTION_START);
							int end = getDlgItemTextAsNumber(hOptionWnd, IDC_DLG_GEN_OPTION_END);
							int multi = getDlgItemTextAsNumber(hOptionWnd, IDC_DLG_GEN_OPTION_MULTIPLIER);

							_stprintf(query16, TEXT("update temp.data_generator set \"%s\" = cast((%i + (%i - %i + 1) * (random()  / 18446744073709551616 + 0.5)) as integer) * %i"), name16, start, end, start, multi);
						}

						if (_tcscmp(type16, TEXT("reference to")) == 0) {
							TCHAR reftable16[256]{0};
							GetDlgItemText(hOptionWnd, IDC_DLG_GEN_OPTION_TABLE, reftable16, 255);

							TCHAR refcolumn16[256]{0};
							GetDlgItemText(hOptionWnd, IDC_DLG_GEN_OPTION_COLUMN, refcolumn16, 255);

							_stprintf(query16, TEXT("with t as (select %s value from \"%s\" order by random()), "\
								"t2 as (select rownum(1) rownum, t.value FROM t, generate_series(1, (select ceil(%i.0/count(1))  from t), 1) limit %i) "\
								"update temp.data_generator set \"%s\" = (select value from t2 where t2.rownum = temp.data_generator.rownum)"),
								refcolumn16, reftable16, rowCount, rowCount, name16);
						}

						if (_tcscmp(type16, TEXT("date")) == 0) {
							SYSTEMTIME start = {0}, end = {0};
							DateTime_GetSystemtime(GetDlgItem(hOptionWnd, IDC_DLG_GEN_OPTION_START), &start);
							DateTime_GetSystemtime(GetDlgItem(hOptionWnd, IDC_DLG_GEN_OPTION_END), &end);

							TCHAR start16[32] = {0};
							_stprintf(start16, TEXT("%i-%0*i-%0*i"), start.wYear, 2, start.wMonth, 2, start.wDay);

							TCHAR end16[32] = {0};
							_stprintf(end16, TEXT("%i-%0*i-%0*i"), end.wYear, 2, end.wMonth, 2, end.wDay);

							_stprintf(query16, TEXT("update temp.data_generator set \"%s\" = date('%s', '+' || ((strftime('%%s', '%s', '+1 day', '-1 second') - strftime('%%s', '%s')) * (random()  / 18446744073709551616 + 0.5)) || ' second')"),
								name16, start16, end16, start16);
						}

						if (ComboBox_GetCurSel(hTypeWnd) > 4) {
							_stprintf(query16, TEXT("with t as (select type, value from temp.generators where type = \"%s\" order by random()), "\
								"t2 as (select rownum(1) rownum, t.value FROM t, generate_series(1, (select ceil(%i.0/count(1))  from t), 1) limit %i) "\
								"update temp.data_generator set \"%s\" = (select value from t2 where t2.rownum = temp.data_generator.rownum)"),
								type16, rowCount, rowCount, name16);
						}

						char* query8 = utils::utf16to8(query16);
						execute(query8);
						delete [] query8;

						hColumnWnd = GetWindow(hColumnWnd, GW_HWNDNEXT);
					}

					bool isTruncate = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_GEN_ISTRUNCATE));
					prefs::set("data-generator-row-count", rowCount);
					prefs::set("data-generator-truncate", +isTruncate);

					if (isTruncate) {
						sprintf(query8, "delete from \"%s\"", table8);
						execute(query8);
					}

					sprintf(query8, "insert into \"%s\" (%s) select %s from temp.data_generator", table8, columns8, columns8);
					if (execute(query8))
						MessageBox(hWnd, TEXT("Done!"), TEXT("Info"), MB_OK);

					delete [] table8;
				}
			}
			break;
		}

		return false;
	}

	char* readFile(const char* path) {
		FILE *fp = fopen (path , "rb");
		if(!fp)
			return 0;

		fseek(fp, 0L, SEEK_END);
		long size = ftell(fp);
		rewind(fp);

		char* buf = new char[size + 1]{0};
		int rc = fread(buf, size, 1 , fp);
		fclose(fp);

		if (!rc) {
			delete [] buf;
			return 0;
		}

		buf[size] = '\0';

		return buf;
	}

	void importSqlFile(TCHAR *path16){
		char* path8 = utils::utf16to8(path16);
		char* data8 = readFile(path8);
		if (data8 != 0) {
			char* err8 = 0;
			sqlite3_exec(db, data8, NULL, 0, &err8);
			if (err8) {
				TCHAR* err16 = utils::utf8to16(err8);
				MessageBox(hMainWnd, err16, TEXT("Error"), MB_OK);
				delete [] err16;
			} else {
				TCHAR msg16[255];
				_stprintf(msg16, TEXT("Done!"), sqlite3_changes(db));
				MessageBox(hMainWnd, msg16, TEXT("Info"), MB_OK);
			}
			delete [] data8;
		}
		delete [] path8;
	}
}
