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

				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select rowid, type, name from sqlite_master where sql is not null order by case when type = 'table' then 0 when type = 'view' then 1 when type = 'trigger' then 2 else 4 end, name", -1, &stmt, 0)) {
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_OBJECTLIST);
					setListViewData(hListWnd, stmt);
					ListView_SetColumnWidth(hListWnd, 0, 0);
					ListView_SetColumnWidth(hListWnd, 2, LVSCW_AUTOSIZE_USEHEADER);
				} else {
					sqlite3_finalize(stmt);
				}
			}
			break;
			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
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
				TCHAR tmpname16[257] = {0};
				tmpname16[0] = TEXT('$');
				_tcscat(tmpname16, name16);
				SetDlgItemText(hWnd, IDC_DLG_TABLENAME, tmpname16);
				SetWindowLong(hWnd, GWL_USERDATA, lParam);

				SendMessage(hWnd, WM_SOURCE_UPDATED, 0, 0);
			}
			break;

			case WM_SOURCE_UPDATED: {
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
					SendMessage(hWnd, WM_SOURCE_UPDATED, 0, 0);

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
						sqlite3_free(err8);
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
