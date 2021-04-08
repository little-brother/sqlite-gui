#include "global.h"
#include "missing.h"
#include "resource.h"
#include "tools.h"
#include "utils.h"
#include "dialogs.h"
#include "prefs.h"

namespace tools {
	const TCHAR* DELIMITERS[4] = {TEXT(","), TEXT(";"), TEXT("\t"), TEXT("|")};

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

				char* table8 = prefs::get("csv-export-last-table", "");
				TCHAR* table16 = utils::utf8to16(table8);
				int idx = ComboBox_FindString(hTable, 0, table16);
				ComboBox_SetCurSel(hTable, idx == -1 ? 0 : idx);
				delete [] table8;
				delete [] table16;

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

					// Use binary mode
					// https://stackoverflow.com/questions/32143707/how-do-i-stop-fprintf-from-printing-rs-to-file-along-with-n-in-windows
					FILE* f = _tfopen(path16, TEXT("wb"));
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
							int colCount = sqlite3_column_count(stmt);
							int size = 0;
							for(int i = 0; i < colCount; i++)
								size += sqlite3_column_type(stmt, i) == SQLITE_TEXT ? sqlite3_column_bytes(stmt, i) + 1 : 20;

							// https://en.wikipedia.org/wiki/Comma-separated_values
							size += colCount + 64; // add place for quotes
							TCHAR line16[size] = {0};
							for(int i = 0; i < colCount; i++) {
								if (i != 0)
									_tcscat(line16, delimiter16);

								TCHAR* value16 = utils::utf8to16(
									isColumns ? (char *)sqlite3_column_name(stmt, i) :
									sqlite3_column_type(stmt, i) != SQLITE_BLOB ? (char *)sqlite3_column_text(stmt, i) : "(BLOB)");
								TCHAR* qvalue16 = utils::replaceAll(value16, TEXT("\""), TEXT("\"\""));
								if (_tcschr(qvalue16, TEXT(',')) || _tcschr(qvalue16, TEXT('"')) || _tcschr(qvalue16, TEXT('\n'))) {
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
							char* line8 = utils::utf16to8(line16);
							fprintf(f, line8);
							delete [] line8;
							isColumns = false;
						}
					}
					sqlite3_finalize(stmt);
					fclose(f);
					delete [] sql8;

					prefs::set("csv-export-delimiter", iDelimiter);
					prefs::set("csv-export-is-unix-line", +isUnixNewLine);

