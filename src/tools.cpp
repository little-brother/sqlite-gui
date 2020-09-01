#include "global.h"
#include "missing.h"
#include "resource.h"
#include "tools.h"
#include "utils.h"
#include "dialogs.h"
#include "prefs.h"

namespace tools {
	const TCHAR* DELIMITERS[4] = {TEXT(";"), TEXT(","), TEXT("\t"), TEXT("|")};

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
				for (int i = 0; i < 4; i++)
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
								TCHAR* qvalue16 = utils::replaceAll(value16, TEXT("\""), TEXT("\"\""));
								if (_tcschr(qvalue16, TEXT(','))) {
									TCHAR val16[_tcslen(qvalue16) + 3]{0};
									_stprintf(val16, TEXT("\"%s\""), qvalue16);
									_tcscat(line16, val16);
								} else {
									_tcscat(line16, qvalue16);
								}
								delete [] value16;
								delete [] qvalue16;
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
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select type, name from sqlite_master where sql is not null order by case when type = 'table' then 0 when type = 'view' then 1 when type = 'trigger' then 2 else 4 end, name", -1, &stmt, 0)) {
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
								_ftprintf(f, TEXT("%s;\n\n"), line16);
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
								_ftprintf(f, TEXT("-- %s\n"), table16);

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
				for (int i = 0; i < 4; i++)
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

				auto addCell = [hPreviewWnd, isColumns] (int lineNo, int colNo, TCHAR* column) {
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
				};

				TCHAR line[size + 1]{0};
				int lineNo = 0;

				while(!feof (f) && lineNo < 5) {
					if (_fgetts(line, size, f)) {
						int colNo = 0;

						TCHAR value[size + 1];
						bool inQuotes = false;
						int valuePos = 0;
						int i = 0;
						do {
							value[valuePos++] = line[i];

							if (!inQuotes && (line[i] == delimiter[0] || line[i] == TEXT('\n'))) {
								value[valuePos - 1] = 0;
								valuePos = 0;

								addCell(lineNo, colNo, value);
								colNo++;
							}

							if (line[i] == TEXT('"') && line[i + 1] != TEXT('"')) {
								valuePos--;
								inQuotes = !inQuotes;
							}

							if (line[i] == TEXT('"') && line[i + 1] == TEXT('"'))
								i++;

						} while (line[++i]);
					}
					lineNo++;
				}

				fclose(f);
				ListView_SetExtendedListViewStyle(hPreviewWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_AUTOSIZECOLUMNS);
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

							TCHAR value[size + 1];
							bool inQuotes = false;
							int valuePos = 0;
							int i = 0;
							do {
								value[valuePos++] = line16[i];

								if (!inQuotes && (line16[i] == delimiter[0] || line16[i] == TEXT('\n'))) {
									value[valuePos - 1] = 0;
									valuePos = 0;

									TCHAR* tvalue16 = utils::trim(value);
									char* value8 = utils::utf16to8(tvalue16);
									utils::sqlite3_bind_variant(stmt, colNo + 1, value8);
									delete [] value8;
									delete [] tvalue16;

									colNo++;
								}

								if (line16[i] == TEXT('"') && line16[i + 1] != TEXT('"')) {
									valuePos--;
									inQuotes = !inQuotes;
								}

								if (line16[i] == TEXT('"') && line16[i + 1] == TEXT('"'))
									i++;

							} while (line16[++i]);

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