					char* table8 = utils::utf16to8(table16);
					prefs::set("csv-export-last-table", table8);
					delete [] table8;

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
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select type, name, rowid from sqlite_master where sql is not null order by case when type = 'table' then 0 when type = 'view' then 1 when type = 'trigger' then 2 else 4 end, name", -1, &stmt, 0)) {
					ListView_SetData(hListWnd, stmt);
					ListView_SetColumnWidth(hListWnd, 0, 0);
					ListView_SetColumnWidth(hListWnd, 3, 0);
					ListView_SetColumnWidth(hListWnd, 2, LVSCW_AUTOSIZE_USEHEADER);
				}
				sqlite3_finalize(stmt);

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
								ListView_GetItemText(hListWnd, pos, 3, buf16, 128);
								sqlite3_bind_int64(stmt, i + 1, _tcstol(buf16, NULL, 10));
							}

							while (SQLITE_ROW == sqlite3_step(stmt)) {
								TCHAR* line16 = utils::utf8to16((char *)sqlite3_column_text(stmt, 0));
								_ftprintf(f, TEXT("%s;\n\n"), line16);
								delete [] line16;
							}
						} else {
							showDbError(hWnd);
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
							if (_tcscmp(type16, TEXT("table")))
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
											_tcscat(line16, TEXT("'BLOB (unsupported)'"));
										else if (type == SQLITE_NULL)
											_tcscat(line16, TEXT("null"));
										else if (type == SQLITE_TEXT) {
											_tcscat(line16, TEXT("'"));
											TCHAR* dvalue16 = utils::replaceAll(value16, TEXT("'"), TEXT("''"));
											TCHAR* ddvalue16 = utils::replaceAll(dvalue16, TEXT("%"), TEXT("%%"));
											_tcscat(line16, ddvalue16);
											_tcscat(line16, TEXT("'"));
											delete [] dvalue16;
											delete [] ddvalue16;
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

	TCHAR* csvReadLine(FILE* f) {
		size_t size = 32000, bsize = 2000;
		TCHAR* line = new TCHAR[size + 1] {0};
		TCHAR buf[bsize + 1]{0};
		int qCount = 0;

		while (!feof(f)) {
			if (_fgetts(buf, bsize + 1, f)) {
				if (_tcslen(line) + bsize > size) {
					size *= 2;
					TCHAR* oLine = line;
					line = new TCHAR[size + 1]{0};
					_tcscpy(line, oLine);
					delete [] oLine;
				}
				_tcscat(line, buf);

				for (size_t i = 0; i < _tcslen(buf); i++)
					qCount += buf[i] == TEXT('"');

				if ((_tcslen(buf) < bsize) && (qCount % 2 == 0))
					break;
			} else {
				break;
			}
		}

		return line;
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

				FILE* f = _tfopen(path16, isUTF8 ? TEXT("r, ccs=UTF-8") : TEXT("r"));
				if (f == NULL) {
					MessageBox(hWnd, TEXT("Error to open file"), NULL, MB_OK);
					return true;
				}

				ListView_Reset(hPreviewWnd);

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

				int lineNo = 0;
				while(!feof (f) && lineNo < 5) {
					TCHAR* line = csvReadLine(f);
					int colNo = 0;

					TCHAR value[_tcslen(line)];
					bool inQuotes = false;
					int valuePos = 0;
					int i = 0;
					do {
						value[valuePos++] = line[i];

						if ((!inQuotes && (line[i] == delimiter[0] || line[i] == TEXT('\n'))) || !line[i + 1]) {
							value[valuePos - (line[i + 1] != 0 || inQuotes)] = 0;
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

					lineNo++;
					delete [] line;
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

					TCHAR* schema16 = utils::getName(buf16, true);
					TCHAR* tablename16 = utils::getName(buf16);

					_stprintf(create16, TEXT("create table \"%s\".\"%s\" ("), schema16, tablename16);
					_stprintf(insert16, TEXT("insert into \"%s\".\"%s\" ("), schema16, tablename16);

					delete [] tablename16;
					delete [] schema16;

					auto catQuotted = [](TCHAR* a, TCHAR* b) {
						_tcscat(a, TEXT("\""));
						TCHAR* tb = utils::trim(b);
						TCHAR* qb = utils::replaceAll(tb, TEXT("\""), TEXT("\\\""));
						_tcscat(a, qb);
						_tcscat(a, TEXT("\""));
						delete [] qb;
						delete [] tb;
					};

					for (int i = 0; i < colCount; i++) {
						if (i != 0) {
							_tcscat(create16, TEXT(", "));
							_tcscat(insert16, TEXT(", "));
						}

						Header_GetItemText(hHeader, i, buf16, 255);
						catQuotted(create16, buf16);
						catQuotted(insert16, buf16);
					}
					_tcscat(create16, TEXT(");"));
					_tcscat(insert16, TEXT(") values ("));
					for (int i = 0; i < colCount; i++)
						_tcscat(insert16, i != colCount - 1 ? TEXT("?, ") : TEXT("?);"));

					TCHAR* path16 = (TCHAR*)GetWindowLong(hWnd, GWL_USERDATA);
					FILE* f = _tfopen(path16, isUTF8 ? TEXT("r, ccs=UTF-8") : TEXT("r"));
					if (f == NULL) {
						MessageBox(hWnd, TEXT("Error to open file"), NULL, MB_OK);
						return true;
					}

					bool isAutoTransaction = sqlite3_get_autocommit(db) > 0;
					if (isAutoTransaction)
						sqlite3_exec(db, "begin", NULL, 0, NULL);

					char* create8 = utils::utf16to8(create16);
					char* insert8 = utils::utf16to8(insert16);
					sqlite3_exec(db, create8, NULL, 0, NULL);

					int lineNo = 0;
					sqlite3_stmt *stmt;
					bool rc = SQLITE_OK == sqlite3_prepare_v2(db, insert8, -1, &stmt, 0);
					if (rc) {
						while(!feof (f)) {
							TCHAR* line16 = csvReadLine(f);
							if (lineNo == 0 && isColumns) {
								lineNo++;
								continue;
							}

							if (_tcslen(line16) == 0)
								continue;

							int colNo = 0;

							TCHAR value[_tcslen(line16) + 1];
							bool inQuotes = false;
							int valuePos = 0;
							int i = 0;
							do {
								value[valuePos++] = line16[i];

								if ((!inQuotes && (line16[i] == delimiter[0] || line16[i] == TEXT('\n'))) || !line16[i + 1]) {
									value[valuePos - (line16[i + 1] != 0 || inQuotes)] = 0;
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
							delete [] line16;
						}
					}

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
						EndDialog(hWnd, lineNo - isColumns);
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, -1);
			}
			break;
		}

		return false;
	}

	// USERDATA: 0 - import 1 - export
	BOOL CALLBACK cbDlgExportImportODBC (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				bool isExport = lParam;
				SetWindowLong(hWnd, GWL_USERDATA, isExport);
				SetWindowText(hWnd, isExport ? TEXT("Export data via ODBC") : TEXT("Import data via ODBC"));
				SetDlgItemText(hWnd, IDC_DLG_ODBC_SCHEMA_LABEL, isExport ? TEXT("Source schema") : TEXT("Import to schema"));
				SetDlgItemText(hWnd, IDC_DLG_OK, isExport ? TEXT("Export table(s)") : TEXT("Import table(s)"));

				HWND hStrategyWnd = GetDlgItem(hWnd, IDC_DLG_ODBC_STRATEGY);
				ComboBox_AddString(hStrategyWnd, TEXT("Do nothing"));
				ComboBox_AddString(hStrategyWnd, TEXT("Skip"));
				ComboBox_AddString(hStrategyWnd, TEXT("Clear existing data"));
				ComboBox_AddString(hStrategyWnd, TEXT("Drop and create new table"));
				ComboBox_SetCurSel(hStrategyWnd, prefs::get("odbc-strategy"));

				HWND hSchemaWnd = GetDlgItem(hWnd, IDC_DLG_ODBC_SCHEMA);
				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select name from pragma_database_list where name <> 'temp' order by iif(name = 'main', 0, name)", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* schema16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
						ComboBox_AddString(hSchemaWnd, schema16);
						delete [] schema16;
					}
				}
				sqlite3_finalize(stmt);
				ComboBox_SetCurSel(hSchemaWnd, 0);

				if (isExport)
					SendMessage(hWnd, WMU_TARGET_CHANGED, 0, 0);
			}
			break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;

			case WM_COMMAND: {
				bool isExport = GetWindowLong(hWnd, GWL_USERDATA);

				if (LOWORD(wParam) == IDC_DLG_CONNECTION_STRING && HIWORD(wParam) == CBN_SELCHANGE)
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_DLG_CONNECTION_STRING, CBN_EDITCHANGE), (LPARAM)GetDlgItem(hWnd, IDC_DLG_CONNECTION_STRING));

				if (LOWORD(wParam) == IDC_DLG_CONNECTION_STRING && HIWORD(wParam) == CBN_DROPDOWN) {
					HWND hCSWnd = GetDlgItem(hWnd, IDC_DLG_CONNECTION_STRING);
					int size = ComboBox_GetTextLength(hCSWnd) + 1;
					TCHAR cs[size];
					ComboBox_GetText(hCSWnd, cs, size);
					ComboBox_ResetContent(hCSWnd);
					ComboBox_SetText(hCSWnd, cs);

					sqlite3_stmt *stmt;
					BOOL rc = SQLITE_OK == sqlite3_prepare_v2(db, "select 'DSN=' || value from json_each(json_extract(odbc_dsn(), '$.result'))", -1, &stmt, 0);
					while (rc && SQLITE_ROW == sqlite3_step(stmt)) {
						TCHAR* connectionString16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
						ComboBox_AddString(hCSWnd, connectionString16);
						delete [] connectionString16;
					}
					sqlite3_finalize(stmt);
				}

				if (LOWORD(wParam) == IDC_DLG_ODBC_SCHEMA && HIWORD(wParam) == CBN_SELCHANGE && isExport)
					SendMessage(hWnd, WMU_TARGET_CHANGED, 0, 0);

				if (LOWORD(wParam) == IDC_DLG_ODBC_MANAGER) {
					TCHAR winPath[MAX_PATH], appPath[MAX_PATH];
					GetWindowsDirectory(winPath, MAX_PATH);
					_stprintf(appPath, TEXT("%s/SysWOW64/odbcad32.exe"), winPath);
					ShellExecute(0, 0, appPath, 0, 0, SW_SHOW);
					SetFocus(0);
					return 0;
				}

				if (LOWORD(wParam) == IDC_DLG_CONNECTION_STRING && (HIWORD(wParam) == CBN_EDITCHANGE) && !isExport) {
					TCHAR connectionString16[1024]{0};
					GetDlgItemText(hWnd, IDC_DLG_CONNECTION_STRING, connectionString16, 1024);

					if (!_tcslen(connectionString16))
						return 0;

					if (SQLITE_OK != sqlite3_exec(db, "drop table if exists temp.odbc_tables", NULL, NULL, NULL))
						return showDbError(hWnd);

					bool rc = false;
					sqlite3_stmt *stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(db, "select odbc_read(?1, 'TABLES', 'temp.odbc_tables')", -1, &stmt, 0)) {
						char* connectionString8 = utils::utf16to8(connectionString16);
						sqlite3_bind_text(stmt, 1, connectionString8, strlen(connectionString8), SQLITE_TRANSIENT);
						delete [] connectionString8;

						rc = SQLITE_ROW == sqlite3_step(stmt);
					}
					sqlite3_finalize(stmt);

					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_TABLES);
					ListView_Reset(hListWnd);
					if (rc) {
						if (SQLITE_OK == sqlite3_prepare_v2(db, "select table_name from temp.odbc_tables where table_type in ('TABLE', 'VIEW', 'SYSTEM TABLE') order by 1", -1, &stmt, 0)) {
							ListView_SetData(hListWnd, stmt);
							ListView_SetColumnWidth(hListWnd, 1, 290);
						}
						sqlite3_finalize(stmt);
					}
				}

				if (wParam == IDC_DLG_HELP) {
					TCHAR buf[MAX_TEXT_LENGTH];
					LoadString(GetModuleHandle(NULL), IDS_ODBC_HELP, buf, MAX_TEXT_LENGTH);
					MessageBox(hWnd, buf, TEXT("ODBC Help"), MB_OK);
				}

				if (wParam == IDC_DLG_OK) {
					HWND hListWnd = GetDlgItem(hWnd, IDC_DLG_TABLES);
					if (ListView_GetSelectedCount(hListWnd) == 0)
						return MessageBox(0, TEXT("You should specify at least one table"), NULL, 0);

					TCHAR connectionString16[1024];
					GetDlgItemText(hWnd, IDC_DLG_CONNECTION_STRING, connectionString16, 1024);
					if (_tcslen(connectionString16) == 0)
						return MessageBox(0, TEXT("You should provide connection string"), NULL, 0);

					char* connectionString8 = utils::utf16to8(connectionString16);

					TCHAR result16[MAX_TEXT_LENGTH]{0};

					// 0 - do nothing, 1 - skip, 2 - clear, 3 - drop
					int strategy = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_ODBC_STRATEGY));

					int rowNo = -1;
					int rc = true;
					while((rowNo = ListView_GetNextItem(hListWnd, rowNo, LVNI_SELECTED)) != -1) {
						TCHAR table16[1024];
						ListView_GetItemText(hListWnd, rowNo, 1, table16, 1023);
						char* table8 = utils::utf16to8(table16);

						TCHAR schema16[255];
						GetDlgItemText(hWnd, IDC_DLG_ODBC_SCHEMA, schema16, 255);
						char* schema8 = utils::utf16to8(schema16);

						TCHAR res16[_tcslen(table16) + 1024]{0};

						if (strategy && !isExport) {
							bool isExists = false;
							sqlite3_stmt* stmt;
							if (SQLITE_OK == sqlite3_prepare_v2(db, "select 1 from sqlite_master where tbl_name = ?1 ", -1, &stmt, 0)) {
								sqlite3_bind_text(stmt, 1, table8, strlen(table8), SQLITE_TRANSIENT);
								isExists = SQLITE_ROW == sqlite3_step(stmt);
							}
							sqlite3_finalize(stmt);

							if (isExists) {
								if (strategy == 1) {
									_stprintf(res16, TEXT("%s - skipped\n"), table16);
									_tcscat(result16, res16);
									continue;
								}

								char query8[strlen(table8) + 255];
								sprintf(query8, strategy == 3 ? "drop table if exists \"%s\".\"%s\"" : "delete from \"%s\".\"%s\"", schema8, table8);
								sqlite3_exec(db, query8, NULL, NULL, NULL);
							}
						}

						bool isError = false;
						if (strategy && isExport) {
							bool isExists = false;
							sqlite3_stmt* stmt;
							if (SQLITE_OK == sqlite3_prepare_v2(db,
									"with t(res) as (select odbc_query(?1, 'select * from \"'|| ?2 || '\" where 1=2')) " \
									"select 1 from t where coalesce(json_extract(res, '$.error'),'') = ''", -1, &stmt, 0)) {
								sqlite3_bind_text(stmt, 1, connectionString8, strlen(connectionString8), SQLITE_TRANSIENT);
								sqlite3_bind_text(stmt, 2, table8, strlen(table8), SQLITE_TRANSIENT);

								isExists = SQLITE_ROW == sqlite3_step(stmt);
							}
							sqlite3_finalize(stmt);

							if (isExists) {
								if (strategy == 1) {
									_stprintf(res16, TEXT("%s - skipped\n"), table16);
									_tcscat(result16, res16);
									continue;
								}

								sqlite3_stmt* stmt;
								if (SQLITE_OK == sqlite3_prepare_v2(db,
									"select json_extract(odbc_query(?1, printf('%s \"%s\"', ?3, ?2)), '$.error')", -1, &stmt, 0)) {
									sqlite3_bind_text(stmt, 1, connectionString8, strlen(connectionString8), SQLITE_TRANSIENT);
									sqlite3_bind_text(stmt, 2, table8, strlen(table8), SQLITE_TRANSIENT);
									sqlite3_bind_text(stmt, 3, strategy == 3 ?  "drop table " : "delete from ", 12, SQLITE_TRANSIENT);

									if (SQLITE_ROW == sqlite3_step(stmt)) {
										const char* err8 = (const char*)sqlite3_column_text(stmt, 0);
										if (err8 && strlen(err8)) {
											TCHAR res16[1024];
											TCHAR* err16 = utils::utf8to16(err8);
											_stprintf(res16, TEXT("Couldn't %s table %s. Perhaps the driver doesn't support this operation.\n%s"), strategy == 3 ? TEXT("drop") : TEXT("clear"), table16, err16);
											MessageBox(hWnd, res16, TEXT("Error"), MB_OK);
											delete [] err16;
											isError = true;
										}
									} else {
										showDbError(hWnd);
									}
								} else {
									showDbError(hWnd);
								}
								sqlite3_finalize(stmt);
							}
						}

						if (!isError) {
							sqlite3_stmt *stmt;
							rc = SQLITE_OK == sqlite3_prepare_v2(db,
								isExport ?
									"with t(res) as (select odbc_write(?2, ?1, ?3)) select coalesce(json_extract(res, '$.error'), printf('read: %i, inserted: %i', json_extract(res, '$.read'), json_extract(res, '$.inserted')) ) from t" :
									"with t(res) as (select odbc_read(?1, ?2, ?3)) select coalesce(json_extract(res, '$.error'), printf('read: %i, inserted: %i', json_extract(res, '$.read'), json_extract(res, '$.inserted')) ) from t",
								-1, &stmt, 0);

							if (rc) {
								sqlite3_bind_text(stmt, 1, connectionString8, strlen(connectionString8), SQLITE_TRANSIENT);

								char query8[strlen(schema8) + strlen(table8) + 64];;
								if (isExport)
									sprintf(query8, "select * from \"%s\".\"%s\"", schema8, table8);
								else
									sprintf(query8, "select * from \"%s\"", table8);
								sqlite3_bind_text(stmt, 2, query8, strlen(query8), SQLITE_TRANSIENT);

								char target8[strlen(schema8) + strlen(table8) + 64];
								if (isExport)
									sprintf(target8, "\"%s\"", table8);
								else
									sprintf(target8, "\"%s\".\"%s\"", schema8, table8);
								sqlite3_bind_text(stmt, 3, target8, strlen(target8), SQLITE_TRANSIENT);

								rc = SQLITE_ROW == sqlite3_step(stmt);
							}

							TCHAR* _res16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
							_stprintf(res16, TEXT("%s - %s\n"), table16, _res16);
							_tcscat(result16, res16);
							delete [] _res16;

							sqlite3_finalize(stmt);
						} else {
							_stprintf(res16, TEXT("%s - error\n"), table16);
							_tcscat(result16, res16);
						}

						delete [] table8;
						delete [] schema8;
					}

					delete [] connectionString8;

					if (rc) {
						prefs::set("odbc-strategy", strategy);
						MessageBox(hWnd, result16, isExport ? TEXT("Export result") : TEXT("Import result"), MB_OK);
					} else {
						showDbError(hWnd);
					}
				}

				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WMU_TARGET_CHANGED: {
				TCHAR schema16[255];
				GetDlgItemText(hWnd, IDC_DLG_ODBC_SCHEMA, schema16, 255);
				HWND hTablesWnd = GetDlgItem(hWnd, IDC_DLG_TABLES);

				sqlite3_stmt* stmt;
				char query8[1024];
				char* schema8 = utils::utf16to8(schema16);
				sprintf(query8, "select name from \"%s\".sqlite_master where type in ('table', 'view') order by type, name", schema8);
				delete [] schema8;

				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
					ListView_SetData(hTablesWnd, stmt);
					ListView_SetColumnWidth(hTablesWnd, 1, 290);
				}

				sqlite3_finalize(stmt);
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgCompareDatabase (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				HWND hDbWnd = GetDlgItem(hWnd, IDC_DLG_DATABASE);
				sqlite3_stmt *stmt;
				BOOL rc = SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select path from recents order by time desc", -1, &stmt, 0);
				while (rc && SQLITE_ROW == sqlite3_step(stmt) && ComboBox_GetCount(hDbWnd) < 40) {
					TCHAR* db16 = utils::utf8to16((char*)sqlite3_column_text(stmt, 0));
					if (utils::isFileExists(db16))
						ComboBox_AddString(hDbWnd, db16);
					delete [] db16;
				}
				sqlite3_finalize(stmt);

				SetFocus(hDbWnd);
				setEditorFont(GetDlgItem(hWnd, IDC_DLG_ORIGINAL_DDL));
				setEditorFont(GetDlgItem(hWnd, IDC_DLG_COMPARED_DDL));
			}
			break;

			case WM_CLOSE:
				sqlite3_exec(db, "detach database compared", NULL, NULL, NULL);
				EndDialog(hWnd, DLG_CANCEL);
				break;

			case WM_COMMAND: {
				TCHAR path16[MAX_PATH]{0};
				if (wParam == IDC_DLG_DATABASE_SELECTOR && utils::openFile(path16, TEXT("Databases (*.sqlite, *.sqlite3, *.db)\0*.sqlite;*.sqlite3;*.db\0All\0*.*\0")))
					SetDlgItemText(hWnd, IDC_DLG_DATABASE, path16);

				if (wParam == IDC_DLG_COMPARE_SCHEMA || wParam == IDC_DLG_COMPARE_DATA) {
					GetDlgItemText(hWnd, IDC_DLG_DATABASE, path16, MAX_PATH);
					if(!utils::isFileExists(path16))
						return MessageBox(hWnd, TEXT("Specify a compared database"), NULL, 0);

					sqlite3_exec(db, "detach database compared", NULL, NULL, NULL);
					sqlite3_stmt *stmt;
					BOOL rc = SQLITE_OK == sqlite3_prepare_v2(db, "attach database ?1 as compared", -1, &stmt, 0);
					if (rc) {
						char* path8 = utils::utf16to8(path16);
						sqlite3_bind_text(stmt, 1, path8, strlen(path8), SQLITE_TRANSIENT);
						delete [] path8;

						rc = SQLITE_DONE == sqlite3_step(stmt);
					}
					sqlite3_finalize(stmt);

					BOOL isSchema = wParam == IDC_DLG_COMPARE_SCHEMA;

					ShowWindow(GetDlgItem(hWnd, IDC_DLG_SCHEMA_DIFF), isSchema ? SW_SHOW : SW_HIDE);
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_ORIGINAL), isSchema ? SW_SHOW : SW_HIDE);
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_COMPARED), isSchema ? SW_SHOW : SW_HIDE);
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_ORIGINAL_DDL), isSchema ? SW_SHOW : SW_HIDE);
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_COMPARED_DDL), isSchema ? SW_SHOW : SW_HIDE);
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_DATA_DIFF), !isSchema ? SW_SHOW : SW_HIDE);
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_DIFF_ROWS), !isSchema ? SW_SHOW : SW_HIDE);

					SetDlgItemText(hWnd, IDC_DLG_ORIGINAL_DDL, TEXT(""));
					SetDlgItemText(hWnd, IDC_DLG_COMPARED_DDL, TEXT(""));
					ListView_Reset(GetDlgItem(hWnd, IDC_DLG_DIFF_ROWS));

					if (rc && isSchema) {
						HWND hDiffWnd = GetDlgItem(hWnd, IDC_DLG_SCHEMA_DIFF);
						sqlite3_prepare_v2(db,
							"with t as (select type, name, sql from sqlite_master), " \
							"t2 as (select type, name, sql from compared.sqlite_master), " \
							"t3 as (select 1 src, * from t except select 1 src, * from t2), " \
							"t4 as (select 2 src, * from t2 except select 2 src, * from t), " \
							"t5 as (select * from t3 union  select * from t4),"
							"t6 as (select sum(src) idx, type, name from t5 group by type, name)"
							"select case when idx == 1 then '-->' when idx == 2 then '<--' else cast(x'e289a0' as text) end ' ', type, name from t6", -1, &stmt, 0);
						int rowCount = ListView_SetData(hDiffWnd, stmt);
						ListView_SetColumnWidth(hDiffWnd, 3, 495);
						if (!rowCount)
							MessageBox(hWnd, TEXT("Database schemas are the same"), TEXT("Info"), 0);
						sqlite3_finalize(stmt);
					}

					if (rc && !isSchema) {
						HWND hDiffWnd = GetDlgItem(hWnd, IDC_DLG_DATA_DIFF);
						sqlite3_exec(db, "drop table temp.compare_tables", NULL, NULL, NULL);
						sqlite3_exec(db,
							"create table temp.compare_tables as " \
							"select sm.name, null cnt1, null cnt2, null diff from sqlite_master sm inner join compared.sqlite_master sm2 on " \
							"sm.type = sm2.type and sm.name = sm2.name and sm.type = 'table' where not exists( " \
							"select name from pragma_table_info(sm.name) except select name from pragma_table_info(sm.name) where schema = 'compared' " \
							"union all " \
							"select name from pragma_table_info(sm.name) where schema = 'compared' except select name from pragma_table_info(sm.name))",
							NULL, NULL, NULL);
						sqlite3_prepare_v2(db, "select name from temp.compare_tables", -1, &stmt, 0);
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							const char* name = (const char*)sqlite3_column_text(stmt, 0);
							char query[1024 + 3 * strlen(name)];
							sprintf(query, "update temp.compare_tables set cnt1 = (select count(1) from \"%s\"), cnt2 = (select count(1) from compared.\"%s\") where name = \"%s\"", name, name, name);
							sqlite3_exec(db, query, NULL, NULL, NULL);
						}
						sqlite3_finalize(stmt);

						sqlite3_prepare_v2(db, "select name from temp.compare_tables where cnt1 = cnt2", -1, &stmt, 0);
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							const char* name = (const char*)sqlite3_column_text(stmt, 0);
							char query[1024 + 3 * strlen(name)];
							sprintf(query,
								"with t as (select * from \"%s\" except select * from compared.\"%s\" union all select * from compared.\"%s\" except select * from \"%s\") "\
								"update temp.compare_tables set diff = (select count(1) from t) where name = \"%s\"", name, name, name, name, name);
							sqlite3_exec(db, query, NULL, NULL, NULL);
						}
						sqlite3_finalize(stmt);

						sqlite3_prepare_v2(db,
							"select name, iif(cnt1 <> cnt2, 'Rows count: ' || cnt1 || ' vs ' || cnt2, 'Different rows: ' || diff || ' of ' || cnt1) 'Reason' " \
							"from temp.compare_tables where cnt1 <> cnt2 or coalesce(diff, 0) > 0", -1, &stmt, 0);
						int rowCount = ListView_SetData(hDiffWnd, stmt);
						if (!rowCount)
							MessageBox(hWnd, TEXT("No differences found"), TEXT("Info"), 0);
						sqlite3_finalize(stmt);
					}

					if (!rc)
						showDbError(hWnd);
				}
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (pHdr->idFrom == IDC_DLG_SCHEMA_DIFF && pHdr->code == (DWORD)LVN_ITEMCHANGED) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					TCHAR type16[32], name16[1024];
					ListView_GetItemText(pHdr->hwndFrom, ia->iItem, 2, type16, 32);
					ListView_GetItemText(pHdr->hwndFrom, ia->iItem, 3, name16, 32);

					sqlite3_stmt *stmt;
					BOOL rc = SQLITE_OK == sqlite3_prepare_v2(db,
						"select (select max(sql) from sqlite_master where type = ?1 and name = ?2)," \
						"(select max(sql) from compared.sqlite_master where type = ?1 and name = ?2)", -1, &stmt, 0);
					if (rc) {
						char* type8 = utils::utf16to8(type16);
						char* name8 = utils::utf16to8(name16);
						sqlite3_bind_text(stmt, 1, type8, strlen(type8), SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt, 2, name8, strlen(name8), SQLITE_TRANSIENT);
						delete [] type8;
						delete [] name8;

						if (SQLITE_ROW == sqlite3_step(stmt)) {
							TCHAR* sql = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
							TCHAR* sql2 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 1));

							SetDlgItemText(hWnd, IDC_DLG_ORIGINAL_DDL, sql);
							SetDlgItemText(hWnd, IDC_DLG_COMPARED_DDL, sql2);

							delete [] sql;
							delete [] sql2;
						} else {
							SetDlgItemText(hWnd, IDC_DLG_ORIGINAL_DDL, TEXT(""));
							SetDlgItemText(hWnd, IDC_DLG_COMPARED_DDL, TEXT(""));
						}

						rc = SQLITE_DONE == sqlite3_step(stmt);
					}
					sqlite3_finalize(stmt);
				}

				if (pHdr->idFrom == IDC_DLG_DATA_DIFF && pHdr->code == (DWORD)LVN_ITEMCHANGED) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					TCHAR name16[1024];
					ListView_GetItemText(pHdr->hwndFrom, ia->iItem, 1, name16, 32);

					char* name8 = utils::utf16to8(name16);
					char query8[1024 + 4 * strlen(name8)];
					sprintf(query8,
						"with t as (select '->' ' ', * from \"%s\" except select '->' ' ', * from compared.\"%s\"), " \
						"t2 as (select '<-' ' ', * from compared.\"%s\" except select '<-' ' ', * from \"%s\") " \
						"select * from t union all select * from t2",
						name8, name8, name8, name8);
					delete [] name8;

					HWND hRowsWnd = GetDlgItem(hWnd, IDC_DLG_DIFF_ROWS);
					ListView_Reset(hRowsWnd);

					sqlite3_stmt *stmt;
					sqlite3_prepare_v2(db, query8, -1, &stmt, 0);
					ListView_SetData(hRowsWnd, stmt);
					sqlite3_finalize(stmt);
				}
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgDatabaseSearch (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				SetDlgItemText(hWnd, IDC_DLG_TABLENAMES, TEXT("All"));
				HWND hPatternWnd = GetDlgItem(hWnd, IDC_DLG_PATTERN);
				ComboBox_AddString(hPatternWnd, TEXT("Left and right wildcard"));
				ComboBox_AddString(hPatternWnd, TEXT("Exact match"));
				ComboBox_AddString(hPatternWnd, TEXT("Left wildcard"));
				ComboBox_AddString(hPatternWnd, TEXT("Right wildcard"));
				ComboBox_SetCurSel(hPatternWnd, 0);

				HWND hColTypeWnd = GetDlgItem(hWnd, IDC_DLG_COLTYPE);
				ComboBox_AddString(hColTypeWnd, TEXT("Any"));
				ComboBox_AddString(hColTypeWnd, TEXT("Number"));
				ComboBox_AddString(hColTypeWnd, TEXT("Text"));
				ComboBox_AddString(hColTypeWnd, TEXT("BLOB"));
				ComboBox_SetCurSel(hColTypeWnd, 0);

				HWND hNamesWnd = GetDlgItem(hWnd, IDC_DLG_TABLENAMES);
				LONG style = GetWindowLong(hNamesWnd, GWL_EXSTYLE);
				style &= ~WS_EX_NOPARENTNOTIFY;
				SetWindowLong(hNamesWnd, GWL_EXSTYLE, style);

				HWND hTablesWnd = GetDlgItem(hWnd, IDC_DLG_TABLES);
				ListView_SetExtendedListViewStyle(hTablesWnd, LVS_EX_CHECKBOXES);
				SetWindowPos(hTablesWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

				sqlite3_stmt *stmt;
				BOOL rc = SQLITE_OK == sqlite3_prepare_v2(db,
					"select * from (select name from sqlite_master where type = 'table' union all select 'sqlite_master') order by 1 desc",
					-1, &stmt, 0);

				while (rc && SQLITE_ROW == sqlite3_step(stmt)) {
					TCHAR* name16 = utils::utf8to16((const char*)sqlite3_column_text(stmt, 0));
					LVITEM lvi;
					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 0;
					lvi.pszText = name16;
					lvi.cchTextMax = _tcslen(name16) + 1;
					ListView_InsertItem(hTablesWnd, &lvi);
					delete [] name16;
				}

				sqlite3_finalize(stmt);
			}
			break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;

			case WM_LBUTTONDOWN: {
				ShowWindow(GetDlgItem(hWnd, IDC_DLG_TABLES), SW_HIDE);
			}
			break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_SEARCH) {
					HWND hSearchWnd = GetDlgItem(hWnd, IDC_DLG_SEARCH_TEXT);
					int len = GetWindowTextLength(hSearchWnd);
					if (!len)
						return 0;

					TCHAR text16[len + 1]{0};
					GetWindowText(hSearchWnd, text16, len + 1);

					HWND hTablesWnd = GetDlgItem(hWnd, IDC_DLG_TABLES);
					BOOL isAll = TRUE;
					for (int i = 0; isAll && i < ListView_GetItemCount(hTablesWnd); i++)
						isAll = isAll && (ListView_GetCheckState(hTablesWnd, i) == BST_UNCHECKED);

					TCHAR names[MAX_TEXT_LENGTH]{0};
					for (int i = 0; i < ListView_GetItemCount(hTablesWnd); i++) {
						if (isAll || (ListView_GetCheckState(hTablesWnd, i) == BST_CHECKED)) {
							TCHAR name[256];
							ListView_GetItemText(hTablesWnd, i, 0, name, 255);
							_tcscat(names, _tcslen(names) > 0 ? TEXT("\", \"") : TEXT("\""));
							_tcscat(names, name);
						}
					}
					_tcscat(names, TEXT("\""));
					char* names8 = utils::utf16to8(names);
					int matchType = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_PATTERN));
					int colType = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DLG_COLTYPE));

					char query8[MAX_TEXT_LENGTH]{0};
					sprintf(query8, "select 'insert into temp.search_result (name, cnt, whr) select \"' || t.name || '\", count(1), " \
						" ''select * from \"'|| t.name || '\" where ' || group_concat('\"' || c.name || '\" like (\"%s\" || ?1 || \"%s\")', ' or ') || ''' " \
						"from \"' || t.name || '\" where ' || group_concat('\"' || c.name || '\" like (\"%s\" || ?1 || \"%s\")', ' or ')" \
						"from (select name from sqlite_master where sql is not null and type = 'table' union all select 'sqlite_master') t " \
						"left join pragma_table_info c on t.name = c.arg and c.schema = 'main' " \
						"where %s and t.name in (%s) group by t.name order by 1",
						matchType == 0 || matchType == 2 ? "%" : "",
						matchType == 0 || matchType == 3 ? "%" : "",
						matchType == 0 || matchType == 2 ? "%" : "",
						matchType == 0 || matchType == 3 ? "%" : "",
						colType == 1 ? "lower(c.type) in ('integer', 'real')" : colType == 2 ? "lower(c.type) = 'text'" : colType == 3 ? "lower(c.type) = 'blob'" : "1 = 1",
						names8);
					delete [] names8;

					sqlite3_exec(db, "drop table temp.search_result", NULL, 0, NULL);
					sqlite3_exec(db, "create table temp.search_result (name, cnt, whr)", NULL, 0, NULL);
					sqlite3_stmt *stmt;
					char* text8 = utils::utf16to8(text16);
					BOOL rc = SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0);
					while (rc && SQLITE_ROW == sqlite3_step(stmt)) {
						sqlite3_stmt *stmt2;
						if(SQLITE_OK == sqlite3_prepare_v2(db, (const char*)sqlite3_column_text(stmt, 0), -1, &stmt2, 0)) {
							sqlite3_bind_text(stmt2, 1, text8, strlen(text8), SQLITE_TRANSIENT);
							sqlite3_step(stmt2);
						}
						sqlite3_finalize(stmt2);
					}
					sqlite3_finalize(stmt);
					delete [] text8;

					HWND hResultWnd = GetDlgItem(hWnd, IDC_DLG_SEARCH_RESULT);
					if (SQLITE_OK == sqlite3_prepare_v2(db, "select name 'Table', cnt 'Rows found', whr 'Query' from temp.search_result where cnt > 0", -1, &stmt, 0)) {
						ListView_DeleteAllItems(hResultWnd);
						ListView_Reset(hResultWnd);
						ListView_SetData(hResultWnd, stmt);
						ListView_SetColumnWidth(hResultWnd, 2, 300);
						ListView_SetColumnWidth(hResultWnd, 3, 0);
						ListView_Reset(GetDlgItem(hWnd, IDC_DLG_SEARCH_ROWS));
					}
					sqlite3_finalize(stmt);

					SetDlgItemText(hWnd, IDC_DLG_SEARCH_QUERY_TEXT, text16);
				}

				if (LOWORD(wParam) == IDC_DLG_TABLENAMES && (HIWORD(wParam) == (UINT)EN_KILLFOCUS) && (GetFocus() != GetDlgItem(hWnd, IDC_DLG_TABLES)))
					ShowWindow(GetDlgItem(hWnd, IDC_DLG_TABLES), SW_HIDE);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;

				if (pHdr->idFrom == IDC_DLG_TABLES && pHdr->code == LVN_ITEMCHANGED) {
					HWND hTablesWnd = pHdr->hwndFrom;
					TCHAR names[MAX_TEXT_LENGTH]{0};
					for (int i = 0; i < ListView_GetItemCount(hTablesWnd); i++) {
						if (ListView_GetCheckState(hTablesWnd, i) == BST_CHECKED) {
							TCHAR name[256];
							ListView_GetItemText(hTablesWnd, i, 0, name, 255);
							if (_tcslen(names) > 0)
								_tcscat(names, TEXT(", "));
							_tcscat(names, name);
						}
					}

					SetDlgItemText(hWnd, IDC_DLG_TABLENAMES, _tcslen(names) > 0 ? names : TEXT("All"));
				}

				if (pHdr->idFrom == IDC_DLG_TABLES && (pHdr->code == (UINT)NM_KILLFOCUS) && (GetFocus() != GetDlgItem(hWnd, IDC_DLG_TABLENAMES)))
					ShowWindow(pHdr->hwndFrom, SW_HIDE);

				if (pHdr->idFrom == IDC_DLG_SEARCH_RESULT && pHdr->code == (DWORD)LVN_KEYDOWN) {
					NMLVKEYDOWN* kd = (LPNMLVKEYDOWN) lParam;
					if (kd->wVKey == 0x43 && GetKeyState(VK_CONTROL)) { // Ctrl + C
						HWND hResultWnd = pHdr->hwndFrom;
						int pos = ListView_GetNextItem(hResultWnd, -1, LVNI_SELECTED);
						if (pos == -1)
							return 0;

						TCHAR text16[1024];
						GetDlgItemText(hWnd, IDC_DLG_SEARCH_QUERY_TEXT, text16, 1024);

						TCHAR query16[MAX_TEXT_LENGTH]{0};
						ListView_GetItemText(hResultWnd, pos, 3, query16, MAX_TEXT_LENGTH);

						TCHAR* res16 = utils::replaceAll(query16, TEXT("\" || ?1 || \""), text16);
						utils::setClipboardText(res16);
						delete [] res16;
					}
				}

				if (pHdr->idFrom == IDC_DLG_SEARCH_RESULT && pHdr->code == (DWORD)NM_CLICK) {
					NMITEMACTIVATE* ia = (LPNMITEMACTIVATE) lParam;
					TCHAR text16[1024];
					GetDlgItemText(hWnd, IDC_DLG_SEARCH_QUERY_TEXT, text16, 1024);

					TCHAR query16[MAX_TEXT_LENGTH]{0};
					ListView_GetItemText(pHdr->hwndFrom, ia->iItem, 3, query16, MAX_TEXT_LENGTH);

					char* text8 = utils::utf16to8(text16);
					char* query8 = utils::utf16to8(query16);
					sqlite3_stmt *stmt;

					if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
						sqlite3_bind_text(stmt, 1, text8, strlen(text8), SQLITE_TRANSIENT);
						HWND hRowsWnd = GetDlgItem(hWnd, IDC_DLG_SEARCH_ROWS);
						ListView_Reset(hRowsWnd);
						ListView_SetData(hRowsWnd, stmt);
					}
					sqlite3_finalize(stmt);

					delete [] text8;
					delete [] query8;

				}
			}
			break;

			case WM_PARENTNOTIFY: {
				if (LOWORD(wParam) == WM_LBUTTONDOWN) {
					HWND hTablesWnd = GetDlgItem(hWnd, IDC_DLG_TABLES);
					ShowWindow(hTablesWnd, SW_SHOW);
				}
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

	// USERDATA = 1 if a last generation is ok
	// lParam is a target table (optional)
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

				if (lParam)	{
					ComboBox_AddString(hTable, (TCHAR*)lParam);
					EnableWindow(hTable, !lParam);
				} else {
					sqlite3_stmt *stmt, *stmt2;
					if (SQLITE_OK == sqlite3_prepare_v2(db,
						"with t as (select name from pragma_database_list() where name <> 'temp') " \
						"select name from t order by iif(name = 'main', 1, name)", -1, &stmt, 0)) {
						while (SQLITE_ROW == sqlite3_step(stmt)) {
							char* schema8 = (char *)sqlite3_column_text(stmt, 0);
							char query8[strlen(schema8) + 1024];
							sprintf(query8,
								"select iif('%s' = 'main', name, '%s' || '.' || name) from \"%s\".sqlite_master where type = 'table' and name <> 'sqlite_sequence' order by 1",
								schema8, schema8, schema8);
							if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt2, 0)) {
								while (SQLITE_ROW == sqlite3_step(stmt2)) {
									TCHAR* name16 = utils::utf8to16((char *)sqlite3_column_text(stmt2, 0));
									ComboBox_AddString(hTable, name16);
									delete [] name16;
								}
							}
							sqlite3_finalize(stmt2);
						}
					}
					sqlite3_finalize(stmt);
				}
				ComboBox_SetCurSel(hTable, 0);

				if (!GENERATOR_TYPE[0]) {
					int i = 0;
					sqlite3_stmt *stmt;
					while (GENERATOR_TYPE[i] && i < 1024) {
						delete [] GENERATOR_TYPE[i];
						GENERATOR_TYPE[i] = 0;
						i++;
					}

					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select distinct type from generators order by 1", -1, &stmt, 0)) {
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

					// Copy generators table
					sqlite3_exec(db, "drop table temp.generators", 0, 0, 0);
					sqlite3_exec(db, "create table temp.generators (type, value)", 0, 0, 0);
					sqlite3_prepare_v2(prefs::db, "select type, value from generators order by 1", -1, &stmt, 0);

					sqlite3_stmt *istmt;
					sqlite3_prepare_v2(db, "insert into temp.generators (type, value) values (?1, ?2)", -1, &istmt, 0);
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						const char*	key = (const char*)sqlite3_column_text(stmt, 0);
						const char* value = (const char*)sqlite3_column_text(stmt, 1);
						sqlite3_bind_text(istmt, 1, key, strlen(key), SQLITE_TRANSIENT);
						sqlite3_bind_text(istmt, 2, value, strlen(value), SQLITE_TRANSIENT);
						sqlite3_step(istmt);
						sqlite3_reset(istmt);
					}
					sqlite3_finalize(stmt);
					sqlite3_finalize(istmt);
				}

				SetWindowLong(GetDlgItem(hWnd, IDC_DLG_GEN_COLUMNS), GWL_WNDPROC, (LONG)&dialogs::cbNewScroll);
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

				TCHAR name16[1024]{0};
				GetDlgItemText(hWnd, IDC_DLG_TABLENAME, name16, 1024);
				TCHAR* schema16 = utils::getName(name16, true);
				TCHAR* tablename16 = utils::getName(name16);

				TCHAR query16[MAX_TEXT_LENGTH]{0};
				_stprintf(query16, TEXT("select name from pragma_table_info(\"%s\") where schema = \"%s\" order by cid"), tablename16, schema16);
				delete [] tablename16;
				delete [] schema16;

				SendMessage(hColumnsWnd, WM_SETREDRAW, FALSE, 0);
				char* query8 = utils::utf16to8(query16);
				sqlite3_stmt *stmt;
				int rowNo = 0;
				if (SQLITE_OK == sqlite3_prepare_v2(db, query8, -1, &stmt, 0)) {
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
				SendMessage(hColumnsWnd, WM_SETREDRAW, TRUE, 0);

				EnumChildWindows(hColumnsWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETDEFFONT);
				SetProp(hColumnsWnd, TEXT("SCROLLY"), 0);
				SendMessage(hColumnsWnd, WMU_SET_SCROLL_HEIGHT, rowNo * 30, 0);
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
					CreateWindow(WC_EDIT, TEXT("100"), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | WS_TABSTOP, 110, 1, 40, 18, hOptionWnd, (HMENU)IDC_DLG_GEN_OPTION_MULTIPLIER, GetModuleHandle(0), 0);
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
					EndDialog(hWnd, GetWindowLong(hWnd, GWL_USERDATA) ? DLG_OK : DLG_CANCEL);
				}


				if (wParam == IDC_DLG_OK || wParam == IDOK)	{
					execute("drop table if exists temp.data_generator");

					bool isTruncate = Button_GetCheck(GetDlgItem(hWnd, IDC_DLG_GEN_ISTRUNCATE));
					if (isTruncate && MessageBox(hWnd, TEXT("All data from table will be erased. Continue?"), TEXT("Confirmation"), MB_OKCANCEL | MB_ICONASTERISK) != IDOK)
						return true;

					TCHAR name16[1024]{0};
					GetDlgItemText(hWnd, IDC_DLG_TABLENAME, name16, 1024);
					TCHAR* schema16 = utils::getName(name16, true);
					TCHAR* tablename16 = utils::getName(name16);

					char* schema8 = utils::utf16to8(schema16);
					char* tablename8 = utils::utf16to8(tablename16);
					delete [] schema16;
					delete [] tablename16;

					char query8[MAX_TEXT_LENGTH]{0};
					sprintf(query8, "create table temp.data_generator as select null rownum, t.* from \"%s\".\"%s\" t where 1 = 2", schema8, tablename8);
					execute(query8);

					int rowCount = getDlgItemTextAsNumber(hWnd, IDC_DLG_GEN_ROW_COUNT);

					sprintf(query8,
						"with recursive series(val) as (select 1 union all select val + 1 from series limit %i) " \
						"insert into temp.data_generator (rownum) select val from series", rowCount);
					execute(query8);

					HWND hColumnWnd = GetWindow(GetDlgItem(hWnd, IDC_DLG_GEN_COLUMNS), GW_CHILD);
					char columns8[MAX_TEXT_LENGTH]{0};

					bool rc = true;
					while(IsWindow(hColumnWnd) && rc){
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
							_stprintf(query16, TEXT("update temp.data_generator set \"%s\" = rownum + %i - 1"), name16, start);
						}

						if (_tcscmp(type16, TEXT("number")) == 0) {
							int start = getDlgItemTextAsNumber(hOptionWnd, IDC_DLG_GEN_OPTION_START);
							int end = getDlgItemTextAsNumber(hOptionWnd, IDC_DLG_GEN_OPTION_END);
							TCHAR multi[32]{0};
							GetDlgItemText(hOptionWnd, IDC_DLG_GEN_OPTION_MULTIPLIER, multi, 31);
							TCHAR* multi2 = utils::replace(multi, TEXT(","), TEXT("."));

							_stprintf(query16, TEXT("update temp.data_generator set \"%s\" = cast((%i + (%i - %i + 1) * (random()  / 18446744073709551616 + 0.5)) as integer) * %s"), name16, start, end, start, utils::isNumber(multi2, NULL) ? multi2 : TEXT("0"));
							delete [] multi2;
						}

						if (_tcscmp(type16, TEXT("reference to")) == 0) {
							TCHAR reftable16[256]{0};
							GetDlgItemText(hOptionWnd, IDC_DLG_GEN_OPTION_TABLE, reftable16, 255);

							TCHAR refcolumn16[256]{0};
							GetDlgItemText(hOptionWnd, IDC_DLG_GEN_OPTION_COLUMN, refcolumn16, 255);

							_stprintf(query16, TEXT("with t as (select %s value from \"%s\" order by random()), " \
								"series(val) as (select 1 union all select val + 1 from series limit (select ceil(%i.0/count(1)) from t)), " \
								"t2 as (select t.value FROM t, series order by random()), " \
								"t3 as (select rownum(1) rownum, t2.value from t2 order by 1 limit %i)"
								"update temp.data_generator set \"%s\" = t3.value from t3 where t3.rownum = temp.data_generator.rownum"),
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
						rc = execute(query8);
						delete [] query8;

						hColumnWnd = GetWindow(hColumnWnd, GW_HWNDNEXT);
					}

					if (!rc) {
						showDbError(hWnd);
						return 0;
					}

					prefs::set("data-generator-row-count", rowCount);
					prefs::set("data-generator-truncate", +isTruncate);

					if (isTruncate) {
						sprintf(query8, "delete from \"%s\".\"%s\"", schema8, tablename8);
						execute(query8);
					}

					snprintf(query8, MAX_TEXT_LENGTH, "insert into \"%s\".\"%s\" (%s) select %s from temp.data_generator", schema8, tablename8, columns8, columns8);
					rc = execute(query8);
					if (rc)
						MessageBox(hWnd, TEXT("Done!"), TEXT("Info"), MB_OK);
					else
						showDbError(hWnd);
					SetWindowLong(hWnd, GWL_USERDATA, rc);

					delete [] schema8;
					delete [] tablename8;
				}
			}
			break;
		}

		return false;
	}

	BOOL CALLBACK cbDlgStatistics (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				sqlite3_exec(db, "drop table temp.row_statistics;", NULL, 0, NULL);
				sqlite3_exec(db, "create table temp.row_statistics (name text, cnt integer);", NULL, 0, NULL);

				sqlite3_stmt *stmt;
				BOOL rc = SQLITE_OK == sqlite3_prepare_v2(db, "select 'insert into temp.row_statistics (name, cnt) select ''' || name || ''', count(1) from \"' || name || '\"' from sqlite_master where type in ('table')", -1, &stmt, 0);
				while (rc && SQLITE_ROW == sqlite3_step(stmt))
					rc = SQLITE_OK == sqlite3_exec(db, (const char*)sqlite3_column_text(stmt, 0), NULL, 0, NULL);
				sqlite3_finalize(stmt);

				if (!rc)
					return showDbError(hWnd);

				if (SQLITE_OK == sqlite3_prepare_v2(db, "SELECT s.name, SUM(payload) 'Payload size, B', tosize(SUM(pgsize)) 'Total size', r.cnt 'Rows' from dbstat s left join temp.row_statistics r on s.name = r.name  group by s.name;", -1, &stmt, 0))
					ListView_SetData(GetDlgItem(hWnd, IDC_DLG_STATISTICS), stmt);
				sqlite3_finalize(stmt);
			}
			break;

			case WM_CLOSE:
				EndDialog(hWnd, DLG_CANCEL);
				break;

			case WM_COMMAND: {
				if (wParam == IDC_DLG_CANCEL || wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_NOTIFY: {
				NMHDR* pHdr = (LPNMHDR)lParam;
				if (pHdr->code == LVN_COLUMNCLICK) {
					NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
					return ListView_Sort(pHdr->hwndFrom, pLV->iSubItem);
				}
			}
			break;
		}

		return false;
	}

	bool importSqlFile(TCHAR *path16){
		char* path8 = utils::utf16to8(path16);
		char* data8 = utils::readFile(path8);
		bool rc = true;
		if (data8 != 0) {
			char* ldata8 = new char[strlen(data8) + 1];
			strcpy(ldata8, data8);
			strlwr(ldata8);
			bool hasTransaction = strstr(ldata8, "begin;") || strstr(ldata8, "begin transaction") || strstr(ldata8, "commit");
			delete [] ldata8;

			if (!hasTransaction)
				sqlite3_exec(db, "begin transaction;", NULL, 0, NULL);
			if (prefs::get("synchronous-off"))
				sqlite3_exec(db, "pragma synchronous = 0", NULL, 0, NULL);
			bool hasBOM = data8[0] == '\xEF' && data8[1] == '\xBB' && data8[2] == '\xBF';
			rc = SQLITE_OK == sqlite3_exec(db, hasBOM ? data8 + 3 : data8, NULL, 0, NULL);
			if (!rc)
				showDbError(hMainWnd);

			if (prefs::get("synchronous-off"))
				sqlite3_exec(db, "pragma synchronous = 1", NULL, 0, NULL);
			if (!hasTransaction)
				sqlite3_exec(db, rc ? "commit;" : "rollback;", NULL, 0, NULL);

			delete [] data8;
		}
		delete [] path8;

		return rc;
	}

	bool reindexDatabase() {
		sqlite3_stmt *stmt;
		BOOL rc = SQLITE_OK == sqlite3_prepare_v2(db, "select 'reindex \"' || name || '\"' from sqlite_master where type in ('table', 'index')", -1, &stmt, 0);
		while (rc && SQLITE_ROW == sqlite3_step(stmt))
			rc = SQLITE_OK == sqlite3_exec(db, (const char*)sqlite3_column_text(stmt, 0), NULL, 0, NULL);
		sqlite3_finalize(stmt);
		return rc;
	}

	#define LINK_FK 1
	#define LINK_VIEW 2
	#define LINK_TRIGGER 3
	#define MAX_LINK_COUNT 1024
	struct Link {
		HWND hWndFrom;
		int posFrom;
		HWND hWndTo;
		int posTo;
		int type;
	};

	char* dbname8 = 0;
	Link links[MAX_LINK_COUNT]{0};

	bool isMove = false;
	POINT cursor = {0, 0};
	HMENU hDiagramMenu = 0;

	WNDPROC cbOldTable, cbOldDatabaseDiagramToolbar;
	LRESULT CALLBACK cbNewTable(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_COMMAND) {
			TCHAR table16[255]{0};
			GetWindowText(hWnd, table16, 255);

			if (wParam == IDM_EDIT_DATA) {
				_tcscpy(editTableData16, table16);
				DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_EDITDATA), hWnd, (DLGPROC)&dialogs::cbDlgEditData);
			}

			if (wParam == IDM_DDL) {
				TCHAR* DDL = getDDL(table16, TABLE, false);
				if (!DDL) {
					delete [] DDL;
					DDL = getDDL(table16, VIEW, false);
				}
				DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DDL), hWnd, (DLGPROC)&dialogs::cbDlgDDL, (LPARAM)DDL);
				delete [] DDL;
			}
		}

		if (msg == WM_WINDOWPOSCHANGED) {
			RECT rcSize{0};
			GetWindowRect(hWnd, &rcSize);
			RECT rcPos{0};
			GetWindowRect(hWnd, &rcPos);
			POINT pos{rcPos.left, rcPos.top};
			ScreenToClient(GetParent(hWnd), &pos);
			TCHAR table16[255]{0};
			GetWindowText(hWnd, table16, 255);

			prefs::setSyncMode(0);
			char* table8 = utils::utf16to8(table16);
			RECT rect{pos.x, pos.y, rcSize.right - rcSize.left, rcSize.bottom - rcSize.top};
			sqlite3_stmt* stmt;
			if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "replace into diagrams (dbname, tblname, x, y, width, height) values (?1, ?2, ?3, ?4, ?5, ?6)", -1, &stmt, 0)) {
				sqlite3_bind_text(stmt, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
				sqlite3_bind_text(stmt, 2, table8, strlen(table8), SQLITE_TRANSIENT);
				sqlite3_bind_int(stmt, 3, rect.left);
				sqlite3_bind_int(stmt, 4, rect.top);
				sqlite3_bind_int(stmt, 5, rect.right);
				sqlite3_bind_int(stmt, 6, rect.bottom);

				sqlite3_step(stmt);
			}
			sqlite3_finalize(stmt);
			prefs::setSyncMode(1);
			delete [] table8;
		}

		if (msg == WM_LBUTTONDOWN) {
			HWND hParentWnd = GetParent(hWnd);
			SetWindowLong(hParentWnd, GWL_USERDATA, (LONG)hWnd);

			int tblNo = 0;
			while(HWND hTableWnd = GetDlgItem(hParentWnd, IDC_DATABASE_DIAGRAM_TABLE + tblNo)) {
				ShowWindow(hTableWnd, hTableWnd == hWnd ? SW_SHOW : SW_HIDE);
				tblNo++;
			}

			InvalidateRect(hParentWnd, NULL, true);
		}

		if (msg == WM_WINDOWPOSCHANGED || msg == WM_LBUTTONDBLCLK)
			SetFocus(GetParent(hWnd));

		if (msg == WM_CONTEXTMENU) {
			POINT p = {LOWORD(lParam), HIWORD(lParam)};
			if (!hDiagramMenu) {
				hDiagramMenu = LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDC_MENU_DIAGRAM));
				hDiagramMenu = GetSubMenu(hDiagramMenu, 0);
			}

			TrackPopupMenu(hDiagramMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
		}

		return CallWindowProc(cbOldTable, hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK cbNewDatabaseDiagramToolbar(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_GETDLGCODE)
			return (DLGC_WANTALLKEYS | CallWindowProc(cbOldDatabaseDiagramToolbar, hWnd, msg, wParam, lParam));

		if (msg == WM_COMMAND && (LOWORD(wParam) == IDC_DLG_FILTER) && (HIWORD(wParam) == EN_CHANGE))
			SendMessage(GetAncestor(hWnd, GA_ROOT), WMU_UPDATE_DIAGRAM, 0, 0);

		return CallWindowProc(cbOldDatabaseDiagramToolbar, hWnd, msg, wParam, lParam);
	}

	// USERDATA stores a handle of current diagram table
	BOOL CALLBACK cbDlgDatabaseDiagram (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_INITDIALOG: {
				dbname8 = utils::getFileName(sqlite3_db_filename(db, 0));

				TBBUTTON tbButtons [] = {
					{0, IDM_LINK_FK, (byte)(prefs::get("link-fk") ? TBSTATE_CHECKED | TBSTATE_ENABLED : TBSTATE_ENABLED), TBSTYLE_CHECK, {0}, 0L, (INT_PTR)TEXT("Foreign keys")},
					{1, IDM_LINK_VIEW, (byte)(prefs::get("link-view") ? TBSTATE_CHECKED | TBSTATE_ENABLED : TBSTATE_ENABLED), TBSTYLE_CHECK, {0}, 0L, (INT_PTR)TEXT("References in views")},
					{2, IDM_LINK_TRIGGER, (byte)(prefs::get("link-trigger") ? TBSTATE_CHECKED | TBSTATE_ENABLED : TBSTATE_ENABLED), TBSTYLE_CHECK, {0}, 0L, (INT_PTR)TEXT("References in triggers")},
					{-1, IDM_LAST_SEPARATOR, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0L, 0}
				};

				HWND hToolbarWnd = CreateToolbarEx (hWnd, WS_CHILD |  WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS |TBSTYLE_FLAT | TBSTYLE_LIST, IDC_DLG_TOOLBAR, 0, NULL, 0,
					tbButtons, sizeof(tbButtons)/sizeof(tbButtons[0]), 0, 0, 0, 0, sizeof (TBBUTTON));

				// Should be run before TB_SETIMAGELIST to get the correct delimiter position
				RECT rc{0};
				SendMessage(hToolbarWnd, TB_GETRECT, IDM_LAST_SEPARATOR, (LPARAM)&rc);
				cbOldDatabaseDiagramToolbar = (WNDPROC)SetWindowLong(hToolbarWnd, GWL_WNDPROC, (LONG)cbNewDatabaseDiagramToolbar);

				HIMAGELIST tbImages = ImageList_LoadBitmap(GetModuleHandle (0), MAKEINTRESOURCE(IDB_DIAGRAM_TOOLBAR), 32, 0, RGB(255, 255, 255));
				SendMessage(hToolbarWnd, TB_SETIMAGELIST, 0, (LPARAM)tbImages);
				SendMessage(hToolbarWnd, TB_SETBITMAPSIZE, 0, MAKELPARAM(32, 16));

				CreateWindow(WC_STATIC, TEXT("Find"), WS_CHILD | WS_VISIBLE, rc.right + 16 * 3 - 5, 5, 30, 14, hToolbarWnd, 0, GetModuleHandle(0), 0);
				HWND hFilterWnd = CreateWindow(WC_EDIT, NULL, WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, rc.right + 16 * 3 + 25, 3, 180, 19, hToolbarWnd, (HMENU) IDC_DLG_FILTER, GetModuleHandle(0), 0);
				SendMessage(hFilterWnd, WM_SETFONT, (LPARAM)SendMessage(hToolbarWnd, WM_GETFONT, 0, 0), true);

				auto addTable = [hWnd](char* name8, int tblNo) {
					TCHAR* tblname16 = utils::utf8to16((char *)name8);
					RECT rect{10 + (tblNo % 5) * 150, 40 + 150 * (tblNo / 5), 100, 100};

					sqlite3_stmt * stmt;
					if (SQLITE_OK == sqlite3_prepare_v2(prefs::db, "select x, y, width, height from 'diagrams' where dbname = ?1 and tblname = ?2", -1, &stmt, 0)) {
						sqlite3_bind_text(stmt, 1, dbname8, strlen(dbname8), SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt, 2, name8, strlen(name8), SQLITE_TRANSIENT);
						if (sqlite3_step(stmt) == SQLITE_ROW) {
							rect.left = sqlite3_column_int(stmt, 0);
							rect.top = sqlite3_column_int(stmt, 1);
							rect.right = sqlite3_column_int(stmt, 2);
							rect.bottom = sqlite3_column_int(stmt, 3);
						}
					}
					sqlite3_finalize(stmt);

					HWND hTableWnd = CreateWindow(WC_LISTBOX, tblname16,
						WS_CAPTION | WS_VISIBLE | WS_CHILD | WS_OVERLAPPED | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LBS_MULTIPLESEL | LBS_NOTIFY,
						rect.left, rect.top, rect.right, rect.bottom + 15, hWnd, (HMENU)(IDC_DATABASE_DIAGRAM_TABLE + tblNo), GetModuleHandle(0), NULL);

					cbOldTable = (WNDPROC)SetWindowLong(hTableWnd, GWL_WNDPROC, (LONG)cbNewTable);
					ShowWindow(hTableWnd, SW_SHOW);

					delete [] tblname16;
					return hTableWnd;
				};

				auto addColumn = [] (HWND hTableWnd, char* name8, char* type8) {
					TCHAR* colname16 = utils::utf8to16(name8);
					TCHAR* type16 = utils::utf8to16(type8);
					TCHAR buf[1024]{0};
					_stprintf(buf, TEXT("%s: %s"), colname16, type16);
					ListBox_AddString(hTableWnd, buf);

					delete [] type16;
					delete [] colname16;
				};

				HWND hTableWnd = 0;
				int tblNo = 0;
				sqlite3_stmt *stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(db, "select t.name tblname, c.name colname, c.cid, iif(length(c.type), c.type, 'any') || iif(c.pk, ' [PK]', '') " \
					"from sqlite_master t, pragma_table_xinfo(t.tbl_name) c " \
					"where t.sql is not null and t.name not like 'sqlite_%' and t.type in ('view', 'table')" \
					"order by 1, 3", -1, &stmt, 0)) {
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						int colNo = sqlite3_column_int(stmt, 2);
						if (colNo == 0) {
							hTableWnd = addTable((char *)sqlite3_column_text(stmt, 0), tblNo);
							tblNo++;
						}

						addColumn(hTableWnd, (char*)sqlite3_column_text(stmt, 1), (char*)sqlite3_column_text(stmt, 3));
					}
				}
				sqlite3_finalize(stmt);

				if (SQLITE_OK != sqlite3_errcode(db) || tblNo == 0) {
					sqlite3_prepare_v2(db, "select name from sqlite_master t where t.sql is not null and t.name not like 'sqlite_%' and t.type in ('view', 'table') order by 1", -1, &stmt, 0);
					while (SQLITE_ROW == sqlite3_step(stmt)) {
						char* name8 = (char*)sqlite3_column_text(stmt, 0);
						hTableWnd = addTable(name8, tblNo);
						tblNo++;

						sqlite3_stmt *substmt;
						if (SQLITE_OK == sqlite3_prepare_v2(db, "select name, cid, iif(length(c.type), c.type, 'any') || iif(c.pk, ' [PK]', '') from pragma_table_xinfo(?1) c order by cid", -1, &substmt, 0)) {
							sqlite3_bind_text(substmt, 1, name8, strlen(name8), SQLITE_TRANSIENT);
							while (SQLITE_ROW == sqlite3_step(substmt))
								addColumn(hTableWnd, (char*) sqlite3_column_text(substmt, 0), (char *) sqlite3_column_text(substmt, 2));

							if (ListBox_GetCount(hTableWnd) == 0)
								addColumn(hTableWnd, (char*)"Error", (char*)sqlite3_errmsg(db));
						}
						sqlite3_finalize(substmt);
					}
					sqlite3_finalize(stmt);
				}

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
					SendMessage(hToolbarWnd, TB_SETSTATE, IDM_LINK_VIEW, 0);
					SendMessage(hToolbarWnd, TB_SETSTATE, IDM_LINK_TRIGGER, 0);
				}
				sqlite3_finalize(stmt);

				prefs::setSyncMode(0);
				EnumChildWindows(hWnd, (WNDENUMPROC)cbEnumChildren, (LPARAM)ACTION_SETDEFFONT);
				prefs::setSyncMode(1);

				SetWindowPos(hWnd, 0, prefs::get("x") + 30, prefs::get("y") + 70, prefs::get("width") - 60, prefs::get("height") - 100,  SWP_NOZORDER);
				ShowWindow (hWnd, prefs::get("maximized") == 1 ? SW_MAXIMIZE : SW_SHOW);
			}
			break;

			case WM_LBUTTONDOWN: {
				cursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				isMove = wParam == MK_LBUTTON;
				SetCapture(hWnd);
				if (GetWindowLong(hWnd, GWL_USERDATA)) {
					SetWindowLong(hWnd, GWL_USERDATA, 0);
					int tblNo = 0;
					while(HWND hTableWnd = GetDlgItem(hWnd, IDC_DATABASE_DIAGRAM_TABLE + tblNo)) {
						ShowWindow(hTableWnd, SW_SHOW);
						tblNo++;
					}
				}
				InvalidateRect(hWnd, NULL, true);
			}
			break;

			case WM_LBUTTONUP: {
				int dx = cursor.x - GET_X_LPARAM(lParam);
				int dy = cursor.y - GET_Y_LPARAM(lParam);

				if (isMove && (dx != 0 ||dy != 0)) {
					prefs::setSyncMode(0);
					int tblNo = 0;
					while(HWND hTableWnd = GetDlgItem(hWnd, IDC_DATABASE_DIAGRAM_TABLE + tblNo)) {
						RECT rc;
						GetWindowRect(hTableWnd, &rc);
						POINT p = {rc.left, rc.top};
						ScreenToClient(hWnd, &p);
						SetWindowPos(hTableWnd, 0, p.x - dx, p.y - dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
						tblNo++;
					}
					prefs::setSyncMode(1);
				}

				isMove = false;
				ReleaseCapture();
			}
			break;

			case WM_MOUSEMOVE: {
				if (isMove)
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

				PAINTSTRUCT ps{0};
				ps.fErase = true;
				HDC hdc = BeginPaint(hWnd, &ps);

				if (isMove) {
					POINT p;
					GetCursorPos(&p);
					ScreenToClient(hWnd, &p);
					int dx = cursor.x - p.x;
					int dy = cursor.y - p.y;

					if (isMove && (dx != 0 ||dy != 0)) {
						HPEN hPen = CreatePen(PS_DOT, 1, RGB(128, 128, 128));
						SelectObject(hdc, hPen);

						int tblNo = 0;
						while(HWND hTableWnd = GetDlgItem(hWnd, IDC_DATABASE_DIAGRAM_TABLE + tblNo)) {
							RECT rc;
							GetWindowRect(hTableWnd, &rc);
							POINT p = {rc.left, rc.top};
							ScreenToClient(hWnd, &p);
							Rectangle(hdc, p.x - dx, p.y - dy,  p.x - dx + rc.right - rc.left, p.y - dy + rc.bottom - rc.top);
							tblNo++;
						}

						DeleteObject(hPen);
					}
				}

				bool isFk = prefs::get("link-fk");
				bool isView = prefs::get("link-view");
				bool isTrigger = prefs::get("link-trigger");
				HWND hCurrWnd = (HWND)GetWindowLong(hWnd, GWL_USERDATA);

				int tblNo = 0;
				while(HWND hTableWnd = GetDlgItem(hWnd, IDC_DATABASE_DIAGRAM_TABLE + tblNo)) {
					tblNo++;

					if (GetProp(hTableWnd, TEXT("HIGHLIGHTED")) == 0 || !IsWindowVisible(hTableWnd))
						continue;

					RECT rc{0};
					GetWindowRect(hTableWnd, &rc);
					POINT p = {rc.left, rc.top};
					ScreenToClient(hWnd, &p);

					HPEN hPen = CreatePen(PS_DOT, 3, RGB(255, 200, 255));
					SelectObject(hdc, hPen);
					Rectangle(hdc, p.x - 5, p.y - 5,  p.x + rc.right - rc.left + 5, p.y + rc.bottom - rc.top + 5);
					DeleteObject(hPen);
				}

				if (hCurrWnd) {
					RECT rc{0};
					GetWindowRect(hCurrWnd, &rc);
					POINT p = {rc.left, rc.top};
					ScreenToClient(hWnd, &p);

					HPEN hPen = CreatePen(PS_DOT, 1, RGB(128, 128, 128));
					SelectObject(hdc, hPen);
					Rectangle(hdc, p.x - 5, p.y - 5,  p.x + rc.right - rc.left + 5, p.y + rc.bottom - rc.top + 5);
					DeleteObject(hPen);
				}

				int captionH = GetSystemMetrics(SM_CYCAPTION) +  ListBox_GetItemHeight(links[0].hWndFrom, 0) - 2;
				for (int i = 0; links[i].type != 0; i++) {
					int type = links[i].type;
					if ((type == LINK_FK && !isFk) || (type == LINK_VIEW && !isView) || (type == LINK_TRIGGER && !isTrigger))
						continue;

					if (hCurrWnd && (hCurrWnd != links[i].hWndFrom && hCurrWnd != links[i].hWndTo))
						continue;

					ShowWindow(links[i].hWndFrom, SW_SHOW);
					ShowWindow(links[i].hWndTo, SW_SHOW);

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

						int midX = isRightA && isRightB ? MAX(from.x + wA, to.x + wB) + minStick :
							!isRightA && !isRightB ? MIN(from.x, to.x) - minStick :
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

						int midY = isBottomA && isBottomB ? MAX(from.y + hA, to.y + hB) + minStick :
							!isBottomA && !isBottomB ? MIN(from.y, to.y) - minStick :
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

					if (GetWindowLong(hWnd, GWL_USERDATA)) {
						int tblNo = 0;
						while(HWND hTableWnd = GetDlgItem(hWnd, IDC_DATABASE_DIAGRAM_TABLE + tblNo)) {
							ShowWindow(hTableWnd, SW_HIDE);
							tblNo++;
						}
					}

					InvalidateRect(hWnd, NULL, true);
				}

				if (HIWORD(wParam) == LBN_SELCHANGE && GetParent((HWND)lParam) == hWnd)
					return SendMessage(hWnd, WMU_UPDATE_DIAGRAM, 0, 0);


				if (wParam == IDCANCEL)
					EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WM_CLOSE: {
				delete [] dbname8;
				for (int i = 0; i < MAX_LINK_COUNT; i++)
					links[i].type = 0;

				int tblNo = 0;
				while(HWND hTableWnd = GetDlgItem(hWnd, IDC_DATABASE_DIAGRAM_TABLE + tblNo)) {
					RemoveProp(hTableWnd, TEXT("HIGHLIGHTED"));
					tblNo++;
				}
				EndDialog(hWnd, DLG_CANCEL);
			}
			break;

			case WMU_UPDATE_DIAGRAM: {
				HWND hFilterWnd = GetDlgItem(GetDlgItem(hWnd, IDC_DLG_TOOLBAR), IDC_DLG_FILTER);

				TCHAR filter[256]{0};
				GetWindowText(hFilterWnd, filter, 255);
				_tcslwr(filter);

				int tblNo = 0;
				while(HWND hTableWnd = GetDlgItem(hWnd, IDC_DATABASE_DIAGRAM_TABLE + tblNo)) {
					TCHAR title[256]{0};
					GetWindowText(hTableWnd, title, 255);
					_tcslwr(title);
					SetProp(hTableWnd, TEXT("HIGHLIGHTED"), (HANDLE)(_tcslen(filter) > 0 && _tcsstr(title, filter) != 0));

					ListBox_SetSel(hTableWnd, 0, -1);
					int rowCount = ListBox_GetCount(hTableWnd);
					for (int rowNo = 0; rowNo < rowCount; rowNo++) {
						int size = ListBox_GetTextLen(hTableWnd, rowNo) + 1;
						TCHAR buf[size]{0};
						ListBox_GetText(hTableWnd, rowNo, buf);
						_tcslwr(buf);
						_tcstok(buf, TEXT(":"));
						ListBox_SetSel(hTableWnd, _tcslen(filter) > 0 && _tcsstr(buf, filter) != 0, rowNo);
					}
					tblNo++;
				}
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}

		return false;
	}
}