					bool isTruncate = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_GEN_ISTRUNCATE));
					if (isTruncate && MessageBox(hWnd, TEXT("All data from table will be erased. Continue?"), TEXT("Confirmation"), MB_OKCANCEL | MB_ICONASTERISK) != IDOK)
						return true;

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
								"t2 as (select t.value FROM t, generate_series(1, (select ceil(%i.0/count(1)) from t), 1) order by random()), "\
								"t3 as (select rownum(1) rownum, t2.value from t2 order by 1 limit %i)"
								"update temp.data_generator set \"%s\" = (select value from t3 where t3.rownum = temp.data_generator.rownum)"),
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
								"t2 as (select t.value FROM t, generate_series(1, (select ceil(%i.0/count(1)) from t), 1) order by random()), "\
								"t3 as (select rownum(1) rownum, t2.value from t2 order by 1 limit %i)"
								"update temp.data_generator set \"%s\" = (select value from t3 where t3.rownum = temp.data_generator.rownum)"),
								type16, rowCount, rowCount, name16);
						}

						char* query8 = utils::utf16to8(query16);
						execute(query8);
						delete [] query8;

						hColumnWnd = GetWindow(hColumnWnd, GW_HWNDNEXT);
					}


					prefs::set("data-generator-row-count", rowCount);
					prefs::set("data-generator-truncate", +isTruncate);

					if (isTruncate) {
						sprintf(query8, "delete from \"%s\"", table8);
						execute(query8);
					}

					sprintf(query8, "insert into \"%s\" (%s) select %s from temp.data_generator", table8, columns8, columns8);
					if (execute(query8))
						MessageBox(hWnd, TEXT("Done!"), TEXT("Info"), MB_OK);
					else
						showDbError(hWnd);

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

	bool importSqlFile(TCHAR *path16){
		char* path8 = utils::utf16to8(path16);
		char* data8 = readFile(path8);
		bool rc = true;
		if (data8 != 0) {
			sqlite3_exec(db, "pragma synchronous = 0", NULL, 0, NULL);
			rc = SQLITE_OK == sqlite3_exec(db, data8, NULL, 0, NULL);
			if (!rc)
				showDbError(hMainWnd);

			delete [] data8;
			sqlite3_exec(db, "pragma synchronous = 1", NULL, 0, NULL);
		}
		delete [] path8;

		return rc;
	}

	#define LINK_FK 1
	#define LINK_VIEW 2
	#define LINK_TRIGGER 3
	#define MAX_LINK_COUNT 255
	struct Link {
		HWND hWndFrom;
		int posFrom;
		HWND hWndTo;
		int posTo;
		int type;
	};

	char* dbname8 = 0;
	Link links[MAX_LINK_COUNT]{0};
	HIMAGELIST tbImages = ImageList_LoadBitmap(GetModuleHandle (0), MAKEINTRESOURCE(IDB_DLG_TOOLBAR), 32, 0, RGB(255,255,255));

	WNDPROC cbOldTable;
	LRESULT CALLBACK cbNewTable(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_WINDOWPOSCHANGED) {
			RECT rcSize{0};
			GetWindowRect(hWnd, &rcSize);
			RECT rcPos{0};
			GetWindowRect(hWnd, &rcPos);
			POINT pos{rcPos.left, rcPos.top};
			ScreenToClient(GetParent(hWnd), &pos);
			TCHAR table16[255]{0};
			GetWindowText(hWnd, table16, 255);
			char* table8 = utils::utf16to8(table16);
			RECT rc = {pos.x, pos.y, rcSize.right - rcSize.left, rcSize.bottom - rcSize.top};
			prefs::setDiagramRect(dbname8, table8, rc);
			delete [] table8;
		}

		if (msg == WM_LBUTTONDOWN) {
			HWND hParentWnd = GetParent(hWnd);
			SetWindowLong(hParentWnd, GWL_USERDATA, (LONG)hWnd);
			InvalidateRect(hParentWnd, NULL, true);
		}

		if (msg == WM_LBUTTONDBLCLK) {
			TCHAR table16[255]{0};
			GetWindowText(hWnd, table16, 255);
			_tcscpy(editTableData16, table16);
			DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_EDITDATA), hMainWnd, (DLGPROC)&dialogs::cbDlgEditData);
		}

		if (msg == WM_WINDOWPOSCHANGED || msg == WM_LBUTTONDBLCLK)
			SetFocus(GetParent(hWnd));

		return CallWindowProc(cbOldTable, hWnd, msg, wParam, lParam);
	}

	BOOL CALLBACK cbDlgDatabaseDiagram (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				TCHAR* dbpath16 = utils::utf8to16(sqlite3_db_filename(db, 0));
				TCHAR dbname16[255];
				_tsplitpath(dbpath16, NULL, NULL, dbname16, NULL);
				dbname8 = utils::utf16to8(dbpath16);
				delete [] dbpath16;

				TBBUTTON tbButtons [] = {
					{0, IDM_LINK_FK, (byte)(prefs::get("link-fk") ? TBSTATE_CHECKED | TBSTATE_ENABLED : TBSTATE_ENABLED), TBSTYLE_CHECK, {0}, 0L, (INT_PTR)TEXT("Foreign keys")},
					{1, IDM_LINK_VIEW, (byte)(prefs::get("link-view") ? TBSTATE_CHECKED | TBSTATE_ENABLED : TBSTATE_ENABLED), TBSTYLE_CHECK, {0}, 0L, (INT_PTR)TEXT("References in views")},
					{2, IDM_LINK_TRIGGER, (byte)(prefs::get("link-trigger") ? TBSTATE_CHECKED | TBSTATE_ENABLED : TBSTATE_ENABLED), TBSTYLE_CHECK, {0}, 0L, (INT_PTR)TEXT("References in triggers")}
				};

				HWND hToolWnd = CreateToolbarEx (hWnd, WS_CHILD |  WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS |TBSTYLE_FLAT | TBSTYLE_LIST, IDC_DLG_TOOLBAR, 0, NULL, 0,
					tbButtons, sizeof(tbButtons)/sizeof(tbButtons[0]), 0, 0, 0, 0, sizeof (TBBUTTON));

				SendMessage(hToolWnd, TB_SETIMAGELIST, 0, (LPARAM)tbImages);
				SendMessage(hToolWnd, TB_SETBITMAPSIZE, 0, MAKELPARAM(32, 16));

				HWND hTableWnd = 0;
				int tblNo = 0;
				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select t.name tblname, c.name colname, c.cid, iif(length(c.type),c.type, 'any'), c.pk " \
					"from sqlite_master t, pragma_table_info(t.tbl_name) c " \
					"where t.sql is not null and t.name not like 'sqlite_%' and t.type in ('view', 'table')" \
					"order by 1, 3", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* tblname16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
						TCHAR* colname16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 1));
						int no = sqlite3_column_int(stmt, 2);
						TCHAR* type16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 3));
						int isPk = sqlite3_column_int(stmt, 4);

						if (!no) {
							RECT rect = {10 + (tblNo % 5) * 150, 40 + 150 * (tblNo / 5), 100, 100};
							prefs::getDiagramRect(dbname8, (const char*)sqlite3_column_text(stmt, 0), &rect);
							hTableWnd = CreateWindow(WC_LISTBOX, tblname16,
								WS_CAPTION | WS_VISIBLE| WS_CHILD | WS_OVERLAPPED | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LBS_NOSEL,
								rect.left, rect.top, rect.right, rect.bottom + 15, hWnd, (HMENU)(IDC_DATABASE_DIAGRAM_TABLE + tblNo), GetModuleHandle(0), NULL);

							cbOldTable = (WNDPROC)SetWindowLong(hTableWnd, GWL_WNDPROC, (LONG)cbNewTable);
							ShowWindow(hTableWnd, SW_SHOW);
							tblNo++;
						}

						TCHAR buf[1024]{0};
						_stprintf(buf, TEXT("%s: %s %s"), colname16, type16, isPk? TEXT(" [PK]") : TEXT(""));
						ListBox_AddString(hTableWnd, buf);

						delete [] tblname16;
						delete [] colname16;
					}
				}
				sqlite3_finalize(stmt);

				int linkNo = 0;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select t.name tblfrom, c.'from' colfrom, c.'table' tblto, c.'to' colto " \
					"from sqlite_master t, pragma_foreign_key_list(t.tbl_name) c " \
					"where t.sql is not null and t.type in ('view', 'table')" \
					"order by 1, 2, 3", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* tblfrom16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
						TCHAR* colfrom16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 1));
						TCHAR* tblto16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 2));
						TCHAR* colto16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 3));

						HWND hWndFrom = FindWindowEx(hWnd, 0, NULL, tblfrom16);
						HWND hWndTo = FindWindowEx(hWnd, 0, NULL, tblto16);
						int posFrom = ListBox_FindString(hWndFrom, 0, colfrom16);
						int posTo = ListBox_FindString(hWndTo, 0, colto16);

						if (hWndFrom && hWndTo && posFrom != -1 && posTo != -1) {
							links[linkNo] = {hWndFrom, posFrom, hWndTo, posTo, LINK_FK};
							linkNo++;
						}

						delete [] tblfrom16;
						delete [] colfrom16;
						delete [] tblto16;
						delete [] colto16;
					}
				}
				sqlite3_finalize(stmt);

				if (SQLITE_OK == sqlite3_prepare_v2(db, "select sm.tbl_name, +(sm.type = 'view'), ref.name " \
					"from sqlite_master sm inner join sqlite_master ref " \
					"on sm.type in ('view', 'trigger') and ref.type in ('table', 'view') " \
					"and lower(sm.sql) regexp '(from |join |into )([ \"''])*' || lower(ref.name) || '(\\D|\\b|''])+'", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* tblname16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
						int isView = sqlite3_column_int(stmt, 1);
						TCHAR* refname16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 2));

						HWND hWndFrom = FindWindowEx(hWnd, 0, NULL, tblname16);
						HWND hWndTo = FindWindowEx(hWnd, 0, NULL, refname16);
						if (hWndFrom && hWndTo) {
							links[linkNo] = {hWndFrom, -1, hWndTo, -1, isView ? LINK_VIEW : LINK_TRIGGER};
							linkNo++;
						}

						delete [] tblname16;
						delete [] refname16;
					}
				} else {
					SendMessage(hToolWnd, TB_SETSTATE, IDM_LINK_VIEW, 0);
					SendMessage(hToolWnd, TB_SETSTATE, IDM_LINK_TRIGGER, 0);
				}
				sqlite3_finalize(stmt);

				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETDEFFONT);

				SetWindowPos(hWnd, 0, prefs::get("x") + 30, prefs::get("y") + 70, prefs::get("width") - 60, prefs::get("height") - 100,  SWP_NOZORDER);
				ShowWindow (hWnd, prefs::get("maximized") == 1 ? SW_MAXIMIZE : SW_SHOW);
			}
			break;

			case WM_LBUTTONDOWN: {
				SetWindowLong(hWnd, GWL_USERDATA, 0);
				InvalidateRect(hWnd, NULL, true);
			}
			break;

			case WM_SIZE: {
				SendMessage(GetDlgItem(hWnd, IDC_DLG_TOOLBAR), WM_SIZE, 0, 0);
			}
			break;

			case WM_ERASEBKGND: {
				RECT rc{0};
				GetClientRect(hWnd, &rc);
				FillRect((HDC)wParam, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

				return 1;
			}
			break;

			case WM_PAINT : {
				InvalidateRect(hWnd, NULL, true);

				auto max = [](int a, int b) {
					return a > b ? a : b;
				};
				auto min = [](int a, int b) {
					return a < b ? a : b;
				};

				bool isFk = prefs::get("link-fk");
				bool isView = prefs::get("link-view");
				bool isTrigger = prefs::get("link-trigger");
				HWND hCurrWnd = (HWND)GetWindowLong(hWnd, GWL_USERDATA);

				PAINTSTRUCT ps{0};
				ps.fErase = true;
				HDC hdc = BeginPaint(hWnd, &ps);

				int captionH = GetSystemMetrics(SM_CYCAPTION) +  ListBox_GetItemHeight(links[0].hWndFrom, 0) - 2;
				for (int i = 0; links[i].type != 0; i++) {
					int type = links[i].type;
					if ((type == LINK_FK && !isFk) || (type == LINK_VIEW && !isView) || (type == LINK_TRIGGER && !isTrigger))
						continue;

					RECT rcA{0}, rcB{0};
					GetWindowRect(links[i].hWndFrom, &rcA);
					GetWindowRect(links[i].hWndTo, &rcB);

					POINT from{rcA.left, rcA.top}, to{rcB.left, rcB.top};
					ScreenToClient(hWnd, &from);
					ScreenToClient(hWnd, &to);
					int hA = rcA.bottom - rcA.top;
					int wA = rcA.right - rcA.left;
					int hB = rcB.bottom - rcB.top;
					int wB = rcB.right - rcB.left;

					int minStick = 10;
					POINT a = {0}, b = {0}, c = {0}, d = {0};

					HPEN hPen = CreatePen(PS_SOLID,
						(hCurrWnd == links[i].hWndFrom || hCurrWnd == links[i].hWndTo) ? 2 : 1,
						type == LINK_FK ? RGB(0, 0, 0) : type == LINK_VIEW ? RGB(0, 0, 255) : RGB(255, 0, 0));
					SelectObject(hdc, hPen);

					if (type == LINK_FK) {
						bool isRightA = false;
						bool isRightB = false;

						if (rcB.left - rcA.right > 2 * minStick) {
							isRightA = true;
						} else if (rcA.left - rcB.right > 2 * minStick) {
							isRightB = true;
						} else if (abs(rcA.left - rcB.left) < abs(rcA.right - rcB.right)) {
							isRightA = true;
							isRightB = true;
						}

						RECT rcFrom{0}, rcTo{0};
						ListBox_GetItemRect(links[i].hWndFrom, links[i].posFrom, &rcFrom);
						ListBox_GetItemRect(links[i].hWndTo, links[i].posTo, &rcTo);

						a = {from.x + (isRightA ? wA : 0), from.y + rcFrom.top  + captionH};
						b = {to.x + (isRightB ? wB : 0), to.y + rcTo.top  + captionH};

						int midX = isRightA && isRightB ? max(from.x + wA, to.x + wB) + minStick :
							!isRightA && !isRightB ? min(from.x, to.x) - minStick :
							isRightA && !isRightB ? from.x + wA + (to.x - from.x - wA) / 2 :
							!isRightA && isRightB ? to.x + wB + (from.x - to.x - wB) / 2 :
							0;
						c = {midX, a.y};
						d = {midX, b.y};

						MoveToEx(hdc, a.x + (isRightA ? 5 : -5), a.y, NULL);
						LineTo(hdc, a.x, a.y + 5);
						MoveToEx(hdc, a.x + (isRightA ? 5 : -5), a.y, NULL);
						LineTo(hdc, a.x, a.y - 5);

						MoveToEx(hdc, b.x, b.y - 5, NULL);
						LineTo(hdc, b.x, b.y + 5);
					}

					if ((type == LINK_VIEW) || (type == LINK_TRIGGER)) {
						bool isBottomA = false;
						bool isBottomB = false;
						int shift = LINK_VIEW ? 5 : 10;

						if (rcB.top - rcA.bottom > 2 * minStick) {
							isBottomA = true;
						} else if (rcA.top - rcB.bottom > 2 * minStick) {
							isBottomB = true;
						} else if (abs(rcA.top - rcB.top) < abs(rcA.bottom - rcB.bottom)) {
							isBottomA = true;
							isBottomB = true;
						}

						a = {from.x + wA/2 - shift, from.y + (isBottomA ? hA : 0)};
						b = {to.x + wB/2 + shift, to.y + (isBottomB ? hB : 0)};

						int midY = isBottomA && isBottomB ? max(from.y + hA, to.y + hB) + minStick :
							!isBottomA && !isBottomB ? min(from.y, to.y) - minStick :
							isBottomA && !isBottomB ? from.y + hA + (to.y - from.y - hA) / 2 :
							!isBottomA && isBottomB ? to.y + hB + (from.y - to.y - hB) / 2 :
							0;
						c = {a.x, midY};
						d = {b.x, midY};

						MoveToEx(hdc, a.x - 5, a.y, NULL);
						LineTo(hdc, a.x + 5, a.y);

						MoveToEx(hdc, b.x, b.y, NULL);
						LineTo(hdc, b.x + 5, b.y + (isBottomB ? 5 : -5));
						MoveToEx(hdc, b.x, b.y, NULL);
						LineTo(hdc, b.x - 5, b.y + (isBottomB ? 5 : -5));
					}

					MoveToEx(hdc, a.x, a.y, NULL);
					LineTo(hdc, c.x, c.y);
					LineTo(hdc, d.x, d.y);
					LineTo(hdc, b.x, b.y);

					DeleteObject(hPen);
				}

				EndPaint(hWnd, &ps);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDM_LINK_FK || wParam == IDM_LINK_VIEW || wParam == IDM_LINK_TRIGGER) {
					HWND hToolWnd = GetDlgItem(hWnd, IDC_DLG_TOOLBAR);
					int isChecked = SendMessage(hToolWnd, TB_ISBUTTONCHECKED, wParam, 0);

					if (wParam == IDM_LINK_FK)
						prefs::set("link-fk", isChecked);
					if (wParam == IDM_LINK_VIEW)
						prefs::set("link-view", isChecked);
					if (wParam == IDM_LINK_TRIGGER)
						prefs::set("link-trigger", isChecked);

					InvalidateRect(hWnd, NULL, true);
				}

				if (wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_CLOSE: {
				delete [] dbname8;
				for (int i = 0; i < MAX_LINK_COUNT; i++)
					links[i].type = 0;
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;
		}

		return false;
	}
}
